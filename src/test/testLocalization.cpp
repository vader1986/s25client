// Copyright (c) 2016 -2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "rttrDefines.h" // IWYU pragma: keep
#include "Loader.h"
#include "RttrConfig.h"
#include "files.h"
#include "languages.h"
#include "test/helperFuncs.h"
#include "mygettext/mygettext.h"
#include "libsiedler2/ArchivItem_Ini.h"
#include "libutil/StringConversion.h"
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>
#include <string>
#include "helpers/containerUtils.h"

struct LocaleFixture
{
    LocaleFixture() { LOADER.LoadFile(RTTRCONFIG.ExpandPath(FILE_PATHS[95]) + "/languages.ini", NULL, true); }
    std::vector<std::string> getLanguageCodes()
    {
        std::vector<std::string> codes;
        codes.push_back("C");
        const unsigned numLanguages = LANGUAGES.getCount();
        for(unsigned i = 0; i < numLanguages; i++)
            codes.push_back(LANGUAGES.getLanguage(i).code);
        return codes;
    }
};

BOOST_FIXTURE_TEST_SUITE(Locales, LocaleFixture)

BOOST_AUTO_TEST_CASE(ConvertToString)
{
    BOOST_FOREACH(const std::string& curLang, getLanguageCodes())
    {
        LocaleResetter resetter(curLang.c_str());
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(0), "0");
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(-2147483647), "-2147483647"); // -2^31+1
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(2147483647), "2147483647");   // 2^31-1
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(4294967295u), "4294967295"); // 2^32-1
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(0.), "0");
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(-12345678.), "-12345678");
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(12345678.), "12345678");
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(-12345678.5), "-12345678.5");
        BOOST_REQUIRE_EQUAL(s25util::toStringClassic(12345678.5), "12345678.5");
    }
}

BOOST_AUTO_TEST_CASE(ConvertFromString)
{
    using namespace boost::assign;
    std::vector<std::string> invalidInts;
    invalidInts += "", "-", "+", "abc", ".1", ",1", "1.", "1,", "a1", "1a", "1-", "1 2", "--1", "++1";
    invalidInts += s25util::toStringClassic(static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1);
    invalidInts += s25util::toStringClassic(static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1);

    std::vector<std::string> invalidUints;
    invalidUints += "-", "+", "-1", "1-";
    invalidUints += s25util::toStringClassic(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1);

    std::vector<std::string> invalidFloats;
    invalidFloats += "", "-", "+", "abc", ",1", "1,", "a1", "1a", "1-", "1,1", "1 2", "--1", "++1";

    // Partial values. At least all from above. All equal 1
    std::vector<std::string> partials;
    partials += "1.", "1,", "1a", "1-", "1 2", "1,1";

    BOOST_FOREACH(const std::string& curLang, getLanguageCodes())
    {
        LocaleResetter resetter(curLang.c_str());
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("+0"), 0);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("-0"), 0);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("0"), 0);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("-2147483647"), -2147483647); // -2^31+1
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("2147483647"), 2147483647);   // 2^31-1
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>("+2147483647"), 2147483647);   // 2^31-1
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<uint32_t>("4294967295"), 4294967295u); // 2^32-1
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<uint32_t>("+4294967295"), 4294967295u); // 2^32-1
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("0"), 0.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("0."), 0.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("+0."), 0.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("-0."), -0.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("-12345678."), -12345678.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("12345678."), 12345678.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("+12345678."), 12345678.);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("-12345678.5"), -12345678.5);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("12345678.5"), 12345678.5);
        BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>("+12345678.5"), 12345678.5);

        BOOST_FOREACH(const std::string& val, invalidInts)
        {
            int32_t outVal;
            BOOST_REQUIRE_THROW(s25util::fromStringClassic<int32_t>(val), s25util::ConversionError);
            BOOST_REQUIRE(!s25util::tryFromStringClassic<int32_t>(val, outVal));
            int32_t outValDef = rand();
            BOOST_REQUIRE_EQUAL(s25util::fromStringClassicDef<int32_t>(val, outValDef), outValDef);
            if(helpers::contains(partials, val))
                BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<int32_t>(val, true), 1);
            else
                BOOST_REQUIRE_THROW(s25util::fromStringClassic<int32_t>(val, true), s25util::ConversionError);
        }
        BOOST_FOREACH(const std::string& val, invalidUints)
        {
            uint32_t outVal;
            BOOST_REQUIRE_THROW(s25util::fromStringClassic<uint32_t>(val), s25util::ConversionError);
            BOOST_REQUIRE(!s25util::tryFromStringClassic<uint32_t>(val, outVal));
            uint32_t outValDef = rand();
            BOOST_REQUIRE_EQUAL(s25util::fromStringClassicDef<uint32_t>(val, outValDef), outValDef);
            if(helpers::contains(partials, val))
                BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<uint32_t>(val, true), 1u);
            else
                BOOST_REQUIRE_THROW(s25util::fromStringClassic<uint32_t>(val, true), s25util::ConversionError);
        }
        BOOST_FOREACH(const std::string& val, invalidFloats)
        {
            double outVal;
            BOOST_REQUIRE_THROW(s25util::fromStringClassic<double>(val), s25util::ConversionError);
            BOOST_REQUIRE(!s25util::tryFromStringClassic<double>(val, outVal));
            double outValDef = rand();
            BOOST_REQUIRE_EQUAL(s25util::fromStringClassicDef<double>(val, outValDef), outValDef);
            if(helpers::contains(partials, val))
                BOOST_REQUIRE_EQUAL(s25util::fromStringClassic<double>(val, true), 1.);
            else
                BOOST_REQUIRE_THROW(s25util::fromStringClassic<double>(val, true), s25util::ConversionError);
        }
    }
}

BOOST_AUTO_TEST_CASE(IniValues)
{
    libsiedler2::ArchivItem_Ini ini;
    BOOST_FOREACH(const std::string& curLang, getLanguageCodes())
    {
        LocaleResetter resetter(curLang.c_str());
        ini.setValue("int", 123456);
        ini.setValue("string", "123456");
        BOOST_REQUIRE_EQUAL(ini.getValue("int"), "123456");
        BOOST_REQUIRE_EQUAL(ini.getValueI("string"), 123456);
    }
}

BOOST_AUTO_TEST_SUITE_END()
