#ifndef COMMANDARGS_H
#define COMMANDARGS_H


#include <string>
#include <cassert>
#include <string_view>

class CommandArgs final
{
private:
    static constexpr unsigned int MAX_ARGS = 32;

    unsigned int argc = 0;
    std::string argv[MAX_ARGS];

public:
    CommandArgs() = default;
    explicit CommandArgs( std::string_view command );
    ~CommandArgs() = default;

    void Tokenize( std::string_view command );
    unsigned int Argc() const;

    std::string_view CommandName() const;

    const std::string &operator[]( const unsigned int i ) const;
    std::string &operator[]( const unsigned int i );
};

inline unsigned int CommandArgs::Argc() const
{
    return argc;
}

inline std::string_view CommandArgs::CommandName() const
{
    return argv[0];
}

inline const std::string &CommandArgs::operator[]( const unsigned int i ) const
{
    assert( i < argc );
    return argv[i];
}

inline std::string &CommandArgs::operator[]( const unsigned int i )
{
    assert( i < argc );
    return argv[i];
}

#endif // COMMANDARGS_H
