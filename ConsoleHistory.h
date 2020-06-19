#ifndef CONSOLEHISTORY_H
#define CONSOLEHISTORY_H

#include <string>
#include <array>

class ConsoleHistory final
{
private:
    int numLines = 0;
    static constexpr int MAX_LINES = 32;

    int returnLine = 0;
    int upHistory = 0;
    int downHistory = 0;

    std::array<std::string, MAX_LINES> history;

public:
    ConsoleHistory() = default;
    ~ConsoleHistory() = default;

    void AddLine( std::string_view str );

    std::string_view GetFromHistory( const bool toUp );
};

#endif // CONSOLEHISTORY_H
