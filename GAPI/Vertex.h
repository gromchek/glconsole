#ifndef VERTEX_H
#define VERTEX_H

#include "../Misc.h"

struct Color final
{
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    Color() : r( 255 ), g( 255 ), b( 255 ), a( 255 )
    {
    }

    Color( std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t aplha = 255 ) :
        r( red ), g( green ), b( blue ), a( aplha )
    {
    }

    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Black;
    static const Color Gold;
    static const Color LightGold;
    static const Color Yellow;
};

inline const Color Color::White = { 255, 255, 255 };
inline const Color Color::Red = { 255, 63, 63 };
inline const Color Color::Green = { 30, 255, 0 };
inline const Color Color::Blue = { 0, 112, 221 };
inline const Color Color::Black = { 0, 0, 0 };
inline const Color Color::Gold = { 255, 215, 0 };
inline const Color Color::LightGold = { 230, 204, 128 };
inline const Color Color::Yellow = { 255, 255, 0 };

struct Vertex2d
{
    Vertex2d() : position( 0.0f ), texcoord( 0.0f ), color()
    {
    }
    vec2f position;
    vec2f texcoord;
    Color color;
};

namespace Vertex2dAttrib
{
const unsigned int POSITION_LOCATION = 0;
const unsigned int TEX_COORD_LOCATION = 1;
const unsigned int COLOR_LOCATION = 2;

const unsigned int OFFSET_POSITION = offsetof( Vertex2d, position );
const unsigned int OFFSET_TEX_COORD = offsetof( Vertex2d, texcoord );
const unsigned int OFFSET_COLOR = offsetof( Vertex2d, color );
} // namespace Vertex2dAttrib

#endif // VERTEX_H
