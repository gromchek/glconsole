#ifndef CMDSYSTEM_H
#define CMDSYSTEM_H

#include <string>
#include "CommandArgs.h"
#include <string_view>
#include <unordered_map>
#include <vector>

typedef void ( *cmdFunction_t )( const CommandArgs &args );

struct Command
{
    Command() = default;
    ~Command() = default;

    Command( std::string_view n, cmdFunction_t f, std::string_view d = "" ) : name( n ), function( f ), description( d )
    {
    }

    std::string name;
    cmdFunction_t function;
    std::string description;
};

class CmdSystem final
{
private:
    static constexpr unsigned int MAX_BUFFER_SIZE = 0x3FFF + 1;

    unsigned int wait = 0;

    unsigned int buffLength = 0;

    std::string buffer;

    std::unordered_map<std::string, Command> commands;

public:
    CmdSystem();
    ~CmdSystem() = default;

    void AddCommand( const std::string &name, cmdFunction_t function, std::string_view desc );

    void ExecuteCommandBuffer();

    void AppendCommandText( std::string_view text );
    void InsertCommandText( std::string_view text );
    void ExecuteCommandText( std::string_view text );

    std::vector<std::pair<std::string, std::string>> GetAutocompleteList( std::string_view name );

private:
    void setWait( const unsigned int n );
    void execute( const CommandArgs &args );

    friend void List_f( const CommandArgs &args );
    friend void Echo_f( const CommandArgs &args );
    friend void Wait_f( const CommandArgs &args );
};

inline void CmdSystem::setWait( const unsigned int n )
{
    wait = n + 1;
}

inline CmdSystem commandSys;

#endif // CMDSYSTEM_H
