/***********************************************************************\
*
* Contents: map functions
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
#include <algorithm>
#include <exception>
#include <cassert>

#include "map.h"

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/
typedef unsigned int uint;

/***************************** Variables *******************************/

/****************************** Macros *********************************/

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

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
  else
  {
    std::stringstream buffer;
    buffer << "cannot open file '" << filePath << "'";
    throw std::ios_base::failure(buffer.str());
  }
}

uint Map::findIslands()
{
  // init islands: everything which is not water
  for (std::vector<Tile> &row : tiles)
  {
    for (Tile &tile : row)
    {
      if (tile.getType() != Tile::Types::WATER)
      {
        Island *island = new Island(tile);
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

      // merge connected neighbor islands into island
      while (island->hasFrontiers())
      {
        // neighbour offset
        static const std::array<Coordinates, 8> NEIGHBORS =
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

        Coordinates coordinates = island->fetchFrontier();

        for (const Coordinates &neightbor : NEIGHBORS)
        {
          Tile &neighborTile = getTile(coordinates.x + neightbor.x, coordinates.y + neightbor.y);

          if (!neighborTile.isIsland(island) && (neighborTile.getType() != Tile::Types::WATER))
          {
            Island *neighborIsland = neighborTile.getIsland();
            islands.erase(neighborIsland);
            island->merge(neighborIsland);
            delete(neighborIsland);
          }
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

/** print map with detected islands
 */
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

void Map::save(const std::string &filePath)
{
  FILE *handle;

  handle = fopen("x.pnm", "w");

  fprintf(handle, "P3\n");
  fprintf(handle, "%d %d\n", width, height);
  fprintf(handle, "255\n");

  for (uint y = 0; y < height; y++)
  {
    for (uint x = 0; x < width; x++)
    {
// TODO:      fprintf(handle, "%d %d %d ", textureData[y][x].r, textureData[y][x].g, textureData[y][x].b);

      if ((x % 10) == 0)
      {
        fprintf(handle, "\n");
      }
    }

    fprintf(handle, "\n");
  }

  fclose(handle);
}

/* end of file */
