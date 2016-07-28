#ifndef __GOAT_ENUM_H__
#define __GOAT_ENUM_H__

#include <gtk/gtk.h>

typedef enum {
	GOAT_ORIENTATION_INVALID = 0,
	GOAT_ORIENTATION_VERTICAL = 1,
	GOAT_ORIENTATION_HORIZONTAL = 2
} GoatOrientation;

#define GOAT_TYPE_ORIENTATION (goat_orientation_get_type ())
GType goat_orientation_get_type (void);



typedef enum {
	GOAT_POSITION_INVALID = 0,
	GOAT_POSITION_TOP = 1,
	GOAT_POSITION_BOTTOM = 2,
	GOAT_POSITION_LEFT = 3,
	GOAT_POSITION_RIGHT = 4
} GoatPosition;

#define GOAT_TYPE_POSITION (goat_position_get_type ())
GType goat_position_get_type (void);




typedef enum {
	GOAT_HEADING_IN = 1,
	GOAT_HEADING_OUT = 2,
} GoatHeading;

#define GOAT_TYPE_HEADING (goat_heading_get_type ())
GType goat_heading_get_type (void);


typedef enum {
	GOAT_PLOT_SCALE_EXP = 1,
	GOAT_PLOT_SCALE_LIN = 2,
	GOAT_PLOT_SCALE_LOG = 3,
} GoatPlotScaleType;

// FIXME this does not make much sense here, make this an option for the GoatPlot widget itself,
// FIXME it is the users duty to convert his data to the proper form
typedef enum {
	GOAT_MARKER_STYLE_INVALID = 0,
	GOAT_MARKER_STYLE_NONE = 1,
	GOAT_MARKER_STYLE_POINT = 2,
	GOAT_MARKER_STYLE_SQUARE = 3,
	GOAT_MARKER_STYLE_TRIANGLE = 4,
	GOAT_MARKER_STYLE_CROSS = 5
} GoatMarkerStyle;

#define GOAT_TYPE_MARKER_STYLE (goat_marker_style_get_type ())
GType goat_marker_style_get_type (void);


#endif /* __GOAT_ENUM_H__ */
