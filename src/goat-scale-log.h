#ifndef __GOAT_SCALE_LOG_H__
#define __GOAT_SCALE_LOG_H__

#include "goat-plot-enum.h"
#include "goat-scale-interface.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOAT_TYPE_SCALE_LOG (goat_scale_log_get_type ())
#define GOAT_SCALE_LOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE_LOG, GoatScaleLog))
#define GOAT_SCALE_LOG_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE_LOG, GoatScaleLog const))
#define GOAT_SCALE_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_SCALE_LOG, GoatScaleLogClass))
#define GOAT_IS_SCALE_LOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_SCALE_LOG))
#define GOAT_IS_SCALE_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_SCALE_LOG))
#define GOAT_SCALE_LOG_GET_CLASS(obj)                                                                               \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_SCALE_LOG, GoatScaleLogClass))

typedef struct _GoatScaleLog GoatScaleLog;
typedef struct _GoatScaleLogClass GoatScaleLogClass;
typedef struct _GoatScaleLogPrivate GoatScaleLogPrivate;

struct _GoatScaleLog {
	GObject parent;

	GoatScaleLogPrivate *priv;
};

struct _GoatScaleLogClass {
	GObjectClass parent_class;
};

GType goat_scale_log_get_type (void) G_GNUC_CONST;
GoatScaleLog *goat_scale_log_new (GoatPosition position, GoatOrientation orientation);

void goat_scale_log_set_ticks (GoatScaleLog *scale, gdouble major_step, gint minors_per_major);
void goat_scale_log_set_label (GoatScaleLog *plot, gchar *label);
gdouble goat_scale_log_get_major_delta (GoatScaleLog *scale );

G_END_DECLS

#endif /* __GOAT_SCALE_H__ */
