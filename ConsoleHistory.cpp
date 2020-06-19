#include "ConsoleHistory.h"

void ConsoleHistory::AddLine( std::string_view str )
{
    if( str.empty() )
    {
        return;
    }

    if( history[( numLines - 1 ) & ( MAX_LINES - 1 )] == str )
    {
        return;
    }

    upHistory = numLines;
    returnLine = numLines;
    downHistory = numLines + 1;

    history[numLines & ( MAX_LINES - 1 )] = str;

    numLines += 1;
}

std::string_view ConsoleHistory::GetFromHistory( const bool toUp )
{
    if( numLines == 0 )
    {
        return "";
    }

    // move the history point
    if( toUp )
    {
        if( upHistory < numLines - MAX_LINES || upHistory < 0 )
        {
            return "";
        }

        returnLine = upHistory;
        downHistory = upHistory + 1;
        upHistory--;
    }
    else
    {
        if( downHistory >= numLines )
        {
            upHistory = downHistory - 1;

            return "";
        }

        returnLine = downHistory;
        upHistory = downHistory - 1;
        downHistory++;
    }

    return history[returnLine & ( MAX_LINES - 1 )];
}
