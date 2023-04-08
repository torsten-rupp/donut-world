/***********************************************************************\
*
* Contents: map generator
* Systems: all
*          original from: https:// github.com/tmtorr/C_MapGen
\***********************************************************************/

/****************************** Includes *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#include "mapGenerator.h"

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/
//Constants and globals - future plans to minimize these as much as possible
const uint BORDER_Y = 1;
const uint BORDER_X = 1;

// anything that isn't modifiable land is stored as a negative int

//J is largest land val, P is smallest value
#define J 500 // Jungle
#define G 400 // Savanna / Grassland
#define D 300 // Desert
#define F 200 // Forest
#define T 100 // Tundra
#define P 0   // Polar: Snow and Ice

#define W1 -1 // Water
#define W2 -2 // Secondary Water layer; used to plump up rivers & non-detectable water layer
#define L1 -3 // Land Layer 1
#define L2 -4 // Land Layer 2

/***************************** Datatypes *******************************/
typedef int GeneratorTileTypes;

typedef struct
{
  GeneratorTileTypes type;
  Color              color;
} GeneratorTile;

/***************************** Variables *******************************/

/****************************** Macros *********************************/
#define LOCAL static

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

LOCAL int neg_pos_gen() // generates a -1 or 1
{
  int num = ((rand() % 2) - 1);

  if (num == 0)
  {
    num++;
  }

  return num;
}

LOCAL int skewed_neg_pos_gen(int skew_val)
{
  //Generates a -1 or 1 depending on skew val.
  //Skew_val over 6 six has a negative inclination
  //Skew_val under 6 six has a positive inclination

  /*
   skew_val    0  1  2  3  4  5  6  7  8  9 10 11 <- rand_val
         |
      0  V   | 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
      1      |-1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
      2      |-1|-1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
      3      |-1|-1|-1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
      4      |-1|-1|-1|-1| 1| 1| 1| 1| 1| 1| 1| 1|
      5      |-1|-1|-1|-1|-1| 1| 1| 1| 1| 1| 1| 1|
      6      |-1|-1|-1|-1|-1|-1| 1| 1| 1| 1| 1| 1| // equal probability
      7      |-1|-1|-1|-1|-1|-1|-1| 1| 1| 1| 1| 1|
      8      |-1|-1|-1|-1|-1|-1|-1|-1| 1| 1| 1| 1|
      9      |-1|-1|-1|-1|-1|-1|-1|-1|-1| 1| 1| 1|
      10     |-1|-1|-1|-1|-1|-1|-1|-1|-1|-1| 1| 1|
      11     |-1|-1|-1|-1|-1|-1|-1|-1|-1|-1|-1| 1|
      12     |-1|-1|-1|-1|-1|-1|-1|-1|-1|-1|-1|-1|

  */

  int rand_val = (rand() % 12);// num inclusively between 0 and 11

  if ((rand_val) >= skew_val)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

LOCAL inline GeneratorTile mapGet(const GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, uint x, uint y)
{
  assert(x < mapWidth);
  assert(y < mapHeight);

  return generatorMap[(y*mapWidth)+x];
}

LOCAL inline void mapSetType(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, uint x, uint y, GeneratorTileTypes type)
{
  assert(x < mapWidth);
  assert(y < mapHeight);

  generatorMap[(y*mapWidth)+x].type = type;
}

LOCAL inline void mapSetColor(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, uint x, uint y, uint8_t r, uint8_t g, uint8_t b)
{
  assert(x < mapWidth);
  assert(y < mapHeight);

  generatorMap[(y*mapWidth)+x].color.r = r;
  generatorMap[(y*mapWidth)+x].color.g = g;
  generatorMap[(y*mapWidth)+x].color.b = b;
}

LOCAL inline bool mapIs(const GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, uint x, uint y, GeneratorTileTypes type)
{
  assert(x < mapWidth);
  assert(y < mapHeight);

  return mapGet(generatorMap,mapWidth,mapHeight,x,y).type == type;
}

LOCAL void gen_stretched_hexagon(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, int water_or_land, int distortion) // generates a stretched out hexagon
{

  // a value of 80 is a good distortion value
  // variables for a stretched hexagon

  uint dstrt = 100 - distortion;
  if (dstrt < 1)
  {
    dstrt = 0;
  }

  // 1 = land     0 = water
  GeneratorTileTypes pixel = W1;
  GeneratorTileTypes not_pixel = L1;

  if (water_or_land == 1)
  {
    pixel = L1;
    not_pixel = W1;
  }

  uint x_min;
  uint x_max;
  uint y_min;
  uint y_max;

  uint max_line_size;

  uint rigidity = 2; // determines how squarish a map will be (0 = always slanted, huge values mean extreme squareness, e.g. 500 = square-like continents)

  // skew will determine how far left or right a continent border will slant (between 0 - 12 | 0 is left/down, 12 = right/up, 6 is equal slant)
//  int skew1;
//  int skew2;
  uint skew3;
  uint skew4;

  uint u_len_line; // upper edges
  uint u_border_y;
  uint u_border_x;

  uint l_len_line; // lower edges
  uint l_border_y;
  uint l_border_x;

  uint f_line_found;

  uint left_u_tri_skew; // upper triangle skew
  uint right_u_tri_skew;

  uint left_l_tri_skew; // lower triangle skew
  uint right_l_tri_skew;

  uint y_len; // an alternative if the lower triangle overflows

//  skew1 = (rand() % 13);
//  skew2 = (rand() % 13);
  skew3 = (rand() % 13);
  skew4 = (rand() % 13);

  //These do whiles set up the bounds of the continents

  y_max = (BORDER_Y) + (rand() % (mapHeight - (2 * BORDER_Y) - 30)) + 1 ;  //The 30 is a buffer
  do
  {
    y_min = (BORDER_Y) + (rand() % (mapHeight - (2 * BORDER_Y))) ;
  }
  while (y_min > y_max);

  if ((y_min - BORDER_Y) > ((mapHeight - y_max) - BORDER_Y))
  {
    max_line_size = (mapHeight - y_max) - BORDER_Y;
  }
  else
  {
    max_line_size = (y_min - BORDER_Y);
  }

  do
  {
    x_max = (BORDER_X) + (rand() % (mapWidth - (2 * BORDER_X))) + 1 ;    // random location between borders (plus 1, so smallest width is xmin=0 and xmax=1)
  }
  while ((x_max > mapWidth));

  do
  {
    x_min = ((BORDER_X)) + (rand() % (mapWidth - (2 * BORDER_X)));
  }
  while (x_min > x_max || ((x_max - x_min)/*length*/  > max_line_size));

  // These blocks of code remove a rectangular shape with triangles on top and bottom, out of the map
  u_border_y = y_min;
  u_border_x = x_min;
  u_len_line = x_max - x_min;

  f_line_found = 0;

  for (uint y = 0; y < mapHeight; y++)
  {
    for (uint x = 0; x < mapWidth; x++)
    {
      if (x >= x_min && x <= x_max && y >= y_min && y <= y_max)
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel); //W creates Water, L1 creates Land
        f_line_found = 1;

        if (rand() % dstrt == 1) //1 in (distortion_val) chance of a line length change
        {
          x_min = x_min + (rand() % 3 - 1);
          x_max = x_max + (rand() % 3 - 1);
        }
      }
      else
      {
        if (!mapIs(generatorMap,mapWidth,mapHeight,x,y,not_pixel))
        {
          mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel);
        }
      }
    }

    if (f_line_found == 1 && (rand() % dstrt) == 1)
    {
      do
      {
        x_min = x_min + (rand() % 3 - 1);

        if (x_min < BORDER_X)
        {
          x_min = BORDER_X + 2;
        }
        else if (x_min > (mapWidth - BORDER_X))
        {
          x_min = mapWidth - BORDER_X - 2;
        }
      }
      while (x_min < BORDER_X || x_min > (mapWidth - BORDER_X));

      do
      {
        x_max = x_max + (rand() % 3 - 1);

        if (x_max < BORDER_X)
        {
          x_max = BORDER_X + 2;
        }
        else if (x_max > (mapWidth - BORDER_X))
        {
          x_max = mapWidth - BORDER_X - 2;
        }
      }
      while (x_max < BORDER_X || x_max > (mapWidth - BORDER_X));
    }

    if (y == y_max)
    {
      l_border_y = y_max;
      l_border_x = x_min;
      l_len_line = x_max - x_min;
    }

  }

  // this block adds a triangular shape to the top border

  left_u_tri_skew = 1;
  right_u_tri_skew = 1;

  x_min = u_border_x ;
  x_max = u_border_x + u_len_line;
  y_min = u_border_y - u_len_line;
  y_max = u_border_y;

  for (uint y = y_max; y > y_min; y--)
  {
    if (y > y_min)
    {
      x_min -= -1 * (left_u_tri_skew)  ;
      x_max -= (right_u_tri_skew)  ;

      if (((rand() % dstrt) == 1))
      {
        do
        {
          x_min = x_min + (rand() % 3 - 1);

          if (x_min < BORDER_X)
          {
            x_min = BORDER_X + 2;
          }
          else if (x_min > (mapWidth - BORDER_X))
          {
            x_min = mapWidth - BORDER_X - 2;
          }
        }
        while (x_min < BORDER_X || x_min > (mapWidth - BORDER_X));

        do
        {
          x_max = x_max + (rand() % 3 - 1);

          if (x_max < BORDER_X)
          {
            x_max = BORDER_X + 2;
          }
          else if (x_max > (mapWidth - BORDER_X))
          {
            x_max = mapWidth - BORDER_X - 2;
          }
        }
        while (x_max < BORDER_X || x_max > (mapWidth - BORDER_X));
      }
    }

    for (uint x = 0; x < mapWidth; x++)
    {
      if (x >= x_min && x <= x_max && y >= y_min && y <= y_max)
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel); //W creates Water, L1 creates Land

        if (rand() % dstrt == 1) //1 in (distortion_val) chance of a line length change
        {
          x_min = x_min + (rand() % 3 - 1);
          x_max = x_max + (rand() % 3 - 1);
        }
      }
      else
      {
        if (!mapIs(generatorMap,mapWidth,mapHeight,x,y,not_pixel))
        {
          mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel);
        }
      }
    }
  }

  // this block adds a triangular shape to the lower border

  left_l_tri_skew = 1;
  right_l_tri_skew = 1;

  x_min = l_border_x ;
  x_max = l_border_x + l_len_line;
  y_min = l_border_y;
  y_max = l_border_y + l_len_line;

  for (uint y = y_min; y < y_max; y++)
  {
    if (y < y_max)
    {
      x_min -= - (left_l_tri_skew)   ;
      x_max -= (right_l_tri_skew)  ;

      if (((rand() % (rigidity + 1)) == 0))
      {
        do
        {
          x_min = x_min + (skewed_neg_pos_gen(skew3));

          if (x_min < BORDER_X)
          {
            x_min = BORDER_X + 2;
          }
          else if (x_min > (mapWidth - BORDER_X))
          {
            x_min = mapWidth - BORDER_X - 2;
          }
        }
        while (x_min < BORDER_X || x_min > (mapWidth - BORDER_X));

        do
        {
          x_max = x_max + (skewed_neg_pos_gen(skew4));

          if (x_max < BORDER_X)
          {
            x_max = BORDER_X + 2;
          }
          else if (x_max > (mapWidth - BORDER_X))
          {
            x_max = mapWidth - BORDER_X - 2;
          }
        }
        while (x_max < BORDER_X || x_max > (mapWidth - BORDER_X));
      }
    }

    for (uint x = 0; x < mapWidth; x++)
    {
      if (x >= x_min && x <= x_max && y >= y_min && y <= y_max)
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1); //W creates Water, L1 creates Land

        if (rand() % dstrt == 1) //1 in (distortion_val) chance of a line length change
        {
          x_min = x_min + (rand() % 3 - 1);
          x_max = x_max + (rand() % 3 - 1);
        }
      }
      else
      {
        if (mapIs(generatorMap,mapWidth,mapHeight,x,y,not_pixel))
        {
          mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel);
        }
      }
    }

    //THIS BLOCK HERE prevents stack smashing (an overflow)
    if (x_max > (mapWidth - BORDER_X))
    {
      break;
    }

    if (x_min > (mapWidth - BORDER_X))
    {
      break;
    }

    if (x_max < (BORDER_X))
    {
      break;
    }

    if (x_min < (BORDER_X))
    {
      break;
    }

    //The bottoms of the continents have a propensity to overflow
    if (y_max > (mapHeight - BORDER_Y))
    {
      //This block adds bumpiness to the bottom of the rectangle in the case of an overflow
      for (uint x = l_border_x; x <= l_border_x + l_len_line; x++)
      {
        y_len += (neg_pos_gen());

        if (y_len < 1)
        {
          y_len = 2;
        }
        else if (y_len + l_border_y > (mapHeight - BORDER_Y - 30))
        {
          y_len = 1 + neg_pos_gen();
        }

        for (uint y = l_border_y; y < (l_border_y + y_len); y++)
        {
          mapSetType(generatorMap,mapWidth,mapHeight,x,y,pixel);
        }
      }

      break;
    }

    if (y_min > (mapHeight - BORDER_Y))
    {
      break;
    }

    if (y_max < (BORDER_Y))
    {
      break;
    }

    if (y_min < (BORDER_Y))
    {
      break;
    }
  }
}

LOCAL void gen_circle(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight, int water_or_land, int max_size, int distortion, int hard_code_distortion, int render_direction)
{
  // variables for circle
  uint x_min;
  uint x_max;
  uint y_min;
  uint y_max;

  uint c_center_y;
  uint c_center_x;
  uint c_sqrd;

  uint c_size;

  uint c_increase;
  uint c_x_pos;
  uint c_y_pos;

  GeneratorTileTypes terrain_type;// sets the terrain

  if (water_or_land == 1)
  {
    terrain_type = L1;
  }
  else if (water_or_land == 0)
  {
    terrain_type = W1;
  }

  // this block ADDS a left to right circle to somewhere on the map
  c_size = (rand() % max_size);//300ish is good for continents
  // c_size/2 = radius


  x_min = ((rand() % (mapWidth - (2 * BORDER_X) - c_size - 100)) + BORDER_X) + 50 ;   // there is a buffer of 50 on both sides
  x_max = x_min + c_size;
  y_min = ((rand() % (mapHeight - (2 * BORDER_Y) - c_size - 100)) + BORDER_Y) + 50;
  y_max = y_min + c_size;

  if (hard_code_distortion == 0)
  {
    distortion = (rand() % (((distortion / 5) * 4) + 1)) + (distortion / 5); // random value in distortion
  }

  c_x_pos = x_min;
  c_y_pos = y_min;

  // calculates the point in the middle of circle
  c_center_y = y_min + (y_max - y_min) / 2 ;
  c_center_x = x_min + (x_max - x_min) / 2 ;

  c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

  for (uint y = 0; y < mapHeight; y++)
  {
    for (uint x = 0; x < mapWidth; x++)
    {
      // a simple pythagorean formula is used to calculate bounds of circle
      if (x > x_min && x < x_max && y > y_min && y < y_max)
      {
        if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
        {
          mapSetType(generatorMap,mapWidth,mapHeight,x,y,terrain_type);

          if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
          {
            // if(x_min < (mapWidth - BORDER_X - c_size) && x_min > BORDER_X && x_max < (mapWidth - BORDER_X) && x_max > (BORDER_X + 1) ){
            c_increase = (rand() % 3 - 1);
            c_center_x = c_center_x + c_increase;
            x_min += (rand() % 3 - 1);
            x_max +=  c_increase;

            //}
            // else{
            if (x_min < BORDER_X || x_max < (BORDER_X + 1))
            {
              c_center_x++;
              x_min++;
              x_max++;
            }
            else if (x_min > (mapWidth - BORDER_X - c_size) || x_max > (mapWidth - BORDER_X))
            {
              c_center_x--;
              x_min--;
              x_max--;
            }

            //}
          }
        }
        else
        {
          if (!mapIs(generatorMap,mapWidth,mapHeight,x,y,L1))
          {
            mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);
          }
        }
      }
    }
  }

  // this block ADDS an up to down circle ontop of the previous circle, which "bumpifies" the rounded edges
  // c_size/2 is radius

  if (render_direction == 2)
  {
    x_min = c_x_pos;
    x_max = x_min + c_size;
    y_min = c_y_pos;
    y_max = y_min + c_size;

    if (hard_code_distortion == 0)
    {
      distortion = (rand() % (((distortion / 5) * 4) + 1)) + (distortion / 5);
    }

    // calculates the point in the middle of circle
    c_center_y = y_min + (y_max - y_min) / 2 ;
    c_center_x = x_min + (x_max - x_min) / 2 ;

    c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

    for (uint x = 0; x < mapWidth; x++)
    {
      for (uint y = 0; y < mapHeight; y++)
      {
        // a simple pythagorean formula is used to calculate bounds of circle
        if (x > x_min && x < x_max && y > y_min && y < y_max)
        {
          if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
          {
            mapSetType(generatorMap,mapWidth,mapHeight,x,y,terrain_type);

            if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
            {
              // if(y_min < (mapHeight - BORDER_Y - c_size) && y_min > BORDER_Y && y_max < (mapHeight - BORDER_Y) && y_max > (BORDER_Y + 1)){
              c_increase = (rand() % 3 - 1);
              c_center_y = c_center_y + c_increase;
              y_min += (rand() % 3 - 1);
              y_max +=  c_increase;
              //}

              // else{
              if (y_min < BORDER_Y || y_max < (BORDER_Y + 1))
              {
                c_center_y++;
                y_min++;
                y_max++;
              }
              else if (y_min > (mapHeight - BORDER_Y - c_size) || y_max > (mapHeight - BORDER_Y))
              {
                c_center_y--;
                y_min--;
                y_max--;
              }

              //}
            }
          }
          else
          {
            if (!mapIs(generatorMap,mapWidth,mapHeight,x,y,L1))
            {
              mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);
            }
          }
        }
      }
    }
  }
}

LOCAL void gen_ocean_split(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight)
{
  //This function adds a max 150 pixel wide ocean that spits the pieces of land, it runs from BORDER_Y to mapHeight
  //These do whiles set up the bounds of the ocean split

  uint ocean_angle = (rand() % 7) - 3; // angle between -3 and 3

  uint x_min;
  uint x_max;
  uint y_min;
  uint y_max;

  y_max = mapHeight - BORDER_Y;
  y_min = BORDER_Y;

  x_min = (rand() % (mapWidth - (2 * BORDER_X) - 300)) + (BORDER_X + 100) ; // random location with a buffer of 100 on left, and 200 on right

  x_max = x_min + (rand() % 100) + 50;
  // random location between borders with a min of 50 and max of 150

  for (uint y = 0; y < mapHeight; y++)
  {
    for (uint x = 0; x < mapWidth; x++)
    {
      if (x >= x_min && x <= x_max && y >= y_min && y <= y_max)
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1); //W creates Water, L1 creates Land

        if (rand() % 5 == 1) //1 in 5 chance of a line length change
        {
          x_min = x_min + (rand() % 3 - 1);
          x_max = x_max + (rand() % 3 - 1);
        }

        if (x_max > (mapWidth - BORDER_X))
        {
          x_max--;
          x_min--;
          ocean_angle *= -1;
        }

        if (x_min < (BORDER_X))
        {
          x_max++;
          x_min++;
          ocean_angle *= -1;
        }
      }
    }

    x_min = x_min + ocean_angle;
    x_max = x_max + ocean_angle;

    if (rand() % 10 == 1)
    {
      ocean_angle = (rand() % 7) - 3;  //1 in 20 chance of a whole line angle change // angle between -3 and 3
    }
  }
}

LOCAL void gen_ocean_errosion(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight)
{
  //This function adds chunks of ocean flowing into the mainlands from left to right
  // it uses a modified version of the gen_circle algo, slightly optimized for faster runtime
  uint x_min;
  uint x_max;
  uint y_min;
  uint y_max;

  uint distortion;

  uint c_center_y;
  uint c_center_x;
  uint c_sqrd;

  uint c_size;

  uint c_increase;
  uint c_x_pos;
  uint c_y_pos;

  for (uint y = 1; y < (mapHeight - BORDER_Y - 10) ; y++)
  {
    for (uint x = 1; x < (mapWidth - BORDER_X - 10) ; x++)
    {
      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,L1) && mapIs(generatorMap,mapWidth,mapHeight,x-1,y,W1))
      {
        if (rand() % 30 == 1) //1 in 20 chance of a circle of water spawned into the border
        {
          // this inner block REMOVES a left to right circle to the border on the map
          c_size = (rand() % 15) + 5; // size between 5 and 10

          // c_size/2 = radius
          c_x_pos = x - c_size / 2;
          c_y_pos = y - c_size / 2;

          x_min = c_x_pos;
          x_max = x_min + c_size;
          y_min = c_y_pos;
          y_max = y_min + c_size;

          distortion = 3;

          // calculates the point in the middle of circle
          c_center_y = y_min + (y_max - y_min) / 2 ;
          c_center_x = x_min + (x_max - x_min) / 2 ;

          c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

          for (uint y = y_min; y < y_max; y++)
          {
            for (uint x = 0; x < (mapWidth - BORDER_X - 10); x++)
            {
              // a simple pythagorean formula is used to calculate bounds of circle
              if (x > x_min && x < x_max && y > y_min && y < y_max)
              {
                if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
                {
                  mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);

                  if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
                  {
                    c_increase = (rand() % 3 - 1);
                    c_center_x = c_center_x + c_increase;
                    x_min += (rand() % 3 - 1);
                    x_max +=  c_increase;

                    if (x_min < BORDER_X || x_max < (BORDER_X + 1))
                    {
                      c_center_x++;
                      x_min++;
                      x_max++;
                    }
                    else if (x_min > (mapWidth - BORDER_X - c_size) || x_max > (mapWidth - BORDER_X))
                    {
                      c_center_x--;
                      x_min--;
                      x_max--;
                    }
                  }
                }
              }
            }
          }

          // this block ADDS an up to down circle ontop of the previous circle, which "bumpifies" the rounded edges
          // c_size/2 is radius
          x_min = c_x_pos;
          x_max = x_min + c_size;
          y_min = c_y_pos;
          y_max = y_min + c_size;

          distortion = 3;

          // calculates the point in the middle of circle
          c_center_y = y_min + (y_max - y_min) / 2 ;
          c_center_x = x_min + (x_max - x_min) / 2 ;

          c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

          for (uint x = x_min; x < x_max; x++)
          {
            for (uint y = 0; y < (mapHeight - BORDER_Y - 10); y++)
            {
              // a simple pythagorean formula is used to calculate bounds of circle
              if (x > x_min && x < x_max && y > y_min && y < y_max)
              {
                if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
                {
                  mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);

                  if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
                  {
                    c_increase = (rand() % 3 - 1);
                    c_center_y = c_center_y + c_increase;
                    y_min += (rand() % 3 - 1);
                    y_max +=  c_increase;

                    if (y_min < BORDER_Y || y_max < (BORDER_Y + 1))
                    {
                      c_center_y++;
                      y_min++;
                      y_max++;
                    }
                    else if (y_min > (mapHeight - BORDER_Y - c_size) || y_max > (mapHeight - BORDER_Y))
                    {
                      c_center_y--;
                      y_min--;
                      y_max--;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //This block adds chunks of ocean flowing into the mainlands from RIGHT to Left
  for (uint y = 1; y < (mapHeight - BORDER_Y - 10) ; y++)
  {
    for (uint x = (mapWidth - BORDER_X - 11); x > 0 ; x--)
    {
      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,L1) && mapIs(generatorMap,mapWidth,mapHeight,x-1,y,W1))
      {
        if (rand() % 30 == 1) //1 in 20 chance of a circle of water spawned into the border
        {

          // this inner block REMOVES a left to right circle to the border on the map

          c_size = (rand() % 15) + 5; // size between 5 and 10

          // c_size/2 = radius
          c_x_pos = x - c_size / 2;
          c_y_pos = y - c_size / 2;

          x_min = c_x_pos;
          x_max = x_min + c_size;
          y_min = c_y_pos;
          y_max = y_min + c_size;

          distortion = 3;  // 50% chance of a line change

          // calculates the point in the middle of circle
          c_center_y = y_min + (y_max - y_min) / 2 ;
          c_center_x = x_min + (x_max - x_min) / 2 ;

          c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

          for (uint y = y_min; y < y_max; y++)
          {
            for (uint x = 0; x < (mapWidth - BORDER_X - 10); x++)
            {
              // a simple pythagorean formula is used to calculate bounds of circle
              if (x > x_min && x < x_max && y > y_min && y < y_max)
              {
                if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
                {
                  mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);

                  if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
                  {
                    c_increase = (rand() % 3 - 1);
                    c_center_x = c_center_x + c_increase;
                    x_min += (rand() % 3 - 1);
                    x_max +=  c_increase;

                    if (x_min < BORDER_X || x_max < (BORDER_X + 1))
                    {
                      c_center_x++;
                      x_min++;
                      x_max++;
                    }
                    else if (x_min > (mapWidth - BORDER_X - c_size) || x_max > (mapWidth - BORDER_X))
                    {
                      c_center_x--;
                      x_min--;
                      x_max--;
                    }
                  }
                }
              }
            }
          }

          // this block ADDS an up to down circle ontop of the previous circle, which "bumpifies" the rounded edges
          // c_size/2 is radius
          x_min = c_x_pos;
          x_max = x_min + c_size;
          y_min = c_y_pos;
          y_max = y_min + c_size;

          distortion = 3;

          // calculates the point in the middle of circle
          c_center_y = y_min + (y_max - y_min) / 2 ;
          c_center_x = x_min + (x_max - x_min) / 2 ;

          c_sqrd = (c_size / 2) * (c_size / 2); // radius squared

          for (uint x = x_min; x < x_max; x++)
          {
            for (uint y = 0; y < (mapHeight - BORDER_Y - 10); y++)
            {
              // a simple pythagorean formula is used to calculate bounds of circle
              if (x > x_min && x < x_max && y > y_min && y < y_max)
              {
                if ((x - c_center_x) * (x - c_center_x) + (y - c_center_y) * (y - c_center_y) <= c_sqrd)
                {
                  mapSetType(generatorMap,mapWidth,mapHeight,x,y,W1);

                  if (rand() % distortion == 1) //1 in (bumpiness_value) chance of a line length change
                  {
                    c_increase = (rand() % 3 - 1);
                    c_center_y = c_center_y + c_increase;
                    y_min += (rand() % 3 - 1);
                    y_max +=  c_increase;

                    if (y_min < BORDER_Y || y_max < (BORDER_Y + 1))
                    {
                      c_center_y++;
                      y_min++;
                      y_max++;
                    }
                    else if (y_min > (mapHeight - BORDER_Y - c_size) || y_max > (mapHeight - BORDER_Y))
                    {
                      c_center_y--;
                      y_min--;
                      y_max--;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

LOCAL void gen_rivers(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight)
{
  //Here we spawn in rivers

  //GEOGRAPHICAL BACKGROUND: Rivers should always flow from higher elevation to lower elevation

  //"river units" are about 2 to 3 pixels in length, with the size of 2 to 3 varying for aesthetic realism
  // use of W2 below to plump up the river while avoiding mapHeightlision detection - i.e. river route is 1 pixel in length
  //I tried having the a "river unit" at 1 pixel length and it looked choppy and rigid

  uint r_len;// river length in river units
  uint r_x, r_y;// river x and river y
  uint r_x0, r_y0;// river x naught and y naught (the river starting postions)

  uint x_dir, y_dir; // x and y river direction -- i.e. skew values
  // skew between 3 and 9 inclusive, any greater or smaller will create a visually awkward intense skew (the word awkward is spelt so awkwardly)

  //NOTE: our land search iterator still operates pixel by pixel as opposed to through river units
  for (uint y = 1; y < (mapHeight - BORDER_Y - 10)/* -10 is a buffer */; y++)
  {
    for (uint x = 1; x < (mapWidth - BORDER_X - 10); x++)
    {
      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,L1) && mapIs(generatorMap,mapWidth,mapHeight,x-1,y,W1))
      {
        if ((rand() % 8) == 1) // 1 in 10 chance of spawning in river
        {
          // here is the river line algorithm
          r_x0 = r_x = x;
          r_y0 = r_y = y;
          r_len = ((rand() % 64) + 65); // length between 1 and 128

          x_dir = (rand() % 10) + 1;
          y_dir = (rand() % 10) + 1;

          for (uint p = 0; p < r_len; p++) // iterates and draws a line as series of points
          {
            if (mapIs(generatorMap,mapWidth,mapHeight,r_x,r_y,W1))
            {
              break;
            }
            mapSetType(generatorMap,mapWidth,mapHeight,r_x,r_y,W1);// map[(r_y * mapHeight) + r_x] = W1;
            mapSetType(generatorMap,mapWidth,mapHeight,r_x + ((rand() % 3) - 1),r_y + ((rand() % 3) - 1),W2);// map[((r_y + ((rand() % 3) - 1)) * mapHeight) + (r_x + ((rand() % 3) - 1)) ] = W2;

            // map[r_x + ((rand() % 5) - 2 )][r_y + ((rand() % 5) - 2)] = W2;

            do
            {
              if ((rand() % 3) != 1)
              {
                r_x += skewed_neg_pos_gen(x_dir); // there is 33% of a 0 instead of a skewed -1 or 1
              }

              if ((rand() % 3) != 1)
              {
                r_y += skewed_neg_pos_gen(y_dir);
              }
            }
            while (r_x == r_x0 && r_y == r_y0);

            r_x0 = r_x;
            r_y0 = r_y;
          }
        }
      }
    }
  }

  // same algo as before -- but the spawning of the river is from right to left

  for (uint y = 1; y < (mapHeight - BORDER_Y - 10)/* -10 is a buffer */; y++)
  {
    for (uint x = (mapWidth - BORDER_X - 10); x > 0; x--)
    {
      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,L1) && mapIs(generatorMap,mapWidth,mapHeight,x+1,y,W1))
      {
        if ((rand() % 8) == 1) // 1 in 10 chance of spawning in river
        {
          // here is the river line algorithm
          r_x0 = r_x = x;
          r_y0 = r_y = y;
          r_len = ((rand() % 64) + 65); // length between 64 and 128

          x_dir = (rand() % 10) + 1; // should be a value between 1 and 11; it's a parameter in the skewed generator
          y_dir = (rand() % 10) + 1;

          for (uint p = 0; p < r_len; p++) // iterates and draws a line as series of points
          {
            if (mapIs(generatorMap,mapWidth,mapHeight,r_x,r_y,W1))
            {
              break;
            }
            mapSetType(generatorMap,mapWidth,mapHeight,r_x,r_y,W1);//(r_y * mapHeight) + r_x] = W1;
            mapSetType(generatorMap,mapWidth,mapHeight,r_x + ((rand() % 3) - 1),r_y + ((rand() % 3) - 1),W2);// map[((r_y + ((rand() % 3) - 1)) * mapHeight) + (r_x + ((rand() % 3) - 1)) ] = W2;     // see the river units note at top
            // map[r_x + ((rand() % 5) - 2 )][r_y + ((rand() % 5) - 2)] = W2;

            do
            {
              if ((rand() % 3) != 1)
              {
                r_x += skewed_neg_pos_gen(x_dir); // there is 33% of a 0 instead of a skewed -1 or 1
              }

              if ((rand() % 3) != 1)
              {
                r_y += skewed_neg_pos_gen(y_dir);
              }
            }
            while (r_x == r_x0 && r_y == r_y0);

            r_x0 = r_x;
            r_y0 = r_y;
          }
        }
      }
    }
  }
}

LOCAL int gen_biomes(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight)
{
  // this function adds in a rough estimate of climated base on latitude

  //Here we add base layer of land -- in this case P = Polar/Snow or Ice, ontop of the light blue render acting as an outline
  for (uint y = 0; y < mapHeight; y++)
  {
    for (uint x = 0; x < mapWidth; x++)
    {
      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,L1))
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,L1);

        if (mapIs(generatorMap,mapWidth,mapHeight,x,y-1,L1) || mapIs(generatorMap,mapWidth,mapHeight,x,y-1,P)) // check above
        {
          if (mapIs(generatorMap,mapWidth,mapHeight,x+1,y,L1) || mapIs(generatorMap,mapWidth,mapHeight,x+1,y,P)) // check right
          {
            if (mapIs(generatorMap,mapWidth,mapHeight,x,y-1,L1) || mapIs(generatorMap,mapWidth,mapHeight,x,y-1,P)) // check below
            {
              if (mapIs(generatorMap,mapWidth,mapHeight,x-1,y,L1) || mapIs(generatorMap,mapWidth,mapHeight,x-1,y,P)) // check left
              {
                mapSetType(generatorMap,mapWidth,mapHeight,x,y,P);
              }
            }
          }
        }
      }
    }
  }

  uint biome_val = 0;
//  int add_val = (mapHeight / 2) / 500;
  uint equator = (mapHeight / 2);

  uint south_ice_cap = 0; // default is 0
  uint north_ice_cap = 0; // default is 0

  for (uint x = 1; x < mapWidth; x++)
  {
    if ((rand() % 7) == 0)
    {
      south_ice_cap += (rand() % 3) - 1;
    }

    if ((rand() % 7) == 0)
    {
      north_ice_cap += (rand() % 3) - 1;
    }

    if ((rand() % 7) == 0)
    {
      equator += (rand() % 3) - 1;
    }

    biome_val = 0;

    for (uint y = (75 + north_ice_cap); y < mapHeight - (75 + south_ice_cap); y++)
    {
      if (y < equator)
      {
        if ((y % 3) == 0 || (y % 3) == 1)
        {
          if (biome_val < 498)
          {
            biome_val += 3;
          }
        }
      }
      else if (y > equator)
      {
        if ((y % 3) == 0 || (y % 3) == 1)
        {
          if (biome_val > 0)
          {
            biome_val -= 3;
          }
        }
      }

      if (mapIs(generatorMap,mapWidth,mapHeight,x,y,P))
      {
        mapSetType(generatorMap,mapWidth,mapHeight,x,y,biome_val);
      }
    }
  }

  return 0;
}

void debugDumpMemory(const void *address, uint length)
{
  typedef int8_t byte;

  const byte *p;
  uint       z,j;

  assert(address != NULL);

  z = 0;
  while (z < length)
  {
    p = (const byte*)address+z;
    fprintf(stderr,"%08lx  ",(unsigned long)(p-(byte*)address));

    for (j = 0; j < 16; j++)
    {
      if ((z+j) < length)
      {
        p = (const byte*)address+z+j;
        fprintf(stderr,"%02x ",((uint)(*p)) & 0xFF);
      }
      else
      {
        fprintf(stderr,"   ");
      }
    }
    fprintf(stderr,"  ");

    for (j = 0; j < 16; j++)
    {
      if ((z+j) < length)
      {
        p = (const byte*)address+z+j;
        fprintf(stderr,"%c",isprint((int)(*p))?(*p):'.');
      }
      else
      {
      }
    }
    fprintf(stderr,"\n");

    z += 16;
  }
}

LOCAL void blended_colors(GeneratorTile *generatorMap, uint mapWidth, uint mapHeight)
{
  // blends all the colours of the biomes together

  typedef struct
  {
    float r;
    float g;
    float b;
    float biome_val;
  } BlendedColor;

  //    BlendedColor dark_blue;
  //    dark_blue.r = 0.0;
  //    dark_blue.g = 0.0;
  //    dark_blue.b = 130.0;
  //    dark_blue.biome_val = (float)W; // also w2 but this is the bottom/default layer

//  BlendedColor light_blue;
//  light_blue.r = 40.0;
//  light_blue.g = 200.0;
//  light_blue.b = 220.0;
//  light_blue.biome_val = (float)L1;

  BlendedColor dark_green;
  dark_green.r = 0.0;
  dark_green.g = 100.0 - 40.0;
  dark_green.b = 0.0;
  dark_green.biome_val = (float)J;

  BlendedColor light_olive_green;
  light_olive_green.r = 80.0 - 10.0;
  light_olive_green.g = 140.0 - 40.0;
  light_olive_green.b = 80.0 - 10.0;
  light_olive_green.biome_val = (float)G;

  BlendedColor sand;
  sand.r = 222.0 - 30.0;
  sand.g = 206.0 - 40.0;
  sand.b = 126.0 - 40.0;
  sand.biome_val = (float)D;

  BlendedColor grass_green;
  grass_green.r = 80.0 - 40.0;
  grass_green.g = 160.0 - 40.0;
  grass_green.b = 80.0 - 40.0;
  grass_green.biome_val = (float)F;

  BlendedColor tundra_green;
  tundra_green.r = 130.0 - 40.0;
  tundra_green.g = 190.0 - 40.0;
  tundra_green.b = 130.0 - 40.0;
  tundra_green.biome_val = (float)T;

  BlendedColor white;
  white.r = 255.0;
  white.g = 255.0;
  white.b = 255.0;
  white.biome_val = (float)P;

  BlendedColor latitude_colors[6] = {white, tundra_green, grass_green, sand, light_olive_green, dark_green,};

  float r,g,b;

  float d_r,d_g,d_b; // d for delta as in: change in

  float current_biome_val;
  float remainder_biome_val;

  for (uint x = 0; x < mapWidth; x++)
  {
    for (uint y = 0; y < mapHeight; y++)
    {
      current_biome_val   = (float)mapGet(generatorMap,mapWidth,mapHeight,x,y).type; // the biome value is stored in the current pixel mapped to an int
      remainder_biome_val = (float)current_biome_val;

      if (current_biome_val == latitude_colors[0].biome_val)
      {
        r = latitude_colors[0].r;
        g = latitude_colors[0].g;
        b = latitude_colors[0].b;

      }

      for (int i = 0; i < 5; i++)
      {
        if (current_biome_val > latitude_colors[i].biome_val)
        {
          if (current_biome_val < latitude_colors[i + 1].biome_val)
          {
            remainder_biome_val =  current_biome_val - latitude_colors[i].biome_val;

            // red
            d_r = ((latitude_colors[i + 1].biome_val - latitude_colors[i].biome_val) / (remainder_biome_val));
            r = latitude_colors[i].r - ((latitude_colors[i].r - latitude_colors[i + 1].r) / d_r);

            // green
            d_g = ((latitude_colors[i + 1].biome_val - latitude_colors[i].biome_val) / (remainder_biome_val));
            g = latitude_colors[i].g - ((latitude_colors[i].g - latitude_colors[i + 1].g) / d_g);

            // blue
            d_b = ((latitude_colors[i + 1].biome_val - latitude_colors[i].biome_val)  / (remainder_biome_val));
            b = latitude_colors[i].b - ((latitude_colors[i].b - latitude_colors[i + 1].b) / d_b);
          }
          else
          {
            r = latitude_colors[i + 1].r;
            g = latitude_colors[i + 1].g;
            b = latitude_colors[i + 1].b;
          }
        }
      }

      if (   !mapIs(generatorMap,mapWidth,mapHeight,x,y,L1)
          && !mapIs(generatorMap,mapWidth,mapHeight,x,y,W1)
          && !mapIs(generatorMap,mapWidth,mapHeight,x,y,W2)
         )
      {
        mapSetColor(generatorMap,mapWidth,mapHeight,x,y,
              (uint8_t)r, (uint8_t)g, (uint8_t)b
              );
      }
    }
  }
}

void MapGenerator::generate(Map &map, uint minContinents, uint maxContinents)
{
  GeneratorTile *generatorMap;

  generatorMap = (GeneratorTile*)malloc(map.getWidth()*map.getHeight()*sizeof(GeneratorTile));
  assert(generatorMap != NULL);

  // sets the background of map to default water
  for (uint y = 0; y < map.getHeight(); y++)
  {
    for (uint x = 0; x < map.getWidth(); x++)
    {
      mapSetType(generatorMap,map.getWidth(),map.getHeight(),x,y,W1);
    }
  }

  uint continents = minContinents + (rand() % (maxContinents - minContinents));

  // 1. creates the main land masses
  for (uint i = 0; i < continents; i++)
  {
    gen_stretched_hexagon(generatorMap, map.getWidth(), map.getHeight(), 0, 80);
    gen_circle(generatorMap, map.getWidth(), map.getHeight(), 1, 300, 100, 0, 2);
    gen_circle(generatorMap, map.getWidth(), map.getHeight(), 0, 150, 90, 1, 2);
  }

  // 2. add geographic realism to the land masses
  gen_ocean_split(generatorMap, map.getWidth(), map.getHeight());
  gen_ocean_errosion(generatorMap, map.getWidth(), map.getHeight());
  gen_ocean_errosion(generatorMap, map.getWidth(), map.getHeight());
  gen_rivers(generatorMap, map.getWidth(), map.getHeight());
  gen_rivers(generatorMap, map.getWidth(), map.getHeight());

  // 3. generate bio masses colors
  gen_biomes(generatorMap, map.getWidth(), map.getHeight());
  blended_colors(generatorMap, map.getWidth(), map.getHeight());

  // fill-in map tiles
  map.reset();
  for (uint y = 0; y < map.getHeight(); y++)
  {
    for (uint x = 0; x < map.getWidth(); x++)
    {
      GeneratorTile tile = mapGet(generatorMap,map.getWidth(),map.getHeight(),x,y);

      switch (tile.type)
      {
        case W1:
          map.setTile(x, y, Tile::Types::WATER, Color::interpolate(Color::WATER1, Color::WATER2, (double)rand() / RAND_MAX));
          break;
        case W2:
          map.setTile(x, y, Tile::Types::WATER, Color::WATER2);
          break;
        case L1:
          map.setTile(x, y, Tile::Types::LAND, Color::LAND1);
          break;
        case L2:
          map.setTile(x, y, Tile::Types::LAND, Color::LAND2);
          break;
          break;
        default:
          map.setTile(x, y, Tile::Types::LAND, tile.color);
          break;
      }
    }
  }

  free(generatorMap);
}

/* end of file */
