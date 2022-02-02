/*
* Copyright 2017 Justas Masiulis
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef JM_MEMORY_SIGNATURE_HPP
#define JM_MEMORY_SIGNATURE_HPP

#include <stdexcept> // range_error, invalid_argument
#include <algorithm> // search
#include <iterator>  // begin, end
#include <cstdint>   // uint8_t
#include <memory>    // unique_ptr
#include <bitset>    // bitset


/// \brief main namespace
namespace jm {

    /// \brief Internal implementation namespace
    namespace detail {

        template<class ForwardIt, class Cmp>
        inline std::uint8_t find_wildcard(ForwardIt first, ForwardIt last, Cmp cmp)
        {
            std::bitset<256> bits;

            for (; first != last; ++first)
                bits[static_cast<std::uint8_t>(*first)] = cmp(*first);

            for (auto i = 0; i < 256; ++i)
                if (!bits[i])
                    return static_cast<std::uint8_t>(i);

            throw std::range_error("unable to find unused byte in the provided pattern");
        }

        template<class ForwardIt1, class ForwardIt2>
        inline std::uint8_t find_wildcard_masked(ForwardIt1 first, ForwardIt1 last, ForwardIt2 mask_first, std::uint8_t unk)
        {
            return find_wildcard(first, last, [=](std::uint8_t) mutable {
                return *mask_first++ != unk;
            });
        }

        template<class ForwardIt>
        inline std::uint8_t find_wildcard_hybrid(ForwardIt first, ForwardIt last)
        {
            return find_wildcard(first, last, [](std::uint8_t b) { return !(b != ' ' && b != '?'); });
        }

    } // namespace detail

    /// \brief A light wrapper class around a memory signature providing an easy way to search for it in memory
    class memory_signature {
        std::unique_ptr<std::uint8_t[]> _pattern;
        std::uint8_t*                   _end;
        std::uint8_t                    _wildcard;

        /// \private
        std::ptrdiff_t diff() const noexcept { return _end - _pattern.get(); }

        /// \private
        template<class ForwardIt1, class ForwardIt2>
        void masked_to_wildcard(ForwardIt1 first, ForwardIt1 last, ForwardIt2 m_first, std::uint8_t unknown) noexcept
        {
            auto my_pat = _pattern.get();
            for (; first != last; ++first, ++m_first, ++my_pat) {
                if (*m_first != unknown)
                    *my_pat = *first;
                else
                    *my_pat = _wildcard;
            }
        }

        /// \private
        template<class ForwardIt>
        void hybrid_to_wildcard(ForwardIt first, ForwardIt last)
        {
			char tokens[2] = { 0 };
            int  n_tokens  = 0; // do not move - will act as a delimiter for tokens
            auto my_pat    = _pattern.get();

            for (bool prev_was_wildcard = false; first != last; ++first) {
                if (*first == ' ') {
                    prev_was_wildcard = false;
                    if (!n_tokens)
                        continue;

                    n_tokens = 0; // will act as delimiter for the string
					// TODO replace this strtoul call
					*my_pat++ = static_cast<std::uint8_t>(std::strtoul(tokens, nullptr, 16));
                    tokens[1] = 0; // reset it in case it isn't replaced
                }
                else if (*first == '?') {
                    if (!prev_was_wildcard) {
                        *my_pat++ = _wildcard;
                        prev_was_wildcard = true;
                    }
                }
                else
                    tokens[n_tokens++] = *first;
            }

			if (n_tokens)
				*my_pat++ = static_cast<std::uint8_t>(std::strtoul(tokens, nullptr, 16));

            _end = my_pat;
        }

    public:
        /// \brief Construct a new signature that is empty
        /// \throw Nothrow guarantee
        explicit memory_signature() noexcept
                : _pattern(nullptr)
                , _end(nullptr)
                , _wildcard(0) {}

        /// \brief Destroys the stored signature
        ~memory_signature() noexcept = default;

        /// \brief copy constructor
        /// \throw Strong exception safety guarantee
        memory_signature(const memory_signature &other)
                : _pattern(new std::uint8_t[other.diff()])
                , _end(std::copy(other._pattern.get(), other._end, _pattern.get()))
                , _wildcard(other._wildcard) {}

        /// \brief copy assignment operator
        /// \throw Strong exception safety guarantee
        /// \note If size of current signature is larger than the others
        ///       nothrow guarantee is given.
        memory_signature &operator=(const memory_signature &other)
        {
			// self assignment protection
			if (this != std::addressof(other)) {
				// check if we need to re-allocate the storage
				const auto new_size = other.diff();
				if (diff() < new_size)
					_pattern.reset(new std::uint8_t[new_size]);

				_end      = std::copy(other._pattern.get(), other._end, _pattern.get());
				_wildcard = other._wildcard;
			}

            return *this;
        }

        /// \brief Move constructor
        /// \throw Nothrow guarantee
        memory_signature(memory_signature&& other) noexcept
		// no need to move assign - default deleter will be used
                : _pattern(other._pattern.release())
                , _end(other._end)
                , _wildcard(other._wildcard) {}

        /// \brief Move assignment operator
        /// \throw Nothrow guarantee
        memory_signature &operator=(memory_signature&& other) noexcept
        {
			_pattern.reset(other._pattern.release());
            _end      = other._end;
            _wildcard = other._wildcard;
            return *this;
        }

        /// \brief Construct a new signature using a pattern and a wildcard.
        /// \param pattern The pattern in the form of an integral initializer list.
        /// \param wildcard The value to represent an unknown byte in the pattern.
        /// \code{.cpp}
        /// // will match any byte sequence where first byte is 0x11, third is 0x13 and fourth is 0x14
        /// memory_signature{{0x11, 0x12, 0x13, 0x14}, 0x12};
        /// \endcode
        template<class Wildcard>
        memory_signature(std::initializer_list<Wildcard> pattern, Wildcard wildcard)
                : _pattern(new std::uint8_t[pattern.size()])
                , _end(std::copy(pattern.begin(), pattern.end(), _pattern.get()))
                , _wildcard(static_cast<std::uint8_t>(wildcard)) {}

        /// masked signature constructors ------------------------------------------------------------------------------

        /// \brief Construct a new signature using a pattern and a mask.
        /// \param pattern The pattern in the form of an integral initializer list.
        /// \param mask The mask for pattern as a string where value of unknown_byte_identifier
        ///             is the unknown byte which can be anything in the pattern. By default '?'.
        /// \param unknown_byte_identifier The value to represent an unknown byte in the mask.
        /// \code{.cpp}
        /// // will match any byte sequence where first byte is 0x11, third is 0x13 and fourth is 0x14
        /// memory_signature{{0x11, 0x12, 0x13, 0x14}, "x?xx", '?'};
        /// \endcode
        template<class Byte>
        memory_signature(std::initializer_list<Byte> pattern, const std::string &mask
                         , Byte unknown_byte_identifier = '?')
                : _pattern(new std::uint8_t[pattern.size()])
                , _end(_pattern.get() + pattern.size())
                , _wildcard(detail::find_wildcard_masked(pattern.begin(), pattern.end(), mask.begin()
                                                         , unknown_byte_identifier))
        {
            if (pattern.size() != mask.size())
                throw std::invalid_argument("pattern size did not match mask size");

            masked_to_wildcard(pattern.begin(), pattern.end(), mask.begin(), unknown_byte_identifier);
        }

        /// \brief Construct a new signature using a pattern and a mask.
        /// \param pattern The pattern in the form of an integral initializer list.
        /// \param mask The mask for pattern as an integral initializer list where value of unknown_byte_identifier
        ///             is the unknown byte which can be anything in the pattern. By default 0.
        /// \param unknown_byte_identifier The value to represent an unknown byte in the mask.
        /// \code{.cpp}
        /// // will match any byte sequence where first byte is 0x11, third is 0x13 and fourth is 0x14
        /// memory_signature{{0x11, 0x12, 0x13, 0x14}, {1, 0, 1, 1}, 0};
        /// \endcode
        template<class Byte>
        memory_signature(std::initializer_list<Byte> pattern, std::initializer_list<Byte> mask
                         , Byte unknown_byte_identifier = 0)
                : _pattern(new std::uint8_t[pattern.size()])
                , _end(_pattern.get() + pattern.size())
                , _wildcard(detail::find_wildcard_masked(pattern.begin(), pattern.end(), mask.begin()
                                                         , unknown_byte_identifier))
        {
            if (pattern.size() != mask.size())
                throw std::invalid_argument("pattern size did not match mask size");

            masked_to_wildcard(pattern.begin(), pattern.end(), mask.begin(), unknown_byte_identifier);
        }

        /// hybrid / ida style signature constructors ------------------------------------------------------------------

        /// \brief Construct a new ida style signature.
        /// \param pattern The pattern in the form of a string where numbers and letter are transformed to known bytes
        ///                 and question marks are unknown bytes.
        /// \code{.cpp}
        /// // will match any byte sequence where first byte is 0x1, third is 0x13 and fourth is 0x14
        /// memory_signature{"01 ?? 13 14"};
        /// memory_signature{"1 ? 13 14"};
        /// \endcode
        memory_signature(const std::string &pattern)
                : _pattern(new std::uint8_t[pattern.size()])
                , _end(_pattern.get() + pattern.size())
                , _wildcard(detail::find_wildcard_hybrid(pattern.begin(), pattern.end()))
        {
            hybrid_to_wildcard(pattern.begin(), pattern.end());
        }

		/// general use functions --------------------------------------------------------------------------------------

		bool empty() const noexcept { return _pattern.get() == _end; }

        /// \brief Searches for first occurrence of stored signature in the range [first, last - signature_length).
        /// \param first The first element of the range in which to search for.
        /// \param last The one past the last element of the range in which to search for.
        /// \return Returns iterator to the beginning of signature.
        ///         If no such signature is found or if signature is empty returns last.
        template<class ForwardIt>
        ForwardIt find(ForwardIt first, ForwardIt last) const
        {
            if (empty())
                return last;

            // we need this to avoid capture of this in c++11
            const auto my_wildcard = _wildcard;
            return std::search(first, last, _pattern.get(), _end, [=](std::uint8_t lhs, std::uint8_t rhs) {
                return lhs == rhs || rhs == my_wildcard;
            });
        }

        /// \brief Searches for first occurrence of stored signature in the given range.
        /// \param range The range in which to search for.
        /// \return Returns iterator to the beginning of signature.
        ///         If no such signature is found or if signature is empty returns end of range.
        template<class Range>
        auto find(const Range& range) const -> decltype(begin(range))
        {
            using std::end;
            using std::begin;
            return find(begin(range), end(range));
        }

    }; // class memory_signature

} // namespace jm

#endif // include guard
