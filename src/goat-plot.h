/*
 * goat-plot.h
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

#ifndef __GOAT_PLOT_H__
#define __GOAT_PLOT_H__

#include "goat-dataset.h"
#include "goat-plot-enum.h"
#include "goat-scale-interface.h"
#include "goat-scale-linear.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOAT_TYPE_PLOT (goat_plot_get_type ())
#define GOAT_PLOT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_PLOT, GoatPlot))
#define GOAT_PLOT_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_PLOT, GoatPlot const))
#define GOAT_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_PLOT, GoatPlotClass))
#define GOAT_IS_PLOT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_PLOT))
#define GOAT_IS_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_PLOT))
#define GOAT_PLOT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_PLOT, GoatPlotClass))

typedef struct _GoatPlot GoatPlot;
typedef struct _GoatPlotClass GoatPlotClass;
typedef struct _GoatPlotPrivate GoatPlotPrivate;

struct _GoatPlot {
	GtkDrawingArea parent;

	GoatPlotPrivate *priv;
};

struct _GoatPlotClass {
	GtkDrawingAreaClass parent_class;
};



GType goat_plot_get_type (void) G_GNUC_CONST;
GoatPlot *goat_plot_new (GoatScale *scale_x, GoatScale *scale_y);
void goat_plot_prepend_value (GoatPlot *graph, float x, float y);
gint goat_plot_add_dataset (GoatPlot *plot, GoatDataset *dataset);

void goat_plot_set_fixed_x_size (GoatPlot *plot, gdouble min_x, gdouble max_x);
void goat_plot_set_fixed_y_size (GoatPlot *plot, gdouble min_y, gdouble max_y);
void goat_plot_set_dynamic_x_size (GoatPlot *plot, gboolean dyn);
void goat_plot_set_dynamic_y_size (GoatPlot *plot, gboolean dyn);
void goat_plot_set_grid_visible (GoatPlot *plot, gboolean visible);
void goat_plot_set_scale_x (GoatPlot *plot, GoatScale *scale_x);
void goat_plot_set_scale_y (GoatPlot *plot, GoatScale *scale_y);
G_END_DECLS

#endif /* __GOAT_PLOT_H__ */
