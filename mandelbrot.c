#include <gtk/gtk.h>
#include <pthread.h>
#include <stdio.h>

int max(int a, int b) {
    return a > b ? a : b;
}

double square(double x) {
    return x*x;
}

const int WINDOW_WIDTH = 1050;
const int WINDOW_HEIGHT = 600;

void destroy(GtkWidget *widget, gpointer data);
void* make_mandelbrot_pixbuf(void*);
gboolean draw_canvas(GtkWidget *widget, GdkEventExpose *event);
int mandelbrot(int c, int r);

GdkPixbuf* mandelPb = 0;
GtkWidget* canvas;

int main(int argc, char *argv[]) {
    g_thread_init(NULL);
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    GtkWidget* window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "The Mandelbrot Set");
    g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

    mandelPb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, WINDOW_WIDTH, WINDOW_HEIGHT);

    canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, WINDOW_WIDTH, WINDOW_HEIGHT);
    g_signal_connect(canvas, "expose_event", G_CALLBACK(draw_canvas), NULL);

    pthread_t mandelbrot_tid;
    pthread_create(&mandelbrot_tid, NULL, make_mandelbrot_pixbuf, NULL);

    gtk_container_add(GTK_CONTAINER(window), canvas);
    gtk_widget_show_all(window);

    gtk_main();
    gdk_threads_leave();
    return 0;
}

void destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void* make_mandelbrot_pixbuf(void* args) {
    guchar* pbData = gdk_pixbuf_get_pixels(mandelPb);
    gdk_threads_enter();
    GdkGC* gc = gdk_gc_new(canvas->window);
    gdk_threads_leave();
    int row_stride = gdk_pixbuf_get_rowstride(mandelPb);
    for (int r = 0; r < WINDOW_HEIGHT; r++) {
        for (int c = 0; c < WINDOW_WIDTH; c++) {
            int v = mandelbrot(c, r);
            pbData[c*3 + r*row_stride] = v;
            pbData[c*3 + r*row_stride+1] = v;
            pbData[c*3 + r*row_stride+2] = v;
        }
        gdk_threads_enter();
        gdk_draw_pixbuf(canvas->window, gc, mandelPb, 0, r, 0, r, WINDOW_WIDTH, 1, GDK_RGB_DITHER_NONE, 0, 0);
        gdk_flush();
        gdk_threads_leave();
    }
    return NULL;
}

gboolean draw_canvas(GtkWidget *widget, GdkEventExpose *event) {
    GdkGC* gc = gdk_gc_new(widget->window);
    gdk_draw_pixbuf(widget->window, gc, mandelPb, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GDK_RGB_DITHER_NONE, 0, 0);
  return TRUE;
}

double scale_x(int c) {
    return c*3.5/WINDOW_WIDTH - 2.5;
}

double scale_y(int r) {
    return -(r*2.0/WINDOW_HEIGHT - 1.0);
}

int mandelbrot(int c, int r) {
    double x0 = scale_x(c);
    double y0 = scale_y(r);

    // optimization
    double q = square(x0-0.25) + square(y0);
    if (q*(q + (x0-0.25)) < square(y0) / 4 ||
            square(x0+1) + square(y0) < 1.0/16) {
        return 0;
    }

    double x=0, y=0;
    int i = 0;
    while (i < 1000) {
        if (x*x + y*y > 4) break;
        double next_x = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = next_x;
        i++;
    }
    if (i == 1000) {
        return 0;
    } else {
        return max(0, 256-8*i);
    }
}
