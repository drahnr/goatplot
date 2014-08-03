#ifndef __GOAT_SCALE_H__
#define __GOAT_SCALE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOAT_TYPE_SCALE				(goat_scale_get_type ())
#define GOAT_SCALE(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE, GoatScale))
#define GOAT_SCALE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOAT_TYPE_SCALE, GoatScale const))
#define GOAT_SCALE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GOAT_TYPE_SCALE, GoatScaleClass))
#define GOAT_IS_SCALE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOAT_TYPE_SCALE))
#define GOAT_IS_SCALE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOAT_TYPE_SCALE))
#define GOAT_SCALE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOAT_TYPE_SCALE, GoatScaleClass))

typedef struct _GoatScale			GoatScale;
typedef struct _GoatScaleClass		GoatScaleClass;
typedef struct _GoatScalePrivate	GoatScalePrivate;

typedef void (*GoatScaleDrawCallback)(cairo_t *cr);

struct _GoatScale
{
	GObject parent;

	GoatScalePrivate *priv;
};

struct _GoatScaleClass
{
	GObjectClass parent_class;

	GoatScaleDrawCallback draw;
};

GType goat_scale_get_type (void) G_GNUC_CONST;
GoatScale *goat_scale_new (void);

void goat_scale_draw (GoatScale *gs, cairo_t *cr);


G_END_DECLS

#endif /* __GOAT_SCALE_H__ */
