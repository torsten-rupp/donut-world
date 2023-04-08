/***********************************************************************\
*
* Contents: map generator
* Systems: all
*
\***********************************************************************/

#ifndef MAP_GENERATOR
#define MAP_GENERATOR

/****************************** Includes *******************************/
#include <stdlib.h>
#include <stdint.h>

#include "map.h"


/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/

/***************************** Variables *******************************/

/****************************** Macros *********************************/

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

class MapGenerator
{
  public:
    static void generate(Map &map, uint minContinents, uint maxContinents);

  private:
};

#endif // MAP_GENERATOR
