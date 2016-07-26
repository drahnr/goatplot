#ifndef __GOAT_UTILS_H__
#define __GOAT_UTILS_H__

#include <goat-plot-enum.h>
#include <gtk/gtk.h>
#include <math.h>

/**
 * draw a numer at a position
 */
void goat_util_draw_num (cairo_t *cr, double x, double y, double d, GoatPosition penalty)
{
	cairo_save (cr);

	gchar *text = g_strdup_printf ("%.2lf", d);
	g_assert (text);
	PangoLayout *lay = pango_cairo_create_layout (cr);
	PangoFontDescription *fontdesc = pango_font_description_new ();
	pango_font_description_set_size (fontdesc, 8 * PANGO_SCALE);
	pango_layout_set_font_description (lay, fontdesc);
	pango_layout_set_text (lay, text, -1);

	PangoRectangle logrect;
	pango_layout_get_pixel_extents (lay, NULL, &logrect);
	g_print ("logrect//// x=%i y=%i w=%i h=%i\n", logrect.x, logrect.y, logrect.width,
	         logrect.height);
	g_print ("x=%lf   y=%lf\n\n", x, y);
	double modifierx, modifiery;
	switch (penalty) {
	case GOAT_POSITION_BOTTOM:
		modifierx = (double)(logrect.width) * -0.5;
		modifiery = (double)(logrect.height);
		break;
	case GOAT_POSITION_LEFT:
		modifierx = (double)(logrect.width) * -1.0;
		modifiery = (double)(logrect.height) * 0.5;
		break;
	case GOAT_POSITION_RIGHT:
		modifierx = (double)(logrect.width);
		modifiery = (double)(logrect.height) * 0.5;
		break;
	case GOAT_POSITION_TOP:
		modifierx = (double)(logrect.width)* -0.5;
		modifiery = (double)(logrect.height) * -1.;
		break;
	}
	cairo_move_to (cr, x + modifierx, y + modifiery);
	cairo_scale (cr, 1., -1.);

	pango_cairo_show_layout (cr, lay);
	g_object_unref (lay);
	g_free (text);
	pango_font_description_free (fontdesc);
	// restore the initial context properties
	cairo_restore (cr);
}

/**
 * provides nice numerical limits when scaling an axis
 */
double goat_util_nice_num (double x, int round)
{
	float exp, f, niced;
	float signx = (x >= 0) ? 1.f : -1.f;
	float absx = (float)x * signx;

	exp = floorf (log10f (absx));
	f = absx / powf (10.f, exp); // bounded between 1 and 10
	if (round) {
		if (f < 1.5)
			niced = 1.;
		else if (f < 3.)
			niced = 2.;
		else if (f < 7.)
			niced = 5.;
		else
			niced = 10.;
	} else {
		if (f <= 1.)
			niced = 1.;
		else if (f <= 2.)
			niced = 2.;
		else if (f <= 5.)
			niced = 5.;
		else
			niced = 10.;
	}
	return signx * niced * powf (10.f, exp);
}

#endif /* __GOAT_UTILS_H__ */
