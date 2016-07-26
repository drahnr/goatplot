
#include "goat-plot.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <math.h>




gboolean draw_nil_lines (GoatPlot *plot, cairo_t *cr, int width, int height, double x_nil_pixel, double y_nil_pixel)
{
	cairo_set_line_width (cr, 1.);
	cairo_set_source_rgba (cr, 0.7, 0., 0., 1.);
	cairo_move_to (cr, 0., y_nil_pixel);
	cairo_line_to (cr, width, y_nil_pixel);
	cairo_move_to (cr, x_nil_pixel, 0.);
	cairo_line_to (cr, x_nil_pixel, height);
	cairo_stroke (cr);
	return TRUE;
}


gboolean draw_scales (GoatPlot *plot, cairo_t *cr, GtkAllocation *allocation, GtkBorder *padding, gdouble x_nil,
                      gdouble y_nil, gdouble x_unit_to_pixel, gdouble y_unit_to_pixel)
{
	int top, left, right, bottom;

	top = 0;
	left = 0;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;

	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);

	g_assert (priv);
	g_assert (priv->scale_x);
	g_assert (GOAT_IS_SCALE (priv->scale_x));
	g_assert (priv->scale_y);
	g_assert (GOAT_IS_SCALE (priv->scale_y));


	g_printf ("scale x : left=%i right=%i top=%i bottom=%i nil=%lf u2pix=%lf\n", left, right, top, bottom, x_nil,
	          x_unit_to_pixel);

	goat_scale_draw (priv->scale_x, cr, left, right, top, bottom, x_nil, x_unit_to_pixel);

	g_printf ("scale y : left=%i right=%i top=%i bottom=%i nil=%lf u2pix=%lf\n", left, right, top, bottom, y_nil,
	          y_unit_to_pixel);
	goat_scale_draw (priv->scale_y, cr, left, right, top, bottom, y_nil, y_unit_to_pixel);
	return TRUE;
}


gboolean draw_background (GoatPlot *plot, cairo_t *cr, GtkAllocation *allocation, GtkBorder *padding, gdouble x_nil,
                          gdouble y_nil, gdouble x_factor, gdouble y_factor, GdkRGBA *color_background,
                          GdkRGBA *color_border)
{
	int top, bottom, left, right;

	// NOT allocation.x/.y!
	// we are right where we need to be
	top = 0;
	left = 0;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;


	cairo_rectangle (cr, left, top, right - left, bottom - top);
	gdk_cairo_set_source_rgba (cr, color_background);
	cairo_fill (cr);


	cairo_rectangle (cr, left, top, right - left, bottom - top);
	gdk_cairo_set_source_rgba (cr, color_border);
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);

	return TRUE;
}


gboolean clip_drawable_area (GoatPlot *plot, cairo_t *cr, GtkAllocation *allocation, GtkBorder *padding)
{
	int top, bottom, left, right;

	top = 0;
	left = 0;
	bottom = allocation->height - padding->bottom - padding->top;
	right = allocation->width - padding->right - padding->left;


	cairo_rectangle (cr, left, top, right - left, bottom - top);
	cairo_clip (cr);

	return TRUE;
}
