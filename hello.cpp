/*#include <functional>

#include <emscripten.h>
#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

// Shader sources
const GLchar* vertexSource =
    "attribute vec4 position;                     \n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_Position = vec4(position.xyz, 1.0);     \n"
    "}                                            \n";
const GLchar* fragmentSource =
    "precision mediump float;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor[0] = gl_FragCoord.x/640.0;    \n"
    "  gl_FragColor[1] = gl_FragCoord.y/480.0;    \n"
    "  gl_FragColor[2] = 0.5;                     \n"
    "}                                            \n";

// an example of something we will control from the javascript side
bool background_is_black = false;

// the function called by the javascript code
extern "C" void EMSCRIPTEN_KEEPALIVE toggle_background_color() { background_is_black = !background_is_black; }

std::function<void()> loop;
void main_loop() { loop(); }

int main()
{
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, nullptr);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    loop = [&]
    {
        // move a vertex
        const uint32_t milliseconds_since_start = SDL_GetTicks();
        const uint32_t milliseconds_per_loop = 3000;
        vertices[0] = ( milliseconds_since_start % milliseconds_per_loop ) / float(milliseconds_per_loop) - 0.5f;
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Clear the screen
        if( background_is_black )
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        else
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    };

    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}*/

/*
#include <assert.h>
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SDL_Window * window;
SDL_Renderer * renderer;

int done = 0; // Main loop flag (in desktop mode)
int frameCount = 0;

void render()
{
    int w,h;
    SDL_GetWindowSize(window, &w,&h);

    // Clear background
    SDL_SetRenderDrawColor(renderer, 10, 25, 50, 255);
    SDL_RenderClear(renderer);

    // Draw some lines
    frameCount +=1;
    for (int r=frameCount %20; r<w; r+=20)
    {
        SDL_SetRenderDrawColor(renderer, 64-r/20, 64-r/10, 255-r/3, 255);
        SDL_RenderDrawLine(renderer, 0, 0, r, h);
        SDL_RenderDrawLine(renderer, w, h, w-r,0);
    }
    SDL_RenderPresent(renderer);
}

void mainloop_func(void)
{
    // Draw our Scene
    render();
    // Check for SDL Events
    SDL_Event event;
    while( SDL_PollEvent( &event ) )
    {
        switch(event.type)
        {
        case SDL_QUIT:
            done = 1;
            break;
        default:
            printf("[%u] Event Type=%d\n", SDL_GetTicks(), event.type);  //to stdout
            //SDL_Log("[%u] Event Type=%d", SDL_GetTicks(), event.type); //to JS console
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    // Initialize SDL ideo subsystem and create a 640x480 window, and a renderer
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
#ifdef EMSCRIPTEN
    // Setup periodic call of our mainloop. Framerate is controlled by the web browser.
    emscripten_set_main_loop(mainloop_func, 0, 0);
    // Exit main() but keep web application running
    emscripten_exit_with_live_runtime();
#else
    // In desktop mode, we call continuously our main loop function, and wait 10mili seconds.
    while(done==0)
    {
        mainloop_func();
        SDL_Delay(10);
    }
#endif
    // This is actually never executed with EMSCRIPTEN enabled
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}*/


/*
 * Converted to C code taken from:
 * https://github.com/timhutton/sdl-canvas-wasm
 * Some of the variable names and comments were also
 * slightly updated.
 */
#include <SDL2/SDL.h>
#include <emscripten.h>
#include <stdlib.h>

// This enables us to have a single point of reference
// for the current iteration and renderer, rather than
// have to refer to them separately.
typedef struct Context
{
	SDL_Renderer *renderer;
	int iteration;
} Context;


 //Looping function that draws a blue square on a red
 //background and moves it across the <canvas>.

void mainloop(void *arg)
{
	Context *ctx = (Context *)arg;
	SDL_Renderer *renderer = ctx->renderer;
	int iteration = ctx->iteration;

	// This sets the background color to red:
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);

	// This creates the moving blue square, the rect.x
	// and rect.y values update with each iteration to move
	// 1px at a time, so the square will move down and
	// to the right infinitely:
	SDL_Rect rect;
	rect.x = iteration;
	rect.y = iteration;
	rect.w = 50;
	rect.h = 50;
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderFillRect(renderer, &rect);

	SDL_RenderPresent(renderer);

	// This resets the counter to 0 as soon as the iteration
	// hits the maximum canvas dimension (otherwise you'd
	// never see the blue square after it travelled across
	// the canvas once).
	if (iteration == 200)
	{
		ctx->iteration = 0;
	}
	else
	{
		ctx->iteration++;
	}
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window;
	SDL_Renderer *renderer;

	// The first two 255 values represent the size of the <canvas>
	// element in pixels.
	SDL_CreateWindowAndRenderer(255, 255, 0, &window, &renderer);

	Context ctx;
	ctx.renderer = renderer;
	ctx.iteration = 0;

	// Call the function repeatedly:
	int infinite_loop = 1;

	// Call the function as fast as the browser wants to render
	// (typically 60fps):
	int fps = -1;

	// This is a function from emscripten.h, it sets a C function
	// as the main event loop for the calling thread:
	emscripten_set_main_loop_arg(mainloop, &ctx, fps, infinite_loop);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif

void EMSCRIPTEN_KEEPALIVE myFunction()
{
	printf("FUNCTION CALLED");
}

#ifdef __cplusplus
}
#endif