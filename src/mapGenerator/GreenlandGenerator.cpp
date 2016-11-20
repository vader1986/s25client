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

#include "defines.h" // IWYU pragma: keep

#include "mapGenerator/GreenlandGenerator.h"
#include "mapGenerator/ObjectGenerator.h"
#include "mapGenerator/VertexUtility.h"

#include "Random.h"

#include <vector>
#include <cstdlib>
#include <cmath>

// harbor placement
#define MIN_HARBOR_DISTANCE     35.0
#define MIN_HARBOR_WATER        200

TerrainType GreenlandGenerator::Textures[MAXIMUM_HEIGHT] =
{
    TT_WATER, TT_WATER, TT_WATER, TT_WATER,     // 0-3
    TT_DESERT,                                  // 4
    TT_STEPPE,                                  // 5
    TT_SAVANNAH,                                // 6
    TT_MEADOW1,                                 // 7
    TT_MEADOW_FLOWERS,                          // 8
    TT_MEADOW2, TT_MEADOW2,                     // 9-10
    TT_MOUNTAINMEADOW,                          // 11
    TT_MOUNTAIN1, TT_MOUNTAIN1, TT_MOUNTAIN1,   // 12-14
    TT_SNOW, TT_SNOW, TT_SNOW, TT_SNOW, TT_SNOW,
    TT_SNOW, TT_SNOW, TT_SNOW, TT_SNOW, TT_SNOW // 15-24
};

int GreenlandGenerator::GetMaxTerrainHeight(const TerrainType terrain)
{
    int maxHeight = -1;
    for (int i = 0; i < MAXIMUM_HEIGHT; i++)
    {
        if (Textures[i] == terrain)
        {
            maxHeight = i;
        }
    }
    
    return maxHeight;
}

int GreenlandGenerator::GetMinTerrainHeight(const TerrainType terrain)
{
    for (int i = 0; i < MAXIMUM_HEIGHT; i++)
    {
        if (Textures[i] == terrain)
        {
            return i;
        }
    }
    
    return -1;
}

void GreenlandGenerator::CreateEmptyTerrain(const MapSettings& settings, Map* map)
{
    const int width = map->width;
    const int height = map->height;
    
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            map->vertex[j * width + i].z = 0;
            map->vertex[j * width + i].texture = ObjectGenerator::CreateTexture(TT_MEADOW1);
            map->vertex[j * width + i].build = 0x04;
            map->vertex[j * width + i].shading = 0x80;
            map->vertex[j * width + i].resource = 0x00;
            map->vertex[j * width + i].road = 0x00;
            map->vertex[j * width + i].object.first = 0x00;
            map->vertex[j * width + i].object.second = 0x00;
            map->vertex[j * width + i].animal = 0x00;
            map->vertex[j * width + i].unknown1 = 0x00;
            map->vertex[j * width + i].unknown2 = 0x07;
            map->vertex[j * width + i].unknown3 = 0x00;
            map->vertex[j * width + i].unknown5 = 0x00;
        }
    }
}

void GreenlandGenerator::PlacePlayers(const MapSettings& settings, Map* map)
{
    const int width = map->width;
    const int height = map->height;
    const int length = std::min(width / 2, height / 2);
    
    // compute center of the map
    Vec2 center(width / 2, height / 2);

    // radius for player distribution
    const int rMin = (int)(0.3 * length);;
    const int rMax = (int)(0.8 * length);
    const int rnd = RANDOM.Rand(__FILE__, __LINE__, 0, rMax - rMin);
    
    // player headquarters for the players
    for (int i = 0; i < settings.players; i++)
    {
        // compute headquater position
        Vec2 position = ComputePointOnCircle(i, settings.players, center, (double)(rMin + rnd));

        // create headquarter
        map->positions[i] = position;
        map->vertex[position.y * width + position.x].object = ObjectGenerator::CreateHeadquarter(i);
    }
}

void GreenlandGenerator::PlacePlayerResources(const MapSettings& settings, Map* map)
{
    for (int i = 0; i < settings.players; i++)
    {
        const int offset1 = RANDOM.Rand(__FILE__, __LINE__, i, 180);
        const int offset2 = RANDOM.Rand(__FILE__, __LINE__, i, 180) + 180;

        SetStones(map, ComputePointOnCircle(offset1, 360, map->positions[i], 12), 2.0F);
        SetStones(map, ComputePointOnCircle(offset2, 360, map->positions[i], 12), 2.7F);
    }
}

void GreenlandGenerator::CreateHills(const MapSettings& settings, Map* map)
{
    const int width = map->width;
    const int height = map->height;
    const int players = settings.players;
    
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            double distanceToPlayer = (double)(width + height);
            
            for (int i = 0; i < players; i++)
            {
                distanceToPlayer = std::min(distanceToPlayer,
                                       VertexUtility::Distance(x, y,
                                                               map->positions[i].x,
                                                               map->positions[i].y,
                                                               width, height));
            }
            
            for (std::vector<AreaDesc>::iterator it = _areas.begin(); it != _areas.end(); ++it)
            {
                
                if (it->IsInArea(x, y, distanceToPlayer, width, height))
                {
                    const int pr = (int)(*it).likelyhoodHill;
                    const int rnd = rand() % (pr > 0 ? 101 : (int)(100.0 / (*it).likelyhoodHill));
                    const int minZ = (*it).minElevation;
                    const int maxZ = (*it).maxElevation;
                    
                    if (maxZ > 0 && rnd <= pr)
                    {
                        int z = minZ + rand() % (maxZ - minZ + 1);
                        SetHill(map, Vec2(x, y), z == GetMinTerrainHeight(TT_MOUNTAIN1) - 1 ? z-1 : z);
                    }
                }
            }
        }
    }
}

void GreenlandGenerator::FillRemainingTerrain(const MapSettings& settings, Map* map)
{
    const int width = map->width;
    const int height = map->height;
    const int players = settings.players;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            const int index = y * width + x;
            const int level = map->vertex[index].z;
            
            // create texture for current height value
            map->vertex[index].texture = ObjectGenerator::CreateTexture(Textures[level]);
            
            // post-processing of texture (add animals, adapt height, ...)
            switch (Textures[level])
            {
                case TT_WATER:
                    map->vertex[index].z = GetMaxTerrainHeight(TT_WATER);
                    map->vertex[index].animal = ObjectGenerator::CreateDuck(3);
                    map->vertex[index].resource = 0x87; // fish
                    break;
                case TT_MEADOW1:
                    map->vertex[index].animal = ObjectGenerator::CreateRandomForestAnimal(4);
                    break;
                case TT_MEADOW_FLOWERS:
                    map->vertex[index].animal = ObjectGenerator::CreateSheep(4);
                    break;
                case TT_MOUNTAIN1:
                    map->vertex[index].resource = ObjectGenerator::CreateRandomResource();
                    break;
                default:
                    break;
            }
            
            double distanceToPlayer = (double)(width + height);
            for (int i = 0; i < players; i++)
            {
                distanceToPlayer = std::min(distanceToPlayer,
                                            VertexUtility::Distance(x, y,
                                                                    map->positions[i].x,
                                                                    map->positions[i].y,
                                                                    width,
                                                                    height));
            }

            for (std::vector<AreaDesc>::iterator it = _areas.begin(); it != _areas.end(); ++it)
            {
                if (it->IsInArea(x, y, distanceToPlayer, width, height))
                {
                    if (rand() % 100 < (*it).likelyhoodTree)
                    {
                        SetTree(map, Vec2(x,y));
                    }
                    else if (rand() % 100 < (*it).likelyhoodStone)
                    {
                        SetStone(map, Vec2(x,y));
                    }
                }
            }
        }
    }
    
    ///////
    /// Harbour placement
    ///////
    std::vector<Vec2> harbors;
    
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            const int index = VertexUtility::GetIndexOf(x, y, width, height);
            
            // under certain circumstances replace dessert texture by harbor position
            Vec2 water(0,0);
            if (ObjectGenerator::IsTexture(map->vertex[index].texture, TT_DESERT))
            {
                // ensure there's water close to the dessert texture
                bool waterNeighbor = false;
                std::vector<int> neighbors = VertexUtility::GetNeighbors(x, y, width, height, 1);
                for (std::vector<int>::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
                {
                    if (ObjectGenerator::IsTexture(map->vertex[*it].texture, TT_WATER))
                    {
                        waterNeighbor = true;
                        water = Vec2(*it % width, *it / width);
                        break;
                    }
                }
                
                // ensure there's no other harbor nearby
                double closestHarbor = MIN_HARBOR_DISTANCE + 1.0;
                for (std::vector<Vec2>::iterator it = harbors.begin(); it != harbors.end(); ++it)
                {
                    closestHarbor = std::min(closestHarbor,
                                             VertexUtility::Distance(x, y, it->x, it->y, width, height));
                }
                
                const int waterTiles = (closestHarbor >= MIN_HARBOR_DISTANCE && waterNeighbor)
                                        ? ComputeWaterSize(map, water, MIN_HARBOR_WATER) : 0;

                // setup harbor position
                if (waterTiles >= MIN_HARBOR_WATER)
                {
                    SetHarbour(map, Vec2(x, y), GetMaxTerrainHeight(TT_WATER));
                    harbors.push_back(Vec2(x,y));
                }
            }
        }
    }
}

Map* GreenlandGenerator::GenerateMap(const MapSettings& settings)
{
    RANDOM.Init(0);
    
    Map* map = new Map();
    
    // configuration of the map header
    std::string name("Random");
    for (unsigned int i = 0; i < name.length(); i++)
    {
        map->name[i] = name[i];
    }
    std::string author("auto");
    for (unsigned int i = 0; i < author.length(); i++)
    {
        map->author[i] = author[i];
    }
    
    // configuration of the map settings
    map->width = settings.width;
    map->height = settings.height;
    map->type = settings.type;
    map->players = settings.players;
    map->vertex = new Vertex[settings.width * settings.height];

    // the actual map generation
    CreateEmptyTerrain(settings, map);
    PlacePlayers(settings, map);
    PlacePlayerResources(settings, map);
    CreateHills(settings, map);
    FillRemainingTerrain(settings, map);
    
    return map;
}


