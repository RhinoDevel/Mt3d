
// MT, 2017mar12

#include "GuiSingleton_sdl.h"
#include "Deb.h"
#include "Dim.h"

#include <assert.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define USER_EVENT_TIMER 1
#define USER_EVENT_DONE 2
static int const userEventTimer = USER_EVENT_TIMER;
static int const userEventDone = USER_EVENT_DONE;

static SDL_Window * win = NULL;
static SDL_Surface * winSurface = NULL;
static SDL_Surface * imgSurface = NULL;
static SDL_TimerID timerId = 0;

static double scaleFactor = -1.0;
static struct Dim dim = { .w = 0, .h = 0 };
static double fullScreenScaleFactor = -1.0;
static void (*keyPressHandler)(char const) = NULL;
static void (*keyReleaseHandler)(char const) = NULL;
static void (*timerHandler)(void) = NULL;
static bool fullscreen = false;

static Uint32 timerCallback(Uint32 interval, void * param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    // Callback pushes an SDL_USEREVENT event into the queue
    // and causes callback to be called again at the same interval.

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = (void*)&userEventTimer;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}


static void eventQueue()
{
    bool done = false;
    
    while(!done)
    {
        SDL_Event e;
        
        while(SDL_PollEvent(&e)!=0)
        {
            switch(e.type)
            {
                case SDL_KEYDOWN:
                    keyPressHandler((char)e.key.keysym.sym); // MT_TODO: TEST: Cast may be not correct in every case?!
                    break;
                case SDL_KEYUP:
                    keyReleaseHandler((char)e.key.keysym.sym); // MT_TODO: TEST: Cast may be not correct in every case?!
                    break;
                case SDL_USEREVENT:
                    switch(*((int*)e.user.data1))
                    {
                        case USER_EVENT_TIMER:
                            timerHandler();
                            break;
                        case USER_EVENT_DONE:
                            done = true;
                            break;
                            
                        default:
                            assert(false);
                            break;
                    }
                    
                    break;
                default:
                    break;
            }
        }
    }
}


void GuiSingleton_sdl_prepareForDirectDraw()
{
}

void GuiSingleton_sdl_draw()
{
    assert(imgSurface!=NULL);
    assert(winSurface!=NULL);
    assert(win!=NULL);
    
    assert(!fullscreen); // MT_TODO: TEST: Implement!
    SDL_Rect imgRect = (SDL_Rect)
        {
            .x = 0,
            .y = 0,
            .w = dim.w,
            .h = dim.h
        },
        winRect = (SDL_Rect)
        {
            .x = 0,
            .y = 0,
            .w = (int)(scaleFactor*(double)dim.w), // Truncates
            .h = (int)(scaleFactor*(double)dim.h) // Truncates
        };
    
    //SDL_BlitSurface(imgSurface, NULL, winSurface, NULL);
    SDL_BlitScaled(imgSurface,
        &imgRect,
        winSurface,
        &winRect);
    SDL_UpdateWindowSurface(win);
}

void GuiSingleton_sdl_toggleFullscreen()
{
    // MT_TODO: TEST: Implement!
}

void GuiSingleton_sdl_quit()
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = (void*)&userEventDone;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
}

void GuiSingleton_sdl_init(
    int const inWidth,
    int const inHeight,
    double const inScaleFactor,
    char const * const inWinTitle,
    uint32_t * const inPixels,
    void (*inKeyPressHandler)(char const),
    void (*inKeyReleaseHandler)(char const),
    void (*inTimerHandler)(void),
    int const inTimerInterval)
{
    assert(win==NULL);
    assert(winSurface==NULL);
    assert(imgSurface==NULL);
    assert(timerId==0);
    
    assert(dim.w==0&&dim.h==0);
    assert(scaleFactor==-1.0);
    assert(fullScreenScaleFactor==-1.0);
    assert(keyPressHandler==NULL);
    assert(keyReleaseHandler==NULL);
    assert(timerHandler==NULL);
    
#ifdef NDEBUG
    SDL_InitSubSystem(SDL_INIT_VIDEO); // Automatically initializes the events subsystem.
#else //NDEBUG
    assert(SDL_InitSubSystem(SDL_INIT_VIDEO)==0);
#endif //NDEBUG

#if SDL_BYTEORDER==SDL_BIG_ENDIAN
static Uint32 const rMask = 0x0000FF00,
    gMask = 0x00FF0000,
    bMask = 0xFF000000,
    aMask = 0x000000FF;
#else // Little endian (like x86):
static Uint32 const rMask = 0x00FF0000,
    gMask = 0x0000FF00,
    bMask = 0x000000FF,
    aMask = 0xFF000000;
#endif

    scaleFactor = inScaleFactor;
    dim.w = inWidth;
    dim.h = inHeight;
    keyPressHandler = inKeyPressHandler;
    keyReleaseHandler = inKeyReleaseHandler;
    
    timerHandler = inTimerHandler;
    
    int const stride = dim.w*4,
        scaledWidth = (int)(scaleFactor*dim.w), // Truncates
        scaledHeight = (int)(scaleFactor*dim.h); // Truncates

    win = SDL_CreateWindow(
        inWinTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        scaledWidth+30,
        scaledHeight+30,
        SDL_WINDOW_SHOWN/*SDL_WINDOW_FULLSCREEN*/);
    assert(win!=NULL);
    
    imgSurface = SDL_CreateRGBSurfaceFrom(
        (void*)inPixels,
        dim.w,
        dim.h,
        32,
        stride,
        rMask,
        gMask,
        bMask,
        aMask);
    assert(imgSurface!=NULL);
        
    winSurface = SDL_GetWindowSurface(win);

    GuiSingleton_sdl_draw();
    
#ifdef NDEBUG
    SDL_InitSubSystem(SDL_INIT_TIMER);
#else //NDEBUG
    assert(SDL_InitSubSystem(SDL_INIT_TIMER)==0);
#endif //NDEBUG
    timerId = SDL_AddTimer((Uint32)inTimerInterval, timerCallback, NULL);
    
    eventQueue();
}

void GuiSingleton_sdl_deinit()
{
    SDL_RemoveTimer(timerId);
    timerId = 0;
    SDL_DestroyWindow(win);
    win = NULL;
    SDL_FreeSurface(winSurface);
    winSurface = NULL;
    //SDL_FreeSurface(imgSurface); // Does not seem to be necessary.
    imgSurface = NULL;

    SDL_Quit();

    scaleFactor = -1.0;
    dim.w = 0;
    dim.h = 0;
    fullScreenScaleFactor = -1.0;
    keyPressHandler = NULL;
    keyReleaseHandler = NULL;
    timerHandler = NULL;
}