// Copyright (c) 2016 - 2020 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#ifndef stubFunction_h__
#define stubFunction_h__

#include <boost/preprocessor/cat.hpp>

namespace rttr {

template<typename T>
struct StubFunctionReset
{
    T** funcRef_;
    T* origFunc_;
    StubFunctionReset(T*& function, T* newFunction) : funcRef_(&function), origFunc_(function) { function = newFunction; }
    ~StubFunctionReset()
    {
        if(funcRef_)
            *funcRef_ = origFunc_;
    }
    StubFunctionReset(const StubFunctionReset&) = delete;
    StubFunctionReset(StubFunctionReset&& rhs) noexcept : funcRef_(rhs.funcRef_), origFunc_(rhs.origFunc_) { rhs.funcRef_ = nullptr; }
};

#define RTTR_STUB_FUNCTION(function, newFunction) \
    rttr::StubFunctionReset<decltype(newFunction)> BOOST_PP_CAT(_stub_, __COUNTER__)(function, newFunction)

} // namespace rttr

#endif // stubFunction_h__
