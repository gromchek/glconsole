#include "Console.h"

#include "CmdSystem.h"
#include "CVarSystem.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "GAPI/GLmisc.h"

#include <vector>

#include <fstream>
#include <cmath>

#include <iostream>
#include <algorithm>

namespace
{

constexpr char END_OF_THE_TEXT = 3;

const Color defaultFontColor{ 216, 216, 216 };

const float CONSOLE_FONT_SIZE = 17.0f;


const Color colorTable[] = { defaultFontColor, Color::Red, Color::Green, Color::Blue, Color::Yellow };

Color getColor( const std::size_t idx )
{
    if( std::size( colorTable ) <= idx )
    {
        return colorTable[0];
    }
    else
    {
        return colorTable[idx];
    }
}

std::vector<std::uint16_t> buildIndices5( const std::uint32_t quadCount )
{
    if( quadCount == 1 )
    {
        return { 0, 1, 2, 0, 2, 3 };
    }

    std::vector<std::uint16_t> indices;
    indices.reserve( quadCount * 6 );

    for( auto i = 0u; i < quadCount; i++ )
    {
        auto baseVertex = i * 4;
        indices.push_back( std::uint16_t( 0 + baseVertex ) );
        indices.push_back( std::uint16_t( 1 + baseVertex ) );
        indices.push_back( std::uint16_t( 2 + baseVertex ) );
        indices.push_back( std::uint16_t( 0 + baseVertex ) );
        indices.push_back( std::uint16_t( 2 + baseVertex ) );
        indices.push_back( std::uint16_t( 3 + baseVertex ) );
    }

    return indices;
}

void updateCharQuad( Vertex2d *quad, const BMchar &charInfo, const Color &color, const vec2f &pos )
{
    if( !quad )
    {
        return;
    }

    float u1 = charInfo.texCoord.x;
    float u2 = charInfo.texCoord.z;
    float v1 = charInfo.texCoord.y;
    float v2 = charInfo.texCoord.w;

    float xo = charInfo.offset.x * CONSOLE_FONT_SIZE;
    float yo = charInfo.offset.y * CONSOLE_FONT_SIZE;
    float dimX = CONSOLE_FONT_SIZE * charInfo.charSize.x + xo;
    float dimY = CONSOLE_FONT_SIZE * charInfo.charSize.y + yo;

    quad[0].position = { pos.x + xo, pos.y + yo };
    quad[1].position = { pos.x + dimX, pos.y + yo };
    quad[2].position = { pos.x + dimX, pos.y + dimY };
    quad[3].position = { pos.x + xo, pos.y + dimY };

    quad[0].texcoord = { u1, v1 };
    quad[1].texcoord = { u2, v1 };
    quad[2].texcoord = { u2, v2 };
    quad[3].texcoord = { u1, v2 };

    quad[0].color = color;
    quad[1].color = color;
    quad[2].color = color;
    quad[3].color = color;
}

struct Autocomplete
{
    std::vector<std::pair<std::string, std::string>> matchList;
    unsigned int matchIndex = 0;
    std::string lastSearch = "";

    Autocomplete();

    void Clear();
    std::string_view GetNextMatch();
} autocomplete;

Autocomplete::Autocomplete()
{
    matchList.reserve( 32 );
}

void Autocomplete::Clear()
{
    matchList.clear();
    matchIndex = 0;
    lastSearch.clear();
}

std::string_view Autocomplete::GetNextMatch()
{
    matchIndex++;
    matchIndex %= matchList.size();

    return matchList[matchIndex].first;
}

} // namespace

void Dump_f( const CommandArgs &args )
{
    const auto &self = con;

    std::ofstream f( "dump.txt" );

    if( !f.good() )
    {
        con.EPrintf( "Fail write to file" );
        return;
    }

    std::string buff;
    buff.reserve( self.text.length() );

    unsigned int offset = 0;
    for( auto i = 0u; i < self.LINE_WIDTH; i++ )
    {
        if( i + offset >= self.text.length() )
        {
            break;
        }

        if( self.text[i + offset] == END_OF_THE_TEXT ) // new line
        {
            offset += self.LINE_WIDTH;
            i = 0;

            buff.push_back( '\n' );
        }

        buff.push_back( char( self.text[i + offset] & 0xFF ) );
    }

    f.write( buff.data(), buff.length() );
}

void Clear_f( const CommandArgs &args )
{
    auto &self = con;

    self.text.clear();
    self.currentLine = 0;
    self.x = 0;
    self.display = 0;

    self.text.append( self.LINE_WIDTH - 1, ' ' );
    self.text.append( 1, '\n' );

    self.updateTCoord = true;
}


void Console::handleKeyEvent( const SDL_Event &ev )
{
    if( ev.type != SDL_KEYDOWN )
    {
        return;
    }

    const auto key = ev.key;
    const bool ctrl = SDL_GetModState() & KMOD_CTRL;

    if( key.keysym.scancode == SDL_SCANCODE_RETURN || key.keysym.scancode == SDL_SCANCODE_KP_ENTER )
    {
        auto str = textInput.GetText();

        if( !str.empty() )
        {
            Print( str );

            commandSys.InsertCommandText( str );

            history.AddLine( str );
            textInput.Clear();
        }

        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_PAGEUP )
    {
        scrollUp();
        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_PAGEDOWN )
    {
        scrollDown();
        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_HOME && ctrl )
    {
        home();
        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_END && ctrl )
    {
        end();
        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_UP || ( key.keysym.scancode == SDL_SCANCODE_P && ctrl ) )
    {
        auto str = history.GetFromHistory( true );

        if( !str.empty() )
        {
            textInput.SetText( str );
        }
        else
        {
            textInput.SetText( "" );
        }

        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_DOWN || ( key.keysym.scancode == SDL_SCANCODE_N && ctrl ) )
    {
        auto str = history.GetFromHistory( false );

        if( !str.empty() )
        {
            textInput.SetText( str );
        }
        else
        {
            textInput.SetText( "" );
        }

        return;
    }

    if( key.keysym.scancode == SDL_SCANCODE_TAB )
    {
        printAutoComplete();
        return;
    }
}

void Console::scrollUp()
{
    display -= 2;

    if( currentLine - display >= TOTAL_LINES )
    {
        display = currentLine - TOTAL_LINES + 1;
    }

    updateTCoord = true;
}

void Console::scrollDown()
{
    display += 2;

    if( display > currentLine )
    {
        display = currentLine;
    }

    updateTCoord = true;
}

void Console::home()
{
    display = 0;
    updateTCoord = true;
}

void Console::end()
{
    display = currentLine;
    updateTCoord = true;
}

void Console::lineFeed()
{
    text[( currentLine % TOTAL_LINES ) * LINE_WIDTH + x] = END_OF_THE_TEXT;

    if( display == currentLine )
    {
        display++;
    }
    currentLine++;
    x = 0;

    text.append( LINE_WIDTH - 1, ' ' );
    text.append( 1, '\n' );

    updateTCoord = true;
}

void Console::updateTextInputVertexData( Vertex2d *verts, vec2f pos )
{
    if( !textInput.updateTCoord || textInput.GetText().empty() )
    {
        return;
    }

    const auto vertNum = vertsCount - textInputStringBaseVertex - 4;

    std::for_each( verts, verts + vertNum, []( Vertex2d &vert ) { vert.texcoord = { 0.0f, 0.0f }; } );

    const std::string textInputString( textInput.GetText() );
    for( auto i = 0u; i < textInputString.length(); i++ )
    {
        const char ch = textInputString[i];
        const auto &charInfo = font.chars.find( std::uint32_t( ch ) );

        if( charInfo == font.chars.end() )
        {
            continue;
        }

        if( ch == ' ' )
        {
            pos.x += ( charInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE;
            continue;
        }

        updateCharQuad( &verts[i * 4], charInfo->second, defaultFontColor, pos );

        pos.x += std::roundf( ( charInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE );
        textInputIndexToDraw = i + 1;
    }

    textInput.updateTCoord = false;
}

void Console::updateConsoleTextVertexData( Vertex2d *verts, vec2f pos )
{
    if( !updateTCoord )
    {
        return;
    }

    std::for_each( verts, verts + textInputStringBaseVertex, []( Vertex2d &vert ) { vert.texcoord = { 0.0f, 0.0f }; } );

    const float xOffset = pos.x;

    auto [rowBegin, rowEnd] = getBeginEndRowsIndex();

    Color color = defaultFontColor;

    for( auto i = 0u, index = 0u; i < LINE_WIDTH; i++, index++ )
    {
        if( i + rowBegin + 1 >= rowEnd || i + rowBegin + 1 >= text.length() )
        {
            break;
        }

        if( text[i + rowBegin] == END_OF_THE_TEXT ) // new line
        {
            rowBegin += LINE_WIDTH;
            index += LINE_WIDTH - i - 1;
            i = 0;

            pos.x = xOffset;
            pos.y += std::roundf( ( font.lineHeight / font.base ) * CONSOLE_FONT_SIZE );

            if( rowBegin == rowEnd )
            {
                break;
            }
        }


        const int colorIndex = text[i + rowBegin] >> 8;
        const std::uint8_t ch = text[i + rowBegin] & 0xFF;

        const auto &charInfo = font.chars.find( std::uint32_t( ch ) );

        if( charInfo == font.chars.end() )
        {
            continue;
        }

        if( ch == ' ' )
        {
            pos.x += std::roundf( ( charInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE );
            color = defaultFontColor;
            continue;
        }

        color = getColor( std::size_t( colorIndex ) );

        updateCharQuad( &verts[index * 4], charInfo->second, color, pos );

        pos.x += std::roundf( ( charInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE );
        consoleTexIndexToDraw = index + 1;
    }

    updateTCoord = false;
}

void Console::updateCursorVertexData( Vertex2d *verts, vec2f pos )
{
    const char cursor = '|';

    const auto &charInfo = font.chars.find( uint32_t( cursor ) );

    if( charInfo == font.chars.end() )
    {
        return;
    }

    if( textInput.GetText().empty() )
    {
        pos.x = 0.0f;
    }
    else
    {
        const auto &str = textInput.GetText();
        for( int i = 0; i < textInput.cursorPos; i++ )
        {
            const auto &chInfo = font.chars.find( uint32_t( str[i] ) );

            if( str[i] == ' ' )
            {
                pos.x += std::roundf( ( chInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE );
                continue;
            }

            pos.x += std::roundf( ( charInfo->second.xadvance / font.base ) * CONSOLE_FONT_SIZE );
        }
    }

    Vertex2d *quad = &verts[0];
    updateCharQuad( quad, charInfo->second, defaultFontColor, pos );
}

std::pair<int, int> Console::getBeginEndRowsIndex() const
{
    const int lineCount = int( ( R_WINDOW_HEIGHT * 0.5f ) / ( font.lineHeight / font.base * CONSOLE_FONT_SIZE ) ) - 1;

    int beginLineNum = display - lineCount;
    beginLineNum = std::max( 0, beginLineNum );

    int endLineNum = beginLineNum + lineCount;
    endLineNum = std::min( int( currentLine ), endLineNum );

    return { beginLineNum * LINE_WIDTH, endLineNum * LINE_WIDTH };
}

void Console::printAutoComplete()
{
    if( textInput.GetText().empty() )
    {
        return;
    }

    if( autocomplete.lastSearch == textInput.GetText() )
    {
        if( autocomplete.matchList.size() > 0 )
        {
            textInput.SetText( autocomplete.GetNextMatch() );
            autocomplete.lastSearch = textInput.GetText();
        }
        return;
    }

    autocomplete.Clear();

    auto cmdList = commandSys.GetAutocompleteList( textInput.GetText() );
    auto cvarList = cvarSystem.GetAutocompleteList( textInput.GetText() );

    autocomplete.matchList.insert( autocomplete.matchList.end(), std::make_move_iterator( cmdList.begin() ),
                                   std::make_move_iterator( cmdList.end() ) );

    autocomplete.matchList.insert( autocomplete.matchList.end(), std::make_move_iterator( cvarList.begin() ),
                                   std::make_move_iterator( cvarList.end() ) );

    if( autocomplete.matchList.empty() )
    {
        return;
    }

    autocomplete.lastSearch = textInput.GetText();

    for( const auto &e : autocomplete.matchList )
    {
        Printf( "\t{}\t{}", std::string( e.first ).c_str(), std::string( e.second ).c_str() );
    }
}

Console::Console()
{
    LINE_WIDTH = ( static_cast<unsigned int>( float( R_WINDOW_WIDTH ) / CONSOLE_FONT_SIZE ) );
    TOTAL_LINES = ( BUFF_SIZE / LINE_WIDTH );

    text.reserve( BUFF_SIZE );
    text.append( LINE_WIDTH - 1, ' ' );
    text.append( 1, '\n' );

    vertsCount = LINE_WIDTH * MAX_LINES_DISPLAY * 4 + 8; // console text + 2 quad
    textInputStringBaseVertex = LINE_WIDTH * ( MAX_LINES_DISPLAY - 1 ) * 4;
    backgroundBaseVertex = LINE_WIDTH * MAX_LINES_DISPLAY * 4;
    cursorBaseVertex = LINE_WIDTH * MAX_LINES_DISPLAY * 4 + 4;
}

void Console::Init()
{
    commandSys.AddCommand( "clear", Clear_f, "Clear console text" );
    commandSys.AddCommand( "cls", Clear_f, "Clear console text" );
    commandSys.AddCommand( "dump", Dump_f, "Dump console text to file" );


    vao.Bind();

    auto index = buildIndices5( MAX_LINES_DISPLAY * LINE_WIDTH );

    indexBuffer.Init( BufferUsage::Index, true );
    indexBuffer.Data( index.data(), sizeof( std::uint16_t ) * index.size() );

    vertexBuffer.Init( BufferUsage::Vertex, false );
    vertexBuffer.Data( nullptr, vertsCount * sizeof( Vertex2d ) );

    GL_VertexAttrib( 0, 2, GL_FLOAT, sizeof( Vertex2d ), Vertex2dAttrib::POSITION_LOCATION );
    GL_VertexAttrib( 1, 2, GL_FLOAT, sizeof( Vertex2d ), Vertex2dAttrib::OFFSET_TEX_COORD );
    GL_VertexAttrib( 2, 4, GL_UNSIGNED_BYTE, sizeof( Vertex2d ), Vertex2dAttrib::OFFSET_COLOR );

    vao.Unbind();
}

void Console::Open()
{
    if( opened )
    {
        return;
    }
    opened = true;

    textInput.Clear();
    autocomplete.Clear();
}

void Console::HandleEvent( const SDL_Event &ev )
{
    if( ev.type == SDL_KEYDOWN )
    {
        const auto key = ev.key;
        const bool shift = SDL_GetModState() & KMOD_SHIFT;

        if( key.keysym.scancode == SDL_SCANCODE_GRAVE || ( key.keysym.scancode == SDL_SCANCODE_GRAVE && shift ) )
        {
            if( IsOpened() )
            {
                Close();
            }
            else
            {
                Open();
            }
        }
    }

    if( !IsOpened() )
    {
        return;
    }

    if( ev.type == SDL_MOUSEWHEEL )
    {
        if( ev.wheel.y > 0 )
        {
            scrollUp();
        }

        if( ev.wheel.y < 0 )
        {
            scrollDown();
        }
    }


    handleKeyEvent( ev );
    textInput.HandleEvent( ev );
}

void Console::Update( const unsigned int dt )
{
    if( ( updateTCoord || textInput.updateTCoord ) && font.chars.size() > 0 )
    {
        Vertex2d *verts = reinterpret_cast<Vertex2d *>( vertexBuffer.Map( MapBufferAccess::WriteOnly ) );

        updateConsoleTextVertexData( verts, { 4.0f, 2.0f } );

        updateTextInputVertexData( &verts[textInputStringBaseVertex],
                                   { 4.0f, ( R_WINDOW_HEIGHT - CONSOLE_FONT_SIZE ) * 0.5f } );

        updateCursorVertexData( &verts[cursorBaseVertex], { 0.0f, ( R_WINDOW_HEIGHT - CONSOLE_FONT_SIZE ) * 0.5f } );

        const auto &charInfo = font.chars.find( std::uint32_t( '#' ) );
        updateCharQuad( &verts[backgroundBaseVertex], charInfo->second, Color::Black, { 0.0f, 0.0f } );

        auto *quad = &verts[backgroundBaseVertex];

        quad[0].position = { 0.0f, 0.0f };
        quad[1].position = { float( R_WINDOW_WIDTH ), 0.0f };
        quad[2].position = { float( R_WINDOW_WIDTH ), float( R_WINDOW_HEIGHT ) * 0.52f };
        quad[3].position = { 0.0f, float( R_WINDOW_HEIGHT ) * 0.52f };

        vertexBuffer.Unmap();
    }

    cursorBlinkTime += dt;
}


void Console::Print( std::string_view str )
{
    if( str.empty() )
    {
        return;
    }

    uint16_t color = uint16_t( COLOR_DEFAULT );

    for( auto i = 0u; i < str.length(); i++ )
    {
        const auto ch = str[i];

        if( ch == COLOR_RED || ch == COLOR_BLUE || ch == COLOR_GOLD || ch == COLOR_GREEN )
        {
            color = uint16_t( ch );
            continue;
        }

        const auto idx = ( currentLine % TOTAL_LINES ) * LINE_WIDTH + x;

        if( ch == ' ' )
        {
            color = uint16_t( COLOR_DEFAULT );
            unsigned int l = 0u;
            for( l = 0u; l < LINE_WIDTH && ( l + i + 1 < str.length() ); l++ )
            {
                if( str[l + i + 1] == ' ' )
                {
                    break;
                }
            }

            if( l != LINE_WIDTH && ( ( x + l ) >= LINE_WIDTH ) )
            {
                lineFeed();
            }
        }

        switch( ch )
        {
        case '\n':
            lineFeed();
            break;
        case '\t':
            do
            {
                text[idx] = std::uint16_t( color << 8 ) | std::uint16_t( ' ' );
                x++;
                if( x >= LINE_WIDTH )
                {
                    lineFeed();
                }
            } while( x & 3 );
            break;
        default:
            text[idx] = std::uint16_t( color << 8 ) | std::uint16_t( ch );
            x++;

            if( x >= LINE_WIDTH )
            {
                lineFeed();
            }
            break;
        }
    }

    lineFeed();
}

void Console::Draw()
{
    if( !IsOpened() )
    {
        return;
    }

    vao.Bind();

    glDisable( GL_BLEND );
    glDrawElementsBaseVertex( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, backgroundBaseVertex );
    glEnable( GL_BLEND );

    glDrawElementsBaseVertex( GL_TRIANGLES, consoleTexIndexToDraw * 6, GL_UNSIGNED_SHORT, nullptr, 0 );

    if( !textInput.GetText().empty() )
    {
        glDrawElementsBaseVertex( GL_TRIANGLES, textInputIndexToDraw * 6, GL_UNSIGNED_SHORT, nullptr,
                                  textInputStringBaseVertex );
    }

    if( ( cursorBlinkTime >> 9 ) & 1 )
    {
        glDrawElementsBaseVertex( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, cursorBaseVertex );
    }
}
