#include <gtk/gtk.h>
#include <stdlib.h>

#include "goat-plot.h"

void destroy(GtkWidget *widget, gpointer data)
{
	g_print ("exiting test-simple...\n");
    gtk_main_quit();
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GoatPlot *plot;
	int i;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	plot = goat_plot_new ();

	GList *list = NULL;
	for (i=0; i<120; i++) {
		GoatPair *pair = g_new (GoatPair, 1);
		pair->x = i*3.f;
		pair->y = i*i % 11 - 3.f;
		list = g_list_prepend (list, pair);
	}
	GoatDataset *dataset = goat_dataset_new (list);
	g_assert (goat_dataset_get_length (dataset) > 0);
	goat_plot_add_dataset (plot, dataset);

	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (plot));
	gtk_widget_show_all (window);
	g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (destroy), NULL);

	gtk_main ();

	return EXIT_SUCCESS;
}
