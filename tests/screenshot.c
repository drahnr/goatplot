#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>

#include "goat-plot.h"

void destroy (GtkWidget *widget, gpointer data)
{
	g_print ("\n\n>>>>> creating screenshot...\n");
	{
		cairo_surface_t *surface;
		cairo_t *cr;
		GtkAllocation allocation;

		gtk_widget_get_allocation (widget, &allocation);
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
		g_assert (surface);
		cr = cairo_create (surface);
		gtk_widget_draw (GTK_WIDGET (widget), cr);
		const cairo_status_t status = cairo_surface_write_to_png (surface, "./screenshot.png");
		g_assert (status == CAIRO_STATUS_SUCCESS);
		cairo_surface_destroy (surface);
		cairo_destroy (cr);
	}
	g_print ("\n\n>>>>> exiting test-simple...\n");
	gtk_main_quit ();
}

gboolean self_destruct (GtkWidget *widget)
{
	destroy (widget, NULL);
	return G_SOURCE_REMOVE;
}

#define TEST_MULTIPLE 1
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *box;
	GoatPlot *plot, *plot_clone;
	GoatScale *scale_x, *scale_y, *scale_x_auto;
	int i;
	GdkRGBA color;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	scale_x = GOAT_SCALE (goat_scale_linear_new (GOAT_POSITION_TOP, GOAT_ORIENTATION_HORIZONTAL));
	goat_scale_set_range (scale_x, -44., +30.);
	goat_scale_linear_set_ticks (GOAT_SCALE_LINEAR (scale_x), 100, 5);
	scale_y = GOAT_SCALE (goat_scale_linear_new (GOAT_POSITION_LEFT, GOAT_ORIENTATION_VERTICAL));
	goat_scale_set_range_auto (scale_y);
	goat_scale_linear_set_ticks (GOAT_SCALE_LINEAR (scale_y), 50, 5);
	plot = goat_plot_new (scale_x, scale_y);

	scale_x_auto = GOAT_SCALE (goat_scale_linear_new (GOAT_POSITION_BOTTOM, GOAT_ORIENTATION_HORIZONTAL));
	goat_scale_set_range (scale_x, -100., +300.);
	plot_clone = goat_plot_new (scale_x_auto, scale_y); // yep, we can re-use those

	GList *list1 = NULL;
#if TEST_MULTIPLE
	GList *list2 = NULL;
	GList *list3 = NULL;
#endif
	for (i = -10; i < 20; i++) {
		GoatPair *pair;

		pair = g_new (GoatPair, 1);
		pair->x = i;
		pair->y = (double)i * ((double)i - 16.) - 4.;
		list1 = g_list_prepend (list1, pair);
#if TEST_MULTIPLE
		pair = g_new (GoatPair, 1);
		pair->x = i;
		pair->y = sin (2 * M_PI / 64 * i) * 25;
		list2 = g_list_prepend (list2, pair);

		pair = g_new (GoatPair, 1);
		pair->x = i * 15.f;
		pair->y = sin (2 * M_PI / 64 * i + M_PI / 2) * 55;
		list3 = g_list_prepend (list3, pair);
#endif
	}
	GoatDataset *dataset;

	dataset = goat_dataset_new (list1);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_TRIANGLE);
	g_assert (goat_dataset_get_length (dataset) > 0);
	gdk_rgba_parse (&color, "royalblue");
	goat_dataset_set_color (dataset, &color);
	goat_plot_add_dataset (plot, dataset);
	goat_plot_add_dataset (plot_clone, dataset);
#if TEST_MULTIPLE
	dataset = goat_dataset_new (list2);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_POINT);
	g_assert (goat_dataset_get_length (dataset) > 0);
	gdk_rgba_parse (&color, "green");
	goat_dataset_set_color (dataset, &color);
	goat_plot_add_dataset (plot, dataset);

	dataset = goat_dataset_new (list3);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_SQUARE);
	g_assert (goat_dataset_get_length (dataset) > 0);
	gdk_rgba_parse (&color, "goldenrod");
	goat_dataset_set_color (dataset, &color);
	goat_plot_add_dataset (plot, dataset);
#endif

	gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (plot));
	gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (plot_clone));
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (box));
	gtk_widget_show_all (window);
	g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (destroy), NULL);

	g_timeout_add (1000, (GSourceFunc)self_destruct, GTK_WIDGET (window));

	gtk_main ();

	return EXIT_SUCCESS;
}
