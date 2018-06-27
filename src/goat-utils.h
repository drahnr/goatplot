#ifndef __GOAT_UTILS_H__
#define __GOAT_UTILS_H__

#include <goat-plot-enum.h>
#include <gtk/gtk.h>
#include <math.h>

/**
 * draw a numer at a position
 */
void goat_util_draw_num (cairo_t *cr, double x, double y, double d, GoatPosition penalty);

/**
 * provides nice numerical limits when scaling an axis
 */
double goat_util_nice_num (double x, int round);


#endif /* __GOAT_UTILS_H__ */
