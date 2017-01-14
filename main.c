
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Mt3dSingleton.h"

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static double const SCALE_FACTOR = 4.0;

static struct 
{
  GtkWidget* darea;
  cairo_surface_t* image;  
} glob;

static void drawFrame()
{
    cairo_surface_flush (glob.image);
    
    Mt3dSingleton_draw();
    
    cairo_surface_mark_dirty(glob.image);
    gtk_widget_queue_draw(glob.darea);
}

static void do_drawing(cairo_t* cr)
{
    cairo_scale(cr, SCALE_FACTOR, SCALE_FACTOR);
    cairo_set_source_surface(cr, glob.image, 0, 0); 
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
    cairo_paint(cr);    
}

static gboolean on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{      
    do_drawing(cr);
    return FALSE;
}

static gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    gboolean retVal = FALSE;

    switch (event->keyval)
    {
        case GDK_KEY_a:
            retVal = Mt3dSingleton_ang_left(); // (implicit "cast")
            break;
        case GDK_KEY_d:
            retVal = Mt3dSingleton_ang_right(); // (implicit "cast")
            break;
        case GDK_KEY_w:
            retVal = Mt3dSingleton_pos_forward(); // (implicit "cast")
            break;
        case GDK_KEY_s:
            retVal = Mt3dSingleton_pos_backward(); // (implicit "cast")
            break;  
        case GDK_KEY_q:
            retVal = Mt3dSingleton_pos_left(); // (implicit "cast")
            break;
        case GDK_KEY_e:
            retVal = Mt3dSingleton_pos_right(); // (implicit "cast")
            break;  
        case GDK_KEY_l:
            retVal = Mt3dSingleton_pos_up(); // (implicit "cast")
            break;
        case GDK_KEY_k:
            retVal = Mt3dSingleton_pos_down(); // (implicit "cast")
            break;
        case GDK_KEY_p:
            retVal = Mt3dSingleton_fov_wider(); // (implicit "cast")
            break;
        case GDK_KEY_o: 
            retVal = Mt3dSingleton_fov_narrower(); // (implicit "cast")
            break;
            
        default:
            break;
    }
    if(retVal==TRUE)
    {
        drawFrame();
    }
    return retVal; 
}

int main(int argc, char *argv[])
{
    Mt3dSingleton_init();
    
    GtkWidget* window;
    
    int const width = Mt3dSingleton_getWidth(),
        height = Mt3dSingleton_getHeight(),
        stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width),
        scaledWidth = SCALE_FACTOR*width,
        scaledHeight = SCALE_FACTOR*height;
    assert(stride>0);
    assert(stride==width*4);
    glob.image = cairo_image_surface_create_for_data(Mt3dSingleton_getPixels(), CAIRO_FORMAT_RGB24, width, height, stride);
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    glob.darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER (window), glob.darea);
    g_signal_connect(G_OBJECT(glob.darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), scaledWidth+30, scaledHeight+30); 
    gtk_window_set_title(GTK_WINDOW(window), "MT 3D");
    gtk_widget_show_all(window);
    drawFrame();
    gtk_main();
    cairo_surface_destroy(glob.image);
    
    Mt3dSingleton_deinit();
    return EXIT_SUCCESS;
}
