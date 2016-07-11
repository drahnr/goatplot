#include "goat-plot-grid.h"
#include "goat-plot-enum.h"
#include "goat-scale.h"

#define GOAT_GRID_GET_PRIVATE(object)                                                              \
	(G_TYPE_INSTANCE_GET_PRIVATE ((object), GOAT_TYPE_GRID, GoatGridPrivate))

struct _GoatGridPrivate {
	GoatScale *scale_x; // includes scale type
	GoatScale *scale_y;
	GdkRGBA col_bg;
	GdkRGBA col_x_major_lines;
	GdkRGBA col_y_major_lines;
	GdkRGBA col_x_minor_lines;
	GdkRGBA col_y_minor_lines;
	gboolean draw_x_nil;
	gboolean draw_y_nil;
};

G_DEFINE_TYPE (GoatGrid, goat_grid, G_TYPE_OBJECT)

static void goat_grid_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_grid_parent_class)->finalize (object);
}

static void goat_grid_class_init (GoatGridClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = goat_grid_finalize;

	g_type_class_add_private (object_class, sizeof (GoatGridPrivate));
}

static void goat_grid_init (GoatGrid *self) { self->priv = GOAT_GRID_GET_PRIVATE (self); }

GoatGrid *goat_grid_new () { return g_object_new (GOAT_TYPE_GRID, NULL); }
