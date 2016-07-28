#ifndef GOAT_DATASET_H
#define GOAT_DATASET_H

#include <glib-object.h>
#include <goat-plot-enum.h>

G_BEGIN_DECLS


#define GOAT_TYPE_DATASET (goat_dataset_get_type ())

G_DECLARE_INTERFACE (GoatDataset, goat_dataset, GOAT, DATASET, GObject)

typedef struct {
	GoatDataset *dataset;

	gpointer state;
	gpointer p1;
	gpointer p2;
	gpointer p3;
} GoatDatasetIter;

struct _GoatDatasetInterface {
	GTypeInterface parent;

	GoatMarkerStyle (*get_marker_style) (GoatDataset *dataset);
	gboolean (*iter_init) (GoatDataset *dataset, GoatDatasetIter *iter);
	gboolean (*iter_next) (GoatDataset *dataset, GoatDatasetIter *iter);
	gboolean (*get) (GoatDataset *dataset, GoatDatasetIter *iter, gdouble *x, gdouble *y, gdouble *ystddev);
	gboolean (*get_extrema) (GoatDataset *dataset, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax);
	void (*get_color) (GoatDataset *dataset, GdkRGBA *color);
};

/**
 * @param dataset
 * @returns node style of #dataset
 */
GoatMarkerStyle goat_dataset_get_marker_style (GoatDataset *self);

/**
 * @param dataset
 * @param xmin [out]
 * @param xmax [out]
 * @param ymin [out]
 * @param ymax [out]
 */
gboolean goat_dataset_get_extrema (GoatDataset *self, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax);

/**
 * @param dataset
 * @param color [out]
 */
void goat_dataset_get_color (GoatDataset *self, GdkRGBA *color);

gboolean goat_dataset_get_iter_first (GoatDataset *self, GoatDatasetIter *iter);

gboolean goat_dataset_iter_next (GoatDataset *self, GoatDatasetIter *iter);

gboolean goat_dataset_get (GoatDataset *self, GoatDatasetIter *iter, gdouble *x, gdouble *y, gdouble *ystddev);

gboolean goat_dataset_interpolate (GoatDataset *self);

gboolean goat_dataset_has_valid_standard_deviation (GoatDataset *self);

G_END_DECLS

#endif /* GOAT_DATASET_H */
