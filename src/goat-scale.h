#ifndef __GOAT_SCALE_H__
#define __GOAT_SCALE_H__

#include <gtk/gtk.h>

#include "goat-plot-enum.h"

G_BEGIN_DECLS

#define GOAT_TYPE_SCALE (goat_scale_get_type ())
#define GOAT_SCALE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE, GoatScale))
#define GOAT_SCALE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE, GoatScale const))
#define GOAT_SCALE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_SCALE, GoatScaleClass))
#define GOAT_IS_SCALE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_SCALE))
#define GOAT_IS_SCALE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_SCALE))
#define GOAT_SCALE_GET_CLASS(obj)                                                                  \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_SCALE, GoatScaleClass))

typedef struct _GoatScale GoatScale;
typedef struct _GoatScaleClass GoatScaleClass;
typedef struct _GoatScalePrivate GoatScalePrivate;

typedef void (*GoatScaleDrawCallback) (GoatScale *gs, cairo_t *cr);

struct _GoatScale {
	GObject parent;

	GoatScalePrivate *priv;
};

struct _GoatScaleClass {
	GObjectClass parent_class;
};

GType goat_scale_get_type (void) G_GNUC_CONST;
GoatScale *goat_scale_new (GoatPosition position, GoatOrientation orientation);

gboolean goat_scale_draw (GoatScale *scale, cairo_t *cr, int left, int right, int top, int bottom,
                          double nil, gdouble factor, GoatPosition where, gboolean grid);

void goat_scale_get_range (GoatScale *scale, gdouble *min, gdouble *max);
void goat_scale_set_range_auto (GoatScale *scale);
void goat_scale_set_range (GoatScale *scale, gdouble min, gdouble max);
void goat_scale_update_range (GoatScale *scale, gdouble min, gdouble max);
gboolean goat_scale_is_auto_range (GoatScale *scale);
void goat_scale_set_ticks (GoatScale *scale, gdouble major, gint minors_per_major);
void goat_scale_set_label (GoatScale *plot, gchar *label);

G_END_DECLS

#endif /* __GOAT_SCALE_H__ */
