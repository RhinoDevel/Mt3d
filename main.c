
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Mt3d.h"
#include "MapSample.h"
#include "Sys.h"
#include "Calc.h"

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static int const WIDTH = 320;
static int const HEIGHT = 200;
static double const SCALE_FACTOR = 4.0;
static double const ALPHA = CALC_TO_RAD(45.0);
static double const ALPHA_MIN = CALC_TO_RAD(20.0);
static double const ALPHA_MAX = CALC_TO_RAD(160.0);
static double const ALPHA_STEP = CALC_TO_RAD(5.0);
static double const H = 0.3; // As part of room height (e.g. 0.5 = 50% of room height).
static double const H_MIN = 0.1;
static double const H_MAX = 0.9;
static double const H_STEP = 0.1;

static struct Mt3d * o = NULL;

static struct 
{
  GtkWidget* darea;
  cairo_surface_t* image;  
} glob;

static double getBeta(double const inAlpha)
{
    return 2.0*atan((double)HEIGHT*tan(inAlpha/2.0)/(double)WIDTH);
}

static void drawFrame()
{
    cairo_surface_flush (glob.image);
    Mt3d_draw(o);
    cairo_surface_mark_dirty(glob.image);
    gtk_widget_queue_draw(glob.darea);

    int const playerX = (int)o->posX,
        playerY = (int)o->posY;

    Map_print(o->map, &playerX, &playerY);
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
            retVal = Mt3d_ang_leftOrRight(o, true); // (implicit "cast")
            break;
        case GDK_KEY_d:
            retVal = Mt3d_ang_leftOrRight(o, false); // (implicit "cast")
            break;
            
        case GDK_KEY_w:
            retVal = Mt3d_pos_forwardOrBackward(o, true); // (implicit "cast")
            break;
        case GDK_KEY_s:
            retVal = Mt3d_pos_forwardOrBackward(o, false); // (implicit "cast")
            break;
            
        case GDK_KEY_q:
            retVal = Mt3d_pos_leftOrRight(o, true); // (implicit "cast")
            break;
        case GDK_KEY_e:
            retVal = Mt3d_pos_leftOrRight(o, false); // (implicit "cast")
            break;
           
        case GDK_KEY_l:
        {
            double h = o->h+H_STEP;
            
            if(h>H_MAX)
            {
                h = H_MIN;
            }
            
            Mt3d_update(o->alpha, o->beta, h, o);
            retVal = TRUE;
            break;
        }
        case GDK_KEY_k:
         {
            double h = o->h-H_STEP;
            
            if(h<H_MIN)
            {
                h = H_MAX;
            }
            
            Mt3d_update(o->alpha, o->beta, h, o);
            retVal = TRUE;
            break;
        }
        
        case GDK_KEY_p:
        {
            double alpha = o->alpha+ALPHA_STEP;
            
            if(alpha>ALPHA_MAX)
            {
                alpha = ALPHA_MIN;
            }
            
            Mt3d_update(alpha, getBeta(alpha), o->h, o);
            retVal = TRUE;
            break;
        }
        case GDK_KEY_o:
        {
            double alpha = o->alpha-ALPHA_STEP;
            
            if(alpha<ALPHA_MIN)
            {
                alpha = ALPHA_MAX;
            }
            
            Mt3d_update(alpha, getBeta(alpha), o->h, o);
            retVal = TRUE;
            break;
        }
            
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
    o = Mt3d_create(WIDTH, HEIGHT, ALPHA, getBeta(ALPHA), H);
    
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
    int const scaledWidth = SCALE_FACTOR*WIDTH,
        scaledHeight = SCALE_FACTOR*HEIGHT;
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
    gtk_window_set_default_size(GTK_WINDOW(window), scaledWidth+30, scaledHeight+30); 
    gtk_window_set_title(GTK_WINDOW(window), "MT 3D");
    gtk_widget_show_all(window);
    drawFrame();
    gtk_main();
    cairo_surface_destroy(glob.image);
    
    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    return EXIT_SUCCESS;
}
