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

#include "goat-dataset-simple.h"
#include <goat-dataset-interface.h>
#include <gtk/gtk.h>
#include <math.h>

static void update_extrema_cache (GoatDatasetSimple *dataset);

struct _GoatDatasetSimplePrivate {
	GList *list;
	gint count;
	double x_min;
	double y_min;
	double x_max;
	double y_max;
	double x_log_min;
	double y_log_min;
	GdkRGBA color;
	GdkRGBA marker_line_color;
	GdkRGBA marker_fill_color;
	double line_width;
	double marker_line_width;
	double marker_size;
	gboolean marker_fill;

	GoatMarkerStyle style;

	gboolean interpolation_enabled;
	gboolean valid_stddev;
};

static void goat_dataset_simple_interface_init (GoatDatasetInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GoatDatasetSimple, goat_dataset_simple, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GOAT_TYPE_DATASET, goat_dataset_simple_interface_init)
                             G_ADD_PRIVATE (GoatDatasetSimple));

static void goat_dataset_simple_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_dataset_simple_parent_class)->finalize (object);
}

enum {
	PROP_0,

	PROP_LIST,
	PROP_COUNT,
	PROP_VALID_STDDEV,
	PROP_INTERPOLATION_ENABLED,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void goat_dataset_simple_set_gproperty (GObject *object, guint prop_id, const GValue *value, GParamSpec *spec)
{
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (object);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	switch (prop_id) {
	case PROP_INTERPOLATION_ENABLED:
		priv->interpolation_enabled = g_value_get_boolean (value);
		break;
	case PROP_VALID_STDDEV:
		priv->valid_stddev = g_value_get_boolean (value);
		break;
	case PROP_COUNT:
		priv->count = g_value_get_int (value);
		break;
	case PROP_LIST:
		g_list_free_full (priv->list, g_free);
		priv->list = g_value_get_pointer (value);
		update_extrema_cache (self);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (self, prop_id, spec);
	}
}

static void goat_dataset_simple_get_gproperty (GObject *object, guint prop_id, GValue *value, GParamSpec *spec)
{
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (object);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	switch (prop_id) {
	case PROP_INTERPOLATION_ENABLED:
		g_value_set_boolean(value, priv->interpolation_enabled);
		break;
	case PROP_VALID_STDDEV:
		g_value_set_boolean(value, priv->valid_stddev);
		break;
	case PROP_COUNT:
		g_value_set_int (value, priv->count);
		break;
	case PROP_LIST:
		g_value_set_pointer (value, priv->list);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (self, prop_id, spec);
	}
}

static void goat_dataset_simple_class_init (GoatDatasetSimpleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = goat_dataset_simple_finalize;

	object_class->set_property = goat_dataset_simple_set_gproperty;
	object_class->get_property = goat_dataset_simple_get_gproperty;

	obj_properties[PROP_LIST] = g_param_spec_pointer ("list", "GoatDataset::list", "the store data",
	                                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	obj_properties[PROP_COUNT] =
	    g_param_spec_int ("count", "GoatDataset::count", "count of datapoints", -1, 10000, -1, G_PARAM_READABLE);

	obj_properties[PROP_VALID_STDDEV] =
	    g_param_spec_boolean ("valid_stddev", "GoatDataset::valid_stddev", "is the stdandard deviation associated to this datatset valid", FALSE, G_PARAM_READWRITE);

	obj_properties[PROP_INTERPOLATION_ENABLED] =
	    g_param_spec_boolean ("interpolation_enabled", "GoatDataset::interpolation_enabled", "shall the points be interpolated", FALSE, G_PARAM_READWRITE);

	g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

static void goat_dataset_simple_init (GoatDatasetSimple *self)
{
	GoatDatasetSimplePrivate *priv = self->priv = goat_dataset_simple_get_instance_private (self);
	priv->list = NULL;
	priv->count = -1;
	priv->x_min = +G_MAXDOUBLE;
	priv->y_min = +G_MAXDOUBLE;
	priv->x_log_min = +G_MAXDOUBLE;
	priv->y_log_min = +G_MAXDOUBLE;
	priv->x_max = -G_MAXDOUBLE;
	priv->y_max = -G_MAXDOUBLE;
	gdk_rgba_parse (&priv->color, "blue");
	gdk_rgba_parse (&priv->marker_line_color, "blue");
	gdk_rgba_parse (&priv->marker_fill_color, "blue");
	priv->line_width = 1.5;
	priv->marker_line_width = 1.5;
	priv->marker_size = 8.0;
	priv->marker_fill = 0;
	priv->style = GOAT_MARKER_STYLE_SQUARE;
}

/**
 * create a new dataset from a GList containing GoatTriple values
 */
GoatDatasetSimple *goat_dataset_simple_new (GList *list, gboolean valid_stddev, gboolean interpolate)
{
	return g_object_new (GOAT_TYPE_DATASET_SIMPLE,
						 "valid_stddev", valid_stddev,
						 "interpolation_enabled", interpolate,
						 "list", list,
						 NULL);
}


/**
 * @param dataset
 * @param oxq node style to use for drawing of #dataset
 */
void goat_dataset_simple_set_style (GoatDatasetSimple *self, GoatMarkerStyle oxq)
{

	g_return_if_fail (self);
	g_return_if_fail (GOAT_IS_DATASET_SIMPLE (self));
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);
	priv->style = oxq;
}




static void update_extrema_cache (GoatDatasetSimple *self)
{
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	GoatDatasetIter iter;
	double x, y, ystddev;
	double register x_min, y_min, x_log_min, y_log_min;
	double register x_max, y_max;
	double register y_upper;
	double register y_lower;
	const gboolean register valid_stddev = goat_dataset_has_valid_standard_deviation (GOAT_DATASET(self));

	x_log_min = x_min = +G_MAXDOUBLE;
	y_log_min = y_min = +G_MAXDOUBLE;
	x_max = -G_MAXDOUBLE;
	y_max = -G_MAXDOUBLE;

	if (goat_dataset_get_iter_first (GOAT_DATASET (self), &iter)) {
		goat_dataset_get (GOAT_DATASET (self), &iter, &x, &y, &ystddev);
		x_min = x_max = x;
		y_min = y_max = y;

		if( x > 0. ) x_log_min = x;
		if( y > 0. ) y_log_min = y;

		if (valid_stddev) {
			y_min -= ystddev;
			y_max += ystddev;
		}
		while (goat_dataset_iter_next (GOAT_DATASET (self), &iter)) {
			goat_dataset_get (GOAT_DATASET (self), &iter, &x, &y, &ystddev);
			if (x < x_min) {
				x_min = x;
			}
			if (x > x_max) {
				x_max = x;
			}
			if( x < x_log_min && x > 0. ) {
				x_log_min = x;
			}
			y_upper = y_lower = y;
			if (valid_stddev) {
				g_assert(ystddev >= 0.);
				y_upper += ystddev;
				y_lower -= ystddev;
			}
			if (y_lower < y_min) {
				y_min = y_lower;
			}
			if (y_upper > y_max) {
				y_max = y_upper;
			}
			if( y < y_log_min && y > 0. ) {
				y_log_min = y;
			}
		}
	}

	priv->x_min = x_min;
	priv->y_min = y_min;
	priv->x_max = x_max;
	priv->y_max = y_max;
	priv->x_log_min = x_log_min;
	priv->y_log_min = y_log_min;
}

void goat_dataset_simple_append (GoatDatasetSimple *self, gdouble x, gdouble y, gdouble ystddev)
{
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	GoatTriple *pair = g_new0 (GoatTriple, 1);
	pair->x = x;
	pair->y = y;
	if (ystddev < 0.) {
		g_warning("goat_dataset_simple_append: ystddev has to be positive");
		ystddev = fabs(ystddev);
	}
	pair->ystddev = ystddev;
	if (priv->count < 0) {
		priv->count = g_list_length (priv->list);
	}
	priv->count++;
	if (priv->x_min > x)
		priv->x_min = x;
	if (priv->x_max < x)
		priv->x_max = x;
	if( priv->x_log_min > x && x > 0. )
		priv->x_log_min = x;

	const gboolean register valid_stddev = goat_dataset_has_valid_standard_deviation (GOAT_DATASET(self));
	double register y_lower = y;
	double register y_upper = y;
	if (valid_stddev) {
		g_assert(ystddev >= 0.);
		y_upper += ystddev;
		y_lower -= ystddev;
	}
	if (priv->y_min > y_lower)
		priv->y_min = y_lower;
	if (priv->y_max < y_upper)
		priv->y_max = y_upper;
	if( priv->y_log_min > y_lower && y_lower > 0. )
		priv->y_log_min = y_lower;

	/* g_printf ("calc x range: %lf..%lf\n", priv->x_min, priv->x_max); */
	/* g_printf ("calc y range: %lf..%lf\n", priv->y_min, priv->y_max); */
	priv->list = g_list_append (priv->list, pair);
}

void goat_dataset_simple_clear (GoatDatasetSimple *self)
{
	g_object_set (self, "list", NULL, NULL);
}

void goat_dataset_simple_set_color (GoatDatasetSimple *self, GdkRGBA *color)
{
	g_return_if_fail (self);
	g_return_if_fail (color);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->color = *color;
}

void goat_dataset_simple_set_marker_line_color (GoatDatasetSimple *self, GdkRGBA *color)
{
	g_return_if_fail (self);
	g_return_if_fail (color);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->marker_line_color = *color;
}

void goat_dataset_simple_set_marker_fill_color (GoatDatasetSimple *self, GdkRGBA *color)
{
	g_return_if_fail (self);
	g_return_if_fail (color);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->marker_fill_color = *color;
}

void goat_dataset_simple_set_marker_line_width (GoatDatasetSimple *self, double width)
{
	g_return_if_fail (self);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->marker_line_width = width;
}

void goat_dataset_simple_set_line_width (GoatDatasetSimple *self, double width)
{
	g_return_if_fail (self);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->line_width = width;
}

void goat_dataset_simple_set_marker_size (GoatDatasetSimple *self, double size)
{
	g_return_if_fail (self);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->marker_size = size;
}

void goat_dataset_simple_set_marker_fill (GoatDatasetSimple *self, gboolean marker_fill)
{
	g_return_if_fail (self);

	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	priv->marker_fill = marker_fill;
}

static void get_color (GoatDataset *dataset, GdkRGBA *color)
{
	g_return_if_fail (dataset);
	g_return_if_fail (color);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*color = priv->color;
}

static void get_marker_line_color (GoatDataset *dataset, GdkRGBA *color)
{
	g_return_if_fail (dataset);
	g_return_if_fail (color);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*color = priv->marker_line_color;
}

static void get_marker_fill_color (GoatDataset *dataset, GdkRGBA *color)
{
	g_return_if_fail (dataset);
	g_return_if_fail (color);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*color = priv->marker_fill_color;
}

static void get_marker_line_width (GoatDataset *dataset, double *width)
{
	g_return_if_fail (dataset);
	g_return_if_fail (width);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*width = priv->marker_line_width;
}

static void get_line_width (GoatDataset *dataset, double *width)
{
	g_return_if_fail (dataset);
	g_return_if_fail (width);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*width = priv->line_width;
}

static void get_marker_size (GoatDataset *dataset, double *size)
{
	g_return_if_fail (dataset);
	g_return_if_fail (size);

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	*size = priv->marker_size;
}

static gboolean get_marker_fill (GoatDataset *dataset)
{
	if(!dataset) return FALSE;

	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	return priv->marker_fill;
}

static gboolean get_extrema (GoatDataset *dataset, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	if (priv->list != NULL) {
		if (xmin)
			*xmin = priv->x_min;
		if (xmax)
			*xmax = priv->x_max;
		if (ymin)
			*ymin = priv->y_min;
		if (ymax)
			*ymax = priv->y_max;
		return TRUE;
	}
	priv->list = NULL;
	priv->count = -1;
	priv->x_min = +G_MAXDOUBLE;
	priv->y_min = +G_MAXDOUBLE;
	priv->x_max = -G_MAXDOUBLE;
	priv->y_max = -G_MAXDOUBLE;
	return FALSE;
}

static gboolean get_log_extrema (GoatDataset *dataset, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);

	if (priv->list != NULL) {
		if (xmin)
			*xmin = priv->x_log_min;
		if (xmax)
			*xmax = priv->x_max;
		if (ymin)
			*ymin = priv->y_log_min;
		if (ymax)
			*ymax = priv->y_max;
		return TRUE;
	}

	priv->list = NULL;
	priv->count = -1;
	priv->x_log_min = +G_MAXDOUBLE;
	priv->y_log_min = +G_MAXDOUBLE;
	priv->x_max = -G_MAXDOUBLE;
	priv->y_max = -G_MAXDOUBLE;
	return FALSE;
}


/**
 * initilialize the iterator #iter for #dataset
 * @param iter
 * @param dataset
 */
gboolean iter_init (GoatDataset *dataset, GoatDatasetIter *iter)
{
	g_return_val_if_fail (dataset, FALSE);
	g_return_val_if_fail (GOAT_IS_DATASET_SIMPLE (dataset), FALSE);
	g_return_val_if_fail (iter, FALSE);
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);
	iter->dataset = dataset;
	iter->state = priv->list;
	return (iter->state != NULL);
}

static gboolean iter_next (GoatDataset *dataset, GoatDatasetIter *iter)
{
	g_return_val_if_fail (iter, FALSE);
	g_return_val_if_fail (iter->state, FALSE);
	g_return_val_if_fail (dataset == iter->dataset, FALSE);
	iter->state = ((GList *)(iter->state))->next;
	return (iter->state != NULL);
}

static gboolean get (GoatDataset *dataset, GoatDatasetIter *iter, gdouble *x, gdouble *y, gdouble *ystddev)
{
	g_return_val_if_fail (iter, FALSE);
	g_return_val_if_fail (iter->state, FALSE);
	g_return_val_if_fail (dataset == iter->dataset, FALSE);
	GoatTriple *pair = ((GList *)(iter->state))->data;
	if (G_LIKELY (pair)) {
		if (G_LIKELY (x))
			*((double *)x) = pair->x;
		if (G_LIKELY (y))
			*((double *)y) = pair->y;
		if (G_LIKELY (ystddev))
			*((double *)ystddev) = pair->ystddev;
		return TRUE;
	}
	return FALSE;
}


static GoatMarkerStyle get_marker_style (GoatDataset *dataset)
{
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);
	return priv->style;
}

static gboolean has_valid_standard_deviation(GoatDataset *dataset) {
	g_return_val_if_fail(GOAT_IS_DATASET_SIMPLE (dataset), FALSE);
	return GOAT_DATASET_SIMPLE(dataset)->priv->valid_stddev;
}

static gboolean is_interpolation_enabled(GoatDataset *dataset) {
	g_return_val_if_fail(GOAT_IS_DATASET_SIMPLE (dataset), FALSE);
	return GOAT_DATASET_SIMPLE(dataset)->priv->interpolation_enabled;
}

/**
 * return the number of items contained in the dataset
 * @param dataset
 */
gint get_length (GoatDataset *dataset)
{
	g_return_val_if_fail(GOAT_IS_DATASET_SIMPLE (dataset), FALSE);
	GoatDatasetSimple *self = GOAT_DATASET_SIMPLE (dataset);
	GoatDatasetSimplePrivate *priv = goat_dataset_simple_get_instance_private (self);
	if (priv->count >= 0)
		return priv->count;
	return priv->count = (gint)g_list_length (priv->list);
}


static void goat_dataset_simple_interface_init (GoatDatasetInterface *iface)
{
	iface->get_color = get_color;
	iface->get_marker_line_color = get_marker_line_color;
	iface->get_marker_fill_color = get_marker_fill_color;
	iface->get_marker_line_width = get_marker_line_width;
	iface->get_line_width = get_line_width;
	iface->get_marker_size = get_marker_size;
	iface->get_extrema = get_extrema;
	iface->get_log_extrema = get_log_extrema;
	iface->is_interpolation_enabled = is_interpolation_enabled;
	iface->has_valid_standard_deviation = has_valid_standard_deviation;
	iface->iter_init = iter_init;
	iface->iter_next = iter_next;
	iface->get_marker_style = get_marker_style;
	iface->get = get;
	iface->get_marker_fill = get_marker_fill;
	iface->get_length = get_length;
}
