mandelbrot: mandelbrot.c
	gcc mandelbrot.c -o mandelbrot `pkg-config --cflags --libs gtk+-2.0` --std=c99
