#include "TextInput.h"
#include "Misc.h"
#include "GAPI/Vertex.h"

TextInput::TextInput()
{
    text.reserve( MAX_LINE_LENGTH );
    Clear();
}

void TextInput::Clear()
{
    text.clear();
    cursorPos = 0;
}

void TextInput::SetText( std::string_view str )
{
    text.clear();
    text.append( str.substr( 0, MAX_LINE_LENGTH ) );
    cursorPos = text.length();

    updateTCoord = true;
}

void TextInput::HandleEvent( const SDL_Event &ev )
{
    if( ev.type == SDL_KEYUP )
    {
        const auto key = ev.key;
        const bool ctrl = SDL_GetModState() & KMOD_CTRL;

        if( key.keysym.scancode == SDL_SCANCODE_LSHIFT || key.keysym.scancode == SDL_SCANCODE_RSHIFT )
        {
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_LCTRL || key.keysym.scancode == SDL_SCANCODE_RCTRL )
        {
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_LALT || key.keysym.scancode == SDL_SCANCODE_RALT )
        {
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_BACKSPACE )
        {
            if( cursorPos > 0 )
            {
                text.erase( cursorPos - 1, 1 );
                cursorPos--;

                updateTCoord = true;
            }
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_LEFT )
        {
            if( ctrl )
            {
                while( ( cursorPos > 0 ) && ( text[cursorPos - 1] == ' ' ) )
                {
                    cursorPos--;
                }

                while( ( cursorPos > 0 ) && ( text[cursorPos - 1] != ' ' ) )
                {
                    cursorPos--;
                }

                updateTCoord = true;

                return;
            }

            cursorPos--;

            if( cursorPos < 0 )
            {
                cursorPos = 0;
            }

            updateTCoord = true;

            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_RIGHT )
        {
            if( ctrl )
            {
                while( ( cursorPos < text.length() ) && ( text[cursorPos] != ' ' ) )
                {
                    cursorPos++;
                }

                while( ( cursorPos < text.length() ) && ( text[cursorPos] == ' ' ) )
                {
                    cursorPos++;
                }

                updateTCoord = true;

                return;
            }

            cursorPos++;

            if( cursorPos > text.length() )
            {
                cursorPos = text.length();
            }

            updateTCoord = true;

            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_DELETE )
        {
            if( cursorPos < text.length() )
            {
                text.erase( cursorPos, 1 );

                updateTCoord = true;
            }
            return;
        }

        if( ( key.keysym.scancode == SDL_SCANCODE_HOME && !ctrl ) || ( key.keysym.scancode == SDL_SCANCODE_A && ctrl ) )
        {
            cursorPos = 0;
            updateTCoord = true;
            return;
        }

        if( ( key.keysym.scancode == SDL_SCANCODE_END && !ctrl ) || ( key.keysym.scancode == SDL_SCANCODE_E && ctrl ) )
        {
            cursorPos = text.length();
            updateTCoord = true;
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_L && ctrl )
        {
            Clear();
            return;
        }

        if( key.keysym.scancode == SDL_SCANCODE_ESCAPE )
        {
            Clear();
            return;
        }
    }

    if( ev.type == SDL_TEXTINPUT )
    {
        const char ch = ev.text.text[0];
        addChar( ch );
    }
}

void TextInput::addChar( const unsigned int ch )
{
    if( ch > 128 || ch < 32 )
    {
        return;
    }

    if( ch == '`' || ch == '~' )
    {
        return;
    }

    if( text.length() == MAX_LINE_LENGTH - 1 )
    {
        return;
    }

    text.insert( cursorPos, 1, char( ch ) );
    cursorPos += 1;
    updateTCoord = true;
}
