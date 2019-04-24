/*
 * goat-dataset.h
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

#ifndef GOAT_DATASET_SIMPLE_H
#define GOAT_DATASET_SIMPLE_H

#include <gdk/gdk.h>
#include <glib-object.h>
#include <goat-dataset-interface.h>
#include <goat-plot-enum.h>

G_BEGIN_DECLS

#define GOAT_TYPE_DATASET_SIMPLE (goat_dataset_simple_get_type ())
#define GOAT_DATASET_SIMPLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_DATASET_SIMPLE, GoatDatasetSimple))
#define GOAT_DATASET_SIMPLE_CONST(obj)                                                                                 \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_DATASET_SIMPLE, GoatDatasetSimple const))
#define GOAT_DATASET_SIMPLE_CLASS(klass)                                                                               \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_DATASET_SIMPLE, GoatDatasetSimpleClass))
#define GOAT_IS_DATASET_SIMPLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_DATASET_SIMPLE))
#define GOAT_IS_DATASET_SIMPLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_DATASET_SIMPLE))
#define GOAT_DATASET_SIMPLE_GET_CLASS(obj)                                                                             \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_DATASET_SIMPLE, GoatDatasetSimpleClass))

typedef struct _GoatDatasetSimple GoatDatasetSimple;
typedef struct _GoatDatasetSimpleClass GoatDatasetSimpleClass;
typedef struct _GoatDatasetSimplePrivate GoatDatasetSimplePrivate;

struct _GoatDatasetSimple {
	GObject parent;
	GoatDatasetSimplePrivate *priv;
};



struct _GoatDatasetSimpleClass {
	GObjectClass parent_class;
};


typedef struct _GoatTriple GoatTriple;

struct _GoatTriple {
	gdouble x, y, ystddev;
};

GType goat_dataset_simple_get_type (void) G_GNUC_CONST;

// TODO use enums instead of boolean
GoatDatasetSimple *goat_dataset_simple_new (GList *list, gboolean valid_stddev, gboolean interpolate);

gint goat_dataset_simple_get_length (GoatDatasetSimple *dataset);

void goat_dataset_simple_set_style (GoatDatasetSimple *dataset, GoatMarkerStyle oxq);

void goat_dataset_simple_append (GoatDatasetSimple *dataset, gdouble x, gdouble y, gdouble ystddev);

void goat_dataset_simple_set_color (GoatDatasetSimple *dataset, GdkRGBA *color);

void goat_dataset_simple_set_marker_line_color (GoatDatasetSimple *self, GdkRGBA *color);

void goat_dataset_simple_set_marker_fill_color (GoatDatasetSimple *self, GdkRGBA *color);

void goat_dataset_simple_set_marker_line_width (GoatDatasetSimple *self, double width);

void goat_dataset_simple_set_line_width (GoatDatasetSimple *self, double width);

void goat_dataset_simple_set_marker_size (GoatDatasetSimple *self, double size);

void goat_dataset_simple_set_marker_fill (GoatDatasetSimple *self, gboolean marker_fill);


G_END_DECLS

#endif /* GOAT_DATASET_SIMPLE_H */
