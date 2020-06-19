#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <string>
#include <SDL2/SDL_events.h>

class TextInput final
{
private:
    static constexpr unsigned int MAX_LINE_LENGTH = 64;

    std::string text;
    int cursorPos = 0;

    bool updateTCoord = false;

    void addChar( const unsigned int ch );

    friend class Console;

public:
    TextInput();
    ~TextInput() = default;

    void Clear();

    void SetText( std::string_view str );
    std::string_view GetText() const;

    unsigned int GetMaxLineLength() const;

    void HandleEvent( const SDL_Event &ev );
};

inline std::string_view TextInput::GetText() const
{
    return text;
}

inline unsigned int TextInput::GetMaxLineLength() const
{
    return MAX_LINE_LENGTH;
}

#endif // TEXTINPUT_H
