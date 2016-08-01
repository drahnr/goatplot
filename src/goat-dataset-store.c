#include "goat-dataset-store.h"

typedef struct {
	GtkTreeModel *tree_model;
	gint x_index;
	gint y_index;
	gint ystddev_index;
} GoatDatasetStorePrivate;

static void goat_dataset_store_interface_init (GoatDatasetInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GoatDatasetStore, goat_dataset_store, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GOAT_TYPE_DATASET, goat_dataset_store_interface_init)
                             G_ADD_PRIVATE (GoatDatasetStore))

enum { PROP_0, PROP_TREE_MODEL, PROP_X_INDEX, PROP_Y_INDEX, PROP_YSTDDEV_INDEX, N_PROPS };

static GParamSpec *obj_properties[N_PROPS];

GoatDatasetStore *goat_dataset_store_new (GtkTreeModel *tree_model)
{
	return g_object_new (GOAT_TYPE_DATASET_STORE, "tree-model", tree_model, NULL);
}


static void goat_dataset_store_finalize (GObject *object)
{
	GoatDatasetStore *self = (GoatDatasetStore *)object;
	/* GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self); */

	G_OBJECT_CLASS (goat_dataset_store_parent_class)->finalize (object);
}

static void goat_dataset_store_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GoatDatasetStore *self = GOAT_DATASET_STORE (object);
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);

	switch (prop_id) {
	case PROP_TREE_MODEL:
		g_value_set_pointer (value, priv->tree_model);
		break;
	case PROP_X_INDEX:
		g_value_set_int (value, priv->x_index);
		break;
	case PROP_Y_INDEX:
		g_value_set_int (value, priv->y_index);
		break;
	case PROP_YSTDDEV_INDEX:
		g_value_set_int (value, priv->ystddev_index);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void goat_dataset_store_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GoatDatasetStore *self = GOAT_DATASET_STORE (object);
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);

	switch (prop_id) {
	case PROP_TREE_MODEL:
		priv->tree_model = g_value_get_pointer (value);
		break;
	case PROP_X_INDEX:
		priv->x_index = g_value_get_int (value);
		break;
	case PROP_Y_INDEX:
		priv->y_index = g_value_get_int (value);
		break;
	case PROP_YSTDDEV_INDEX:
		priv->ystddev_index = g_value_get_int (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void goat_dataset_store_class_init (GoatDatasetStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = goat_dataset_store_finalize;
	object_class->get_property = goat_dataset_store_get_property;
	object_class->set_property = goat_dataset_store_set_property;

	obj_properties[PROP_TREE_MODEL] =
	    g_param_spec_object ("tree-model", "GoatDatasetStore::tree-model", "the backend tree store", GTK_TYPE_TREE_MODEL,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

	obj_properties[PROP_X_INDEX] = g_param_spec_int ("x-index", "GoatDatasetStore::x-index", "mapping", -1, G_MAXINT, -1,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

	obj_properties[PROP_Y_INDEX] = g_param_spec_int ("y-index", "GoatDatasetStore::y-index", "mapping", -1, G_MAXINT, -1,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

	obj_properties[PROP_YSTDDEV_INDEX] = g_param_spec_int ("ystddev-index", "GoatDatasetStore::ystddev-index",
	                                                          "mapping", -1, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

	g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void goat_dataset_store_init (GoatDatasetStore *self)
{
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);
	priv->tree_model = NULL;
  	priv->x_index = -1; // minus one so gtk_tree_model_get stops parsing
  	priv->y_index = -1;
  	priv->ystddev_index = -1;
}

static gboolean iter_init (GoatDataset *dataset, GoatDatasetIter *iter)
{
	g_return_val_if_fail (dataset, FALSE);
	g_return_val_if_fail (GOAT_IS_DATASET_STORE (dataset), FALSE);
	g_return_val_if_fail (iter, FALSE);
	GoatDatasetStore *self = GOAT_DATASET_STORE (dataset);
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);
	return gtk_tree_model_get_iter_first (priv->tree_model, (GtkTreeIter *)(iter->state));
}

static gboolean iter_next (GoatDataset *dataset, GoatDatasetIter *iter)
{
	g_return_val_if_fail (dataset, FALSE);
	g_return_val_if_fail (GOAT_IS_DATASET_STORE (dataset), FALSE);
	g_return_val_if_fail (iter, FALSE);
	GoatDatasetStore *self = GOAT_DATASET_STORE (dataset);
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);
	return gtk_tree_model_iter_next (priv->tree_model, (GtkTreeIter *)(iter->state));
}

static gboolean get (GoatDataset *dataset, GoatDatasetIter *iter, gdouble *x, gdouble *y, gdouble *ystddev)
{
	g_return_val_if_fail (dataset, FALSE);
	g_return_val_if_fail (GOAT_IS_DATASET_STORE (dataset), FALSE);
	g_return_val_if_fail (iter, FALSE);
	GoatDatasetStore *self = GOAT_DATASET_STORE (dataset);
	GoatDatasetStorePrivate *priv = goat_dataset_store_get_instance_private (self);

	gtk_tree_model_get (priv->tree_model, (GtkTreeIter *)(iter->state), priv->x_index, x, priv->y_index, y, priv->ystddev_index, ystddev, -1);
	return TRUE;
}

static gboolean get_extrema(GoatDataset *dataset, gdouble *x, gdouble *y, gdouble *ystddev) {
	return FALSE; // FIXME
}

static void goat_dataset_store_interface_init (GoatDatasetInterface *iface)
{
	iface->iter_init = iter_init;
	iface->iter_next = iter_next;
	iface->get = get;
	iface->get_extrema = get_extrema;
}
