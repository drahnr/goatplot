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

static gboolean goat_plot_draw (GtkWidget *widget, cairo_t *cr);
static void goat_plot_get_prefered_width (GtkWidget *widget, int *minimal, int *natural);
static void goat_plot_get_prefered_height (GtkWidget *widget, int *minimal, int *natural);
static gboolean goat_plot_event (GtkWidget *widget, GdkEvent *event);
static gboolean goat_plot_scroll_event (GtkWidget *widget, GdkEventScroll *event);

#define GOAT_PLOT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GOAT_TYPE_PLOT, GoatPlotPrivate))

struct _GoatPlotPrivate
{
	GArray *array;
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
	widget_class->draw = goat_plot_draw;
#if 0
	widget_class->button_press_event = goat_plot_button_press_event;
	widget_class->button_release_event = goat_plot_button_release_event;
	widget_class->key_press_event = goat_plot_key_press_event;
	widget_class->key_release_event = goat_plot_key_release_event;
#endif
	widget_class->scroll_event = goat_plot_scroll_event;
	widget_class->get_preferred_width = goat_plot_get_prefered_width;
	widget_class->get_preferred_height = goat_plot_get_prefered_height;
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


//FIXME draw on a surface so redraw is really really fast
static gboolean
draw_dataset (GoatPlot *plot, cairo_t *cr, GoatDataset *dataset)
{
	int column_width;
	int x_max, x_min;
	int y_max, y_min;
	int bar_height;

	if (goat_dataset_get_length(dataset) <= 0) {
		return FALSE;
	}

	gdouble x,y;
	GoatDatasetIter dit;


	column_width = 20; //allocation->width / plot->num_valy;

	// find max
	goat_dataset_iter_init (&dit, dataset);
	if (goat_dataset_iter_next (&dit, &x, &y)) {
		x_min = x_max = x;
		y_min = y_max = y;
		while (goat_dataset_iter_next (&dit, &x, &y)) {
			if (x_max < y) {
				x_max = y;
			} else if (x_min > y) {
				x_min = y;
			}
			if (y_max < y) {
				y_max = y;
			} else if (y_min > y) {
				y_min = y;
			}
		}
	}

	goat_dataset_iter_init (&dit, dataset);

	// draw points
	const double diameter = 4.;
	cairo_set_source_rgba (cr, 0., 1., 0., 1.);
	while (goat_dataset_iter_next (&dit, &x, &y)) {
		// TODO spline/linear interconnect here
		switch (goat_dataset_get_style (dataset)) {
		case GOAT_STYLE_SQUARE:
			cairo_rectangle (cr, x-diameter/2., y-diameter/2.,
				                 diameter, diameter);
			break;
		case GOAT_STYLE_POINT:
			cairo_move_to (cr, x + diameter / 2., y + diameter / 2.);
			cairo_arc (cr,
			           x, y,
			           diameter / 2.,
			           0., 2 * M_PI);
			break;
		default:
			g_warning ("not implemented yet... sorry.");
			break;
		}
	}
	cairo_fill (cr);
	return TRUE;
}


static gboolean
goat_plot_draw (GtkWidget *widget, cairo_t *cr)
{
	GoatPlot *plot;
	GoatDataset *dataset;
	GoatPlotPrivate *priv;
	GtkAllocation allocation; //==gint x,y,width,height
	GtkBorder padding = {18, 18, 18, 18}; // left, right, top, bottom
	gint i;

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

		// draw the background
		draw_background (plot, cr, &allocation, &padding);

		// the scale we just drew should not be drawable either
		// now the coords are y↑ x→
		cairo_rectangle (cr,
			             -allocation.x + 1,
			             -allocation.y + 1,
			             allocation.width - padding.left - padding.right - 2,
			             allocation.height - padding.top - padding.bottom - 2);
		cairo_clip (cr);
		// draw the actual data
		for (i=0; i<priv->array->len; i++) {
			dataset = g_array_index (priv->array, GoatDataset *, i);
			draw_dataset (plot, cr, dataset);
		}
		cairo_restore (cr);
		return TRUE;
	}
	return FALSE;
}

void
goat_plot_get_prefered_width (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}

void
goat_plot_get_prefered_height (GtkWidget *widget, int *minimal, int *natural)
{
	*minimal = 200;
	*natural = 350;
}






static gboolean
goat_plot_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
	g_print ("scroll event\n");
}





static gboolean
goat_plot_event (GtkWidget *widget, GdkEvent *event)
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
