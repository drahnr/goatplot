#ifndef GOAT_DATASET_STORE_H
#define GOAT_DATASET_STORE_H

#include <goat-dataset-interface.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOAT_TYPE_DATASET_STORE (goat_dataset_store_get_type ())

G_DECLARE_DERIVABLE_TYPE (GoatDatasetStore, goat_dataset_store, GOAT, DATASET_STORE, GObject)

struct _GoatDatasetStoreClass {
	GObjectClass parent;
};

GoatDatasetStore *goat_dataset_store_new (GtkTreeModel *tree_model);

G_END_DECLS

#endif /* GOAT_DATASET_STORE_H */
