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

#include "goat-dataset.h"
#include <math.h>
#include <glib/gprintf.h>

static gboolean draw (GtkWidget *widget, cairo_t *cr);
static void get_prefered_width (GtkWidget *widget, int *minimal, int *natural);
static void get_prefered_height (GtkWidget *widget, int *minimal, int *natural);
static gboolean event (GtkWidget *widget, GdkEvent *event);
static gboolean scroll_event (GtkWidget *widget, GdkEventScroll *event);

#define GOAT_PLOT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GOAT_TYPE_PLOT, GoatPlotPrivate))

struct _GoatPlotPrivate
{
	GArray *array; // array of GoatDataset pointers
	GoatPlotScaleType scale_x, scale_y; //remove this, its the users duty to prescale data properly FIXME
	// FIXME GoatPlotGridType gridtype [LOG, EXP, LIN=DEFAULT]
	gboolean scale_fixed;

	// visible area of datasets
	gdouble x_min, x_max;
	gdouble y_min, y_max;
	// automatically choose x_min, ... y_max according to the datasets
	gboolean x_autorange;
	gboolean y_autorange;

	// ticks probably move into GoatTicks/Scale object
	gdouble major_delta_x;
	gint minors_per_major_x;

	gdouble major_delta_y;
	gint minors_per_major_y;

	GdkRGBA color_major_x;
	GdkRGBA color_major_y;

	GdkRGBA color_minor_x;
	GdkRGBA color_minor_y;

	gint width_minor_x;
	gint width_major_x;
	gint width_minor_y;
	gint width_major_y;

};

G_DEFINE_TYPE (GoatPlot, goat_plot, GTK_TYPE_DRAWING_AREA);

#include "goat-plot-internal.h"

static void
goat_plot_finalize (GObject *object)
{
	GoatPlot *plot = GOAT_PLOT (object);
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	gint register i = priv->array->len;
	while (--i >= 0) {
		g_object_unref (g_array_index (priv->array, GoatDataset*, i));
	}
	g_array_free (plot->priv->array, TRUE);

	G_OBJECT_CLASS (goat_plot_parent_class)->finalize (object);
}

static void
goat_plot_class_init (GoatPlotClass *klass)
{
	g_type_class_add_private (klass, sizeof(GoatPlotPrivate));

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = goat_plot_finalize;

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
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

static void
goat_plot_init (GoatPlot *self)
{
	GtkWidget *widget = GTK_WIDGET (self);
	gtk_widget_set_has_window (widget, FALSE);

	self->priv = GOAT_PLOT_GET_PRIVATE (self);

	gtk_widget_add_events (widget, GDK_SCROLL_MASK);
	g_assert ((gtk_widget_get_events (widget) & GDK_SCROLL_MASK) != 0);

	gtk_widget_set_sensitive (widget, TRUE);
	gtk_widget_set_can_focus (widget, TRUE);
	gtk_widget_grab_focus (widget);

	self->priv->scale_x = GOAT_PLOT_SCALE_LIN; //KICKE MEE
	self->priv->scale_y = GOAT_PLOT_SCALE_LIN; //KICK MEE
	self->priv->scale_fixed = TRUE;
	self->priv->array = g_array_new (TRUE, TRUE, sizeof(gpointer));
	self->priv->x_min = +G_MAXDOUBLE;
	self->priv->x_max = -G_MAXDOUBLE;
	self->priv->y_min = +G_MAXDOUBLE;
	self->priv->y_max = -G_MAXDOUBLE;
	self->priv->x_autorange = TRUE;
	self->priv->y_autorange = TRUE;
	self->priv->color_major_x.red = 0.8;
	self->priv->color_major_x.green = 0.;
	self->priv->color_major_x.blue = 0.;
	self->priv->color_major_x.alpha = 1.;

	self->priv->color_minor_x.red = 0.;
	self->priv->color_minor_x.green = 0.;
	self->priv->color_minor_x.blue = 0.;
	self->priv->color_minor_x.alpha = 1.;

	self->priv->color_major_y = self->priv->color_major_x;
	self->priv->color_minor_y = self->priv->color_minor_x;

	self->priv->width_minor_x = 5;
	self->priv->width_major_x = 10;
	self->priv->width_minor_y = 5;
	self->priv->width_major_y = 10;

	self->priv->minors_per_major_x = 5;
	self->priv->minors_per_major_y = 5;
	self->priv->major_delta_x = 50.;
	self->priv->major_delta_y = 50.;
}


/**
 * create a new GoatPlot widget
 */
GoatPlot *
goat_plot_new ()
{
	return GOAT_PLOT (gtk_widget_new (GOAT_TYPE_PLOT, NULL));
}


/**
 * add a dataset to the plot
 * @returns identifier to use for removal
 */
gint
goat_plot_add_dataset (GoatPlot *plot, GoatDataset *dataset)
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
GoatDataset *
goat_plot_remove_dataset (GoatPlot *plot, gint datasetid)
{
	GoatDataset *dataset = NULL;
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	GoatDataset **datasetptr = &(g_array_index (priv->array, GoatDataset*, datasetid));
	if (datasetptr!=NULL) {
		dataset = *datasetptr;
		*datasetptr = NULL;
	}
	return dataset;
}


/**
 * @param window window height or width, or if fixed scale, us a const for that
 * @param min in units
 * @param max in units
 * @param unit_to_pixel [out]
 */
gboolean
get_unit_to_pixel_factor (int window, gdouble min, gdouble max, gdouble *unit_to_pixel)
{
	gdouble delta = max - min;
	if (delta==0.) { //yes really equal
		return FALSE;
	} else {
		*unit_to_pixel = (double)window/delta;
	}
	return TRUE;
}

/**
 * UNUSED
 * may be useful when ScaleTicks get factored into their own objects
 */
gboolean
goat_plot_get_x_unit_to_pixel (GoatPlot *plot, gdouble *unit_to_pixel)
{
	GtkAllocation allocation;
	const GtkBorder padding = {25, 25, 25, 25}; // left, right, top, bottom
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	gtk_widget_get_allocation (GTK_WIDGET (plot), &allocation);
	const gint register width = allocation.width - padding.left - padding.right;
	return get_unit_to_pixel_factor (width, priv->x_min, priv->x_max, unit_to_pixel);
}


/**
 * UNUSED
 * may be useful when SacleTicks get factored into their own objects
 */
gboolean
goat_plot_get_y_unit_to_pixel (GoatPlot *plot, gdouble *unit_to_pixel)
{
	GtkAllocation allocation;
	const GtkBorder padding = {25, 25, 25, 25}; // left, right, top, bottom
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	gtk_widget_get_allocation (GTK_WIDGET (plot), &allocation);
	const gint register height = allocation.height - padding.top - padding.bottom;
	return get_unit_to_pixel_factor (height, priv->y_min, priv->y_max, unit_to_pixel);
}


//TODO draw on a surface so redraw is really really fast? Do some benchmarking (and check CPU load)
/**
 * @param plot
 * @param cr the cairo context to draw on
 * @param dataset which dateset to draw ontop of #cr
 * @param height the height of the drawable plotting area
 * @param width the width of the drawable plotting area
 * @returns TRUE if drawing was successful (zer0 length is also TRUE), otherwise FALSE
 */
static gboolean
draw_dataset (GoatPlot *plot, cairo_t *cr,
              GoatDataset *dataset,
              gint height, gint width,
              gdouble x_min, gdouble x_max, gdouble x_nil_pixel, gdouble x_unit_to_pixel,
              gdouble y_min, gdouble y_max, gdouble y_nil_pixel, gdouble y_unit_to_pixel)
{
	g_return_val_if_fail (plot, FALSE);
	if (goat_dataset_get_length(dataset) <= 0) {
		return TRUE;
	}

	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	(void)priv;
#if 0
	// find max and apply the extra scale if necessary
	switch (priv->scale_y) {
	case GOAT_PLOT_SCALE_EXP:
		y_min = exp(y_min);
		break;
	case GOAT_PLOT_SCALE_LOG:
		y_min = log(y_min * (y_min < 0. ? -1 : +1));
		break;
	default:
		break;
	}
	switch (priv->scale_y) {
	case GOAT_PLOT_SCALE_EXP:
		y_max = exp(y_max);
		break;
	case GOAT_PLOT_SCALE_LOG:
		y_max = log(y_max * (y_max < 0. ? -1 : +1));
		break;
	default:
		break;
	}

	switch (priv->scale_x) {
	case GOAT_PLOT_SCALE_EXP:
		x_min = exp(x_min);
		break;
	case GOAT_PLOT_SCALE_LOG:
		x_min = log (x_min * (x_min < 0. ? -1 : +1));
		break;
	default:
		break;
	}
	switch (priv->scale_x) {
	case GOAT_PLOT_SCALE_EXP:
		x_max = exp(x_max);
		break;
	case GOAT_PLOT_SCALE_LOG:
		x_max = log(x_max * (x_max < 0. ? -1 : +1));
		break;
	default:
		break;
	}
#endif


	gdouble x,y;
	GoatDatasetIter dit;

	goat_dataset_iter_init (&dit, dataset);

	// draw points
	const double register diameter = 8.;
	cairo_set_source_rgba (cr, g_random_double_range (0.1,0.9),
	                           g_random_double_range (0.1,0.9),
	                           g_random_double_range (0.1,0.9), 1.);
	while (goat_dataset_iter_next (&dit, &x, &y)) {
	#if 0
		switch (priv->scale_x) {
		case GOAT_PLOT_SCALE_EXP:
			x = exp (x) * x_unit_to_pixel;
			break;
		case GOAT_PLOT_SCALE_LOG:
			x = log (x) * x_unit_to_pixel;
			break;
		default:
			x *= x_unit_to_pixel;
			break;
		}
		x -= x_nil; //plottable

		switch (priv->scale_y) {
		case GOAT_PLOT_SCALE_EXP:
			y = exp (y) * y_unit_to_pixel;
			break;
		case GOAT_PLOT_SCALE_LOG:
			y = log (y) * y_unit_to_pixel;
			break;
		default:
			y *= y_unit_to_pixel;
			break;
		}
		y -= y_nil; //plottable
	#endif

		x *= x_unit_to_pixel;
		x += x_nil_pixel;
		y *= y_unit_to_pixel;
		y += y_nil_pixel;
		// TODO spline/linear interconnect here
		switch (goat_dataset_get_style (dataset)) {
		case GOAT_DATASET_STYLE_TRIANGLE:
		    cairo_move_to(cr, x+diameter/2., y-diameter/2.);
		    cairo_line_to(cr, x-diameter/2., y-diameter/2.);
		    cairo_line_to(cr, x+0., y+diameter/2.);
			break;
		case GOAT_DATASET_STYLE_SQUARE:
			cairo_rectangle (cr, x - diameter/2., y - diameter/2.,
				                 diameter, diameter);
			break;
		case GOAT_DATASET_STYLE_POINT:
			cairo_move_to (cr, x + diameter / 2., y + diameter / 2.);
			cairo_arc (cr,
			           x, y,
			           diameter / 2.,
			           0., 2 * M_PI);
			break;
		case GOAT_DATASET_STYLE_CROSS:
			cairo_move_to (cr, x+diameter/2., y+diameter / 2.);
		    cairo_line_to (cr, x-diameter/2., y-diameter/2.);
			cairo_move_to (cr, x-diameter/2., y+diameter / 2.);
		    cairo_line_to (cr, x+diameter/2., y-diameter/2.);
			break;

		case GOAT_DATASET_STYLE_UNKNOWN:
			g_warning ("psst .. I have no clue what to do...");
			return FALSE;
		default:
			{
				gint gds = (gint)goat_dataset_get_style(dataset);
				g_warning ("DatasetStyle enum out of bounds %i", gds);
			}
			return FALSE;
		}
	}
	cairo_fill (cr);

	return TRUE;
}



static gboolean
draw (GtkWidget *widget, cairo_t *cr)
{
	GoatPlot *plot;
	GoatDataset *dataset;
	GoatPlotPrivate *priv;
	GtkAllocation allocation; //==gint x,y,width,height
	GtkBorder padding = {25, 25, 25, 25}; // left, right, top, bottom
	gint i;
	gdouble x_nil_pixel;
	gdouble y_nil_pixel;
	gdouble x_unit_to_pixel;
	gdouble y_unit_to_pixel;

	if (gtk_widget_is_drawable (widget)) {
		plot = GOAT_PLOT (widget);
		priv = GOAT_PLOT_GET_PRIVATE (plot);
		cairo_save (cr);

		gtk_widget_get_allocation (widget, &allocation);

#if 0
		//FIXME this is all zer0es
		gtk_style_context_get_padding (gtk_widget_get_style_context (widget), GTK_STATE_FLAG_ACTIVE, &padding);

		g_print ("alocation x,y %i %i    w,h %i %i\n",
		         allocation.x, allocation.y, allocation.width, allocation.height);
		g_print ("padding left,right,top,bottom %i %i %i %i\n",
		         padding.left, padding.right, padding.top, padding.bottom);
#endif

		// translate origin to plot graphs to (0,0) of our plot

		cairo_translate (cr, allocation.x + padding.left,
		                     allocation.height + allocation.y - padding.bottom);

		// make it plot naturally +up, -down
		cairo_scale (cr, 1., -1.);
		// draw the background (scale + grid + white box)

		// the scale we just drew should not be drawable either
		// now the coords are y↑ x→
		const gint width = allocation.width - padding.left - padding.right;
		const gint height = allocation.height - padding.top - padding.bottom;

		// draw the actual data
		if (priv->x_autorange || priv->y_autorange) {
			for (i=0; i<priv->array->len; i++) {
				dataset = g_array_index (priv->array, GoatDataset *, i);
				gdouble x_min, x_max, y_min, y_max;
				goat_dataset_get_extrema (dataset,
				                          &x_min, &x_max,
				                          &y_min, &y_max);
				if (priv->x_autorange) {
					if (priv->x_min > x_min)
						priv->x_min = x_min;
					if (priv->x_max < x_max)
						priv->x_max = x_max;
				}
				if (priv->y_autorange) {
					if (priv->y_min > y_min)
						priv->y_min = y_min;
					if (priv->y_max < y_max)
						priv->y_max = y_max;
				}
			}
			// TODO add some fixup if x_min is very close to x_max
			// TODO add some additional padding for niceness :)
		}
		g_printf ("x range %lf %lf\n", priv->x_min, priv->x_max);
		g_printf ("y range %lf %lf\n", priv->y_min, priv->y_max);

		if (get_unit_to_pixel_factor (width, priv->x_min, priv->x_max, &x_unit_to_pixel)) {
			g_warning ("Bad x range. This is too boring to plot.");
		}
		if (get_unit_to_pixel_factor (height, priv->y_min, priv->y_max, &y_unit_to_pixel)) {
			g_warning ("Bad y range. This is too boring to plot.");
		}

		x_nil_pixel = priv->x_min * -x_unit_to_pixel;
		y_nil_pixel = priv->y_min * -y_unit_to_pixel;

		draw_background (plot, cr, &allocation, &padding,
		                 x_nil_pixel, y_nil_pixel,
		                 x_unit_to_pixel, y_unit_to_pixel);

		draw_scales (plot, cr, &allocation, &padding,
		             x_nil_pixel, y_nil_pixel,
		             x_unit_to_pixel, y_unit_to_pixel);

		clip_drawable_area (plot, cr, &allocation, &padding);

		draw_nil_lines (plot, cr, width, height, x_nil_pixel, y_nil_pixel);

		for (i=0; i<priv->array->len; i++) {
			dataset = g_array_index (priv->array, GoatDataset *, i);
			draw_dataset (plot, cr, dataset,
			              height, width,
			              priv->x_min, priv->x_max, x_nil_pixel, x_unit_to_pixel,
			              priv->y_min, priv->y_max, y_nil_pixel, y_unit_to_pixel);
		}
		cairo_restore (cr);

		return TRUE;
	}
	return FALSE;
}

static void
get_prefered_width (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}

static void
get_prefered_height (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}




void
goat_plot_set_range_x_auto (GoatPlot *plot)
{
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));

	plot->priv->x_min = G_MAXDOUBLE;
	plot->priv->x_max = -G_MAXDOUBLE;
	plot->priv->x_autorange = TRUE;
}

void
goat_plot_set_range_y_auto (GoatPlot *plot)
{
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));


	plot->priv->y_min = G_MAXDOUBLE;
	plot->priv->y_max = -G_MAXDOUBLE;
	plot->priv->y_autorange = TRUE;
}

void
goat_plot_set_range_x (GoatPlot *plot, gdouble min_x, gdouble max_x)
{
	g_return_if_fail (min_x < max_x);
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));

	plot->priv->x_autorange = FALSE;
	plot->priv->x_min = min_x;
	plot->priv->x_max = max_x;
}

void
goat_plot_set_range_y (GoatPlot *plot, gdouble min_y, gdouble max_y)
{
	g_return_if_fail (min_y < max_y);
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));

	plot->priv->y_autorange = FALSE;
	plot->priv->y_min = min_y;
	plot->priv->y_max = max_y;
}

void
goat_plot_set_ticks_x (GoatPlot *plot, gdouble major, gint minors_per_major)
{
	g_return_if_fail (major>0.);
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));

	GoatPlotPrivate *priv;

	priv = GOAT_PLOT_GET_PRIVATE (plot);

	priv->major_delta_x = major;
	priv->minors_per_major_x = minors_per_major;
}


void
goat_plot_set_ticks_y (GoatPlot *plot, gdouble major, gint minors_per_major)
{
	g_return_if_fail (major>0.);
	g_return_if_fail (plot);
	g_return_if_fail (GOAT_IS_PLOT (plot));

	GoatPlotPrivate *priv;

	priv = GOAT_PLOT_GET_PRIVATE (plot);

	priv->major_delta_y = major;
	priv->minors_per_major_y = minors_per_major;

}






static gboolean
scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
	g_print ("scroll event\n");
	return FALSE;
}





static gboolean
event (GtkWidget *widget, GdkEvent *event)
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
