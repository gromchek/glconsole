#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <string_view>
#include "TextInput.h"
#include "ConsoleHistory.h"
#include "CommandArgs.h"

#include "GAPI/BufferObject.h"
#include "GAPI/Vertex.h"

#include "Misc.h"

#include "thrid_party/ggformat/ggformat.h"

#include "BMfont.h"

class Console final
{
private:
    static constexpr char COLOR_DEFAULT = 0;
    static constexpr char COLOR_RED = 1;
    static constexpr char COLOR_GREEN = 2;
    static constexpr char COLOR_BLUE = 3;
    static constexpr char COLOR_GOLD = 4;

    static constexpr unsigned int FORMAT_BUFF_SIZE = 2048;

    static constexpr unsigned int BUFF_SIZE = 0x80000;
    static constexpr unsigned int MAX_LINES_DISPLAY = 32;

    unsigned int LINE_WIDTH = 0;
    unsigned int TOTAL_LINES = 0;

    TextInput textInput;
    ConsoleHistory history;
    std::basic_string<uint16_t> text;

    bool opened = false;

    unsigned int currentLine = 0;
    unsigned int x = 0;
    unsigned int display = 0;

    std::uint32_t consoleTexIndexToDraw = 0;
    std::uint32_t textInputIndexToDraw = 0;

    BMfont font;

    VertexArrayObject vao;
    Buffer indexBuffer;
    Buffer vertexBuffer;

    std::uint32_t vertsCount = 0;
    std::uint32_t textInputStringBaseVertex = 0;
    std::uint32_t backgroundBaseVertex = 0;
    std::uint32_t cursorBaseVertex = 0;

    int cursorBlinkTime = 0;

    bool updateTCoord = true;

    void handleKeyEvent( const SDL_Event &ev );

    void scrollUp();
    void scrollDown();

    void home();
    void end();

    void lineFeed();

    void updateTextInputVertexData( Vertex2d *verts, vec2f pos );
    void updateConsoleTextVertexData( Vertex2d *verts, vec2f pos );
    void updateCursorVertexData( Vertex2d *verts, vec2f pos );

    std::pair<int, int> getBeginEndRowsIndex() const;

    void printAutoComplete();

public:
    Console();
    ~Console() = default;

    void Init();

    void Open();
    void Close();

    bool IsOpened() const;

    void HandleEvent( const SDL_Event &ev );
    void Update( const unsigned int dt );

    void SetFont( const BMfont &newFont );
    const BMfont &GetFont() const;

    template <typename... Args>
    void Printf( std::string format, const Args &... args );
    template <typename... Args>
    void WPrintf( std::string format, const Args &... args );
    template <typename... Args>
    void EPrintf( std::string format, const Args &... args );
    template <typename... Args>
    void IPrintf( std::string format, const Args &... args );

    void Print( std::string_view str );

    void Draw();

private:
    friend void Clear_f( const CommandArgs &args );
    friend void Dump_f( const CommandArgs &args );
};

template <typename... Args>
inline void Console::EPrintf( std::string format, const Args &... args )
{
    format.insert( 0, "ERROR: " );
    format.insert( 0, 1, COLOR_RED );

    Printf( format, args... );
}

template <typename... Args>
inline void Console::WPrintf( std::string format, const Args &... args )
{
    format.insert( 0, "WARNING: " );
    format.insert( 0, 1, COLOR_GOLD );

    Printf( format, args... );
}

template <typename... Args>
inline void Console::IPrintf( std::string format, const Args &... args )
{
    format.insert( 0, "INFO: " );
    format.insert( 0, 1, COLOR_BLUE );

    Printf( format, args... );
}

template <typename... Args>
inline void Console::Printf( std::string format, const Args &... args )
{
    char buff[FORMAT_BUFF_SIZE];

    auto len = ggformat( buff, FORMAT_BUFF_SIZE, format.c_str(), args... );
    Print( { buff, len } );
}

inline void Console::Close()
{
    opened = false;
}

inline bool Console::IsOpened() const
{
    return opened;
}

inline void Console::SetFont( const BMfont &newFont )
{
    font = newFont;
}

inline const BMfont &Console::GetFont() const
{
    return font;
}

inline Console con;

#endif // CONSOLE_H
