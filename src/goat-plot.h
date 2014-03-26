#ifndef __GOAT_PLOT_H__
#define __GOAT_PLOT_H__

#include <gtk/gtk.h>
#include "goat-dataset.h"

G_BEGIN_DECLS

#define GOAT_TYPE_PLOT				(goat_plot_get_type ())
#define GOAT_PLOT(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_PLOT, GoatPlot))
#define GOAT_PLOT_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_PLOT, GoatPlot const))
#define GOAT_PLOT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_PLOT, GoatPlotClass))
#define GOAT_IS_PLOT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_PLOT))
#define GOAT_IS_PLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_PLOT))
#define GOAT_PLOT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_PLOT, GoatPlotClass))

typedef struct _GoatPlot		GoatPlot;
typedef struct _GoatPlotClass	GoatPlotClass;
typedef struct _GoatPlotPrivate	GoatPlotPrivate;

struct _GoatPlot
{
	GtkDrawingArea parent;

	GoatPlotPrivate *priv;
};

struct _GoatPlotClass
{
	GtkDrawingAreaClass parent_class;
};

GType goat_plot_get_type (void) G_GNUC_CONST;
GoatPlot *goat_plot_new (void);
void goat_plot_prepend_value (GoatPlot *graph, float x, float y);
gint goat_plot_add_dataset (GoatPlot *plot, GoatDataset *dataset);

G_END_DECLS

#endif /* __GOAT_PLOT_H__ */
