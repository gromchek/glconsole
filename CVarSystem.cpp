#include "CVarSystem.h"
#include "CmdSystem.h"
#include <map>
#include "Console.h"

void CvarList_f( const CommandArgs &args )
{
    auto &self = cvarSystem;
    for( const auto &cvar : self.variables )
    {
        con.Printf( "{} {}", cvar.second->GetName().data(), cvar.second->GetAs<std::string>().data() );
    }
}

void Set_f( const CommandArgs &args )
{
    auto &self = cvarSystem;
    if( args.Argc() == 3 )
    {
        auto cvar = self.Find( args[1] );

        if( !cvar )
        {
            con.EPrintf( "Can't find '{}' variable", args[1].data() );

            return;
        }

        auto *var = cvar.value();
        var->Set( args[2] );
    }
    else
    {
        con.Print( "Usage: cvarSet <var> [arg]" );
    }
}

void Get_f( const CommandArgs &args )
{
    auto &self = cvarSystem;
    if( args.Argc() == 2 )
    {
        auto cvar = self.Find( args[1] );

        if( cvar )
        {
            con.Printf( "{} is {}", cvar.value()->GetName().data(), cvar.value()->GetAs<std::string>().data() );
        }
        else
        {
            con.EPrintf( "Can't find '{}' variable", args[1].data() );
        }
    }
    else
    {
        con.Print( "Usage: cvarGet <var>" );
    }
}

void Reset_f( const CommandArgs &args )
{
    auto &self = cvarSystem;
    if( args.Argc() == 2 )
    {
        auto cvar = self.Find( args[1] );

        if( cvar )
        {
            cvar.value()->Reset();
        }
        else
        {
            con.EPrintf( "Can't find '{}' variable", args[1].c_str() );
        }
    }
    else
    {
        con.Print( "Usage: cvarReset <var>" );
    }
}

CVar::CVar( std::string_view name, std::string_view value, std::string_view desctiption ) :
    name( name ), desc( desctiption ), stringValue( value ), defaultValue( value )
{
    cvarSystem.AddVar( *this );
}

void CVar::Set( std::string_view var )
{
    stringValue = var;
}

void CVar::Reset()
{
    stringValue = defaultValue;
}


void CVarSystem::Init()
{
    init = true;
    commandSys.AddCommand( "cvarList", CvarList_f, "list of cvars" );
    commandSys.AddCommand( "cvarGet", Get_f, "get cvar value" );
    commandSys.AddCommand( "cvarSet", Set_f, "set cvar value" );
    commandSys.AddCommand( "cvarReset", Reset_f, "reset cvar value" );
}

void CVarSystem::AddVar( CVar &cvar )
{
    variables[cvar.GetName()] = &cvar;
}

std::optional<CVar *> CVarSystem::Find( const std::string &name ) const
{
    if( !IsInit() )
    {
        con.Print( "CVarSystem::Find: system is not initialized!" );
        return std::nullopt;
    }

    auto i = variables.find( name );

    if( i != variables.end() )
    {
        return { i->second };
    }

    return std::nullopt;
}

bool CVarSystem::Command( const CommandArgs &args )
{
    if( auto var = Find( args[0] ); var )
    {
        if( args.Argc() == 1 )
        {
            con.Printf( "{} is {}", var.value()->GetName().c_str(), var.value()->GetAs<std::string>().c_str() );
        }
        else
        {
            var.value()->Set( args[1] );
        }

        return true;
    }

    return false;
}

void CVarSystem::SetStringValue( std::string_view name, std::string_view value ) const
{
    setValue( name, value );
}

void CVarSystem::SetIntValue( std::string_view name, const int value ) const
{
    setValue( name, std::to_string( value ) );
}

void CVarSystem::SetFloatValue( std::string_view name, const float value ) const
{
    setValue( name, std::to_string( value ) );
}

void CVarSystem::SetBoolValue( std::string_view name, const bool value ) const
{
    setValue( name, std::to_string( int( value ) ) );
}

std::vector<std::pair<std::string, std::string>> CVarSystem::GetAutocompleteList( std::string_view name )
{
    std::vector<std::pair<std::string, std::string>> list;
    list.reserve( 8 );

    for( auto &&[key, value] : variables )
    {
        if( !key.compare( 0, name.length(), name ) )
        {
            list.emplace_back( value->name, value->desc );
        }
    }

    return list;
}

void CVarSystem::setValue( std::string_view name, std::string_view value ) const
{
    auto cvar = cvarSystem.Find( std::string{ name } );

    if( cvar )
    {
        cvar.value()->Set( value );
        return;
    }
}
