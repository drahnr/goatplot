#include "goat-dataset-interface.h"

G_DEFINE_INTERFACE (GoatDataset, goat_dataset, G_TYPE_OBJECT)


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

int goat_dataset_get_length (GoatDataset *self)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_length) {
		return iface->get_length (self);
	} else {
		return 0;
	}
}


gboolean goat_dataset_get_marker_fill (GoatDataset *self)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_fill) {
		return iface->get_marker_fill (self);
	} else {
		return FALSE;
	}
}

void goat_dataset_get_marker_line_color (GoatDataset *self, GdkRGBA *rgba)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_line_color) {
		iface->get_marker_line_color (self, rgba);
	} else {
		gdk_rgba_parse (rgba, "red");
	}
}

void goat_dataset_get_marker_fill_color (GoatDataset *self, GdkRGBA *rgba)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_fill_color) {
		iface->get_marker_fill_color (self, rgba);
	} else {
		gdk_rgba_parse (rgba, "red");
	}
}

void goat_dataset_get_marker_line_width (GoatDataset *self, double *width)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_line_width) {
		iface->get_marker_line_width (self, width);
	} else {
		*width = 1.5;
	}
}

void goat_dataset_get_line_width (GoatDataset *self, double *width)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_line_width) {
		iface->get_line_width (self, width);
	} else {
		*width = 1.5;
	}
}

void goat_dataset_get_marker_size (GoatDataset *self, double *size)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (self);
	if (iface->get_marker_size) {
		iface->get_marker_size (self, size);
	} else {
		*size = 8.;
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

gboolean goat_dataset_get_log_extrema (GoatDataset *dataset, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
	GoatDatasetInterface *iface;

	iface = GOAT_DATASET_GET_IFACE (dataset);
	if (iface->get_log_extrema) {
		return iface->get_log_extrema (dataset, xmin, xmax, ymin, ymax);
	} else if (iface->get_extrema) {
		return iface->get_extrema (dataset, xmin, xmax, ymin, ymax);
	} else {
		*xmin = *xmax = *ymin = *ymax = 0.;
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

/**
 * default implementation for get extrema
 *
 * @attention this is slow, it is recommended to cache this if possible, see #GoatDatasetSimple
 */
static gboolean get_extrema (GoatDataset *self, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
	GoatDatasetIter iter;
	double x, y, ystddev;
	double register x_min, y_min;
	double register x_max, y_max;
	double register y_upper;
	double register y_lower;
	const gboolean register valid_stddev = goat_dataset_has_valid_standard_deviation (GOAT_DATASET (self));

	x_min = +G_MAXDOUBLE;
	y_min = +G_MAXDOUBLE;
	x_max = -G_MAXDOUBLE;
	y_max = -G_MAXDOUBLE;

	if (goat_dataset_get_iter_first (GOAT_DATASET (self), &iter)) {
		goat_dataset_get (GOAT_DATASET (self), &iter, &x, &y, &ystddev);
		x_min = x_max = x;
		y_min = y_max = y;
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
			y_upper = y_lower = y;
			if (valid_stddev) {
				g_assert (ystddev >= 0.);
				y_upper += ystddev;
				y_lower -= ystddev;
			}
			if (y_lower < y_min) {
				y_min = y_lower;
			}
			if (y_upper > y_max) {
				y_max = y_upper;
			}
		}
	} else {
		return FALSE;
	}

	*xmin = x_min;
	*ymin = y_min;
	*xmax = x_max;
	*ymax = y_max;
	return TRUE;
}

static void goat_dataset_default_init (GoatDatasetInterface *iface)
{
	iface->get_marker_style = NULL;
	iface->iter_init = NULL;
	iface->iter_next = NULL;
	iface->get = NULL;
	iface->get_extrema = get_extrema;
	iface->get_log_extrema = NULL;
	iface->get_color = NULL;
	iface->get_marker_line_color = NULL;
	iface->get_marker_fill_color = NULL;
	iface->get_marker_line_width = NULL;
	iface->get_line_width = NULL;
	iface->get_marker_size = NULL;
	iface->get_marker_fill = NULL;
	iface->has_valid_standard_deviation = NULL;
	iface->is_interpolation_enabled = NULL;
	iface->get_length = NULL;
}
