/*
 * goat-dataset.c
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



#include "goat-dataset.h"
#include <gtk/gtk.h>

#define GOAT_DATASET_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GOAT_TYPE_DATASET, GoatDatasetPrivate))

struct _GoatDatasetPrivate
{
	GList *list;
	gint count;

	GoatDatasetStyle style;
};

G_DEFINE_TYPE_WITH_PRIVATE (GoatDataset, goat_dataset, G_TYPE_OBJECT);

static void
goat_dataset_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_dataset_parent_class)->finalize (object);
}

enum {
	PROP_0,
	PROP_LIST,
	PROP_COUNT,
};


static void
goat_dataset_set_gproperty (GObject *object, guint prop_id, const GValue *value, GParamSpec *spec)
{
	GoatDataset *dataset = GOAT_DATASET (object);
	GoatDatasetPrivate *priv = GOAT_DATASET_GET_PRIVATE (dataset);

	switch (prop_id) {
	case PROP_COUNT:
		priv->count = g_value_get_int (value);
		break;
	case PROP_LIST:
		priv->list = g_value_get_pointer (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (dataset, prop_id, spec);
	}
}

static void
goat_dataset_get_gproperty (GObject *object, guint prop_id, GValue *value, GParamSpec *spec)
{
	GoatDataset *dataset = GOAT_DATASET (object);
	GoatDatasetPrivate *priv = GOAT_DATASET_GET_PRIVATE (dataset);

	switch (prop_id) {
	case PROP_COUNT:
		g_value_set_int (value, priv->count);
		break;
	case PROP_LIST:
		g_value_set_pointer (value, priv->list);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (dataset, prop_id, spec);
	}
}


static void
goat_dataset_class_init (GoatDatasetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = goat_dataset_finalize;

	object_class->set_property = goat_dataset_set_gproperty;
	object_class->get_property = goat_dataset_get_gproperty;


	g_object_class_install_property (object_class, PROP_LIST,
	    g_param_spec_pointer ("list", "GoatDataset::list",
	    "the store data", G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_COUNT,
	    g_param_spec_int ("count", "GoatDataset::count",
	    "count of datapoints", -1, 10000, -1, G_PARAM_READABLE));

}

static void
goat_dataset_init (GoatDataset *self)
{
	self->priv = GOAT_DATASET_GET_PRIVATE (self);
	self->priv->list = NULL;
	self->priv->count = -1;
	self->priv->style = GOAT_DATASET_STYLE_SQUARE;
}

GoatDataset *
goat_dataset_new (GList *list)
{
	return g_object_new (GOAT_TYPE_DATASET,
	                     "list", list,
	                     NULL);
}

gint
goat_dataset_get_length (GoatDataset *dataset)
{
	GoatDatasetPrivate *priv = dataset->priv;
	if (priv->count >= 0)
		return priv->count;
	return priv->count = (gint)g_list_length (priv->list);
}


GoatDatasetStyle
goat_dataset_get_style (GoatDataset *dataset)
{
	return dataset->priv->style;
}


void
goat_dataset_iter_init (GoatDatasetIter *iter, GoatDataset *dataset)
{
	GoatDatasetPrivate *priv = dataset->priv;
	iter->state = priv->list;
}

gboolean
goat_dataset_iter_next (GoatDatasetIter *iter, double *x, double *y)
{
	GList *i = iter->state;
	GoatPair *pair = i->data;
	if (G_LIKELY (pair)) {
		if (G_LIKELY (x))
			*((double *)x) = pair->x;
		if (G_LIKELY (y))
			*((double *)y) = pair->y;
	}
	iter->state = i->next;
	return (iter->state!=NULL);
}



//FIXME add some caching
gboolean
goat_dataset_get_extrema (GoatDataset *dataset,
                          double *xmin, double *xmax,
                          double *ymin, double *ymax)
{
	GoatDatasetIter iter;
	double x, y;
	double register x_min, y_min;
	double register x_max, y_max;

	goat_dataset_iter_init (&iter, dataset);
	if (goat_dataset_iter_next (&iter, &x, &y)) {
		x_min = x_max = x;
		y_min = y_max = y;
		while (goat_dataset_iter_next (&iter, &x, &y)) {
			if (x<x_min) {
				x_min = x;
			} else if (x>x_max) {
				x_max = x;
			}
			if (y<y_min) {
				y_min = y;
			} else if (y>y_max) {
				y_max = y;
			}
		}
	} else {
		return FALSE;
	}
	*xmin = x_min;
	*xmax = x_max;
	*ymin = y_min;
	*ymax = y_max;
	return TRUE;
}
