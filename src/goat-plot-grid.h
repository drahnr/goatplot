#ifndef __GOAT_GRID_H__
#define __GOAT_GRID_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GOAT_TYPE_GRID (goat_grid_get_type ())
#define GOAT_GRID(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_GRID, GoatGrid))
#define GOAT_GRID_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_GRID, GoatGrid const))
#define GOAT_GRID_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_GRID, GoatGridClass))
#define GOAT_IS_GRID(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_GRID))
#define GOAT_IS_GRID_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_GRID))
#define GOAT_GRID_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_GRID, GoatGridClass))

typedef struct _GoatGrid GoatGrid;
typedef struct _GoatGridClass GoatGridClass;
typedef struct _GoatGridPrivate GoatGridPrivate;

struct _GoatGrid {
	GObject parent;

	GoatGridPrivate *priv;
};

struct _GoatGridClass {
	GObjectClass parent_class;
};

GType goat_grid_get_type (void) G_GNUC_CONST;
GoatGrid *goat_grid_new (void);


G_END_DECLS

#endif /* __GOAT_GRID_H__ */
