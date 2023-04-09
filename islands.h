/***********************************************************************\
*
* Contents: islands detector
* Systems: all
*
\***********************************************************************/
#ifndef ISLANDS_H
#define ISLANDS_H

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

#include "color.h"

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/

/***************************** Variables *******************************/

/****************************** Macros *********************************/

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

/** 2D coordinates
 */
class Coordinates
{
  public:
    int x,y;

    Coordinates(int x, int y)
      : x(x)
      , y(y)
    {
    }

    friend std::ostream& operator<<(std::ostream &outputStream, const Coordinates &coordinates)
    {
      outputStream << "Coordinates { " << coordinates.x << ", " << coordinates.y << " }";

      return outputStream;
    }
};

class Island;

/** tile in map
 */
class Tile
{
  public:
    enum class Types
    {
      WATER,
      LAND,
      TREE,
      MOUNTAIN,
      BUILDING
    };

    Tile()
      : coordinates(Coordinates(0,0))
      , type(Types::WATER)
      , island(nullptr)
    {
    }

    Tile(Types type)
      : coordinates(Coordinates(0,0))
      , type(type)
      , island(nullptr)
    {
    }

    Tile(const Coordinates &coordinates, Types type)
      : coordinates(coordinates)
      , type(type)
      , island(nullptr)
    {
    }

    Tile(uint x, uint y, Types type)
      : coordinates(Coordinates(x,y))
      , type(type)
      , island(nullptr)
    {
    }

    const Coordinates &getCoordinates() const
    {
      return coordinates;
    }

    Types getType() const
    {
      return type;
    }

    Color getColor() const
    {
      return color;
    }

    void set(const Coordinates &coordinates, Types type, const Color &color)
    {
      this->coordinates = coordinates;
      this->type        = type;
      this->color       = color;
    }

    void set(const Coordinates &coordinates, Types type)
    {
      this->coordinates = coordinates;
      this->type        = type;
    }

    Island *getIsland() const
    {
      return island;
    }

    void setIsland(Island *island)
    {
      assert(island != nullptr);

      this->island = island;
    }

    bool isIsland(const Island *island) const
    {
      return this->island == island;
    }

    friend std::ostream& operator<<(std::ostream &outputStream, const Tile &tile)
    {
      outputStream << "Tile { " << tile.coordinates << ", type=";
      switch (tile.getType())
      {
        case Types::WATER:
          outputStream << "water";
          break;
        case Types::LAND:
          outputStream << "land";
          break;
        case Types::TREE:
          outputStream << "tree";
          break;
        case Types::MOUNTAIN:
          outputStream << "mountain";
          break;
        case Types::BUILDING:
          outputStream << "building";
          break;
      }
      outputStream << " }";

      return outputStream;
    }

  private:
    Coordinates coordinates;
    Types       type;
    Color       color;
    Island      *island;
};

/** island
 */
class Island
{
  public:
    Island()
    {
    }

    Island(const Coordinates &coordinates)
      : frontiers({coordinates})
    {
    }

    char getId() const
    {
      return id;
    }

    void setId(char id)
    {
      this->id = id;
    }

    /** check if there are any unexplorted frontier tiles available
     * @return true iff frontier tiles available
     */
    bool hasFrontiers() const
    {
      return frontiers.size() > 0;
    }

    /** fetch and remove next frontier coordinate
     * @return coordinates
     */
    Coordinates fetchFrontier()
    {
      Coordinates coordinates = frontiers.back();
      frontiers.pop_back();

      return coordinates;
    }

    /** merge tile into island
     * @param tile tile
     */
    void merge(Tile &tile)
    {
      tile.setIsland(this);
      frontiers.push_back(tile.getCoordinates());
    }

    friend std::ostream& operator<<(std::ostream &outputStream, const Island *island)
    {
      outputStream << "Island { frontiers=" << island->frontiers.size() << " }";

      return outputStream;
    }

  private:
    char                     id;
    std::vector<Coordinates> frontiers;
};

/** map
 */
class Map
{
  public:
    Map()
      : width(0)
      , height(0)
    {
    }

    Map(uint width, uint height)
      : width(width)
      , height(height)
    {
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

    virtual ~Map()
    {
      for (const Island *island : islands)
      {
        delete(island);
      }
    }

    void reset();

    uint getWidth() const
    {
      return width;
    }

    uint getHeight() const
    {
      return height;
    }

    Tile &getTile(int x, int y);

    void setTile(uint x, uint y, Tile::Types type, const Color &color)
    {
      assert(x < width);
      assert(y < height);

      tiles[y][x].set(Coordinates(x, y), type, color);
    }

    void setTile(uint x, uint y, Tile::Types type)
    {
      assert(x < width);
      assert(y < height);

      tiles[y][x].set(Coordinates(x, y), type);
    }

    void load(const std::string &filePath);

    void save(const std::string &filePath);

    uint findIslands();

    /** print map with detected islands
     */
    void printIslands() const;

    void print() const;

    friend std::ostream& operator<<(std::ostream &outputStream, const Map &map)
    {
      outputStream << "Map { ";
      outputStream << " }";

      return outputStream;
    }

  private:
    uint                           width, height;
    std::vector<std::vector<Tile>> tiles;
    std::unordered_set<Island*>    islands;
};

#endif // ISLANDS_H

/* end of file */
