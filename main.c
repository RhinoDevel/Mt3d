
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "Mt3d.h"
#include "MapSample.h"
#include "Sys.h"

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static int const WIDTH = 1280;
static int const HEIGHT = 960;
static int const ALPHA = 64;
static int const BETA = 40;
static double const H = 0.3;

static struct Mt3d * o = NULL;

static struct 
{
  GtkWidget* darea;
  cairo_surface_t* image;  
} glob;

static void do_drawing(cairo_t* cr)
{
  cairo_set_source_surface(cr, glob.image, 0, 0);
  cairo_paint(cr);    
}

static gboolean on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{      
  do_drawing(cr);

  return FALSE;
}

gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    gboolean retVal = FALSE;
    
    switch (event->keyval)
    {
        case GDK_KEY_a:
            o->gamma = o->gamma+13.0;
            retVal = TRUE;
            break;
        case GDK_KEY_d:
            o->gamma = o->gamma-13.0;
            retVal = TRUE;
            break;

        default:
            break;
    }
    if(retVal==TRUE)
    {
        if(o->gamma<0.0)
        {
            o->gamma = 360.0+o->gamma;
        }
        else
        {
            if(o->gamma>=360.0)
            {
                o->gamma -= 360.0;
            }
        }
        
        cairo_surface_flush (glob.image);
        Mt3d_draw(o);
        cairo_surface_mark_dirty(glob.image);
        gtk_widget_queue_draw(glob.darea);

        int const playerX = (int)o->posX,
            playerY = (int)o->posY;

        Map_print(o->map, &playerX, &playerY);
    }

    return retVal; 
}

int main(int argc, char *argv[])
{    
    o = Mt3d_create(WIDTH, HEIGHT, ALPHA, BETA, H);
    
    o->map = MapSample_create();
    assert(sizeof *o->pixels==1);
    o->pixels = malloc(WIDTH*HEIGHT*4*sizeof *o->pixels);
    assert(o->pixels!=NULL);
    o->posX = o->map->posX;
    o->posY = o->map->posY;
    o->gamma = o->map->gamma;

    for(int row = 0, col = 0;row<HEIGHT;++row)
    {
        uint32_t * const rowPix = ((uint32_t*)o->pixels)+row*WIDTH;

        for(col = 0;col<WIDTH;++col)
        {
            uint8_t * const colPix = (uint8_t*)(rowPix+col);

            colPix[0] = 0xFF; // Blue
            colPix[1] = 0x0; // Green
            colPix[2] = 0xFF; // Red
            //colPix[3] = 0xFF; // (unused)
        }
    }
    
    GtkWidget* window;
    int const stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, WIDTH);
    assert(stride>0);
    assert(stride==WIDTH*4);
    glob.image = cairo_image_surface_create_for_data (o->pixels, CAIRO_FORMAT_RGB24, WIDTH, HEIGHT, stride);
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    glob.darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER (window), glob.darea);
    g_signal_connect(G_OBJECT(glob.darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), WIDTH+30, HEIGHT+30); 
    gtk_window_set_title(GTK_WINDOW(window), "MT 3D");
    gtk_widget_show_all(window);
    gtk_main();
    cairo_surface_destroy(glob.image);
    
    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    return EXIT_SUCCESS;
}
