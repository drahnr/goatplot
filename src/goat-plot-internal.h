#include "goat-plot.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>

static inline void
goat_set_source (cairo_t *cr, GdkRGBA *rgba)
{
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
}

gboolean
draw_axis (GoatPlot *plot, cairo_t *cr, GtkAllocation *allocation)
{
	int i;
	int top, bottom, left, right;
	const int pad_top = 18; //pad 10pixels in all directions
	const int pad_left = 18; //pad 10pixels in all directions
	const int pad_right = 18; //pad 10pixels in all directions
	const int pad_bottom = 18; //pad 10pixels in all directions

	const int width_tick = 10; //major
	const int width_tock = 5; //minor
	const int step_tick = 50; //FIXME calculate this dynamically based on the size
	const int step_tock = 10;

	GdkRGBA color_tick = {0.8, 0., 0., 1.};
	GdkRGBA color_tock = {0., 0., 0., 1.};
	GdkRGBA color_grid_tick = {0.9, 0.9, 0.9, 1.};
	GdkRGBA color_grid_tock = {0.95, 0.95, 0.95, 1.};

	top = pad_top;
	left = pad_left;
	bottom = allocation->height - pad_bottom;
	right = allocation->width - pad_right;

	cairo_rectangle (cr, left, top, right-left, bottom-top);
	cairo_set_source_rgba (cr, 1., 1., 1., 1.);
	cairo_fill (cr);

	cairo_rectangle (cr, left, top, right-left, bottom-top);
	cairo_set_source_rgba (cr, 0., 0., 0., 1.);
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);

#if 1
	//horizontal top
	i = CLAMP((bottom-top)/step_tock, 1, 10000);
	cairo_set_line_width (cr, 1.);
	for (; i>-1; i--) {
		//left
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, left,            top+step_tock*i);
			cairo_line_to (cr, left-width_tick, top+step_tock*i);
			goat_set_source (cr, &color_tick);
		} else {
			cairo_move_to (cr, left,            top+step_tock*i);
			cairo_line_to (cr, left-width_tock, top+step_tock*i);
			goat_set_source (cr, &color_tock);
		}
		cairo_stroke (cr);
		//grid
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, left,  top+step_tock*i);
			cairo_line_to (cr, right, top+step_tock*i);
			goat_set_source (cr, &color_grid_tick);
		} else {
			cairo_move_to (cr, left,  top+step_tock*i);
			cairo_line_to (cr, right, top+step_tock*i);
			goat_set_source (cr, &color_grid_tock);
		}
		cairo_stroke (cr);
		//right
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, right, top+step_tock*i);
			cairo_line_to (cr, right+width_tick, top+step_tock*i);
			goat_set_source (cr, &color_tick);
		} else {
			cairo_move_to (cr, right,            top+step_tock*i);
			cairo_line_to (cr, right+width_tock, top+step_tock*i);
			goat_set_source (cr, &color_tock);
		}
		cairo_stroke (cr);
	}
#endif
#if 1
	//horizontal axis stiple top
	i = CLAMP((right-left)/step_tock, 1, 10000);
	for (; i>-1; i--) {
		//top
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, left+step_tock*i, top);
			cairo_line_to (cr, left+step_tock*i, top-width_tick);
			goat_set_source (cr, &color_tick);
		} else {
			cairo_move_to (cr, left+step_tock*i, top);
			cairo_line_to (cr, left+step_tock*i, top-width_tock);
			goat_set_source (cr, &color_tock);
		}
		cairo_stroke (cr);
		//grid
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, left+step_tock*i, top);
			cairo_line_to (cr, left+step_tock*i, bottom);
			goat_set_source (cr, &color_grid_tick);
		} else {
			cairo_move_to (cr, left+step_tock*i, top);
			cairo_line_to (cr, left+step_tock*i, bottom);
			goat_set_source (cr, &color_grid_tock);
		}
		cairo_stroke (cr);
		//bottom
		if (i*step_tock%step_tick==0) {
			cairo_move_to (cr, left+step_tock*i, bottom);
			cairo_line_to (cr, left+step_tock*i, bottom+width_tick);
			goat_set_source (cr, &color_tick);
		} else {
			cairo_move_to (cr, left+step_tock*i, bottom);
			cairo_line_to (cr, left+step_tock*i, bottom+width_tock);
			goat_set_source (cr, &color_tock);
		}
		cairo_stroke (cr);
	}

#endif
//	cairo_rectangle (cr, left, top, right-left, bottom-top);
//	cairo_clip (cr);

	// reduce the allocation width / height to what is visible
	allocation->width = right - left;
	allocation->height = bottom - top;
	allocation->x = left;
	allocation->y = top;

	// set the clip region to the bounded data region
	cairo_rectangle (cr,
	                 allocation->x, allocation->y,
	                 allocation->width, allocation->height);
	//cairo_fill_preserve (cr);
	cairo_clip (cr);

	return TRUE;
}



/*
 * provides nice numerical limits when scaling an axis
 */
static double
nicenum (double x, int round)
{
	float exp, f, niced;
	float signx = (x >= 0) ? 1.f : -1.f;
	float absx = (float)x * signx;

    exp = floorf (log10f (absx));
    f = absx/powf(10.f, exp); // bounded between 1 and 10
    if (round) {
		if (f<1.5)
			niced = 1.;
		else if (f<3.)
			niced = 2.;
		else if (f<7.)
			niced = 5.;
		else
			niced = 10.;
	} else {
		if (f<=1.)
			niced = 1.;
		else if (f<=2.)
			niced = 2.;
		else if (f<=5.)
			niced = 5.;
		else
			niced = 10.;
	}
	return signx * niced * powf (10.f, exp);
}
