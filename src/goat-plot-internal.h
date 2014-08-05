#include "goat-plot.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>

static inline void
goat_set_source (cairo_t *cr, GdkRGBA *rgba)
{
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
}


typedef enum {
	GOAT_DIRECTION_UNKNOWN = 0,
	GOAT_DIRECTION_HORIZONTAL,
	GOAT_DIRECTION_VERTICAL,
} GoatDirection;

typedef enum {
	GOAT_BORDER_TOP = 1,
	GOAT_BORDER_LEFT = 3,
	GOAT_BORDER_RIGHT = 4,
	GOAT_BORDER_BOTTOM = 2,
} GoatBorderPosition;


/**
 * @param x_nil in pixel
 * @param x_factor convert unit to pixel
 */
gboolean
draw_scale_horizontal (GoatPlot *plot,
                       cairo_t *cr,
                       int left, int right, int top, int bottom,
                       gdouble x_nil,
                       gdouble x_factor,
                       GoatBorderPosition toporbottom,
                       gboolean grid)
{
	g_assert (x_factor != 0.);
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	const double step_minor = (priv->major_delta_x / priv->minors_per_major_x);
	const int register start = (left-x_nil)/step_minor/x_factor;
	const int register end = (right-x_nil)/step_minor/x_factor;
	int register i;
	const gint width_major = priv->width_major_x;
	const gint width_minor = priv->width_minor_x;
	GdkRGBA color_minor = priv->color_minor_y;
	GdkRGBA color_major = priv->color_major_y;

	g_print ("[] right %i   left %i   x_nil %lf\n", right, left, x_nil);
	g_print (">>> start=%i end=%i  %lf x_factor\n", start, end, x_factor);
	cairo_set_line_width (cr, 1.);

	if (toporbottom == GOAT_BORDER_BOTTOM) {
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major_x == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, top);
			if (majorstip) {
				cairo_line_to (cr, x, top-width_major);
				goat_set_source (cr, &color_major);
			} else {
				cairo_line_to (cr, x, top-width_minor);
				goat_set_source (cr, &color_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (grid) {
		GdkRGBA color_grid_major = {0.9, 0.9, 0.9, 1.};
		GdkRGBA color_grid_minor = {0.95, 0.95, 0.95, 1.};
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major_x == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, top);
			cairo_line_to (cr, x, bottom);
			if (majorstip) {
				goat_set_source (cr, &color_grid_major);
			} else {
				goat_set_source (cr, &color_grid_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (toporbottom == GOAT_BORDER_TOP) {
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major_x == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, bottom);
			if (majorstip) {
				cairo_line_to (cr, x, bottom+width_major);
				goat_set_source (cr, &color_major);
			} else {
				cairo_line_to (cr, x, bottom+width_minor);
				goat_set_source (cr, &color_minor);
			}
			cairo_stroke (cr);
		}
	}
	return TRUE;
}

/**
 * @param y_nil in pixel
 * @param y_factor convert unit to pixel
 */
gboolean
draw_scale_vertical (GoatPlot *plot,
                  cairo_t *cr,
                  int left, int right, int top, int bottom,
                  double y_nil,
                  gdouble y_factor,
		          GoatBorderPosition leftorright,
                  gboolean grid)
{
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	const double step_minor = (priv->major_delta_y / priv->minors_per_major_y);
	const int register start = (top-y_nil)/step_minor/y_factor;
	const int register end = (bottom-y_nil)/step_minor/y_factor;
	int register i;
	const gint width_major = priv->width_major_x;
	const gint width_minor = priv->width_minor_x;
	GdkRGBA color_minor = priv->color_minor_x;
	GdkRGBA color_major = priv->color_major_x;

	g_print ("[] bottom %i   top %i   x_nil %lf\n", bottom, top, y_nil);
	g_print ("> start=%i end=%i         %lf y_factor\n", start, end,y_factor);
	cairo_set_line_width (cr, 1.);

	if (leftorright == GOAT_BORDER_LEFT) {
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major_y == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			if (majorstip) {
				cairo_line_to (cr, left-width_major, y);
				goat_set_source (cr, &color_major);
			} else {
				cairo_line_to (cr, left-width_minor, y);
				goat_set_source (cr, &color_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (grid) {

		GdkRGBA color_grid_major = {0.9, 0.9, 0.9, 1.};
		GdkRGBA color_grid_minor = {0.95, 0.95, 0.95, 1.};
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major_y == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			cairo_line_to (cr, right, y);
			if (majorstip) {
				goat_set_source (cr, &color_grid_major);
			} else {
				goat_set_source (cr, &color_grid_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (leftorright == GOAT_BORDER_RIGHT) {
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major_y == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			if (majorstip) {
				cairo_line_to (cr, left-width_major, y);
				goat_set_source (cr, &color_major);
			} else {
				cairo_line_to (cr, left-width_minor, y);
				goat_set_source (cr, &color_minor);
			}
			cairo_stroke (cr);
		}
	}
	return TRUE;
}





gboolean
draw_nil_lines (GoatPlot *plot, cairo_t *cr,
                int width, int height,
                double x_nil_pixel, double y_nil_pixel)
{
	cairo_set_line_width (cr, 1.);
	cairo_set_source_rgba (cr, 0.7, 0., 0., 1.);
	cairo_move_to (cr,    0., y_nil_pixel);
	cairo_line_to (cr, width, y_nil_pixel);
	cairo_move_to (cr, x_nil_pixel, 0.);
	cairo_line_to (cr, x_nil_pixel, height);
	cairo_stroke (cr);
	return TRUE;
}


gboolean
draw_scales (GoatPlot *plot,
             cairo_t *cr,
             GtkAllocation *allocation,
             GtkBorder *padding,
             gdouble x_nil,
             gdouble y_nil,
             gdouble x_unit_to_pixel,
             gdouble y_unit_to_pixel)
{
	int top,left,right,bottom;

	top = allocation->x;
	left = allocation->y;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;

	draw_scale_horizontal (plot, cr,
	                       left, right, top, bottom,
	                       x_nil, x_unit_to_pixel,
	                       GOAT_BORDER_TOP,
	                       TRUE);
	draw_scale_vertical (plot, cr,
                         left, right, top, bottom,
	                     y_nil, y_unit_to_pixel,
	                     GOAT_BORDER_LEFT,
	                     TRUE);
	return TRUE;
}



gboolean
draw_background (GoatPlot *plot,
                 cairo_t *cr,
                 GtkAllocation *allocation,
                 GtkBorder *padding,
                 gdouble x_nil,
                 gdouble y_nil,
                 gdouble x_factor,
                 gdouble y_factor)
{
	int top, bottom, left, right;

	top = allocation->x;
	left = allocation->y;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;


	cairo_rectangle (cr, left, top, right-left, bottom-top);
	cairo_set_source_rgba (cr, 1., 1., 1., 1.);
	cairo_fill (cr);


	cairo_rectangle (cr, left, top, right-left, bottom-top);
	cairo_set_source_rgba (cr, 0., 0., 0., 1.);
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);

	return TRUE;
}

gboolean
clip_drawable_area (GoatPlot *plot,
                 cairo_t *cr,
                 GtkAllocation *allocation,
                 GtkBorder *padding)
{
	int top, bottom, left, right;

	top = allocation->x;
	left = allocation->y;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;


	cairo_rectangle (cr, left, top, right-left, bottom-top);
	cairo_clip (cr);

	return TRUE;
}


/**
 * nice upper bound
 */


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
