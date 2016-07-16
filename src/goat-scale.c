#include "goat-scale.h"
#include "goat-plot-enum.h"
#include "goat-utils.h"

#define GOAT_SCALE_GET_PRIVATE(object)                                                             \
	(G_TYPE_INSTANCE_GET_PRIVATE ((object), GOAT_TYPE_SCALE, GoatScalePrivate));

struct _GoatScalePrivate {
	gboolean scale_fixed;

	// scale bounds
	gdouble min, max;
	// automatically choose x_min, ... y_max according to the datasets
	gboolean autorange;

	// ticks probably move into GoatTicks/Scale object
	gdouble major_delta;
	gint minors_per_major;

	GdkRGBA color_major;
	GdkRGBA color_minor;

	GdkRGBA color_background;
	GdkRGBA color_border;

	gint width_minor;
	gint width_major;

	GoatOrientation orientation;
	GoatPosition position;

	void (*draw) ();
};

G_DEFINE_TYPE (GoatScale, goat_scale, G_TYPE_OBJECT)

enum {
	PROP_0,

	PROP_ORIENTATION,
	PROP_POSITION,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void goat_scale_set_property (GObject *object, guint property_id, const GValue *value,
                                     GParamSpec *pspec)
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

static void goat_scale_get_property (GObject *object, guint property_id, GValue *value,
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

static void goat_scale_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_scale_parent_class)->finalize (object);
}

static void goat_scale_class_init (GoatScaleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = goat_scale_set_property;
	object_class->get_property = goat_scale_get_property;

	obj_properties[PROP_ORIENTATION] = g_param_spec_enum (
	    "orientation", "Set orientation property", "Set the orientation ot vertical of horizontal",
	    GOAT_TYPE_ORIENTATION, GOAT_ORIENTATION_HORIZONTAL, G_PARAM_READWRITE);
	obj_properties[PROP_POSITION] = g_param_spec_enum (
	    "position", "Set position property", "Set the position to left,right,top or bottom",
	    GOAT_TYPE_POSITION, GOAT_POSITION_LEFT, G_PARAM_READWRITE);

	g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

	object_class->finalize = goat_scale_finalize;

	g_type_class_add_private (object_class, sizeof (GoatScalePrivate));
}

static void goat_scale_init (GoatScale *self) { self->priv = GOAT_SCALE_GET_PRIVATE (self); }

GoatScale *goat_scale_new (GoatPosition position, GoatOrientation orientation)
{
	return g_object_new (GOAT_TYPE_SCALE, "orientation", orientation, "position", position, NULL);
}

void goat_scale_set_range_auto (GoatScale *scale)
{
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->min = G_MAXDOUBLE;
	scale->priv->max = -G_MAXDOUBLE;
	scale->priv->autorange = TRUE;
}

void goat_scale_update_range (GoatScale *scale, gdouble min, gdouble max)
{
	g_return_if_fail (min < max);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->min = min;
	scale->priv->max = max;
}

void goat_scale_set_range (GoatScale *scale, gdouble min, gdouble max)
{
	g_return_if_fail (min < max);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->autorange = FALSE;
	scale->priv->min = min;
	scale->priv->max = max;
}

void goat_scale_get_range (GoatScale *scale, gdouble *min, gdouble *max)
{
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	if (min) {
		*min = scale->priv->min;
	}
	if (max) {
		*max = scale->priv->max;
	}
}

gboolean goat_scale_is_auto_range (GoatScale *scale) { return scale->priv->autorange; }

void goat_scale_set_ticks (GoatScale *scale, gdouble major, gint minors_per_major)
{
	g_return_if_fail (major > 0.);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	GoatScalePrivate *priv;

	priv = GOAT_SCALE_GET_PRIVATE (scale);

	priv->major_delta = major;
	priv->minors_per_major = minors_per_major;
}

/**
 * @param x/y-nil in pixel
 * @param x/y-factor convert unit to pixel
 */
gboolean goat_scale_draw (GoatScale *scale, cairo_t *cr, int left, int right, int top, int bottom,
                          double nil, gdouble factor, GoatPosition where, gboolean grid)
{
	GoatScalePrivate *priv = goat_scale_get_instance_private (scale);
	const double step_minor = (priv->major_delta / priv->minors_per_major);
	const int register start = (top - nil) / step_minor / factor;
	const int register end = (bottom - nil) / step_minor / factor;
	int register i;
	const gint width_major = priv->width_major;
	const gint width_minor = priv->width_minor;
	GdkRGBA color_minor = priv->color_minor;
	GdkRGBA color_major = priv->color_major;

	g_print ("[] bottom %i   top %i  _nil %lf\n", bottom, top, nil);
	g_print ("> start=%i end=%i         %lf _factor\n", start, end, factor);
	cairo_set_line_width (cr, 1.);

	for (i = start; i <= end; i++) {
		const gboolean register majorstip = (i % priv->minors_per_major == 0);
		if (where == GOAT_POSITION_LEFT || where == GOAT_POSITION_RIGHT) {
			const double register y = nil + top + step_minor * factor * i;
			cairo_move_to (cr, left, y);
			if (majorstip) {
				cairo_line_to (cr, left - width_major, y);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, left - width_minor, y);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			const double register x = (where == GOAT_POSITION_LEFT) * (left - off) +
			                          (where == GOAT_POSITION_RIGHT) * (right + off);
			goat_util_draw_num (cr, x, y, step_minor * i, where);
		}
		if (where == GOAT_POSITION_TOP || where == GOAT_POSITION_BOTTOM) {
			const double register x = nil + left + step_minor * factor * i;
			cairo_move_to (cr, x, top);
			if (majorstip) {
				cairo_line_to (cr, x, top - width_major);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, x, top - width_minor);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			const double register y = (where == GOAT_POSITION_TOP) * (top - off) +
			                          (where == GOAT_POSITION_BOTTOM) * (bottom + off);
			goat_util_draw_num (cr, x, y, step_minor * i, where);
		}
	}
	return TRUE;
}

// TODO merge this with the aboe
void goat_scale_draw2 (GoatScale *gs, cairo_t *cr)
{
	GoatScaleClass *klass = GOAT_SCALE_GET_CLASS (gs);
	if (gs->priv->draw) {
		gs->priv->draw (gs, cr);
	} else {
		g_warning ("This type of Scale does not draw a single crappy dot.");
	}
}
