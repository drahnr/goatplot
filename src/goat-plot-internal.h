
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

	GoatPlotPrivate *priv = goat_plot_get_instance_private (plot);

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


// TODO draw on a surface so redraw is really really fast?
// TODO Do some benchmarking (and check CPU load)
/**
 * @param plot
 * @param cr the cairo context to draw on
 * @param dataset which dataset to draw ontop of #cr
 * @param height the height of the drawable plotting area
 * @param width the width of the drawable plotting area
 * @returns TRUE if drawing was successful (zer0 length is also TRUE), otherwise
 * FALSE
 */
gboolean draw_dataset (cairo_t *cr, GoatDataset *dataset, gint height, gint width, gdouble x_min, gdouble x_max,
                       gdouble x_nil_pixel, gdouble x_unit_to_pixel, gdouble y_min, gdouble y_max, gdouble y_nil_pixel,
                       gdouble y_unit_to_pixel)
{
	gdouble x, y, ystddev;
	GoatDatasetIter dit;
	GdkRGBA color, color_meta;
	if (!goat_dataset_get_iter_first (dataset, &dit)) {
		g_debug ("Dataset appears to be empty");
		return TRUE;
	}
	goat_dataset_get_color (dataset, &color);

	const double register diameter = 8.;
	color_meta = color;
	color_meta.alpha *= 0.5;
	gdk_cairo_set_source_rgba (cr, &color_meta);

	// draw interconnection in the first round
	if (goat_dataset_interpolate (dataset)) {
		cairo_set_line_width (cr, 1.5);
		if (goat_dataset_get_iter_first (dataset, &dit)) {
			goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
			x *= x_unit_to_pixel;
			x += x_nil_pixel;
			y *= y_unit_to_pixel;
			y += y_nil_pixel;
			cairo_move_to (cr, x, y);
			while (goat_dataset_iter_next (dataset, &dit)) {
				goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
				x *= x_unit_to_pixel;
				x += x_nil_pixel;
				y *= y_unit_to_pixel;
				y += y_nil_pixel;
				cairo_line_to (cr, x, y);
			}
			cairo_stroke (cr);
		}
	}
	// draw the variation (if desired) on top in a separate loop
	if (goat_dataset_has_valid_standard_deviation (dataset)) {
		if (goat_dataset_get_iter_first (dataset, &dit)) {
			cairo_set_line_width (cr, 1.5);
			do {
				goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
				x *= x_unit_to_pixel;
				x += x_nil_pixel;
				y *= y_unit_to_pixel;
				y += y_nil_pixel;
				ystddev *= y_unit_to_pixel;
				if (fabs (ystddev) > G_MAXFLOAT) {
					cairo_move_to (cr, x, y_min * y_unit_to_pixel + 666.);
					cairo_line_to (cr, x, y_max * y_unit_to_pixel + 666.);
				}
				cairo_move_to (cr, x + diameter / 2., y + ystddev);
				cairo_line_to (cr, x - diameter / 2., y + ystddev);
				cairo_move_to (cr, x + diameter / 2., y - ystddev);
				cairo_line_to (cr, x - diameter / 2., y - ystddev);
				cairo_move_to (cr, x, y + ystddev);
				cairo_line_to (cr, x, y - ystddev);
			} while (goat_dataset_iter_next (dataset, &dit));
			cairo_stroke (cr);
		}
	}

	gdk_cairo_set_source_rgba (cr, &color);
	// draw the indivdual data markers on top
	if (goat_dataset_get_iter_first (dataset, &dit)) {
		do {
			goat_dataset_get (dataset, &dit, &x, &y, NULL);
			x *= x_unit_to_pixel;
			x += x_nil_pixel;
			y *= y_unit_to_pixel;
			y += y_nil_pixel;
			switch (goat_dataset_get_marker_style (dataset)) {
			case GOAT_MARKER_STYLE_TRIANGLE:
				cairo_move_to (cr, x + diameter / 2., y - diameter / 2.);
				cairo_line_to (cr, x - diameter / 2., y - diameter / 2.);
				cairo_line_to (cr, x + 0., y + diameter / 2.);
				break;
			case GOAT_MARKER_STYLE_SQUARE:
				cairo_rectangle (cr, x - diameter / 2., y - diameter / 2., diameter, diameter);
				break;
			case GOAT_MARKER_STYLE_POINT:
				cairo_move_to (cr, x + diameter / 2., y + diameter / 2.);
				cairo_arc (cr, x, y, diameter / 2., 0., 2 * M_PI);
				break;
			case GOAT_MARKER_STYLE_CROSS:
				cairo_move_to (cr, x + diameter / 2., y + diameter / 2.);
				cairo_line_to (cr, x - diameter / 2., y - diameter / 2.);
				cairo_move_to (cr, x - diameter / 2., y + diameter / 2.);
				cairo_line_to (cr, x + diameter / 2., y - diameter / 2.);
				break;
			case GOAT_MARKER_STYLE_NONE:
				break;
			case GOAT_MARKER_STYLE_INVALID:
			default: {
				gint gds = (gint)goat_dataset_get_marker_style (dataset);
				g_warning ("DatasetStyle enum out of bounds %i", gds);
			}
				return FALSE;
			}
		} while (goat_dataset_iter_next (dataset, &dit));
		cairo_fill (cr);
	}
	return TRUE;
}
