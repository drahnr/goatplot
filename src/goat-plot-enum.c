#include "goat-plot-enum.h"

GType goat_orientation_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static GEnumValue ts[] = {
		    {GOAT_ORIENTATION_VERTICAL, "Vertical scale", "vertical"},
		    {GOAT_ORIENTATION_HORIZONTAL, "Horizontal scale", "horizontal"},
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
		    {GOAT_POSITION_TOP, "Ontop the graph", "top"},
		    {GOAT_POSITION_BOTTOM, "Below graph", "bottom"},
		    {GOAT_POSITION_LEFT, "Left of the graph", "left"},
		    {GOAT_POSITION_RIGHT, "Right of the graph", "right"},
		    {0, NULL, NULL},
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
		    {GOAT_HEADING_IN, "Scale dots inside", "in"},
		    {GOAT_HEADING_OUT, "Scale dots outside", "out"},
		    {0, NULL, NULL},
		};
		t = g_enum_register_static ("GoatHeadingTypes", ts);
	}

	return t;
}
