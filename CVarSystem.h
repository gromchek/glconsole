#ifndef CVARSYSTEM_H
#define CVARSYSTEM_H

#include <string_view>
#include <string>
#include "Converter.h"
#include "CommandArgs.h"
#include <optional>
#include <map>
#include <vector>

template <class T, class... Args>
inline constexpr bool is_any_of = ( std::is_same_v<T, Args> || ... );


class CVar final
{
private:
    const std::string name;
    const std::string desc;

    std::string stringValue;
    const std::string defaultValue;

    friend class CVarSystem;

public:
    CVar() = delete;
    CVar( std::string_view name, std::string_view value, std::string_view description = "" );
    CVar( const CVar &cvar ) = delete;
    CVar &operator=( const CVar &cvar ) = delete;
    ~CVar() = default;

    const std::string &GetName() const;

    template <typename T>
    T GetAs();

    void Set( std::string_view var );
    void Reset();
};

template <typename T>
inline T CVar::GetAs()
{
    static_assert( is_any_of<T, int, float, bool, std::string>, "CVar::GetAs: wrong type" );
    return Converter<T>( stringValue );
}

inline const std::string &CVar::GetName() const
{
    return name;
}

class CVarSystem final
{
private:
    bool init = false;
    std::map<std::string, CVar *> variables;

    friend void CvarList_f( const CommandArgs &args );
    friend void Set_f( const CommandArgs &args );
    friend void Get_f( const CommandArgs &args );
    friend void Reset_f( const CommandArgs &args );


    void setValue( std::string_view name, std::string_view value ) const;

public:
    CVarSystem() = default;
    ~CVarSystem() = default;

    void Init();

    bool IsInit() const;

    void AddVar( CVar &cvar );
    std::optional<CVar *> Find( const std::string &name ) const;

    bool Command( const CommandArgs &args );

    void SetStringValue( std::string_view name, std::string_view value ) const;
    void SetIntValue( std::string_view name, const int value ) const;
    void SetFloatValue( std::string_view name, const float value ) const;
    void SetBoolValue( std::string_view name, const bool value ) const;

    std::vector<std::pair<std::string, std::string>> GetAutocompleteList( std::string_view name );
};

inline bool CVarSystem::IsInit() const
{
    return init;
}

inline CVarSystem cvarSystem;

#endif // CVARSYSTEM_H
