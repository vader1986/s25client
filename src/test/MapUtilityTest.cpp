// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
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

#include "mapGenerator/MapUtility.h"
#include "mapGenerator/Map.h"
#include <boost/test/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(MapUtilityTest)

/**
 * Tests the MapUtility::ComputeWaterSize method for no water.
 */
BOOST_FIXTURE_TEST_CASE(ComputeWaterSize_NoWater, MapUtility)
{
    Map* map = new Map(64, 64, "map", "author");

    const int water = MapUtility::ComputeWaterSize(map, Vec2(10, 10), 100);
    
    BOOST_REQUIRE_EQUAL(water, 0);
    
    delete map;
}

/**
 * Tests the MapUtility::ComputeWaterSize method for a water map.
 */
BOOST_FIXTURE_TEST_CASE(ComputeWaterSize_Water, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    for (int i = 0; i < 256; i++) map->textureLsd[i] = 0x05;
    for (int i = 0; i < 256; i++) map->textureRsu[i] = 0x05;
    
    const int water = MapUtility::ComputeWaterSize(map, Vec2(10, 10), 300);
    
    BOOST_REQUIRE_EQUAL(water, 256);
    
    delete map;
}

/**
 * Tests the MapUtility::ComputeWaterSize method for a water map with maximum limit.
 */
BOOST_FIXTURE_TEST_CASE(ComputeWaterSize_Limit, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    for (int i = 0; i < 256; i++) map->textureLsd[i] = 0x05;
    for (int i = 0; i < 256; i++) map->textureRsu[i] = 0x05;
    
    const int water = MapUtility::ComputeWaterSize(map, Vec2(10, 10), 100);
    
    BOOST_REQUIRE_EQUAL(water, 100);
    
    delete map;
}

/**
 * Tests the MapUtility::SetHill method.
 */
BOOST_FIXTURE_TEST_CASE(SetHill_Height, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    MapUtility::SetHill(map, Vec2(0, 0), 0x4);
    
    BOOST_REQUIRE_EQUAL(map->z[0], 0x4);
    
    delete map;
}

/**
 * Tests the VertexUtility::Smooth method to ensure mountain-meadow textures are
 * replaced by meadow if they have no neighboring mountain-textures.
 */
BOOST_FIXTURE_TEST_CASE(Smooth_MountainMeadowReplaced, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        map->textureLsd[i] = 0x12;
        map->textureRsu[i] = 0x12;
    }
    
    MapUtility::Smooth(map);

    for (int i = 0; i < map->width * map->height; i++)
    {
        BOOST_REQUIRE_EQUAL(map->textureLsd[i], 0x8);
        BOOST_REQUIRE_EQUAL(map->textureRsu[i], 0x8);
    }
    
    delete map;
}

/**
 * Tests the VertexUtility::Smooth method to ensure that height of mountain-textures
 * is increased.
 */
BOOST_FIXTURE_TEST_CASE(Smooth_MountainIncreased, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        map->z[i] = 0x0;
        map->textureLsd[i] = 0x01;
        map->textureRsu[i] = 0x01;
    }
    
    MapUtility::Smooth(map);
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        BOOST_REQUIRE_GT(map->z[i], 0x0);
    }
    
    delete map;
}

/**
 * Tests the VertexUtility::Smooth method to ensure that height of snow-textures
 * is increased.
 */
BOOST_FIXTURE_TEST_CASE(Smooth_SnowIncreased, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        map->z[i] = 0x0;
        map->textureLsd[i] = 0x02;
        map->textureRsu[i] = 0x02;
    }
    
    MapUtility::Smooth(map);
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        BOOST_REQUIRE_GT(map->z[i], 0x0);
    }
    
    delete map;
}

/**
 * Tests the VertexUtility::Smooth method to ensure that height of meadow-textures
 * are NOT increased.
 */
BOOST_FIXTURE_TEST_CASE(Smooth_MeadowNotIncreased, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        map->z[i] = 0x0;
        map->textureLsd[i] = 0x08;
        map->textureRsu[i] = 0x08;
    }
    
    MapUtility::Smooth(map);
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        BOOST_REQUIRE_EQUAL(map->z[i], 0x0);
    }
    
    delete map;
}

/**
 * Tests the VertexUtility::Smooth method to ensure that single textures which are surounded
 * by other textures are replaced properly.
 */
BOOST_FIXTURE_TEST_CASE(Smooth_SingleTexturesReplaced, MapUtility)
{
    Map* map = new Map(16, 16, "map", "author");
    
    for (int i = 0; i < map->width * map->height; i++)
    {
        map->textureLsd[i] = 0x08;
        map->textureRsu[i] = 0x08;
    }
    map->textureRsu[0] = 0x02;
    
    MapUtility::Smooth(map);
    
    BOOST_REQUIRE_EQUAL(map->textureRsu[0], 0x8);
    
    delete map;
}

BOOST_AUTO_TEST_SUITE_END()

