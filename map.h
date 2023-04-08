/***********************************************************************\
*
* Contents: map functions
* Systems: all
*
\***********************************************************************/
#ifndef MAP_H
#define MAP_H

/****************************** Includes *******************************/
#include <ostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cassert>

#include "color.h"

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/
typedef unsigned int uint;

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

    Coordinates()
      : x(0)
      , y(0)
    {
    }

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

    Tile(const Coordinates &coordinates, Types type, const Color &color)
      : coordinates(coordinates)
      , type(type)
      , color(color)
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

    Island(Tile &tile)
      : tiles({&tile})
      , frontiers({tile.getCoordinates()})
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

    /** fetch and remove next frontier coordinates
     * @return frontier coordinates
     */
    Coordinates fetchFrontier()
    {
      Coordinates coordinates = frontiers.back();
      frontiers.pop_back();

      return coordinates;
    }

    /** merge island into island
     * @param island island to merge
     */
    void merge(Island *island)
    {
      assert(island != nullptr);

      for (Tile *tile : island->tiles)
      {
        assert(tile != nullptr);
        tile->setIsland(this);
        tiles.push_back(tile);
      }
      for (const Coordinates &coordinates : island->frontiers)
      {
        frontiers.push_back(coordinates);
      }
    }

    friend std::ostream& operator<<(std::ostream &outputStream, const Island *island)
    {
      outputStream << "Island { frontiers=" << island->frontiers.size() << " }";

      return outputStream;
    }

  private:
    char                     id;
    std::vector<Tile*>       tiles;
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

    void reset()
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

    uint getWidth() const
    {
      return width;
    }

    uint getHeight() const
    {
      return height;
    }

    Tile &getTile(int x, int y)
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

    uint findIslands();

    std::unordered_set<Island*> &getIslands()
    {
      return islands;
    }

    /** print map with detected islands
     */
    void printIslands() const;

    void print() const;

    void save(const std::string &filePath);

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

#endif // MAP_H

/* end of file */
