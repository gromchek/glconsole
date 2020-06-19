#include "CommandArgs.h"
#include <sstream>

CommandArgs::CommandArgs( std::string_view command )
{
    Tokenize( command );
}

void CommandArgs::Tokenize( std::string_view command )
{
    constexpr std::string_view delims( " " );

    size_t first = 0;

    while( first < command.size() || argc >= MAX_ARGS )
    {
        const auto second = command.find_first_of( delims, first );

        if( first != second )
        {
            argv[argc].append( command.substr( first, second - first ) );
            argc++;
        }

        if( second == std::string_view::npos || argc >= MAX_ARGS )
        {
            break;
        }

        first = second + 1;
    }
}
