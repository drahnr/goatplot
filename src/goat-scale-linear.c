#include "goat-scale-linear.h"
#include "goat-plot-enum.h"
#include "goat-utils.h"

struct _GoatScaleLinearPrivate {
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

	GdkRGBA color_major_grid;
	GdkRGBA color_minor_grid;

	GdkRGBA color_background;
	GdkRGBA color_border;

	gint width_minor;
	gint width_major;

	gboolean draw_grid;

	GoatOrientation orientation;
	GoatPosition position;
};

static void goat_scale_linear_interface_init (GoatScaleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GoatScaleLinear, goat_scale_linear, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GOAT_TYPE_SCALE, goat_scale_linear_interface_init)
                             G_ADD_PRIVATE (GoatScaleLinear))

enum {
	PROP_0,

	PROP_GRID_VISIBLE,
	PROP_ORIENTATION,
	PROP_POSITION,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void goat_scale_linear_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (object);
	GoatScaleLinearPrivate *priv = goat_scale_linear_get_instance_private (self);

	switch (property_id) {
	case PROP_GRID_VISIBLE:
		priv->draw_grid = g_value_get_boolean (value);
		break;
	case PROP_ORIENTATION:
		priv->orientation = g_value_get_enum (value);
		break;
	case PROP_POSITION:
		priv->position = g_value_get_enum (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void goat_scale_linear_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (object);
	GoatScaleLinearPrivate *priv = goat_scale_linear_get_instance_private (self);

	switch (property_id) {
	case PROP_GRID_VISIBLE:
		g_value_set_boolean (value, priv->draw_grid);
		break;
	case PROP_ORIENTATION:
		g_value_set_enum (value, priv->orientation);
		break;
	case PROP_POSITION:
		g_value_set_enum (value, priv->position);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void goat_scale_linear_finalize (GObject *object)
{
	G_OBJECT_CLASS (goat_scale_linear_parent_class)->finalize (object);
}

static void goat_scale_linear_class_init (GoatScaleLinearClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = goat_scale_linear_set_property;
	object_class->get_property = goat_scale_linear_get_property;

	obj_properties[PROP_GRID_VISIBLE] =
	    g_param_spec_boolean ("grid-visible", "Show the grid", "To show or not to show", FALSE, G_PARAM_READWRITE);

	obj_properties[PROP_ORIENTATION] =
	    g_param_spec_enum ("orientation", "Set orientation property", "Set the orientation ot vertical of horizontal",
	                       GOAT_TYPE_ORIENTATION, GOAT_ORIENTATION_INVALID, G_PARAM_READWRITE);
	obj_properties[PROP_POSITION] =
	    g_param_spec_enum ("position", "Set position property", "Set the position to left,right,top or bottom",
	                       GOAT_TYPE_POSITION, GOAT_POSITION_LEFT, G_PARAM_READWRITE);

	g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

	object_class->finalize = goat_scale_linear_finalize;
}

static void goat_scale_linear_init (GoatScaleLinear *self)
{
	GoatScaleLinearPrivate *priv = self->priv = goat_scale_linear_get_instance_private (self);
	goat_scale_set_range_auto (GOAT_SCALE (self));
	priv->draw_grid = TRUE;
	priv->minors_per_major = 4;
	priv->major_delta = 10.;
	priv->width_minor = 5;
	priv->width_major = 8;
	priv->color_major.red = 0.8;
	priv->color_major.green = 0.2;
	priv->color_major.blue = 0.2;
	priv->color_major.alpha = 1.0;
	priv->color_minor.red = 0.4;
	priv->color_minor.green = 0.4;
	priv->color_minor.blue = 0.4;
	priv->color_minor.alpha = 1.0;
	priv->color_minor_grid = priv->color_minor;
	priv->color_minor_grid.alpha = 0.3;
	priv->color_major_grid = priv->color_major;
	priv->color_major_grid.alpha = 0.3;
	priv->orientation = GOAT_ORIENTATION_INVALID;
	priv->position = GOAT_POSITION_INVALID;
}

GoatScaleLinear *goat_scale_linear_new (GoatPosition position, GoatOrientation orientation)
{
	GoatPosition assure_pos;
	GoatOrientation assure_ori;
	GoatScaleLinear *self =
	    g_object_new (GOAT_TYPE_SCALE_LINEAR, "position", position, "orientation", orientation, NULL);
	g_object_get (G_OBJECT (self), "position", &assure_pos, "orientation", &assure_ori, NULL);
	g_assert (assure_pos == position);
	g_assert (assure_ori == orientation);
	return self;
}

void goat_scale_linear_set_ticks (GoatScaleLinear *scale, gdouble major, gint minors_per_major)
{
	g_return_if_fail (major > 0.);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE_LINEAR (scale));

	GoatScaleLinearPrivate *priv;

	priv = goat_scale_linear_get_instance_private (scale);

	priv->major_delta = major;
	priv->minors_per_major = minors_per_major;
}


/**
 * @param x/y-nil in pixel
 * @param x/y-factor convert unit to pixel
 */
static void draw (GoatScale *scale, cairo_t *cr, gint left, gint right, gint top, gint bottom, gdouble nil,
                  gdouble factor)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);
	GoatScaleLinearPrivate *priv = goat_scale_linear_get_instance_private (self);

	const double step_minor = (priv->major_delta / priv->minors_per_major);
	const int register start = (top - nil) / step_minor / factor;
	const int register end = (bottom - nil) / step_minor / factor;
	g_print ("[] bottom %i   top %i  _nil %lf\n", bottom, top, nil);
	g_print ("> start=%i end=%i         %lf _factor\n", start, end, factor);
	int register i;
	const gint width_major = priv->width_major;
	const gint width_minor = priv->width_minor;
	GdkRGBA color_minor = priv->color_minor;
	GdkRGBA color_major = priv->color_major;
	GdkRGBA color_minor_grid = priv->color_minor_grid;
	GdkRGBA color_major_grid = priv->color_major_grid;

	GoatPosition where = priv->position;
	gboolean grid = priv->draw_grid;

	cairo_set_line_width (cr, 1.);

	if (where == GOAT_POSITION_LEFT) {
		for (i = start; i <= end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register y = nil + top + step_minor * factor * i;
			if (grid) {
				cairo_move_to (cr, right, y);
				cairo_line_to (cr, left, y);
				if (majorstip) {
					gdk_cairo_set_source_rgba (cr, &color_major_grid);
				} else {
					gdk_cairo_set_source_rgba (cr, &color_minor_grid);
				}
				cairo_stroke (cr);
			}
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
			const double register x = left - off;
			goat_util_draw_num (cr, x, y, step_minor * i, where);
		}
	}
	if (where == GOAT_POSITION_RIGHT) {
		for (i = start; i <= end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register y = nil + top + step_minor * factor * i;
			if (grid) {
				cairo_move_to (cr, left, y);
				cairo_line_to (cr, right, y);
				if (majorstip) {
					gdk_cairo_set_source_rgba (cr, &color_major_grid);
				} else {
					gdk_cairo_set_source_rgba (cr, &color_minor_grid);
				}
				cairo_stroke (cr);
			}
			cairo_move_to (cr, right, y);
			if (majorstip) {
				cairo_line_to (cr, right + width_major, y);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, right + width_minor, y);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			const double register x = right + off;
			goat_util_draw_num (cr, x, y, step_minor * i, where);
		}
	}
	if (where == GOAT_POSITION_BOTTOM) {
		for (i = start; i <= end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register x = nil + left + step_minor * factor * i;
			if (grid) {
				cairo_move_to (cr, x, top);
				cairo_line_to (cr, x, bottom);
				if (majorstip) {
					gdk_cairo_set_source_rgba (cr, &color_major_grid);
				} else {
					gdk_cairo_set_source_rgba (cr, &color_minor_grid);
				}
				cairo_stroke (cr);
			}
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
			const double register y = top - off;
			goat_util_draw_num (cr, x, y, step_minor * i, where);
		}
	}
	if (where == GOAT_POSITION_TOP) {
		for (i = start; i <= end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register x = nil + left + step_minor * factor * i;
			if (grid) {
				cairo_move_to (cr, x, bottom);
				cairo_line_to (cr, x, top);
				if (majorstip) {
					gdk_cairo_set_source_rgba (cr, &color_major_grid);
				} else {
					gdk_cairo_set_source_rgba (cr, &color_minor_grid);
				}
				cairo_stroke (cr);
			}
			cairo_move_to (cr, x, bottom);
			if (majorstip) {
				cairo_line_to (cr, x, bottom + width_major);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, x, bottom + width_minor);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			goat_util_draw_num (cr, x, bottom + off, step_minor * i, where);
		}
	}
}

static void set_range_auto (GoatScale *scale)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	self->priv->min = +G_MAXDOUBLE;
	self->priv->max = -G_MAXDOUBLE;
	self->priv->autorange = TRUE;
}

/**
 * used for goat plot to rescale depending on data
 */
static void update_range (GoatScale *scale, gdouble min, gdouble max)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	self->priv->min = min;
	self->priv->max = max;
}

static void set_range (GoatScale *scale, gdouble min, gdouble max)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	self->priv->autorange = FALSE;
	self->priv->min = min;
	self->priv->max = max;
}

static void get_range (GoatScale *scale, gdouble *min, gdouble *max)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	if (min) {
		*min = self->priv->min;
	}
	if (max) {
		*max = self->priv->max;
	}
}

static void set_auto_range (GoatScale *scale)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	self->priv->autorange = TRUE;
}

static gboolean is_auto_range (GoatScale *scale)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	return self->priv->autorange;
}

static void show_grid (GoatScale *scale, gboolean show)
{
	GoatScaleLinear *self = GOAT_SCALE_LINEAR (scale);

	self->priv->draw_grid = show;
}

static void goat_scale_linear_interface_init (GoatScaleInterface *iface)
{
	iface->draw = draw;
	iface->render = NULL;
	iface->set_auto_range = set_auto_range;
	iface->set_range = set_range;
	iface->set_range_auto = set_range_auto;
	iface->update_range = update_range;
	iface->is_auto_range = is_auto_range;
	iface->get_range = get_range;
	iface->show_grid = show_grid;
}
