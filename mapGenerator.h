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

#include "islands.h"

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
    /** generate map
     * @param map map
     * @param minContinents min. number of continents
     * @param maxContinents max. number of continents
     */
    static void generate(Map &map, uint minContinents, uint maxContinents);

  private:
};

#endif // MAP_GENERATOR
