#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "GAPI/GLmisc.h"
#include "GAPI/BufferObject.h"
#include "GAPI/Vertex.h"
#include "GAPI/RenderProgram.h"
#include "GAPI/Texture.h"

#include "Misc.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#include "Misc.h"
#include <memory>
#include <iostream>
#include <vector>

#include "BMfont.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thrid_party/stb_image.h"


#include "Console.h"
#include "CmdSystem.h"
#include "CVarSystem.h"

using SDL2_WindowPtr = std::unique_ptr<SDL_Window, void ( * )( SDL_Window * )>;
using SDL2_GLContextPtr = std::unique_ptr<void, void ( * )( SDL_GLContext )>;

struct SDL2_Lib final
{
    SDL2_Lib()
    {
        SDL_SetMainReady();
        SDL_Init( SDL_INIT_VIDEO );
    }

    ~SDL2_Lib()
    {
        SDL_Quit();
    }
};

struct Game
{
    bool running = true;

    std::uint32_t dt = 0;
    std::uint32_t thisTime = 0;
    std::uint32_t lastTime = 0;
};

namespace
{
const std::string sdfVert = R"(
#version 330 core

#define VERT_POSITION 0
#define VERT_TEXCOORD 1
#define VERT_COLOR 2

layout(location = VERT_POSITION) in vec2 inVertex;
layout(location = VERT_TEXCOORD) in vec2 inTexCoord;
layout(location = VERT_COLOR) in vec4 inColor;

uniform mat4 projView;
uniform mat4 model;

out vec2 TexCoord;
out vec4 Color;

void main()
{
    gl_Position = projView * model * vec4( inVertex, 0.0, 1.0 );
    TexCoord = inTexCoord;
    Color = inColor;
})";

const std::string sdfFrag = R"(
#version 330 core

#define FRAG_OUTPUT_BUFFER0 0
uniform sampler2D texture0;

in vec2 TexCoord;
in vec4 Color;

layout( location = FRAG_OUTPUT_BUFFER0 ) out vec4 frag_color;

float contour(in float d, in float w)
{
    // smoothstep(lower edge0, upper edge1, x)
    return smoothstep(0.5 - w, 0.5 + w, d);
}

float samp(in vec2 uv, float w)
{
    return contour(texture2D(texture0, uv).r, w);
}

float linearstep(float a, float b, float x)
{
    return clamp((x - a) / (b - a), 0.0, 1.0);
}

void main()
{
    vec4 VertColor = Color / 255.0;
    float distance = texture2D( texture0, TexCoord).r;
    //    float smoothWidth = fwidth( distance );
    //    float smoothWidth = 1.0/16.0;
    float smoothWidth = 0.75 * length(vec2(dFdx(distance), dFdy(distance)));
    float alpha = smoothstep( 0.5 - smoothWidth, 0.5 + smoothWidth, distance );

    //frag_color = vec4( vec3( alpha ), alpha ) * VertColor;

    float dscale = 0.354; // half of 1/sqrt2; you can play with this
    vec2 duv = dscale * (dFdx(TexCoord) + dFdy(TexCoord));
    vec4 box = vec4(TexCoord-duv, TexCoord+duv);

    float asum = samp( box.xy, smoothWidth )
               + samp( box.zw, smoothWidth )
               + samp( box.xw, smoothWidth )
               + samp( box.zy, smoothWidth );
    alpha = (alpha + 0.5 * asum) / 3.0;

    frag_color = vec4( VertColor.rgb, alpha * VertColor.a );
})";
} // namespace

int main( int argc, char *argv[] )
{
    SDL2_Lib sdlLib;

    SDL2_WindowPtr handle( nullptr, SDL_DestroyWindow );
    SDL2_GLContextPtr glContext( nullptr, SDL_GL_DeleteContext );

    handle.reset( SDL_CreateWindow( "glconsole", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, R_WINDOW_WIDTH,
                                    R_WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL ) );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    glContext.reset( SDL_GL_CreateContext( handle.get() ) );

    SDL_GL_MakeCurrent( handle.get(), glContext.get() );

    SDL_GL_SetSwapInterval( 1 );

    GL_setup();

    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char *image = stbi_load( "base//consolas.sdf.png", &w, &h, &comp, STBI_rgb_alpha );

    if( !image )
    {
        std::cerr << "Error load texture\n";
        return 0;
    }

    Texture texture;
    texture.LoadFromMemory( image, w, h, true, true );
    texture.Bind( 0 );
    stbi_image_free( image );

    Game game;

    mat4 proj = glm::ortho( 0.0f, float( R_WINDOW_WIDTH ), float( R_WINDOW_HEIGHT ), 0.0f, -100.0f, 100.0f );
    mat4 view = mat4( 1.0f );
    mat4 projView = proj * view;

    RenderProgram renderProg;
    renderProg.CreateShaderProgramRaw( sdfVert, sdfFrag );
    renderProg.SetUniformMatrix4f( "projView", projView );
    renderProg.SetUniformMatrix4f( "model", mat4( 1.0f ) );
    renderProg.Bind();


    con.Init();
    cvarSystem.Init();
    CVar r_fov( "r_fov", "90.0", "field of view" );


    auto font = LoadBitmapFont( "base//consolas.json" );
    con.SetFont( font );

    while( game.running )
    {
        game.thisTime = SDL_GetTicks();
        game.dt = game.thisTime - game.lastTime;

        SDL_Event e;

        while( SDL_PollEvent( &e ) )
        {
            if( e.type == SDL_QUIT || ( e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE ) )
            {
                game.running = false;
            }

            con.HandleEvent( e );
        }

        con.Update( game.dt );
        commandSys.ExecuteCommandBuffer();

        GL_Clear( true, false, false, 0, 0.19f, 0.19f, 0.19f, 1.0f );

        con.Draw();

        SDL_GL_SwapWindow( handle.get() );

        GL_CheckError();

        game.lastTime = game.thisTime;
    }

    return 0;
}
