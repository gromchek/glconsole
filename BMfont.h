#ifndef BMFONT_H
#define BMFONT_H

#include "Misc.h"
#include <string>
#include <unordered_map>

struct BMchar
{
    float x, y;
    float width;
    float height;
    float xoffset;
    float yoffset;
    float xadvance;
    unsigned int page;

    FloatRect texCoord;
    vec2f offset;
    vec2f charSize;
};

struct BMfont
{
    float lineHeight;
    float base;
    float scaleW;
    float scaleH;

    std::string textureFile;

    std::unordered_map<unsigned int, BMchar> chars;
};

BMfont LoadBitmapFont( const std::string &fileName );

#endif // BMFONT_H
