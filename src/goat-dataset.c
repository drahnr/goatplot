/*
 * goat-dataset.c
 * This file is part of GoatPlot
 *
 * Copyright (C) 2014,2016 - Bernhard Schuster <bernhard@ahoi.io>
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "goat-dataset.h"
#include <math.h>

static void update_extrema_cache (GoatDataset *dataset);

#define GOAT_DATASET_GET_PRIVATE(object)                                                           \
	(G_TYPE_INSTANCE_GET_PRIVATE ((object), GOAT_TYPE_DATASET, GoatDatasetPrivate))

struct _GoatDatasetPrivate {
	GList *list;
	gint count;
	double x_min;
	double y_min;
	double x_max;
	double y_max;
	GdkRGBA color;

	GoatDatasetStyle style;
};

G_DEFINE_TYPE (GoatDataset, goat_dataset, G_TYPE_OBJECT);

static void goat_dataset_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_dataset_parent_class)->finalize (object);
}

enum {
	PROP_0,

	PROP_LIST,
	PROP_COUNT,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void goat_dataset_set_gproperty (GObject *object, guint prop_id, const GValue *value,
                                        GParamSpec *spec)
{
	GoatDataset *dataset = GOAT_DATASET (object);
	GoatDatasetPrivate *priv = GOAT_DATASET_GET_PRIVATE (dataset);

	switch (prop_id) {
	case PROP_COUNT:
		priv->count = g_value_get_int (value);
		break;
	case PROP_LIST:
		g_list_free_full (priv->list, g_free);
		priv->list = g_value_get_pointer (value);
		update_extrema_cache (dataset);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (dataset, prop_id, spec);
	}
}

static void goat_dataset_get_gproperty (GObject *object, guint prop_id, GValue *value,
                                        GParamSpec *spec)
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

static void goat_dataset_class_init (GoatDatasetClass *klass)
{
	g_type_class_add_private (klass, sizeof (GoatDatasetPrivate));

	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = goat_dataset_finalize;

	object_class->set_property = goat_dataset_set_gproperty;
	object_class->get_property = goat_dataset_get_gproperty;

	obj_properties[PROP_LIST] = g_param_spec_pointer ("list", "GoatDataset::list", "the store data",
	                                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	obj_properties[PROP_COUNT] = g_param_spec_int (
	    "count", "GoatDataset::count", "count of datapoints", -1, 10000, -1, G_PARAM_READABLE);

	g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

static void goat_dataset_init (GoatDataset *self)
{
	self->priv = GOAT_DATASET_GET_PRIVATE (self);
	self->priv->list = NULL;
	self->priv->count = -1;
	self->priv->x_min = +G_MAXDOUBLE;
	self->priv->y_min = +G_MAXDOUBLE;
	self->priv->x_max = -G_MAXDOUBLE;
	self->priv->y_max = -G_MAXDOUBLE;
	gdk_rgba_parse (&self->priv->color, "blue");
	self->priv->style = GOAT_DATASET_STYLE_SQUARE;
}

/**
 * create a new dataset from a GList containing GoatPair values
 */
GoatDataset *goat_dataset_new (GList *list)
{
	return g_object_new (GOAT_TYPE_DATASET, "list", list, NULL);
}

/**
 * return the number of items contained in the dataset
 * @param dataset
 */
gint goat_dataset_get_length (GoatDataset *dataset)
{
	GoatDatasetPrivate *priv = dataset->priv;
	if (priv->count >= 0)
		return priv->count;
	return priv->count = (gint)g_list_length (priv->list);
}

/**
 * @param dataset
 * @returns node style of #dataset
 */
GoatDatasetStyle goat_dataset_get_style (GoatDataset *dataset)
{
	g_return_val_if_fail (dataset, GOAT_DATASET_STYLE_UNKNOWN);
	g_return_val_if_fail (GOAT_IS_DATASET (dataset), GOAT_DATASET_STYLE_UNKNOWN);
	return dataset->priv->style;
}

/**
 * @param dataset
 * @param oxq node style to use for drawing of #dataset
 */
void goat_dataset_set_style (GoatDataset *dataset, GoatDatasetStyle oxq)
{
	g_return_if_fail (dataset);
	g_return_if_fail (GOAT_IS_DATASET (dataset));
	dataset->priv->style = oxq;
}

/**
 * initilialize the iterator #iter for #dataset
 * @param iter
 * @param dataset
 */
void goat_dataset_iter_init (GoatDatasetIter *iter, GoatDataset *dataset)
{
	GoatDatasetPrivate *priv = dataset->priv;
	iter->state = priv->list;
}

/**
 * advance the iterator by one element
 * @param iter
 * @param x [out] x data if available
 * @param y [out] y data if available
 * @returns FALSE if no more data was available, otherwise TRUE
 */
gboolean goat_dataset_iter_next (GoatDatasetIter *iter, double *x, double *y)
{
	GList *i = iter->state;
	if (i == NULL) {
		return FALSE;
	}
	GoatPair *pair = i->data;
	if (G_LIKELY (pair)) {
		if (G_LIKELY (x))
			*((double *)x) = pair->x;
		if (G_LIKELY (y))
			*((double *)y) = pair->y;
	}
	iter->state = i->next;
	return (iter->state != NULL);
}

// TODO add some caching
/**
 * @param dataset
 * @param xmin [out]
 * @param xmax [out]
 * @param ymin [out]
 * @param ymax [out]
 */
gboolean goat_dataset_get_extrema (GoatDataset *dataset, double *xmin, double *xmax, double *ymin,
                                   double *ymax)
{
	if (dataset->priv->list != NULL) {
		if (xmin)
			*xmin = dataset->priv->x_min;
		if (xmax)
			*xmax = dataset->priv->x_max;
		if (ymin)
			*ymin = dataset->priv->y_min;
		if (ymax)
			*ymax = dataset->priv->y_max;
		return TRUE;
	}
	dataset->priv->list = NULL;
	dataset->priv->count = -1;
	dataset->priv->x_min = +G_MAXDOUBLE;
	dataset->priv->y_min = +G_MAXDOUBLE;
	dataset->priv->x_max = -G_MAXDOUBLE;
	dataset->priv->y_max = -G_MAXDOUBLE;
	return FALSE;
}

static void update_extrema_cache (GoatDataset *dataset)
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
			if (x < x_min) {
				x_min = x;
			}
			if (x > x_max) {
				x_max = x;
			}
			if (y < y_min) {
				y_min = y;
			}
			if (y > y_max) {
				y_max = y;
			}
		}
		dataset->priv->x_min = x_min;
		dataset->priv->x_max = x_max;
		dataset->priv->y_min = y_min;
		dataset->priv->y_max = y_max;
	}
}

void goat_dataset_append (GoatDataset *dataset, double x, double y)
{
	GoatPair *pair = g_new0 (GoatPair, 1);
	pair->x = x;
	pair->y = y;
	if (dataset->priv->count < 0) {
		dataset->priv->count = g_list_length (dataset->priv->list);
	}
	dataset->priv->count++;
	if (dataset->priv->x_min > x)
		dataset->priv->x_min = x;
	if (dataset->priv->y_min > y)
		dataset->priv->y_min = y;
	if (dataset->priv->x_max < x)
		dataset->priv->x_max = x;
	if (dataset->priv->y_max < y)
		dataset->priv->y_max = y;
	dataset->priv->list = g_list_append (dataset->priv->list, pair);
	g_print ("total items of %i with bounds [x%lf x%lf y%lf y%lf]\n", dataset->priv->count,
	         dataset->priv->x_min, dataset->priv->x_max, dataset->priv->y_min,
	         dataset->priv->y_max);
}

void goat_dataset_clear (GoatDataset *dataset) { g_object_set (dataset, "list", NULL, NULL); }

void goat_dataset_set_color (GoatDataset *dataset, GdkRGBA *color)
{
	GoatDatasetPrivate *priv;

	g_return_if_fail (dataset);
	g_return_if_fail (color);

	priv = GOAT_DATASET_GET_PRIVATE (dataset);

	priv->color = *color;
}

void goat_dataset_get_color (GoatDataset *dataset, GdkRGBA *color)
{
	GoatDatasetPrivate *priv;

	g_return_if_fail (dataset);
	g_return_if_fail (color);

	priv = GOAT_DATASET_GET_PRIVATE (dataset);

	*color = priv->color;
}
