#ifndef __GOAT_SCALE_LINEAR_H__
#define __GOAT_SCALE_LINEAR_H__

#include "goat-plot-enum.h"
#include "goat-scale-interface.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOAT_TYPE_SCALE_LINEAR (goat_scale_linear_get_type ())
#define GOAT_SCALE_LINEAR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE_LINEAR, GoatScaleLinear))
#define GOAT_SCALE_LINEAR_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE_LINEAR, GoatScaleLinear const))
#define GOAT_SCALE_LINEAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_SCALE_LINEAR, GoatScaleLinearClass))
#define GOAT_IS_SCALE_LINEAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_SCALE_LINEAR))
#define GOAT_IS_SCALE_LINEAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_SCALE_LINEAR))
#define GOAT_SCALE_LINEAR_GET_CLASS(obj)                                                                               \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_SCALE_LINEAR, GoatScaleLinearClass))

typedef struct _GoatScaleLinear GoatScaleLinear;
typedef struct _GoatScaleLinearClass GoatScaleLinearClass;
typedef struct _GoatScaleLinearPrivate GoatScaleLinearPrivate;

struct _GoatScaleLinear {
	GObject parent;

	GoatScaleLinearPrivate *priv;
};

struct _GoatScaleLinearClass {
	GObjectClass parent_class;
};

GType goat_scale_linear_get_type (void) G_GNUC_CONST;
GoatScaleLinear *goat_scale_linear_new (GoatPosition position, GoatOrientation orientation);

void goat_scale_linear_set_ticks (GoatScaleLinear *scale, gdouble major_step, gint minors_per_major);
void goat_scale_linear_set_label (GoatScaleLinear *plot, gchar *label);

G_END_DECLS

#endif /* __GOAT_SCALE_H__ */
