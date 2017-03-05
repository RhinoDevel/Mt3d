
// MT, 2017jan14

#include "GuiSingleton_cairo.h"
#include "Deb.h"
#include "Dim.h"

#include <assert.h>
#include <stdlib.h>

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static GtkWidget * window = NULL;
static GtkWidget * darea = NULL;
static cairo_surface_t * image = NULL;
static guint timer_id = 0;
static double scaleFactor = -1.0;
static struct Dim dim = { .w = 0, .h = 0 };
static double fullScreenScaleFactor = -1.0;
static void (*keyPressHandler)(char const) = NULL;
static void (*keyReleaseHandler)(char const) = NULL;
static void (*timerHandler)(void) = NULL;
static bool fullscreen = false;

static gboolean on_window_state_event(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    if((((GdkEventWindowState*)event)->changed_mask&GDK_WINDOW_STATE_FULLSCREEN) !=0)
    {
        if((((GdkEventWindowState*)event)->new_window_state&GDK_WINDOW_STATE_FULLSCREEN)==0)
        {
            Deb_line("Disabled full screen.")
            fullscreen = false;
            
            fullScreenScaleFactor = -1.0;
            gtk_widget_set_margin_top(darea, 0);
            gtk_widget_set_margin_start(darea, 0);
        }
        else
        {
            Deb_line("Enabled full screen.")
            fullscreen = true;
            
            {
                gint winWidth = 0,
                    winHeight = 0;
                
                gtk_window_get_size((GtkWindow*)window, &winWidth, &winHeight);
                
                struct Dim const winDim = { .w = (int)winWidth, .h = (int)winHeight },
                    scaledDim = Dim_getScaledInto(&winDim, &dim);

                fullScreenScaleFactor = (double)scaledDim.h/dim.h;
                
                gtk_widget_set_margin_top(darea, (winDim.h-scaledDim.h)/2);
                gtk_widget_set_margin_start(darea, (winDim.w-scaledDim.w)/2);
            }
        }
    }
    
    return FALSE;
}

static gboolean on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{      
    double const scaleFactorToUse = fullscreen?fullScreenScaleFactor:scaleFactor;
    
    cairo_scale(cr, scaleFactorToUse, scaleFactorToUse);
    cairo_set_source_surface(cr, image, 0, 0); 
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
    cairo_paint(cr);
    
    return FALSE;
}

static gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    keyPressHandler((char)event->keyval); // MT_TODO: TEST: Cast may be not correct in every case?!
    return TRUE;
}
static gboolean on_key_release(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    keyReleaseHandler((char)event->keyval); // MT_TODO: TEST: Cast may be not correct in every case?!
    return TRUE;
}

/**
 * - See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#GSourceFunc 
 */
static gboolean on_timer_event(gpointer user_data)
{
    timerHandler();
    return G_SOURCE_CONTINUE;//G_SOURCE_REMOVE
}

void GuiSingleton_cairo_prepareForDirectDraw()
{
    assert(image!=NULL);
    cairo_surface_flush(image);
}

void GuiSingleton_cairo_draw()
{
    assert(image!=NULL);
    assert(darea!=NULL);

    cairo_surface_mark_dirty(image);
    gtk_widget_queue_draw(darea);
}

void GuiSingleton_cairo_toggleFullscreen()
{
    if(fullscreen)
    {
        gtk_window_unfullscreen((GtkWindow*)window);
    }
    else
    {
        gtk_window_fullscreen((GtkWindow*)window);
    }
}

void GuiSingleton_cairo_quit()
{
    gtk_main_quit();
}

void GuiSingleton_cairo_init(
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
    assert(window==NULL);
    assert(darea==NULL);
    assert(image==NULL);
    assert(timer_id==0);
    assert(dim.w==0&&dim.h==0);
    assert(scaleFactor==-1.0);
    assert(fullScreenScaleFactor==-1.0);
    assert(keyPressHandler==NULL);
    assert(keyReleaseHandler==NULL);
    assert(timerHandler==NULL);
    
    scaleFactor = inScaleFactor;
    dim.w = inWidth;
    dim.h = inHeight;
    keyPressHandler = inKeyPressHandler;
    keyReleaseHandler = inKeyReleaseHandler;
    
    timerHandler = inTimerHandler;
    
    int const stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, dim.w),
        scaledWidth = (int)(scaleFactor*dim.w), // Truncates
        scaledHeight = (int)(scaleFactor*dim.h); // Truncates
    
    assert(stride>0);
    assert(stride==dim.w*4);
    
    image = cairo_image_surface_create_for_data((unsigned char *)inPixels, CAIRO_FORMAT_RGB24, dim.w, dim.h, stride);
    gtk_init(0, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER (window), darea);
    
    g_signal_connect(G_OBJECT (window), "window-state-event", G_CALLBACK (on_window_state_event), NULL);
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
    g_signal_connect(G_OBJECT (window), "key_release_event", G_CALLBACK (on_key_release), NULL);
    
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), scaledWidth+30, scaledHeight+30); 
    gtk_window_set_title(GTK_WINDOW(window), inWinTitle);
    
    gtk_widget_show_all(window);
    GuiSingleton_cairo_draw();
    
    timer_id = g_timeout_add((guint)inTimerInterval, &on_timer_event, NULL); // See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#g-timeout-add

    gtk_main();
}

void GuiSingleton_cairo_deinit()
{
    g_source_remove(timer_id); // See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#g-source-remove
    cairo_surface_destroy(image);
    image = NULL;
    darea = NULL; // MT_TODO: TEST: Enough?
    window = NULL; // MT_TODO: TEST: Enough?
    timer_id = 0;
    scaleFactor = -1.0;
    dim.w = 0;
    dim.h = 0;
    fullScreenScaleFactor = -1.0;
    keyPressHandler = NULL;
    keyReleaseHandler = NULL;
    timerHandler = NULL;
}