#include "goat-scale-interface.h"

G_DEFINE_INTERFACE (GoatScale, goat_scale, G_TYPE_OBJECT);

static void goat_scale_default_init (GoatScaleInterface *iface)
{
	iface->draw = NULL;
	iface->render = NULL;
	iface->is_auto_range = NULL;
	iface->set_range_auto = NULL;
	iface->set_range = NULL;
	iface->update_range = NULL;
	iface->get_range = NULL;
	iface->get_position = NULL;
	iface->set_position = NULL;
	iface->get_orientation = NULL;
	iface->set_orientation = NULL;
}

void goat_scale_draw (GoatScale *self, cairo_t *cr, gint left, gint right, gint top, gint bottom)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->draw) {
		iface->draw (self, cr, left, right, top, bottom);
	} else {
		g_error ("Missing draw handler for GoatScaleInterface!");
	}
}

void goat_scale_render (GoatScale *self)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->render) {
		iface->render (self);
	}
}

void goat_scale_get_range (GoatScale *self, gdouble *min, gdouble *max)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->get_range) {
		iface->get_range (self, min, max);
	} else {
		*min = -1.;
		*max = 1.;
	}
}

void goat_scale_set_range_auto (GoatScale *self)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->set_range_auto) {
		iface->set_range_auto (self);
	}
}

void goat_scale_set_range (GoatScale *self, gdouble min, gdouble max)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->set_range) {
		iface->set_range (self, min, max);
	}
}

void goat_scale_update_range (GoatScale *self, gdouble min, gdouble max)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->update_range) {
		iface->update_range (self, min, max);
	}
}

gboolean goat_scale_is_auto_range (GoatScale *self)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->is_auto_range) {
		return iface->is_auto_range (self);
	}
	return FALSE;
}

void goat_scale_grid_show (GoatScale *self, gboolean show)
{
	GoatScaleInterface *iface;

	iface = GOAT_SCALE_GET_IFACE (self);
	if (iface->show_grid) {
		iface->show_grid (self, show);
	}
}

void goat_scale_set_position (GoatScale *self, GoatPosition position) {
	GoatScaleInterface* iface = GOAT_SCALE_GET_IFACE(self);
	if (iface->set_position) {
		iface->set_position(self,position);
	}
}
void goat_scale_set_orientation (GoatScale *self, GoatOrientation orientation) {
	GoatScaleInterface* iface = GOAT_SCALE_GET_IFACE(self);
	if (iface->set_orientation) {
		iface->set_orientation(self,orientation);
	}
}
GoatPosition goat_scale_get_position (GoatScale *self) {
	GoatScaleInterface* iface = GOAT_SCALE_GET_IFACE(self);
	if (iface->get_position) {
		return iface->get_position(self);
	}
}
GoatOrientation goat_scale_get_orientation (GoatScale *self) {
	GoatScaleInterface* iface = GOAT_SCALE_GET_IFACE(self);
	if (iface->get_orientation) {
		return iface->get_orientation(self);
	}
}