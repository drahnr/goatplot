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
#include "goat-plot-internal.h"
#include "goat-dataset.h"
#include <math.h>

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
};

G_DEFINE_TYPE_WITH_PRIVATE (GoatPlot, goat_plot, GTK_TYPE_DRAWING_AREA);


static void
goat_plot_finalize (GObject *object)
{
	GoatPlot *plot = GOAT_PLOT (object);
	GoatPlotPrivate *priv = plot->priv;
	gint i = priv->array->len;
	while (--i >= 0) {
		g_object_unref (g_array_index (priv->array, GoatDataset*, i));
	}
	g_array_free (plot->priv->array, TRUE);

	G_OBJECT_CLASS (goat_plot_parent_class)->finalize (object);
}

static void
goat_plot_class_init (GoatPlotClass *klass)
{
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
}



GoatPlot *
goat_plot_new ()
{
	return GOAT_PLOT (gtk_widget_new (GOAT_TYPE_PLOT, NULL));
}



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


//FIXME draw on a surface so redraw is really really fast? Do some benchmarking (and check CPU load)
/**
 * @param height the height of the drawable plotting area
 * @param height the width of the drawable plotting area
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
		return FALSE;
	}

	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
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
	const double diameter = 8.;
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
			g_warning ("spacken!");
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
	GtkBorder padding = {18, 18, 18, 18}; // left, right, top, bottom
	gint i;
	gdouble x_min, x_max;
	gdouble y_min, y_max;
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
		//FIXME this only works for one dataset, find a way to fix it
		for (i=0; i<priv->array->len; i++) {
			dataset = g_array_index (priv->array, GoatDataset *, i);
			goat_dataset_get_extrema (dataset, &x_min, &x_max, &y_min, &y_max);
		}
		if (get_unit_to_pixel_factor (width, x_min, x_max, &x_unit_to_pixel)) {
			g_warning ("Bad x range. This is too boring to plot.");
		}
		if (get_unit_to_pixel_factor (height, y_min, y_max, &y_unit_to_pixel)) {
			g_warning ("Bad y range. This is too boring to plot.");
		}
		x_nil_pixel = -x_min * x_unit_to_pixel;
		y_nil_pixel = -y_min * y_unit_to_pixel;

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
			              x_min, x_max, x_nil_pixel, x_unit_to_pixel,
			              y_min, y_max, y_nil_pixel, y_unit_to_pixel);
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
