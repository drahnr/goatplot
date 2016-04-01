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

#ifndef __GOAT_DATASET_H__
#define __GOAT_DATASET_H__

#include <glib-object.h>

G_BEGIN_DECLS


//FIXME move this to GoatDatasetList
//FIXME and make GoatDataset a GInterface which allows us to have so much fun!

#define GOAT_TYPE_DATASET				(goat_dataset_get_type ())
#define GOAT_DATASET(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_DATASET, GoatDataset))
#define GOAT_DATASET_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_DATASET, GoatDataset const))
#define GOAT_DATASET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_DATASET, GoatDatasetClass))
#define GOAT_IS_DATASET(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_DATASET))
#define GOAT_IS_DATASET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_DATASET))
#define GOAT_DATASET_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_DATASET, GoatDatasetClass))

typedef struct _GoatDataset			GoatDataset;
typedef struct _GoatDatasetClass	GoatDatasetClass;
typedef struct _GoatDatasetPrivate	GoatDatasetPrivate;

struct _GoatDataset
{
	GObject parent;
	GoatDatasetPrivate *priv;
};


typedef struct {
	GoatDataset *dataset;
	//priv
	gpointer state;
} GoatDatasetIter;


struct _GoatDatasetClass
{
	GObjectClass parent_class;
};


typedef struct _GoatPair GoatPair;

struct _GoatPair
{
	gdouble x, y;
};

//FIXME this does not make much sense here, make this an option for the GoatPlot widget itself,
//FIXME it is the users duty to convert his data to the proper form
typedef enum {
	GOAT_DATASET_STYLE_UNKNOWN = 0,
	GOAT_DATASET_STYLE_POINT = 1,
	GOAT_DATASET_STYLE_SQUARE = 2,
	GOAT_DATASET_STYLE_TRIANGLE = 3,
	GOAT_DATASET_STYLE_CROSS = 4,
	GOAT_DATASET_STYLE_CUSTOM = ~1,
} GoatDatasetStyle;

GType
goat_dataset_get_type (void) G_GNUC_CONST;

GoatDataset *
goat_dataset_new (GList *list);

gint
goat_dataset_get_length (GoatDataset *dataset);

GoatDatasetStyle
goat_dataset_get_style (GoatDataset *dataset);

void
goat_dataset_set_style (GoatDataset *dataset, GoatDatasetStyle oxq);

void
goat_dataset_iter_init (GoatDatasetIter *iter, GoatDataset *dataset);

gboolean
goat_dataset_iter_next (GoatDatasetIter *iter, double *x, double *y);

gboolean
goat_dataset_get_extrema (GoatDataset *dataset,
                          double *xmin, double *xmax,
                          double *ymin, double *ymax);

void
goat_dataset_append(GoatDataset *dataset, double x, double y);

G_END_DECLS

#endif /* __GOAT_DATASET_H__ */
