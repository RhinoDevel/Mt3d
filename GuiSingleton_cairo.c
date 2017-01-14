
// MT, 2017jan14

#include "GuiSingleton_cairo.h"
#include "Deb.h"

#include <assert.h>
#include <stdlib.h>

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static GtkWidget * darea = NULL;
static cairo_surface_t * image = NULL;
static guint timer_id = 0;
static double scaleFactor = -1.0;
static bool (*keyHandler)(char const) = NULL;
static void (*gameLoop)(void) = NULL;

static gboolean on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{      
    cairo_scale(cr, scaleFactor, scaleFactor);
    cairo_set_source_surface(cr, image, 0, 0); 
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
    cairo_paint(cr);
    
    return FALSE;
}

static void drawFrame()
{
    assert(image!=NULL);
    assert(darea!=NULL);
    
    cairo_surface_flush(image);
    cairo_surface_mark_dirty(image);
    gtk_widget_queue_draw(darea);
}

static gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    if(keyHandler((char)event->keyval)) // MT_TODO: TEST: Cast may be not correct in every case?!
    {
        drawFrame();
        return TRUE;
    }
    return FALSE;
}

/**
 * - See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#GSourceFunc 
 */
static gboolean on_timer_event(gpointer user_data)
{
    gameLoop();
    return G_SOURCE_CONTINUE;//G_SOURCE_REMOVE
}

void GuiSingleton_cairo_init(
    int const inWidth,
    int const inHeight,
    double const inScaleFactor,
    char const * const inWinTitle,
    unsigned char * const inPixels,
    bool (*inKeyHandler)(char const),
    void (*inGameLoop)(void))
{
    assert(darea==NULL);
    assert(image==NULL);
    assert(timer_id==0);
    assert(scaleFactor==-1.0);
    assert(keyHandler==NULL);
    assert(gameLoop==NULL);
    
    GtkWidget * window = NULL;
    
    scaleFactor = inScaleFactor;
    keyHandler = inKeyHandler;
    gameLoop = inGameLoop;
    
    int const stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, inWidth),
        scaledWidth = (int)(scaleFactor*inWidth), // Truncates
        scaledHeight = (int)(scaleFactor*inHeight); // Truncates
    
    assert(stride>0);
    assert(stride==inWidth*4);
    
    image = cairo_image_surface_create_for_data(inPixels, CAIRO_FORMAT_RGB24, inWidth, inHeight, stride);
    gtk_init(0, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER (window), darea);
    
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
    
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), scaledWidth+30, scaledHeight+30); 
    gtk_window_set_title(GTK_WINDOW(window), inWinTitle);
    
    gtk_widget_show_all(window);
    drawFrame();
    
    timer_id = g_timeout_add(1000, &on_timer_event, NULL); // See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#g-timeout-add
    
    gtk_main();
}

void GuiSingleton_cairo_deinit()
{
    g_source_remove(timer_id); // See: https://developer.gnome.org/glib/unstable/glib-The-Main-Event-Loop.html#g-source-remove
    cairo_surface_destroy(image);
    image = NULL;
    darea = NULL; // MT_TODO: TEST: Enough?
    timer_id = 0;
    scaleFactor = -1.0;
    keyHandler = NULL;
    gameLoop = NULL;
}