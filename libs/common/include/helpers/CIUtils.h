// Copyright (c) 2018 - 2019 Settlers Freaks (sf-team at siedler25.org)
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

#ifndef CI_Utils_h__
#define CI_Utils_h__

#include <cstdlib>
#include <string>

namespace rttr {
inline bool isRunningOnCI()
{
    const auto* ciPtr = std::getenv("CI");
    if(!ciPtr)
        return false;
    const std::string ci = ciPtr;
    return ci == "true" || ci == "True";
}
} // namespace rttr

#endif // CI_Utils_h__
