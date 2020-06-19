#include "CmdSystem.h"
#include "Console.h"
#include "CVarSystem.h"
#include <charconv>

void Echo_f( const CommandArgs &args )
{
    if( args.Argc() < 2 )
    {
        con.Print( "usage: echo [arg]" );
        return;
    }

    std::string line;
    line.reserve( 64 );
    for( auto i = 1u; i < args.Argc(); i++ )
    {
        line.append( args[i].data() );
        line.append( 1, ' ' );
    }

    con.Print( line );
}

void List_f( const CommandArgs &args )
{
    const auto &self = commandSys;
    for( auto &&[key, val] : self.commands )
    {
        con.Printf( "{}\t{}", val.name.data(), val.description.data() );
    }
}

void Wait_f( const CommandArgs &args )
{
    if( args.Argc() == 2 )
    {
        unsigned int cyclesToWait = 0;
        std::from_chars( args[1].data(), args[1].data() + args[1].length(), cyclesToWait );

        commandSys.setWait( cyclesToWait );
        return;
    }

    con.Print( "usage: wait [arg]" );
}


CmdSystem::CmdSystem()
{
    buffer.reserve( MAX_BUFFER_SIZE );

    AddCommand( "echo", Echo_f, "print text" );
    AddCommand( "wait", Wait_f, "dealys one or more frames" );
    AddCommand( "list", List_f, "print command list" );
    AddCommand( "help", List_f, "print command list" );
}

void CmdSystem::AddCommand( const std::string &name, cmdFunction_t function, std::string_view desc )
{
    auto i = commands.find( name );

    if( i == commands.end() )
    {
        commands.emplace( std::piecewise_construct, std::forward_as_tuple( name ),
                          std::forward_as_tuple( name, function, desc ) );
    }
    else
    {
        con.Printf( "Function '{}' already defined", name.data() );
    }
}

void CmdSystem::ExecuteCommandBuffer()
{
    unsigned int head = 0;

    while( buffLength )
    {
        if( !wait )
        {
            wait--;
            break;
        }

        const auto pos = buffer.find( '\n', head );

        if( pos != std::string::npos )
        {
            std::string line( buffer.substr( head, pos - head ) );
            ExecuteCommandText( line );

            head = pos + 1;
            buffLength -= line.length() + 1;
        }
    }
}

// add command without '\n' at end
void CmdSystem::AppendCommandText( std::string_view text )
{
    if( text.empty() )
    {
        return;
    }

    const auto len = text.length();

    if( ( buffLength + len ) > MAX_BUFFER_SIZE )
    {
        con.Print( "AppendCommandText: Buffer overflow" );
        return;
    }

    buffer.insert( buffLength, text );
    buffLength += len;
}

// add command with '\n' at end
void CmdSystem::InsertCommandText( std::string_view text )
{
    if( text.empty() )
    {
        return;
    }

    const auto len = text.length() + 1;

    if( ( buffLength + len ) > MAX_BUFFER_SIZE )
    {
        con.Print( "InsertCommandText: Buffer overflow" );
        return;
    }

    buffer.insert( buffLength, text );
    buffLength += text.length();
    buffer.insert( buffLength, "\n" );
    buffLength += 1;
}

void CmdSystem::ExecuteCommandText( std::string_view text )
{
    unsigned int strBegin = 0u;
    unsigned int strEnd = 0u;

    while( strEnd < text.length() )
    {
        int quotes = 0;

        for( ; strEnd < text.length(); strEnd++ )
        {
            if( text[strEnd] == '"' )
            {
                quotes++;
            }
            if( !( quotes & 1 ) && text[strEnd] == ';' )
            {
                break;
            }
        }

        execute( CommandArgs( text.substr( strBegin, strEnd - strBegin ) ) );
        strEnd++;
        strBegin = strEnd;
    }
}

std::vector<std::pair<std::string, std::string>> CmdSystem::GetAutocompleteList( std::string_view name )
{
    std::vector<std::pair<std::string, std::string>> list;
    list.reserve( 8 );

    for( auto &&[key, value] : commands )
    {
        if( !value.name.compare( 0, name.length(), name ) )
        {
            list.emplace_back( value.name, value.description );
        }
    }

    return list;
}

void CmdSystem::execute( const CommandArgs &args )
{
    if( !args.Argc() )
    {
        return;
    }

    if( auto i = commands.find( args[0] ); i != commands.end() )
    {
        i->second.function( args );
        return;
    }

    if( cvarSystem.Command( args ) )
    {
        return;
    }

    con.Printf( "'{}': unknown command", args[0].data() );
}
