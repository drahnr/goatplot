#ifndef GOAT_SCALE_INTERFACE_H
#define GOAT_SCALE_INTERFACE_H

#include <goat-plot-enum.h>
#include <gtk/gtk.h>
G_BEGIN_DECLS

#define GOAT_TYPE_SCALE (goat_scale_get_type ())

G_DECLARE_INTERFACE (GoatScale, goat_scale, GOAT, SCALE, GObject)

struct _GoatScaleInterface {
	GTypeInterface parent_iface;
	void (*draw) (GoatScale *self, cairo_t *cr, gint left, gint right, gint top, gint bottom, gdouble nil,
	              gdouble factor);
	void (*render) (GoatScale *self);
	void (*get_range) (GoatScale *self, gdouble *min, gdouble *max);
	void (*set_range) (GoatScale *self, gdouble min, gdouble max);
	void (*update_range) (GoatScale *self, gdouble min, gdouble max);
	void (*set_range_auto) (GoatScale *self);
	void (*set_auto_range) (GoatScale *self);
	gboolean (*is_auto_range) (GoatScale *scale);
	void (*show_grid) (GoatScale *scale, gboolean show);
};


/**
 * in case rendering the scale is expensive
 * this may be used to pre-draw the scale
 */
void goat_scale_render (GoatScale *self);

/**
 * does the actual drawing
 */
void goat_scale_draw (GoatScale *self, cairo_t *cr, gint left, gint right, gint top, gint bottom, gdouble nil,
                      gdouble factor);

void goat_scale_get_range (GoatScale *scale, gdouble *min, gdouble *max);
void goat_scale_set_range_auto (GoatScale *scale);
void goat_scale_set_range (GoatScale *scale, gdouble min, gdouble max);
void goat_scale_update_range (GoatScale *scale, gdouble min, gdouble max);
gboolean goat_scale_is_auto_range (GoatScale *scale);
void goat_scale_grid_show (GoatScale *scale, gboolean show);

G_END_DECLS

#endif /* GOAT_SCALE_INTERFACE_H */
