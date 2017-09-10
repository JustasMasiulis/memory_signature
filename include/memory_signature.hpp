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

#include <memory>    // unique_ptr
#include <algorithm> // search

namespace jm {

    class memory_signature {
        std::unique_ptr<unsigned char[]> _pattern;
        unsigned char                    *_end;
        unsigned char                    _wildcard;

        std::size_t size() const noexcept { return _end - _pattern.get(); }

    public:
        /// \brief Construct a new signature that is empty
        explicit constexpr memory_signature() noexcept
                : _pattern(nullptr)
                , _end(nullptr)
                , _wildcard(0) {}

        /// \brief Destroys the stored signature
        ~memory_signature() noexcept = default;

        memory_signature(const memory_signature &other)
                : _wildcard(other._wildcard)
        {
            auto new_pat = std::make_unique<unsigned char[]>(other.size());
            std::copy(other._pattern.get(), other._end, new_pat.get());
            _pattern = std::move(new_pat);
            _end     = _pattern.get() + other.size();
        }

        memory_signature &operator=(const memory_signature &other)
        {
            auto new_pat = std::make_unique<unsigned char[]>(other.size());
            std::copy(other._pattern.get(), other._end, new_pat.get());
            _pattern  = std::move(new_pat);
            _end      = _pattern.get() + other.size();
            _wildcard = other._wildcard;
            return *this;
        }

        memory_signature(const memory_signature &&other) noexcept
                : _pattern(std::move(other._pattern))
                , _end(other._end)
                , _wildcard(other._wildcard) {}

        memory_signature &operator=(memory_signature &&other) noexcept
        {
            _pattern  = std::move(other._pattern);
            _end      = other._end;
            _wildcard = other._wildcard;
            return *this;
        }


        memory_signature(const std::string &pattern, unsigned char wildcard)
                : _pattern(std::make_unique<unsigned char[]>(pattern.size()))
                , _end(_pattern.get() + pattern.size())
                , _wildcard(wildcard)
        {
            std::copy(std::begin(pattern), std::end(pattern), _pattern.get());
        }

        memory_signature(std::initializer_list<unsigned char> pattern, unsigned char wildcard)
                : _pattern(std::make_unique<unsigned char[]>(pattern.size()))
                , _end(_pattern.get() + pattern.size())
                , _wildcard(wildcard)
        {
            std::copy(std::begin(pattern), std::end(pattern), _pattern.get());
        }


        /// \brief Searches for first occurrence of stored signature in the range [first, last - signature_length).
        /// \return Returns iterator to the beginning of signature.
        ///         If no such signature is found or if signature is empty returns last.
        template<class ForwardIt>
        ForwardIt find(ForwardIt first, ForwardIt last) const
        {
            if (_pattern.get() == _end)
                return last;

            return std::search(first, last, _pattern.get(), _end, [wildcard = _wildcard](unsigned char lhs
                                                                                         , unsigned char rhs) {
                return lhs == rhs || rhs == wildcard;
            });
        }
    };

}

#endif // include guard
