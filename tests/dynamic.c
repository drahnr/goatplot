#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <goatplot.h>

typedef struct {
	GoatPlot *plot;
	GoatDatasetSimple *dataset;
} Both;

gboolean dynamic_add (Both *both)
{
	static uint16_t idx = 0;
	idx++;
	double x = idx * 0.1;
	double y = cos (x + 0.5 * M_PI) * (sqrt (idx * idx * idx));
	goat_dataset_simple_append (both->dataset, x, y, fabs(sin(x) * x * 20));
	if (idx < 200) {
		gtk_widget_queue_draw (GTK_WIDGET (both->plot));
		return G_SOURCE_CONTINUE;
	}
	//	g_timeout_add (500, (GSourceFunc)gtk_main_quit, NULL);
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

	goat_scale_linear_set_ticks (GOAT_SCALE_LINEAR (scale_y), 100, 2);

	plot = goat_plot_new (scale_x, scale_y);

	GoatDatasetSimple *dataset;

	GdkRGBA color1, color2, color3;
	gdk_rgba_parse(&color1, "yellow");
	gdk_rgba_parse(&color2, "orange");
	gdk_rgba_parse(&color3, "red");

	dataset = goat_dataset_simple_new (NULL, TRUE, TRUE);
	goat_dataset_simple_set_style (dataset, GOAT_MARKER_STYLE_TRIANGLE);
	goat_dataset_simple_set_color(dataset, &color1);
	goat_dataset_simple_set_marker_line_color(dataset, &color2);
	goat_dataset_simple_set_marker_fill_color(dataset, &color3);
	goat_plot_add_dataset (plot, GOAT_DATASET (dataset));

	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (plot));
	gtk_widget_show_all (window);
	g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_main_quit), NULL);

	Both both;
	both.plot = plot;
	both.dataset = dataset;
	g_timeout_add (10, (GSourceFunc)dynamic_add, &both);

	gtk_main ();

	return EXIT_SUCCESS;
}
