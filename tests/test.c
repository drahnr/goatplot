#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>

#include "goat-plot.h"

void destroy(GtkWidget *widget, gpointer data)
{
	g_print ("exiting test-simple...\n");
    gtk_main_quit();
}

#define TEST_MULTIPLE 1
int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GoatPlot *plot;
	int i;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	plot = goat_plot_new ();

	GList *list1 = NULL;
#if TEST_MULTIPLE
	GList *list2 = NULL;
	GList *list3 = NULL;
#endif
	for (i=-10; i<20; i++) {
		GoatPair *pair;

		pair = g_new (GoatPair, 1);
		pair->x = i;
		pair->y = (double)i*((double)i-16.)-4.;
		list1 = g_list_prepend (list1, pair);
#if TEST_MULTIPLE
		pair = g_new (GoatPair, 1);
		pair->x = i;
		pair->y = sin(2*M_PI/64*i)*25;
		list2 = g_list_prepend (list2, pair);

		pair = g_new (GoatPair, 1);
		pair->x = i*15.f;
		pair->y = sin(2*M_PI/64*i+M_PI/2)*55;
		list3 = g_list_prepend (list3, pair);
#endif
	}
	GoatDataset *dataset;

	dataset = goat_dataset_new (list1);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_TRIANGLE);
	g_assert (goat_dataset_get_length (dataset) > 0);
	goat_plot_add_dataset (plot, dataset);
#if TEST_MULTIPLE
	dataset = goat_dataset_new (list2);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_POINT);
	g_assert (goat_dataset_get_length (dataset) > 0);
	goat_plot_add_dataset (plot, dataset);

	dataset = goat_dataset_new (list3);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_SQUARE);
	g_assert (goat_dataset_get_length (dataset) > 0);
	goat_plot_add_dataset (plot, dataset);
#endif

	goat_plot_set_range_x (plot, -44., +30.);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (plot));
	gtk_widget_show_all (window);
	g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (destroy), NULL);

	gtk_main ();

	return EXIT_SUCCESS;
}
