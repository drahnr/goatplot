#include "goat-plot-scale.h"
#include "goat-plot-enum.h"


#define GOAT_SCALE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GOAT_TYPE_SCALE, GoatScalePrivate));






struct _GoatScalePrivate
{
	GoatOrientation orientation;
	GoatPosition position;
	void (*draw)();
};

G_DEFINE_TYPE (GoatScale, goat_scale, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_ORIENTATION,
  PROP_POSITION,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
goat_scale_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	GoatScale *self = GOAT_SCALE (object);

	switch (property_id) {
    case PROP_ORIENTATION:
		self->priv->orientation = g_value_get_enum (value);
		break;

    case PROP_POSITION:
		self->priv->position = g_value_get_enum (value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
goat_scale_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	GoatScale *self = GOAT_SCALE (object);

	switch (property_id) {
	case PROP_ORIENTATION:
		g_value_set_enum (value, self->priv->orientation);
		break;
	case PROP_POSITION:
		g_value_set_enum (value, self->priv->position);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
goat_scale_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_scale_parent_class)->finalize (object);
}

static void
goat_scale_class_init (GoatScaleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = goat_scale_set_property;
	object_class->get_property = goat_scale_get_property;

	obj_properties[PROP_ORIENTATION] =
		g_param_spec_enum ("orientation",
		                     "Set orientation property",
		                     "Set the orientation ot vertical of horizontal",
		                     GOAT_TYPE_ORIENTATION,
		                     GOAT_ORIENTATION_HORIZONTAL,
		                     G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
	obj_properties[PROP_POSITION] =
		g_param_spec_enum ("position",
		                     "Set position property",
		                     "Set the position to left,right,top or bottom in regard to the graph",
		                     GOAT_TYPE_ORIENTATION,
		                     GOAT_ORIENTATION_HORIZONTAL,
		                     G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);


	object_class->finalize = goat_scale_finalize;

	g_type_class_add_private (object_class, sizeof (GoatScalePrivate));
}

static void
goat_scale_init (GoatScale *self)
{
	self->priv = GOAT_SCALE_GET_PRIVATE (self);
}

GoatScale *
goat_scale_new ()
{
	return g_object_new (GOAT_TYPE_SCALE, NULL);
}


void
goat_scale_draw (GoatScale *gs, cairo_t *cr)
{
	GoatScaleClass *klass = GOAT_SCALE_GET_CLASS (gs);
	if (gs->priv->draw) {
		gs->priv->draw (gs, cr);
	} else {
		g_warning ("This type of Scale does not draw a single crappy dot.");
	}
}
