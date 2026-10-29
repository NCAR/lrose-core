/*
 * @(#)guide.h	2.25 91/10/15 Copyright 1989 Sun Microsystems.
 *
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 * 
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef guide_guide_DEFINED
#define guide_guide_DEFINED

#include	<devguide/c_varieties.h>

/*
 * Miscellaneous definitions.
 */
#define OK	0
#define	ERROR	-1
#define	TRUE	1
#define	FALSE	0

/*
 * String appended to an application name to form the GIL file.
 */
#define	G_GIL_SUFFIX_STRING	".G"

/*
 * GIL version string.  The version number will be incremented when
 * the format of GIL files is changed.
 */
#define	G_VERSION		3
#define	G_VERSION_PREFIX	";GIL-"

/*
 * Public enumerated types.
 */

/*
 * GUIDE object types.
 */
typedef enum
{
	G_BASE_WINDOW,
	G_BUTTON,
	G_CANVAS_PANE,
	G_CONTROL_AREA,
	G_DROP_TARGET,
	G_GAUGE,
	G_GROUP,
	G_MENU,
	G_MENU_ITEMS,
	G_MESSAGE,
	G_POPUP_WINDOW,
	G_SCROLLING_LIST,
	G_SETTING,
	G_SETTING_ITEMS,
	G_SLIDER,
	G_STACK,
	G_TERM_PANE,
	G_TEXT_FIELD,
	G_TEXT_PANE,
}               G_TYPES;


/*
 * Argument type used in the resfile.
 */
typedef enum
{
	G_VOID_TYPE,
	G_INT_TYPE,
	G_FLOAT_TYPE,
	G_STRING_TYPE,
} G_ARG_TYPES;

/*
 * Function type used in the resfile
 */
typedef enum
{
	G_USER_TYPE,
	G_FUNCTION_TYPE,
	G_CODE_TYPE,
} G_FUNC_TYPES;


/*
 * Button types.
 */
typedef enum
{
	G_NORMAL,
	G_ABBREVIATED,
}		G_BUTTON_TYPES;

/*
 * Drawing models.
 */
typedef enum
{
	G_XVIEW,
	G_XWINDOWS,
	G_POSTSCRIPT,
}		G_DRAWING_MODELS;

/*
 * Label types.
 */
typedef enum
{
	G_STRING,
	G_GLYPH,
}               G_LABEL_TYPES;

/*
 * Layout types for the G_LAYOUT attribute.
 */
typedef enum
{
	G_HORIZONTAL,
	G_VERTICAL,
}               G_LAYOUT_TYPES;

/*
 * Types for the G_GROUP_TYPE attribute.
 */
typedef enum
{
	G_NONE,
	G_ROW,
	G_COLUMN,
	G_ROWCOLUMN,
}               G_GROUP_TYPES;

/*
 * Types for the G_{ANCHOR,REFERENCE}_POINT attributes.
 */
typedef enum
{
	G_NORTHWEST,
	G_NORTH,
	G_NORTHEAST,
	G_WEST,
	G_CENTER,
	G_EAST,
	G_SOUTHWEST,
	G_SOUTH,
	G_SOUTHEAST,
}               G_COMPASS_POINTS;

/*
 * Types for the G_COL_ALIGNMENT attribute.
 */
typedef enum
{
	G_LEFT_EDGES,
	G_LABELS,
	G_VCENTERS,
	G_RIGHT_EDGES,
}               G_COL_ALIGNMENTS;

/*
 * Types for the G_ROW_ALIGNMENT attribute.
 */
typedef enum
{
	G_TOP_EDGES,
	G_HCENTERS,
	G_BOTTOM_EDGES,
}               G_ROW_ALIGNMENTS;

/*
 * Menu types.
 */
typedef enum
{
	G_COMMAND_MENU,
	G_EXCLUSIVE_MENU,
	G_NONEXCLUSIVE_MENU,
}		G_MENU_TYPES;

/*
 * Setting types.
 */
typedef enum
{
	G_EXCLUSIVE,
	G_NONEXCLUSIVE,
	G_CHECK,
	G_SETTING_STACK,
}		G_SETTING_TYPES;

/*
 * Event types.
 */
typedef enum
{
	G_KEYBOARD,
	G_KEYBOARD_LEFT,
	G_KEYBOARD_RIGHT,
	G_KEYBOARD_TOP,
	G_MOUSE,
	G_MOUSE_DRAG,
	G_MOUSE_ENTER,
	G_MOUSE_EXIT,
	G_MOUSE_MOVE,
}		G_EVENT_TYPES;

/*
 * Text types.
 */
typedef enum
{
	G_ALPHANUMERIC,
	G_MULTILINE,
	G_NUMERIC,
}		G_TEXT_TYPES;

/*
 * Intitial state types.
 */
typedef enum
{
	G_ACTIVE,
	G_ICONIC,
	G_INACTIVE,
	G_INVISIBLE,
	G_NOTSELECTED,
	G_OPEN,
	G_SELECTED,
	G_VISIBLE,
}		G_INITIAL_STATES;

/*
 * GUIDE object attributes.
 */
typedef enum
{
	G_ACTIONS,
	G_ANCHOR_OBJECT,
	G_ANCHOR_POINT,
	G_BACKGROUND_COLOR,
	G_BUSY_DROP_GLYPH,
	G_BUTTON_TYPE,
	G_CHOICES,
	G_CHOICE_DEFAULTS,
	G_CHOICE_LABEL_TYPES,
	G_CHOICE_COLORS,
	G_COLUMNS,
	G_COL_ALIGNMENT,
	G_CONNECTIONS,
	G_CONSTANT_WIDTH,
	G_DEFAULT_DROP_SITE,
	G_DND_ACCEPT_CURSOR,
	G_DND_ACCEPT_CURSOR_XHOT,
	G_DND_ACCEPT_CURSOR_YHOT,
	G_DND_CURSOR,
	G_DND_CURSOR_XHOT,
	G_DND_CURSOR_YHOT,
	G_DRAGGABLE,
	G_DONE_HANDLER,
	G_DROPPABLE,
	G_DROP_TARGET_WIDTH,
	G_DRAWING_MODEL,
	G_EVENTS,
	G_EVENT_HANDLER,
	G_FOREGROUND_COLOR,
	G_GROUP_TYPE,
	G_HEIGHT,
	G_HELP,
	G_HOFFSET,
	G_HSCROLL,
	G_HSPACING,
	G_ICON,
	G_ICON_LABEL,
	G_ICON_MASK,
	G_INITIAL_LIST_GLYPHS,
	G_INITIAL_LIST_VALUES,
	G_INITIAL_SELECTIONS,
	G_INITIAL_STATE,
	G_INITIAL_VALUE,
	G_INTERNATIONAL_DB_BEGIN,
	G_INTERNATIONAL_DB_END,
	G_LABEL,
	G_LABEL_TYPE,
	G_LABEL_BOLD,
	G_LAYOUT_TYPE,
	G_MAPPED,
	G_MAX_TICK_STRING,
	G_MAX_VALUE,
	G_MAX_VALUE_STRING,
	G_MEMBERS,
	G_MENU_HANDLER,
	G_MENU_ITEM_COLORS,
	G_MENU_ITEM_DEFAULTS,
	G_MENU_ITEM_HANDLERS,
	G_MENU_ITEM_LABELS,
	G_MENU_ITEM_LABEL_TYPES,
	G_MENU_ITEM_MENUS,
	G_MENU_ITEM_STATES,
	G_MENU_NAME,
	G_MENU_TITLE,
	G_MENU_TYPE,
	G_MIN_TICK_STRING,
	G_MIN_VALUE,
	G_MIN_VALUE_STRING,
	G_MULTIPLE_SELECTIONS,
	G_NAME,
	G_NORMAL_DROP_GLYPH,
	G_NOTIFY_HANDLER,
	G_ORIENTATION,
	G_OWNER,
	G_PINNABLE,
	G_PINNED,
	G_READ_ONLY,
	G_REFERENCE_POINT,
	G_REPAINT_PROC,
	G_RESIZABLE,
	G_ROWS,
	G_ROW_ALIGNMENT,
	G_SCROLLABLE_HEIGHT,
	G_SCROLLABLE_WIDTH,
	G_SELECTION_REQUIRED,
	G_SETTING_TYPE,
	G_SHOW_BORDER,
	G_SHOW_ENDBOXES,
	G_SHOW_FOOTER,
	G_SHOW_RANGE,
	G_SHOW_VALUE,
	G_SLIDER_WIDTH,
	G_STORED_LENGTH,
	G_TEXT_TYPE,
	G_TICKS,
	G_TITLE,
	G_TYPE,
	G_USER_DATA,
	G_VALUE_LENGTH,
	G_VALUE_UNDERLINED,
	G_VALUE_X,
	G_VALUE_Y,
	G_VOFFSET,
	G_VSCROLL,
	G_VSPACING,
	G_WIDTH,
	G_X,
	G_Y,
}               G_ATTRS;


/*
 * The action attributes.
 */
typedef enum
{
	G_ACTION_FROM,
	G_ACTION_WHEN,
	G_ACTION_TO,
	G_ACTION_FUNC_TYPE,
	G_ACTION_ARG_TYPE,
	G_ACTION_OPERATION,
}		G_ACTION_ATTRS;


/*
 * GUIDE project attributes.
 */
typedef enum
{
	G_INTERFACES,
	G_PROJ_ACTIONS,
	G_ROOT_WINDOW,
}		G_PROJ_ATTRS;


/*
 * GUIDE resfile attributes.
 */
typedef enum
{
	G_VALID_TYPE,
	G_VALID_EVENTS,
	G_VALID_ACTIONS,
	G_VALID_RECEIVERS,
}		G_RESFILE_ATTRS;


/*
 * A struct used to read info from the static message table.
 */
typedef struct G_MSG_STRUCT
{
	G_FUNC_TYPES	func_type;
	char		*name;
	G_ARG_TYPES	arg_type;
}		G_MSG_STRUCT;


/*
 * Public function declarations.
 */
EXTERN_FUNCTION( G_ATTRS	*G_object_attrs,	(G_TYPES) );
EXTERN_FUNCTION( G_ACTION_ATTRS	*G_action_attrs,	(_VOID_) );
EXTERN_FUNCTION( G_PROJ_ATTRS	*G_project_attrs,	(_VOID_)  );
EXTERN_FUNCTION( char		*G_attr_to_string,	(G_ATTRS) );
EXTERN_FUNCTION( G_ATTRS	G_string_to_attr,	(char *) );
EXTERN_FUNCTION( char		*G_action_attr_to_string, (G_ACTION_ATTRS) );
EXTERN_FUNCTION( G_ACTION_ATTRS	G_string_to_action_attr,	(char *) );
EXTERN_FUNCTION( char		*G_proj_attr_to_string,	(G_PROJ_ATTRS) );
EXTERN_FUNCTION( G_PROJ_ATTRS	G_string_to_proj_attr,	(char *) );

EXTERN_FUNCTION( G_RESFILE_ATTRS	G_string_to_resfile_attr,(char *) );
EXTERN_FUNCTION( G_ARG_TYPES	G_string_to_arg_type,	(char *) );

EXTERN_FUNCTION( char		*G_drawing_model_to_string, (G_DRAWING_MODELS) );
EXTERN_FUNCTION( G_DRAWING_MODELS	G_string_to_drawing_model, (char *) );

EXTERN_FUNCTION( char		*G_event_type_to_string, (G_EVENT_TYPES) );
EXTERN_FUNCTION( G_EVENT_TYPES	G_string_to_event_type, (char *) );

EXTERN_FUNCTION( char		*G_label_type_to_string, (G_LABEL_TYPES) );
EXTERN_FUNCTION( G_LABEL_TYPES	G_string_to_label_type, (char *) );

EXTERN_FUNCTION( char		*G_layout_type_to_string, (G_LAYOUT_TYPES) );
EXTERN_FUNCTION( G_LAYOUT_TYPES	G_string_to_layout_type, (char *) );

EXTERN_FUNCTION( char		*G_intitial_state_to_string, (G_INITIAL_STATES) );
EXTERN_FUNCTION( G_INITIAL_STATES G_string_to_initial_state, (char *) );

EXTERN_FUNCTION( char		*G_group_type_to_string, (G_GROUP_TYPES) );
EXTERN_FUNCTION( G_GROUP_TYPES	G_string_to_group_type, (char *) );

EXTERN_FUNCTION( char		*G_compass_point_to_string, (G_COMPASS_POINTS) );
EXTERN_FUNCTION( G_COMPASS_POINTS G_string_to_compass_point, (char *) );

EXTERN_FUNCTION( char		*G_col_alignment_to_string, (G_COL_ALIGNMENTS) );
EXTERN_FUNCTION( G_COL_ALIGNMENTS	G_string_to_col_alignment, (char *) );

EXTERN_FUNCTION( char		*G_row_alignment_to_string, (G_ROW_ALIGNMENTS) );
EXTERN_FUNCTION( G_ROW_ALIGNMENTS	G_string_to_row_alignment, (char *) );

EXTERN_FUNCTION( char		*G_button_type_to_string, (G_BUTTON_TYPES) );
EXTERN_FUNCTION( G_BUTTON_TYPES	G_string_to_button_type, (char *) );

EXTERN_FUNCTION( char		*G_menu_type_to_string, (G_MENU_TYPES) );
EXTERN_FUNCTION( G_MENU_TYPES	G_string_to_menu_type, (char *) );

EXTERN_FUNCTION( char		*G_setting_type_to_string, (G_SETTING_TYPES) );
EXTERN_FUNCTION( G_SETTING_TYPES	G_string_to_setting_type, (char *) );

EXTERN_FUNCTION( char		*G_text_type_to_string, (G_TEXT_TYPES) );
EXTERN_FUNCTION( G_TEXT_TYPES	G_string_to_text_type, (char *) );

EXTERN_FUNCTION( char		*G_type_to_string, (G_TYPES) );
EXTERN_FUNCTION( G_TYPES	G_string_to_type, (char *) );

#endif
