/***********************************************************************\
*
* Contents: islands detector
* Systems: all
*
\***********************************************************************/

/****************************** Includes *******************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <cassert>

#include "islands.h"

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/

/***************************** Variables *******************************/

/****************************** Macros *********************************/

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

void Map::reset()
{
  tiles.clear();
  for (uint y = 0; y < height; y++)
  {
    std::vector<Tile> row;
    for (uint x = 0; x < height; x++)
    {
      row.push_back(Tile());
    }
    tiles.push_back(row);
  }
}

Tile &Map::getTile(int x, int y)
{
  static Tile OFF_MAP_TILE = Tile(Tile::Types::WATER);

  if ((y >= 0) && (static_cast<size_t>(y) < tiles.size()))
  {
    std::vector<Tile> &row = tiles[y];

    if ((x >= 0) && (static_cast<size_t>(x) < row.size()))
    {
      return row[x];
    }
    else
    {
      return OFF_MAP_TILE;
    }
  }
  else
  {
    return OFF_MAP_TILE;
  }
}

void Map::load(const std::string &filePath)
{
  std::ifstream inputStream(filePath);
  if (inputStream.is_open())
  {
    uint        y = 0;
    std::string line;
    while (getline(inputStream,line) )
    {
      uint              x = 0;
      std::vector<Tile> row;
      for (const char ch : line)
      {
        switch (ch)
        {
          case '.':
            row.push_back(Tile(x, y, Tile::Types::WATER));
            break;
          case '+':
            row.push_back(Tile(x, y, Tile::Types::LAND));
            break;
          case '*':
            row.push_back(Tile(x, y, Tile::Types::TREE));
            break;
          case '^':
            row.push_back(Tile(x, y, Tile::Types::MOUNTAIN));
            break;
          case '@':
            row.push_back(Tile(x, y, Tile::Types::BUILDING));
            break;
          case '\n':
          case '\r':
            break;
          default:
            std::stringstream buffer;
            buffer << "unknown tile " << ch << " at " << y + 1 << ", " << x + 1;
            throw std::ios_base::failure(buffer.str());
            break;
        }

        x++;
      }

      tiles.push_back(row);

      y++;
    }
    inputStream.close();
  }
}

uint Map::findIslands()
{
  // neighbour offset
  std::array<Coordinates, 8> NEIGHBORS =
  {{
    Coordinates{-1,-1},
    Coordinates{ 0,-1},
    Coordinates{+1,-1},
    Coordinates{-1, 0},
    Coordinates{+1, 0},
    Coordinates{-1,+1},
    Coordinates{ 0,+1},
    Coordinates{+1,+1}
  }};

  // init islands: everything which is not water
  for (std::vector<Tile> &row : tiles)
  {
    for (Tile &tile : row)
    {
      if (tile.getType() != Tile::Types::WATER)
      {
        Island *island = new Island(tile.getCoordinates());
        tile.setIsland(island);

        islands.insert(island);
      }
    }
  }

  // merge islands
  std::unordered_set<Island*>::iterator iterator;
  do
  {
    // find an island which has frontier tiles
    iterator = std::find_if(islands.begin(),
                            islands.end(),
                            [](const Island *island) -> bool
                            {
                              assert(island != nullptr);

                              return island->hasFrontiers();
                            }
                           );
    if (iterator != islands.end())
    {
      Island *island = *iterator;
      assert(island != nullptr);

      // get next frontier coordinates
      Coordinates coordinates = island->fetchFrontier();

      // merge connected neighbor tiles into island
      for (const Coordinates &neightbor : NEIGHBORS)
      {
        Tile &neighborTile = getTile(coordinates.x + neightbor.x, coordinates.y + neightbor.y);

        if (!neighborTile.isIsland(island) && (neighborTile.getType() != Tile::Types::WATER))
        {
          Island *neighborIsland = neighborTile.getIsland();
          islands.erase(neighborIsland);
          island->merge(neighborTile);
          delete(neighborIsland);
        }
      }
    }
  }
  while (iterator != islands.end());

  // update island ids: enumerate A..z
  char id = 'A';
  for (Island *island : islands)
  {
    island->setId(id);
    id++;
  }

  return islands.size();
}

void Map::printIslands() const
{
  for (const std::vector<Tile> &row : tiles)
  {
    for (const Tile &tile : row)
    {
      if (tile.getIsland() != nullptr)
      {
        std::cout << tile.getIsland()->getId();
      }
      else
      {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
}

void Map::print() const
{
  for (const std::vector<Tile> &row : tiles)
  {
    for (const Tile &tile : row)
    {
      switch (tile.getType())
      {
        case Tile::Types::WATER:
          std::cout << " ";
          break;
        case Tile::Types::LAND:
          std::cout << "L";
          break;
        case Tile::Types::TREE:
          std::cout << "T";
          break;
        case Tile::Types::MOUNTAIN:
          std::cout << "M";
          break;
        case Tile::Types::BUILDING:
          std::cout << "B";
          break;
      }
    }
    std::cout << std::endl;
  }
}

/* end of file */
