/**
 * @file	main.c
 * @date	Apr 20, 2016
 * @author	beattie
 *
 * @brief	file contents
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <goat-plot.h>
#include <math.h>

typedef struct {
	GoatPlot *plot;
	GoatDataset *dataset;
} Both;

gboolean dynamic_add(Both *both)
{
	static uint16_t idx = 0;
	double x = 0;
	double y = 0;

	if(idx > 100) {
		g_timeout_add(500, (GSourceFunc)gtk_main_quit, NULL);
		return 0;
	}
	g_print("whatsoever callback\n");
	idx++;
	x = idx * 0.1;
	y = cos(x + 0.5 * M_PI) * (sqrt(0.01 * idx * idx * idx));
	goat_dataset_append(both->dataset, x, y);
	gtk_widget_queue_draw(GTK_WIDGET(both->plot));
	return 1;
}

int main(int argc, char *argv[])
{
	GtkBuilder *builder;
	GtkWidget *window;
	GoatPlot *plot;
	GoatDataset *dataset;
	Both both;

	gtk_init(&argc, &argv);

	plot = goat_plot_new();


	if((builder = gtk_builder_new_from_file("../../tests/glade-line.glade")) ==
			NULL) {
		fprintf(stderr, "gtk_builder_new failed\n");
		exit(-1);
	}

	if((window = GTK_WIDGET(gtk_builder_get_object(builder, "MainWindow"))) ==
			NULL) {
		fprintf(stderr, "gtk_builder_get_object failed\n");
		exit(-1);
	};

	if((plot =
			GOAT_PLOT(gtk_builder_get_object(builder, "GoatPlot"))) ==
			NULL) {
		fprintf(stderr, "gtk_builder_get_object GoatPlot failed\n");
		exit (-1);
	}

	gtk_widget_show_all(window);
	g_signal_connect(G_OBJECT (window), "delete-event",
			G_CALLBACK(gtk_main_quit), NULL);

	g_object_unref(builder);

	dataset = goat_dataset_new(NULL);
	goat_dataset_set_style(dataset, GOAT_DATASET_STYLE_LINE);
	goat_plot_add_dataset(plot, dataset);

	goat_plot_set_range_x(plot, 0.0, 10.0);
	goat_plot_set_range_y(plot, -100.0, 100.0);

	both.plot = plot;
	both.dataset = dataset;
	g_timeout_add (20, (GSourceFunc)dynamic_add, &both);

	gtk_main();

	return EXIT_SUCCESS;
}

// called when window is closed
void on_window_main_destroy()
{
	gtk_main_quit();
}
