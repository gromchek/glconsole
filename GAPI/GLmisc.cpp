#define GLEW_STATIC
#include <GL/glew.h>

#include <cstdio>

#include <iostream>
#include <string>

#include "GLmisc.h"

#include "../Misc.h"

#include "../CmdSystem.h"
#include "../Console.h"

std::string GL_ErrorEnumToString( const GLenum err )
{
    switch( err )
    {
    case GL_INVALID_ENUM:
        return { "GL_INVALID_ENUM" };
    case GL_INVALID_VALUE:
        return { "GL_INVALID_VALUE" };
    case GL_INVALID_OPERATION:
        return { "GL_INVALID_OPERATION" };
    case GL_STACK_OVERFLOW:
        return { "GL_STACK_OVERFLOW" };
    case GL_STACK_UNDERFLOW:
        return { "GL_STACK_UNDERFLOW" };
    case GL_OUT_OF_MEMORY:
        return { "GL_OUT_OF_MEMORY" };
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return { "GL_INVALID_FRAMEBUFFER_OPERATION" };
    default:
        return { "Unknown GL error" };
    }
}

void GL_CheckError_( const char *file, int line )
{
    while( 1 )
    {
        unsigned int err = glGetError();

        if( err == GL_NO_ERROR )
        {
            break;
        }

        std::cerr << "GL error: " << GL_ErrorEnumToString( err ) << " in file " << file << " on line " << line << "\n";
    }
}


void GL_Clear( bool color, bool depth, bool stencil, unsigned char stencilValue, float r, float g, float b, float a )
{
    unsigned int clearFlags = 0;
    if( color )
    {
        glClearColor( r, g, b, a );
        clearFlags |= GL_COLOR_BUFFER_BIT;
    }
    if( depth )
    {
        clearFlags |= GL_DEPTH_BUFFER_BIT;
    }
    if( stencil )
    {
        glClearStencil( stencilValue );
        clearFlags |= GL_STENCIL_BUFFER_BIT;
    }
    glClear( clearFlags );
}

void GL_VertexAttrib( unsigned int index, int size, unsigned int type, int stride, unsigned int offset )
{
    glEnableVertexAttribArray( index );
    glVertexAttribPointer( index, size, type, GL_FALSE, stride, (const void *)( offset ) );
}


void gfxInfo_f( const CommandArgs &args )
{
    con.Printf( "OpenGL version: {}", (const char *)glGetString( GL_VERSION ) );
    con.Printf( "GLSL version: {}", (const char *)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    con.Printf( "OpenGL vendor: {}", (const char *)glGetString( GL_VENDOR ) );
    con.Printf( "OpenGL renderer: {}", (const char *)glGetString( GL_RENDERER ) );
}

void initGLEW()
{
    glewExperimental = GL_TRUE;
    unsigned int res = glewInit();
    if( res != GLEW_OK )
    {
        std::cerr << "GLEW Error: " << glewGetErrorString( res ) << "\n";
    }
}

void GL_setup()
{
    initGLEW();

    glViewport( 0, 0, R_WINDOW_WIDTH, R_WINDOW_HEIGHT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glFrontFace( GL_CCW );

    commandSys.AddCommand( "gfxInfo", gfxInfo_f, "print opengl info" );
}
