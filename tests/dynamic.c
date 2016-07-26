#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "goat-plot.h"

typedef struct {
	GoatPlot *plot;
	GoatDataset *dataset;
} Both;

gboolean dynamic_add (Both *both)
{
	g_print ("whatsoever callback\n");
	static uint16_t idx = 0;
	idx++;
	double x = idx * 0.1;
	double y = cos (x + 0.5 * M_PI) * (sqrt (0.01 * idx * idx * idx));
	goat_dataset_append (both->dataset, x, y);
	if (idx < 200) {
		gtk_widget_queue_draw (GTK_WIDGET (both->plot));
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add (500, (GSourceFunc)gtk_main_quit, NULL);
	return G_SOURCE_REMOVE;
}

int main (int argc, char *argv[])
{
	GtkWidget *window;
	GoatPlot *plot;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	GoatScale *scale_x = GOAT_SCALE (goat_scale_linear_new (GOAT_POSITION_BOTTOM, GOAT_ORIENTATION_HORIZONTAL));
	GoatScale *scale_y = GOAT_SCALE (goat_scale_linear_new (GOAT_POSITION_RIGHT, GOAT_ORIENTATION_VERTICAL));

	plot = goat_plot_new (scale_x, scale_y);

	GoatDataset *dataset;

	dataset = goat_dataset_new (NULL);
	goat_dataset_set_style (dataset, GOAT_DATASET_STYLE_TRIANGLE);
	goat_plot_add_dataset (plot, dataset);

	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (plot));
	gtk_widget_show_all (window);
	g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_main_quit), NULL);

	Both both;
	both.plot = plot;
	both.dataset = dataset;
	g_timeout_add (20, (GSourceFunc)dynamic_add, &both);

	gtk_main ();

	return EXIT_SUCCESS;
}
