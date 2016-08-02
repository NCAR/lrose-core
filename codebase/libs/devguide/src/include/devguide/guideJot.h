/*
 * API for guideJot.cc
 */

#include <jot/jot.h>
#include <jot/view.h>

/*
 * In the following, `word' is as defined by JotSearch for word-beginning
 * and word-end regexp elements.  `Display line' indicates a single
 * horizontal line of text, as currently displayed.  Since jot canvases
 * normally wrap lines at word breaks, display lines are not necessarily
 * terminated by a NL character.
 */
typedef enum {
    GJ_NOOP,			/* Do nothing */
    GJ_CHAR_FORWARD,		/* Move caret forward one character */
    GJ_CHAR_BACKWARD,		/* Move caret backward one character */
    GJ_CHAR_DELETE_FORWARD,	/* Delete character after caret */
    GJ_CHAR_DELETE_BACKWARD,	/* Delete character before caret */
    GJ_CHAR_INSERT_SELF,	/* Insert character at caret & advance */
    GJ_CHAR_INSERT_NL,		/* Insert NewLine at caret & advance */
    GJ_WORD_FORWARD,		/* Move caret forward one word */
    GJ_WORD_BACKWARD,		/* Move caret backward one word */
    GJ_WORD_DELETE_FORWARD,	/* Delete word after caret */
    GJ_WORD_DELETE_BACKWARD,	/* Delete word before caret */
    GJ_LINE_BACKWARD,		/* Move caret backward one display line */
    GJ_LINE_FORWARD,		/* Move caret forward one display line */
    GJ_LINE_DELETE_FORWARD,	/* Delete from caret to end of display line */
    GJ_LINE_START,		/* Move caret to beginning of display line */
    GJ_LINE_END,		/* Move caret to end of display line */
    GJ_DOC_START,		/* Move caret to start of document */
    GJ_DOC_END,			/* Move caret to end of document */
    GJ_UNDO,			/* Undo last change */
    GJ_REDO,			/* Redo undone change */
    GJ_ENUM_LIMIT		/* Place and limit holder */
} guide_jot_edit_function ;



#define guide_DefaultKeys	guide_EmacsKeys


EXTERN_FUNCTION( void guide_JotEdit,		(JotView *, char, guide_jot_edit_function) );
EXTERN_FUNCTION( void guide_EmacsKeys,		(JotView *, int) );
