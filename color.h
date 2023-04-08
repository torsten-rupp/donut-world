/***********************************************************************\
*
* Contents: colors
* Systems: all
*
\***********************************************************************/
#ifndef COLOR_H
#define COLOR_H

/****************************** Includes *******************************/
#include <stdint.h>

/****************** Conditional compilation switches *******************/

/***************************** Constants *******************************/

/***************************** Datatypes *******************************/

/***************************** Variables *******************************/

/****************************** Macros *********************************/

/***************************** Forwards ********************************/

/***************************** Functions *******************************/

/** RGB color
 */
class Color
{
  public:
    static const Color WATER1;
    static const Color WATER2;
    static const Color LAND1;
    static const Color LAND2;
    static const Color TREE;
    static const Color MOUNTAIN;
    static const Color BUILDING;

    uint8_t r, g, b;

    bool operator!=(const struct Color &otherColor) const
    {
      return    (r != otherColor.r)
             || (g != otherColor.g)
             || (b != otherColor.b);
    }

    static Color interpolate(const Color &color1, const Color &color2, double factor)
    {
      Color color;

      color.r = color1.r + static_cast<uint8_t>((static_cast<double>(color2.r) - static_cast<double>(color1.r)) * factor);
      color.g = color1.g + static_cast<uint8_t>((static_cast<double>(color2.g) - static_cast<double>(color1.g)) * factor);
      color.b = color1.b + static_cast<uint8_t>((static_cast<double>(color2.b) - static_cast<double>(color1.b)) * factor);

      return color;
    }
};

#endif // COLOR_H

/* end of file */
