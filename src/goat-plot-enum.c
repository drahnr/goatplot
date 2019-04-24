#include "goat-plot-enum.h"

GType goat_orientation_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static GEnumValue ts[] = {
		    {GOAT_ORIENTATION_INVALID, "INVALID", "invalid"},
		    {GOAT_ORIENTATION_VERTICAL, "VERTICAL", "vertical"},
		    {GOAT_ORIENTATION_HORIZONTAL, "HORIZONTAL", "horizontal"},
		    {0, NULL, NULL},
		};
		t = g_enum_register_static ("GoatOrientationTypes", ts);
	}
	return t;
}

GType goat_position_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static GEnumValue ts[] = {
		    {GOAT_POSITION_INVALID, "INVALID", "invalid"}, {GOAT_POSITION_TOP, "TOP", "top"},
		    {GOAT_POSITION_BOTTOM, "BOTTOM", "bottom"},    {GOAT_POSITION_LEFT, "LEFT", "left"},
		    {GOAT_POSITION_RIGHT, "RIGHT", "right"},       {0, NULL, NULL},
		};
		t = g_enum_register_static ("GoatPositionTypes", ts);
	}

	return t;
}

GType goat_heading_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static GEnumValue ts[] = {
		    {GOAT_HEADING_IN, "INSIDE", "in"},
		    {GOAT_HEADING_OUT, "OUTSIDE", "out"},
		    {0, NULL, NULL},
		};
		t = g_enum_register_static ("GoatHeadingTypes", ts);
	}

	return t;
}

GType goat_marker_style_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static GEnumValue ts[] = {
		    {GOAT_MARKER_STYLE_INVALID, "INVALID", "invalid marker style"},
		    {GOAT_MARKER_STYLE_NONE, "NONE", "none"},
		    {GOAT_MARKER_STYLE_POINT, "POINT", "•"},
		    {GOAT_MARKER_STYLE_SQUARE, "SQUARE", "■"},
		    {GOAT_MARKER_STYLE_TRIANGLE, "TRIANGLE", "▲"},
		    {GOAT_MARKER_STYLE_CROSS, "CROSS", "❌"},
		    {0, NULL, NULL},
		};
		t = g_enum_register_static ("GoatMarkerStyleTypes", ts);
	}

	return t;
}
