#include "BMfont.h"
#include <fstream>
#include <sstream>

#include "thrid_party/picojson.h"

template <typename T>
T JsonConverter( const picojson::value &val, const std::string &key )
{
    static_assert( std::is_same<T, void>::value, "Converter for this type is not specialized" );
    return T{};
}

template <>
inline float JsonConverter<float>( const picojson::value &val, const std::string &key )
{
    const auto &v = val.get( key );

    if( v.is<picojson::null>() || !v.is<double>() )
    {
        return {};
    }

    return float( v.get<double>() );
}

template <>
inline unsigned int JsonConverter<unsigned int>( const picojson::value &val, const std::string &key )
{
    const auto &v = val.get( key );

    if( v.is<picojson::null>() || !v.is<double>() )
    {
        return {};
    }

    return static_cast<unsigned int>( v.get<double>() );
}

template <>
inline std::string JsonConverter<std::string>( const picojson::value &val, const std::string &key )
{
    const auto &v = val.get( key );

    if( v.is<picojson::null>() || !v.is<std::string>() )
    {
        return {};
    }

    return v.get<std::string>();
}


BMfont LoadBitmapFont( const std::string &fileName )
{
    std::ifstream file( fileName, std::ios::binary );

    std::stringstream ss;

    ss << file.rdbuf();

    picojson::value v;
    ss >> v;

    std::string err = picojson::get_last_error();
    if( !err.empty() )
    {
        std::cerr << err << std::endl;
        return {};
    }

    if( !v.is<picojson::object>() )
    {
        std::cerr << "Error read " << fileName << " file: corrupt file\n";
        return {};
    }

    const auto &fntObject = v.get<picojson::object>();

    const auto &common = fntObject.find( "common" );

    if( common == fntObject.end() )
    {
        std::cerr << "Error read " << fileName << " file: bad info\n";
        return {};
    }

    BMfont bmFont;
    bmFont.lineHeight = JsonConverter<float>( common->second, "lineHeight" );
    bmFont.base = JsonConverter<float>( common->second, "base" );
    bmFont.scaleW = JsonConverter<float>( common->second, "scaleW" );
    bmFont.scaleH = JsonConverter<float>( common->second, "scaleH" );

    bmFont.textureFile = JsonConverter<std::string>( fntObject.find( "page" )->second.get( 0 ), "file" );

    if( bmFont.textureFile.empty() )
    {
        std::cerr << "Error read " << fileName << " file: texture not found\n";
        return {};
    }

    const auto &chars = fntObject.find( "char" )->second.get<picojson::array>();

    for( const auto &ch : chars )
    {
        auto charId = JsonConverter<unsigned int>( ch, "id" );

        auto &charInfo = bmFont.chars[charId];

        charInfo.x = JsonConverter<float>( ch, "x" );
        charInfo.y = JsonConverter<float>( ch, "y" );
        charInfo.width = JsonConverter<float>( ch, "width" );
        charInfo.height = JsonConverter<float>( ch, "height" );
        charInfo.xoffset = JsonConverter<float>( ch, "xoffset" );
        charInfo.yoffset = JsonConverter<float>( ch, "yoffset" );
        charInfo.xadvance = JsonConverter<float>( ch, "xadvance" );
        charInfo.page = JsonConverter<unsigned int>( ch, "page" );

        if( charInfo.width == 0.0f )
        {
            charInfo.width = bmFont.base;
        }

        charInfo.charSize = { charInfo.width / bmFont.base, charInfo.height / bmFont.base };
        float u1 = charInfo.x / bmFont.scaleW;
        float u2 = ( charInfo.x + charInfo.width ) / bmFont.scaleW;
        float v1 = charInfo.y / bmFont.scaleW;
        float v2 = ( charInfo.y + charInfo.height ) / bmFont.scaleW;

        charInfo.texCoord = { u1, v1, u2, v2 };
        charInfo.offset = { charInfo.xoffset / bmFont.base, charInfo.yoffset / bmFont.base };
    }

    return bmFont;
}
