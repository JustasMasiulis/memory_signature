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

    class signature {
        std::unique_ptr<unsigned char[]> _pattern;
        unsigned char                    *_end;
        unsigned char                    _wildcard;

    public:
        /// \brief Construct a new signature that is empty
        explicit signature() noexcept
                : _pattern(nullptr)
                , _end(nullptr)
                , _wildcard(0)
        {}

        /// \brief Destroys the stored signature
        ~signature() noexcept = default;

        


        /// \brief Searches for first occurrence of stored signature in the range [first, last - signature_length).
        /// \return Returns iterator to the beginning of signature.
        ///         If no such signature is found or if signature is empty returns last.
        template<class ForwardIt>
        ForwardIt find(ForwardIt first, ForwardIt last) const;
    };

    template<class ForwardIt>
    ForwardIt signature::find(ForwardIt first, ForwardIt last) const
    {
        if (_pattern.get() == _end)
            return last;

        return std::search(first, last, _pattern.get(), _end, [wildcard = _wildcard](unsigned char lhs
                                                                                     , unsigned char rhs) {
            return lhs == rhs || rhs == wildcard;
        });
    }

}

#endif // include guard
