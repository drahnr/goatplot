#include "goat-plot-scale.h"
#include "goat-plot-enum.h"


#define GOAT_SCALE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GOAT_TYPE_SCALE, GoatScalePrivate));


struct _GoatScalePrivate
{
	gboolean scale_fixed;

	// visible area of datasets
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

void
goat_scale_set_range_auto (GoatPlot *scale)
{
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->min = G_MAXDOUBLE;
	scale->priv->max = -G_MAXDOUBLE;
	scale->priv->autorange = TRUE;
}

void
goat_scale_update_range (GoatScale *scale, gdouble min, gdouble max)
{
	g_return_if_fail (min < max);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->min = min;
	scale->priv->max = max;
}

void
goat_scale_set_range (GoatScale *scale, gdouble min, gdouble max)
{
	g_return_if_fail (min < max);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	scale->priv->autorange = FALSE;
	scale->priv->min = min;
	scale->priv->max = max;
}

void
goat_scale_set_ticks (GoatScale *scale, gdouble major, gint minors_per_major)
{
	g_return_if_fail (major>0.);
	g_return_if_fail (scale);
	g_return_if_fail (GOAT_IS_SCALE (scale));

	GoatScalePrivate *priv;

	priv = GOAT_SCALE_GET_PRIVATE (scale);

	priv->major_delta = major;
	priv->minors_per_major = minors_per_major;
}



/**
 * @param x_nil in pixel
 * @param x_factor convert unit to pixel
 */
gboolean
draw_scale_horizontal (GoatScale *scale,
                       cairo_t *cr,
                       int left, int right, int top, int bottom,
                       gdouble x_nil,
                       gdouble x_factor,
                       GoatPosition toporbottom,
                       gboolean grid)
{
	g_assert (x_factor != 0.);
	GoatPlotPrivate *priv = GOAT_SCALE_GET_PRIVATE (plot);
	const double step_minor = (priv->major_delta_x / priv->minors_per_major_x);
	const int register start = (left-x_nil)/step_minor/x_factor;
	const int register end = (right-x_nil)/step_minor/x_factor;
	int register i;
	const gint width_major = priv->width_major_x;
	const gint width_minor = priv->width_minor_x;
	GdkRGBA color_minor = priv->color_minor_y;
	GdkRGBA color_major = priv->color_major_y;

	g_print ("[] right %i   left %i   x_nil %lf\n", right, left, x_nil);
	g_print (">>> start=%i end=%i  %lf x_factor\n", start, end, x_factor);
	cairo_set_line_width (cr, 1.);

	if (toporbottom == GOAT_BORDER_BOTTOM) {
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, top);
			if (majorstip) {
				cairo_line_to (cr, x, top-width_major);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, x, top-width_minor);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			draw_num (plot, cr, x, top-off, step_minor * i, toporbottom);
		}
	}

	if (grid) {
		GdkRGBA color_grid_major = {0.9, 0.9, 0.9, 1.};
		GdkRGBA color_grid_minor = {0.95, 0.95, 0.95, 1.};
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, top);
			cairo_line_to (cr, x, bottom);
			if (majorstip) {
				gdk_cairo_set_source_rgba (cr, &color_grid_major);
			} else {
				gdk_cairo_set_source_rgba (cr, &color_grid_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (toporbottom == GOAT_BORDER_TOP) {
		for (i=start; i<=end; i++) {
			gboolean majorstip = (i % priv->minors_per_major == 0);
			const double register x = x_nil+left+step_minor*x_factor*i;
			cairo_move_to (cr, x, bottom);
			if (majorstip) {
				cairo_line_to (cr, x, bottom+width_major);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, x, bottom+width_minor);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			draw_num (plot, cr, x, bottom+off, step_minor * i, toporbottom);
		}
	}
	return TRUE;
}
//FIXME merge the above into the on below


/**
 * @param x/y-nil in pixel
 * @param x/y-factor convert unit to pixel
 */
gboolean
draw_scale (GoatPlot *plot,
	    cairo_t *cr,
	    int left, int right, int top, int bottom,
	    double y_nil,
	    gdouble y_factor,
	    GoatPosition two_options,
	    gboolean grid)
{
	GoatPlotPrivate *priv = GOAT_PLOT_GET_PRIVATE (plot);
	const double step_minor = (priv->major_delta / priv->minors_per_major);
	const int register start = (top-y_nil)/step_minor/y_factor;
	const int register end = (bottom-y_nil)/step_minor/y_factor;
	int register i;
	const gint width_major = priv->width_major;
	const gint width_minor = priv->width_minor;
	GdkRGBA color_minor = priv->color_minor;
	GdkRGBA color_major = priv->color_major;

	g_print ("[] bottom %i   top %i   x_nil %lf\n", bottom, top, y_nil);
	g_print ("> start=%i end=%i         %lf y_factor\n", start, end,y_factor);
	cairo_set_line_width (cr, 1.);

	if (leftorright == GOAT_BORDER_LEFT) {
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			if (majorstip) {
				cairo_line_to (cr, left-width_major, y);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, left-width_minor, y);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			draw_num (plot, cr, left-off, y, step_minor * i, leftorright);
		}
	}

	if (grid) {

		GdkRGBA color_grid_major = {0.9, 0.9, 0.9, 1.};
		GdkRGBA color_grid_minor = {0.95, 0.95, 0.95, 1.};
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			cairo_line_to (cr, right, y);
			if (majorstip) {
				gdk_cairo_set_source_rgba (cr, &color_grid_major);
			} else {
				gdk_cairo_set_source_rgba (cr, &color_grid_minor);
			}
			cairo_stroke (cr);
		}
	}

	if (leftorright == GOAT_BORDER_RIGHT) {
		for (i=start; i<=end; i++) {
			const gboolean register majorstip = (i % priv->minors_per_major == 0);
			const double register y = y_nil+top+step_minor*y_factor*i;
			cairo_move_to (cr, left, y);
			if (majorstip) {
				cairo_line_to (cr, left-width_major, y);
				gdk_cairo_set_source_rgba (cr, &color_major);
			} else {
				cairo_line_to (cr, left-width_minor, y);
				gdk_cairo_set_source_rgba (cr, &color_minor);
			}
			cairo_stroke (cr);

			const double off = majorstip ? width_major : width_minor;
			draw_num (plot, cr, right+off, y, step_minor * i, leftorright);

		}
	}
	return TRUE;
}
