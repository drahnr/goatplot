/*
 * goat-plot.c
 * This file is part of GoatPlot
 *
 * Copyright (C) 2014 - Bernhard Schuster <schuster.bernhard@gmail.com>
 *
 * GoatPlot is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GoatPlot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with GoatPlot. If not, see <http://www.gnu.org/licenses/>.
 */

#include "goat-plot.h"
#include "goat-scale-linear.h"
#include "goat-scale-log.h"

#include <glib/gprintf.h>
#include <math.h>

static gboolean draw (GtkWidget *widget, cairo_t *cr);
static void get_prefered_width (GtkWidget *widget, int *minimal, int *natural);
static void get_prefered_height (GtkWidget *widget, int *minimal, int *natural);
static gboolean event (GtkWidget *widget, GdkEvent *event);
static gboolean scroll_event (GtkWidget *widget, GdkEventScroll *event);

struct _GoatPlotPrivate {
	GArray *array; // array of GoatDataset pointers
	// remove this, its the users duty to prescale data properly FIXME

	GdkRGBA color_background;
	GdkRGBA color_border;

	GoatScale *scale_x;
	GoatScale *scale_y;
};

G_DEFINE_TYPE_WITH_PRIVATE (GoatPlot, goat_plot, GTK_TYPE_DRAWING_AREA);

#include "goat-plot-internal.h"

static void goat_plot_finalize (GObject *object)
{
	GoatPlot *plot = GOAT_PLOT (object);
	GoatPlotPrivate *priv = goat_plot_get_instance_private (plot);
	gint register i = priv->array->len;
	while (--i >= 0) {
		g_object_unref (g_array_index (priv->array, GoatDataset *, i));
	}
	g_array_free (plot->priv->array, TRUE);

	G_OBJECT_CLASS (goat_plot_parent_class)->finalize (object);
}

enum {
	PROP_0,

	PROP_SCALE_X,
	PROP_SCALE_Y,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void goat_plot_set_gproperty (GObject *object, guint prop_id, const GValue *value, GParamSpec *spec)
{
	GoatPlot *dataset = GOAT_PLOT (object);
	GoatPlotPrivate *priv = goat_plot_get_instance_private (dataset);

	switch (prop_id) {
	case PROP_SCALE_X:
		priv->scale_x = g_value_get_pointer (value);
		break;
	case PROP_SCALE_Y:
		priv->scale_y = g_value_get_pointer (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (dataset, prop_id, spec);
	}
}

static void goat_plot_get_gproperty (GObject *object, guint prop_id, GValue *value, GParamSpec *spec)
{
	GoatPlot *dataset = GOAT_PLOT (object);
	GoatPlotPrivate *priv = goat_plot_get_instance_private (dataset);

	switch (prop_id) {
	case PROP_SCALE_X:
		g_value_set_pointer (value, priv->scale_x);
		break;
	case PROP_SCALE_Y:
		g_value_set_pointer (value, priv->scale_y);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (dataset, prop_id, spec);
	}
}

static void goat_plot_class_init (GoatPlotClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = goat_plot_finalize;

	object_class->set_property = goat_plot_set_gproperty;
	object_class->get_property = goat_plot_get_gproperty;

	obj_properties[PROP_SCALE_X] = g_param_spec_pointer ("scale_x", "GoatPlot::scale_x", "scale x", G_PARAM_READWRITE);

	obj_properties[PROP_SCALE_Y] = g_param_spec_pointer ("scale_y", "GoatPlot::scale_y", "scale y", G_PARAM_READWRITE);

	g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	widget_class->draw = draw;

#if 0
	widget_class->button_press_event = goat_plot_button_press_event;
	widget_class->button_release_event = goat_plot_button_release_event;
	widget_class->key_press_event = goat_plot_key_press_event;
	widget_class->key_release_event = goat_plot_key_release_event;
#endif
	widget_class->scroll_event = scroll_event;
	widget_class->get_preferred_width = get_prefered_width;
	widget_class->get_preferred_height = get_prefered_height;
}

static void goat_plot_init (GoatPlot *self)
{
	gboolean assure;
	GtkWidget *widget = GTK_WIDGET (self);
	gtk_widget_set_has_window (widget, FALSE);

	self->priv = goat_plot_get_instance_private (self);

	self->priv->array = g_array_new (FALSE, TRUE, sizeof (void *));

	assure = gdk_rgba_parse (&(self->priv->color_background), "white");
	g_assert (assure);
	assure = gdk_rgba_parse (&(self->priv->color_border), "black");
	g_assert (assure);

	gtk_widget_add_events (widget, GDK_SCROLL_MASK);
	g_assert ((gtk_widget_get_events (widget) & GDK_SCROLL_MASK) != 0);

	gtk_widget_set_sensitive (widget, TRUE);
	gtk_widget_set_can_focus (widget, TRUE);
	gtk_widget_grab_focus (widget);
}

/**
 * create a new GoatPlot widget
 */
GoatPlot *goat_plot_new (GoatScale *x, GoatScale *y)
{
	return GOAT_PLOT (gtk_widget_new (GOAT_TYPE_PLOT, "scale_x", x, "scale_y", y, NULL));
}

/**
 *
 */
void goat_plot_set_scale_x (GoatPlot *plot, GoatScale *scale)
{
	g_object_set (G_OBJECT (plot), "scale_x", scale, NULL);
}

/**
 *
 */
void goat_plot_set_scale_y (GoatPlot *plot, GoatScale *scale)
{
	g_object_set (G_OBJECT (plot), "scale_y", scale, NULL);
}

/**
 * add a dataset to the plot
 * @returns identifier to use for removal
 */
gint goat_plot_add_dataset (GoatPlot *plot, GoatDataset *dataset)
{
	g_return_val_if_fail (plot != NULL, -1);
	g_return_val_if_fail (GOAT_IS_PLOT (plot), -1);
	g_return_val_if_fail (dataset != NULL, -1);
	g_return_val_if_fail (GOAT_IS_DATASET (dataset), -1);

	gpointer tmp = g_object_ref (dataset);
	g_array_append_val (plot->priv->array, tmp);

	return plot->priv->array->len - 1;
}

/**
 * remove a dataset from the plot
 */
GoatDataset *goat_plot_remove_dataset (GoatPlot *plot, gint datasetid)
{
	GoatDataset *dataset = NULL;
	GoatPlotPrivate *priv = goat_plot_get_instance_private (plot);
	GoatDataset **datasetptr = &(g_array_index (priv->array, GoatDataset *, datasetid));
	if (datasetptr != NULL) {
		dataset = *datasetptr;
		*datasetptr = NULL;
	}
	return dataset;
}



/**
 * return dataset
 */
GoatDataset *goat_plot_get_dataset( GoatPlot *plot, gint datasetid )
{
	GoatPlotPrivate *priv = goat_plot_get_instance_private (plot);
	GoatDataset *dataset = g_array_index (priv->array, GoatDataset *, datasetid);
	return dataset;
}

/**
 * @param window window height or width, or if fixed scale, use a const for that
 * @param min in units
 * @param max in units
 * @param unit_to_pixel [out]
 */
gboolean get_unit_to_pixel_factor (int window, gdouble min, gdouble max, gdouble *unit_to_pixel)
{
	gdouble delta = max - min;
	if (delta > 0.) {
		*unit_to_pixel = (double)window / delta;
		//g_printf ("MAPPING: %i --{%lf}--> %lf\n", window, (*unit_to_pixel), delta);
		return TRUE;
	}
	*unit_to_pixel = 1.;
	return FALSE;
}

static gboolean draw (GtkWidget *widget, cairo_t *cr)
{
	GoatPlot *plot;
	GoatDataset *dataset;
	GoatPlotPrivate *priv;
	GtkAllocation allocation; //==gint x,y,width,heightynamically
	// TODO this needs to be done dynamically
	// TODO based on the style context and where the scales are
	// TODO https://github.com/drahnr/goatplot/issues/8
	GtkBorder padding = {90, 30, 30, 30}; // left, right, top, bottom
	gint i;
	gdouble x_nil_pixel;
	gdouble y_nil_pixel;
	gdouble x_unit_to_pixel;
	gdouble y_unit_to_pixel;
	int width, height;
	GdkRGBA color, color_meta, marker_line_color, marker_fill_color;
	double diameter;
	double line_width;
	double marker_line_width;
	gdouble x, y, ystddev, x_log_min, y_log_min;
	GoatDatasetIter dit;
	gboolean x_log, y_log;
	gboolean draw = TRUE;
	int count;


	if (gtk_widget_is_drawable (widget)) {
		plot = GOAT_PLOT (widget);
		priv = goat_plot_get_instance_private (plot);
		cairo_save (cr);

		g_assert (priv);
		g_assert (priv->scale_x);
		g_assert (GOAT_IS_SCALE (priv->scale_x));
		g_assert (priv->scale_y);
		g_assert (GOAT_IS_SCALE (priv->scale_y));

		gtk_widget_get_allocation (widget, &allocation);

		// translate origin to plot graphs to (0,0) of our plot
		cairo_translate (cr, padding.left, allocation.height - padding.bottom);

		// make it plot naturally +up, -down
		cairo_scale (cr, 1., -1.);

		gdouble ref_x_min = G_MAXDOUBLE;
		gdouble ref_x_max = -G_MAXDOUBLE;
		gdouble ref_y_min = G_MAXDOUBLE;
		gdouble ref_y_max = -G_MAXDOUBLE;

		// get the common extends of all data sets if any scale used is set to autorange
		//
		// vfunc, we use it in a loop, so caching is good idea
		const gboolean register autorange_x = goat_scale_is_auto_range (priv->scale_x);
		const gboolean register autorange_y = goat_scale_is_auto_range (priv->scale_y);

		x_log = y_log = FALSE;

		x_log = GOAT_IS_SCALE_LOG (priv->scale_x);
		y_log = GOAT_IS_SCALE_LOG (priv->scale_y);

		goat_scale_get_range (priv->scale_x, &ref_x_min, &ref_x_max);
		goat_scale_get_range (priv->scale_y, &ref_y_min, &ref_y_max);

		if (autorange_x || autorange_y ) {
			/* For each dataset -> find min/max  */
			for (i = 0; i < priv->array->len; i++) {
				gdouble x_min, x_max, y_min, y_max;

				dataset = g_array_index (priv->array, GoatDataset *, i);

				if( !dataset ) continue;

				goat_dataset_get_extrema (dataset, &x_min, &x_max, &y_min, &y_max);

				if (autorange_x ) {
					if (ref_x_min > x_min) ref_x_min = x_min;
					if (ref_x_max < x_max) ref_x_max = x_max;
				}
				if (autorange_y ) {
					if (ref_y_min > y_min) ref_y_min = y_min;
					if (ref_y_max < y_max) ref_y_max = y_max;
				}
			}
		}

		if( x_log ) {
			/* May need to find smallest value > 0. */
			if( ref_x_min <= 0. ) {
				double xlm;
				ref_x_min = G_MAXDOUBLE;
				for (i = 0; i < priv->array->len; i++) {
					dataset = g_array_index (priv->array, GoatDataset *, i);
					goat_dataset_get_log_extrema( dataset, &xlm, NULL, NULL, NULL );
					if( ref_x_min > xlm ) ref_x_min = xlm;
				}
			}

			/* Final check to avoid negative values on scale */
			if( ref_x_min <= 0. ) ref_x_min = 1.;

			/* Round upper scale so whole number of order 10 steps */
			i = (int) (ceil(log10( ref_x_max/ref_x_min ) / goat_scale_log_get_major_delta(GOAT_SCALE_LOG(priv->scale_x))));
			if( i < 1 ) i = 1;
			ref_x_max = ref_x_min * pow (10, i);

			goat_scale_update_range (priv->scale_x, ref_x_min, ref_x_max);

			/* these are now used for data -> pixel mapping - need to be on log scale */
			x_log_min = ref_x_min;
			ref_x_min = log10 (ref_x_min);
			ref_x_max = log10 (ref_x_max);
		} else {
			if( ref_x_max <= ref_x_min ) ref_x_max = ref_x_min + 1.;
			goat_scale_update_range (priv->scale_x, ref_x_min, ref_x_max);
		}


		if( y_log ) {
			if( ref_y_min <= 0. ) {
				double ylm;
				ref_y_min = G_MAXDOUBLE;
				for (i = 0; i < priv->array->len; i++) {
					dataset = g_array_index (priv->array, GoatDataset *, i);
					goat_dataset_get_log_extrema( dataset, NULL, NULL, &ylm, NULL );
					if( ref_y_min > ylm ) ref_y_min = ylm;
				}
			}

			/* Final check to avoid negative values on scale */
			if( ref_y_min <= 0. ) ref_y_min = 1.;

			/* Round upper scale so whole number of order 10 steps */
			i = (int) (ceil(log10( ref_y_max/ref_y_min ) / goat_scale_log_get_major_delta(GOAT_SCALE_LOG(priv->scale_y))));
			if( i < 1 ) i = 1;
			ref_y_max =  ref_y_min * pow (10, i);

			goat_scale_update_range (priv->scale_y, ref_y_min, ref_y_max);

			/* these are now used for data -> pixel mapping - need to be on log scale */
			y_log_min = ref_y_min;
			ref_y_min = log10 (ref_y_min);
			ref_y_max = log10 (ref_y_max);
		} else {
			if( ref_y_max <= ref_y_min ) ref_y_max = ref_y_min + 1.;
			goat_scale_update_range (priv->scale_y, ref_y_min, ref_y_max);
		}


		/* Generate data->pixel mapping */
		height = allocation.height - padding.bottom - padding.top;
		width = allocation.width - padding.right - padding.left;

		if (!get_unit_to_pixel_factor (width, ref_x_min, ref_x_max, &x_unit_to_pixel)) {
			g_warning ("Bad x range: %lf..%lf, delta of %lf", ref_x_min, ref_x_max,ref_x_max - ref_x_min);
			cairo_restore (cr);
			return FALSE;
		}
		x_nil_pixel = ref_x_min * -x_unit_to_pixel;


		if (!get_unit_to_pixel_factor (height, ref_y_min, ref_y_max, &y_unit_to_pixel)) {
			g_warning ("Bad x range: %lf..%lf, delta of %lf", ref_y_min, ref_y_max, ref_y_max - ref_y_min);
			cairo_restore (cr);
			return FALSE;
		}
		y_nil_pixel = ref_y_min * -y_unit_to_pixel;

		/* Draw background */
		cairo_rectangle (cr, 0, 0, width, height);
		gdk_cairo_set_source_rgba (cr, &priv->color_background);
		cairo_fill (cr);

		/* Draw border */
		cairo_rectangle (cr, 0, 0, width, height);
		gdk_cairo_set_source_rgba (cr, &priv->color_border);
		cairo_set_line_width (cr, 1.);
		cairo_stroke (cr);

		/* Draw scales */
		if( draw ) {
			goat_scale_draw (priv->scale_x, cr, 0, width, 0, height );
			goat_scale_draw (priv->scale_y, cr, 0, width, 0, height );
		}

		if (draw) {
			for (i = 0; i < priv->array->len; i++) {
				dataset = g_array_index (priv->array, GoatDataset *, i);

				if( !dataset ) continue;

				/* Load colours for rendering */
				goat_dataset_get_color (dataset, &color);
				goat_dataset_get_marker_line_color (dataset, &marker_line_color);
				goat_dataset_get_marker_fill_color (dataset, &marker_fill_color);
				goat_dataset_get_line_width (dataset, &line_width);
				goat_dataset_get_marker_line_width (dataset, &marker_line_width);
				goat_dataset_get_marker_size (dataset, &diameter);
				color_meta = color;
				color_meta.alpha *= 0.5;

				if (!goat_dataset_get_iter_first (dataset, &dit)) {
					g_debug ("Dataset appears to be empty");
					return FALSE;
				}

				/* Draw line */
				gdk_cairo_set_source_rgba (cr, &color_meta);

				// draw interconnection in the first round
				if (goat_dataset_interpolate (dataset)) {
					cairo_set_line_width (cr, line_width);
					if (goat_dataset_get_iter_first (dataset, &dit)) {

						goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
						if( x_log ) {
							if( x < x_log_min ) x = x_log_min;
							x = log10 (x);
						}
						x = x*x_unit_to_pixel + x_nil_pixel;
						if( y_log ) {
							if( y < y_log_min ) y = y_log_min;
							y = log10 (y);
							ystddev = log10 (ystddev);
						}
						y = y*y_unit_to_pixel + y_nil_pixel;
						cairo_move_to (cr, x, y);

						while (goat_dataset_iter_next (dataset, &dit)) {
							goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
							if( x_log ) {
								if( x < x_log_min ) x = x_log_min;
								x = log10 (x);
							}
							x = x*x_unit_to_pixel + x_nil_pixel;
							if( y_log ) {
								if( y < y_log_min ) y = y_log_min;
								y = log10 (y);
								ystddev = log10 (ystddev);
							}
							y = y*y_unit_to_pixel + y_nil_pixel;
							cairo_line_to (cr, x, y);
						}
						cairo_stroke (cr);
					}
				}
				// draw the variation (if desired) on top in a separate loop
				if (goat_dataset_has_valid_standard_deviation (dataset)) {
					if (goat_dataset_get_iter_first (dataset, &dit)) {
						cairo_set_line_width (cr, line_width);
						do {
							goat_dataset_get (dataset, &dit, &x, &y, &ystddev);
							if( x_log ) {
								if( x < x_log_min ) x = x_log_min;
								x = log10 (x);
							}
							x = x*x_unit_to_pixel + x_nil_pixel;
							if( y_log ) {
								if( y < y_log_min ) y = y_log_min;
								y = log10 (y);
								ystddev = log10 (ystddev);
							}
							y = y*y_unit_to_pixel + y_nil_pixel;
							ystddev *= y_unit_to_pixel;
							if (fabs (ystddev) > G_MAXFLOAT) {
								cairo_move_to (cr, x, 0);
								cairo_line_to (cr, x, height);
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

				// draw the indivdual data markers on top
				if (goat_dataset_get_iter_first (dataset, &dit) && goat_dataset_get_marker_style (dataset) != GOAT_MARKER_STYLE_NONE ) {
					/* TODO: Switch outside loops? */
					do {
						goat_dataset_get (dataset, &dit, &x, &y, NULL);
						if( x_log ) {
							if( x < x_log_min ) x = x_log_min;
							x = log10 (x);
						}
						x = x*x_unit_to_pixel + x_nil_pixel;
						if( y_log ) {
							if( y < y_log_min ) y = y_log_min;
							y = log10 (y);
							ystddev = log10 (ystddev);
						}
						y = y*y_unit_to_pixel + y_nil_pixel;
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

					if( goat_dataset_get_marker_fill (dataset) ) {
						gdk_cairo_set_source_rgba (cr, &marker_fill_color);
						cairo_fill_preserve (cr);
					}

					gdk_cairo_set_source_rgba (cr, &marker_line_color);
					cairo_stroke (cr);
				}
			}
		}
		cairo_restore (cr);

		return TRUE;
	}
	return FALSE;
}

static void get_prefered_width (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}

static void get_prefered_height (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}

void goat_plot_set_background_color (GoatPlot *plot, GdkRGBA *color)
{
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));
	g_return_if_fail (color != NULL);

	GoatPlotPrivate *priv;

	priv = goat_plot_get_instance_private (plot);

	priv->color_background = *color;
}

void goat_plot_set_border_color (GoatPlot *plot, GdkRGBA *color)
{
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));
	g_return_if_fail (color != NULL);

	GoatPlotPrivate *priv;

	priv = goat_plot_get_instance_private (plot);

	priv->color_border = *color;
}

/**
 * TODO handle zooming and scrolling
 * https://github.com/drahnr/goatplot/issues/9
 */
static gboolean scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
	g_print ("scroll event\n");
	return FALSE;
}

/**
 * TODO handle envents
 * https://github.com/drahnr/goatplot/issues/9
 */
static gboolean event (GtkWidget *widget, GdkEvent *event)
{
	g_warning ("ehllllo\n??\n");
	switch (event->type) {
	case GDK_SCROLL:
	case GDK_SCROLL_UP:
	case GDK_SCROLL_DOWN:
		g_print ("got a scroll event!");
		break;
	default:
		break;
	}
	return TRUE;
}
