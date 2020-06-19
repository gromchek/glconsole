#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>

template <typename T>
T Converter( const std::string & )
{
    static_assert( std::is_same<T, void>::value, "Converter for this type is not specialized" );
    return T{};
}

template <>
inline int Converter<int>( const std::string &str )
{
    return std::stoi( str );
}

template <>
inline float Converter<float>( const std::string &str )
{
    return std::stof( str );
}

template <>
inline std::string Converter<std::string>( const std::string &str )
{
    return str;
}

template <>
inline bool Converter<bool>( const std::string &str )
{
    return std::stoi( str ) != 0;
}


#endif // CONVERTER_H
