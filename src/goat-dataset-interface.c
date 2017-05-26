#include "goat-dataset-interface.h"

G_DEFINE_INTERFACE (GoatDataset, goat_dataset, G_TYPE_OBJECT)

static void goat_dataset_default_init (GoatDatasetInterface *iface)
{
	iface->get_marker_style = NULL;
	iface->iter_init = NULL;
	iface->iter_next = NULL;
	iface->get = NULL;
	iface->get_extrema = NULL;
	iface->get_color = NULL;
	iface->has_valid_standard_deviation = NULL;
	iface->is_interpolation_enabled = NULL;
}

GoatMarkerStyle goat_dataset_get_marker_style (GoatDataset *self)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_style) {
		return iface->get_marker_style (self);
	}
	return GOAT_MARKER_STYLE_INVALID;
}


void goat_dataset_get_color (GoatDataset *self, GdkRGBA *rgba)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_color) {
		iface->get_color (self, rgba);
	} else {
		gdk_rgba_parse (rgba, "red");
	}
}

gboolean goat_dataset_get_iter_first (GoatDataset *self, GoatDatasetIter *iter)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->iter_init) {
		return iface->iter_init (self, iter);
	} else {
		g_error ("Implementing the `iter_init` interface for GoatDataset is necessary!");
	}
	return FALSE;
}

gboolean goat_dataset_iter_next (GoatDataset *self, GoatDatasetIter *iter)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->iter_next) {
		return iface->iter_next (self, iter);
	} else {
		g_error ("Implementing the `iter_next` interface for GoatDataset is necessary!");
	}
	return FALSE;
}

gboolean goat_dataset_get (GoatDataset *self, GoatDatasetIter *iter, gdouble *x, gdouble *y, gdouble *ystddev)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get) {
		return iface->get (self, iter, x, y, ystddev);
	} else {
		g_error ("Implementing the `get` interface for GoatDataset is necessary!");
	}
	return FALSE;
}

gboolean goat_dataset_get_extrema (GoatDataset *self, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_extrema) {
		return iface->get_extrema (self, xmin, xmax, ymin, ymax);
	} else {
		g_error ("Implementing the `get` interface for GoatDataset is necessary!");
	}
	return FALSE;
}

gboolean goat_dataset_interpolate (GoatDataset *self)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->is_interpolation_enabled) {
		return iface->is_interpolation_enabled (self);
	} else {
		g_error ("Implementing the `is_interpolation_enabled` interface for GoatDataset is necessary!");
	}
	return FALSE;
}

gboolean goat_dataset_has_valid_standard_deviation (GoatDataset *self)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->has_valid_standard_deviation) {
		return iface->has_valid_standard_deviation (self);
	} else {
		g_error ("Implementing the `has_valid_standard_deviation` interface for GoatDataset is necessary!");
	}
	return FALSE;
}
