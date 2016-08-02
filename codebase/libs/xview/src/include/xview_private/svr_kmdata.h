/*      @(#)svr_kmdata.h  1.19  93/06/28  SMI   */

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 */

#ifndef svr_keymap_data_DEFINED
#define svr_keymap_data_DEFINED

#include <X11/keysym.h>
#include <xview/win_input.h>

#define MAX_NBR_MAPPINGS 6

#if (defined(__linux) || defined(__APPLE__)) && !defined(NULL)
#define NULL 0
#endif

/***************************************************************************
 *
 * Mouseless Model bindings.  Translates keysyms to XView semantic actions.
 *
 ***************************************************************************/
typedef struct key_binding {
    short	    action;	/* XView semantic action */
    char	   *name;	/* resource name and class */
    char	   *value;	/* default value.  May contain up to
				 * MAX_NBR_MAPPINGS mappings. */
} Key_binding;

static Key_binding sunview1_kbd_cmds[] = {
    /*
     * Keyboard Core Functions
     */
    ACTION_STOP, "OpenWindows.KeyboardCommand.Stop", "L1",
    ACTION_AGAIN, "OpenWindows.KeyboardCommand.Again", "a+Meta,a+Ctrl+Meta,L2",
    ACTION_PROPS, "OpenWindows.KeyboardCommand.Props", "L3",
    ACTION_UNDO, "OpenWindows.KeyboardCommand.Undo", "u+Meta,L4",
    ACTION_COPY, "OpenWindows.KeyboardCommand.Copy", "c+Meta,L6",
    ACTION_PASTE, "OpenWindows.KeyboardCommand.Paste", "v+Meta,L8",
    ACTION_FIND_FORWARD, "OpenWindows.KeyboardCommand.FindForward",
	"f+Meta,L9",
    ACTION_FIND_BACKWARD, "OpenWindows.KeyboardCommand.FindBackward",
	"F+Meta,L9+Shift",
    ACTION_CUT, "OpenWindows.KeyboardCommand.Cut", "x+Meta,L10",
    ACTION_HELP, "OpenWindows.KeyboardCommand.Help", "Help",
    ACTION_MORE_HELP, "OpenWindows.KeyboardCommand.MoreHelp", "Help+Shift",
    ACTION_TEXT_HELP, "OpenWindows.KeyboardCommand.TextHelp", "Help+Ctrl",
    ACTION_MORE_TEXT_HELP, "OpenWindows.KeyboardCommand.MoreTextHelp",
        "Help+Shift+Ctrl",       
    ACTION_DEFAULT_ACTION, "OpenWindows.KeyboardCommand.DefaultAction",
	"Return+Meta",
    ACTION_COPY_THEN_PASTE, "OpenWindows.KeyboardCommand.CopyThenPaste",
	"p+Meta",
    ACTION_TRANSLATE, "OpenWindows.KeyboardCommand.Translate", "R2",
    /*
     * Local Navigation Commands
     */
    ACTION_UP, "OpenWindows.KeyboardCommand.Up",
	"p+Ctrl,N+Ctrl,Up,R8,Up+Shift",
    ACTION_DOWN, "OpenWindows.KeyboardCommand.Down",
	"n+Ctrl,P+Ctrl,Down,R14,Down+Shift",
    ACTION_LEFT, "OpenWindows.KeyboardCommand.Left",
	"b+Ctrl,F+Ctrl,Left,R10,Left+Shift",
    ACTION_RIGHT, "OpenWindows.KeyboardCommand.Right",
	"f+Ctrl,B+Ctrl,Right,R12,Right+Shift",
    ACTION_JUMP_LEFT, "OpenWindows.KeyboardCommand.JumpLeft",
	"comma+Ctrl,greater+Ctrl",
    ACTION_JUMP_RIGHT, "OpenWindows.KeyboardCommand.JumpRight", "period+Ctrl",
    ACTION_GO_PAGE_BACKWARD, "OpenWindows.KeyboardCommand.GoPageBackward", "Prior,R9",
    ACTION_GO_PAGE_FORWARD, "OpenWindows.KeyboardCommand.GoPageForward", "Next,R15",
    ACTION_GO_WORD_FORWARD, "OpenWindows.KeyboardCommand.GoWordForward",
	"slash+Ctrl,less+Ctrl",
    ACTION_LINE_START, "OpenWindows.KeyboardCommand.LineStart", "a+Ctrl,E+Ctrl",
    ACTION_LINE_END, "OpenWindows.KeyboardCommand.LineEnd", "e+Ctrl,A+Ctrl",
    ACTION_GO_LINE_FORWARD, "OpenWindows.KeyboardCommand.GoLineForward",
	"apostrophe+Ctrl,R11",
    ACTION_DATA_START, "OpenWindows.KeyboardCommand.DataStart",
	"Home,R7,Return+Shift+Ctrl,Home+Shift",
    ACTION_DATA_END, "OpenWindows.KeyboardCommand.DataEnd",
	"End,R13,Return+Ctrl,End+Shift",
    /*
     * Text Editing Commands
     */
    ACTION_SELECT_FIELD_FORWARD,
	"OpenWindows.KeyboardCommand.SelectFieldForward", "Tab+Ctrl",
    ACTION_SELECT_FIELD_BACKWARD,
	"OpenWindows.KeyboardCommand.SelectFieldBackward", "Tab+Shift+Ctrl",
    ACTION_ERASE_CHAR_BACKWARD, "OpenWindows.KeyboardCommand.EraseCharBackward",
	"Delete,BackSpace",
    ACTION_ERASE_CHAR_FORWARD, "OpenWindows.KeyboardCommand.EraseCharForward",
	"Delete+Shift,BackSpace+Shift",
    ACTION_ERASE_WORD_BACKWARD, "OpenWindows.KeyboardCommand.EraseWordBackward",
	"w+Ctrl",
    ACTION_ERASE_WORD_FORWARD, "OpenWindows.KeyboardCommand.EraseWordForward",
	"W+Ctrl",
    ACTION_ERASE_LINE_BACKWARD, "OpenWindows.KeyboardCommand.EraseLineBackward",
	"u+Ctrl",
    ACTION_ERASE_LINE_END, "OpenWindows.KeyboardCommand.EraseLineEnd", "U+Ctrl",
    ACTION_MATCH_DELIMITER, "OpenWindows.KeyboardCommand.MatchDelimiter",
	"d+Meta",
    ACTION_EMPTY, "OpenWindows.KeyboardCommand.Empty", "e+Meta,e+Ctrl+Meta",
    ACTION_INCLUDE_FILE, "OpenWindows.KeyboardCommand.IncludeFile", "i+Meta",
    ACTION_INSERT, "OpenWindows.KeyboardCommand.Insert", "Insert",
    ACTION_LOAD, "OpenWindows.KeyboardCommand.Load", "l+Meta",
    ACTION_STORE, "OpenWindows.KeyboardCommand.Store", "s+Meta",
    0, NULL, NULL
};

static Key_binding basic_kbd_cmds[] = {
    /*
     * Keyboard Core Functions
     */
    ACTION_QUOTE_NEXT_KEY, "OpenWindows.KeyboardCommand.QuoteNextKey", "q+Alt",
    /*
     * Local Navigation Commands
     */
    ACTION_UP, "OpenWindows.KeyboardCommand.Up", "Up",
    ACTION_DOWN, "OpenWindows.KeyboardCommand.Down", "Down",
    ACTION_LEFT, "OpenWindows.KeyboardCommand.Left", "Left",
    ACTION_RIGHT, "OpenWindows.KeyboardCommand.Right", "Right",
    ACTION_JUMP_UP, "OpenWindows.KeyboardCommand.JumpUp", "Up+Ctrl",
    ACTION_JUMP_DOWN, "OpenWindows.KeyboardCommand.JumpDown", "Down+Ctrl",
    ACTION_JUMP_LEFT, "OpenWindows.KeyboardCommand.JumpLeft", "Left+Ctrl",
    ACTION_JUMP_RIGHT, "OpenWindows.KeyboardCommand.JumpRight", "Right+Ctrl",
    ACTION_ROW_START, "OpenWindows.KeyboardCommand.RowStart", "Home,R7",
    ACTION_ROW_END, "OpenWindows.KeyboardCommand.RowEnd", "End,R13",
    ACTION_PANE_UP, "OpenWindows.KeyboardCommand.PaneUp", "Prior,R9",
    ACTION_PANE_DOWN, "OpenWindows.KeyboardCommand.PaneDown", "Next,R15",
    ACTION_PANE_LEFT, "OpenWindows.KeyboardCommand.PaneLeft", "Prior+Ctrl,R9+Ctrl",
    ACTION_PANE_RIGHT, "OpenWindows.KeyboardCommand.PaneRight", "Next+Ctrl,R15+Ctrl",
    ACTION_DATA_START, "OpenWindows.KeyboardCommand.DataStart",
	"Home+Ctrl,R7+Ctrl",
    ACTION_DATA_END, "OpenWindows.KeyboardCommand.DataEnd", "End+Ctrl,R13+Ctrl",
    /*
     * Text Editing Commands
     */
    ACTION_SELECT_UP, "OpenWindows.KeyboardCommand.SelectUp", "Up+Shift",
    ACTION_SELECT_DOWN, "OpenWindows.KeyboardCommand.SelectDown", "Down+Shift",
    ACTION_SELECT_LEFT, "OpenWindows.KeyboardCommand.SelectLeft", "Left+Shift",
    ACTION_SELECT_RIGHT, "OpenWindows.KeyboardCommand.SelectRight",
	"Right+Shift",
    ACTION_SELECT_JUMP_UP, "OpenWindows.KeyboardCommand.SelectJumpUp",
	"Up+Shift+Ctrl",
    ACTION_SELECT_JUMP_DOWN, "OpenWindows.KeyboardCommand.SelectJumpDown",
	"Down+Shift+Ctrl",
    ACTION_SELECT_JUMP_LEFT, "OpenWindows.KeyboardCommand.SelectJumpLeft",
	"Left+Shift+Ctrl",
    ACTION_SELECT_JUMP_RIGHT, "OpenWindows.KeyboardCommand.SelectJumpRight",
	"Right+Shift+Ctrl",
    ACTION_SELECT_ROW_START, "OpenWindows.KeyboardCommand.SelectRowStart",
	"Home+Shift,R7+Shift",
    ACTION_SELECT_ROW_END, "OpenWindows.KeyboardCommand.SelectRowEnd",
	"End+Shift,R13+Shift",
    ACTION_SELECT_PANE_UP, "OpenWindows.KeyboardCommand.SelectPaneUp",
	"Prior+Shift,R9+Shift",
    ACTION_SELECT_PANE_DOWN, "OpenWindows.KeyboardCommand.SelectPaneDown",
	"Next+Shift,R15+Shift",
    ACTION_SELECT_PANE_LEFT, "OpenWindows.KeyboardCommand.SelectPaneLeft",
	"Prior+Shift+Ctrl,R9+Shift+Ctrl",
    ACTION_SELECT_PANE_RIGHT, "OpenWindows.KeyboardCommand.SelectPaneRight",
	"Next+Shift+Ctrl,R15+Shift+Ctrl",
    ACTION_SELECT_DATA_START, "OpenWindows.KeyboardCommand.SelectDataStart",
	"Home+Shift+Ctrl,R7+Shift+Ctrl",
    ACTION_SELECT_DATA_END, "OpenWindows.KeyboardCommand.SelectDataEnd",
	"End+Shift+Ctrl,R13+Shift+Ctrl",
    ACTION_SELECT_ALL, "OpenWindows.KeyboardCommand.SelectAll",
	"End+Shift+Meta,R13+Shift+Meta",
    ACTION_SELECT_NEXT_FIELD, "OpenWindows.KeyboardCommand.SelectNextField",
	"Tab+Meta",
    ACTION_SELECT_PREVIOUS_FIELD,
	"OpenWindows.KeyboardCommand.SelectPreviousField", "Tab+Shift+Meta",
    ACTION_SCROLL_UP, "OpenWindows.KeyboardCommand.ScrollUp", "Up+Alt",
    ACTION_SCROLL_DOWN, "OpenWindows.KeyboardCommand.ScrollDown", "Down+Alt",
    ACTION_SCROLL_LEFT, "OpenWindows.KeyboardCommand.ScrollLeft", "Left+Alt",
    ACTION_SCROLL_RIGHT, "OpenWindows.KeyboardCommand.ScrollRight", "Right+Alt",
    ACTION_SCROLL_JUMP_UP, "OpenWindows.KeyboardCommand.ScrollJumpUp",
	"Up+Alt+Ctrl",
    ACTION_SCROLL_JUMP_DOWN, "OpenWindows.KeyboardCommand.ScrollJumpDown",
	"Down+Alt+Ctrl",
    ACTION_SCROLL_JUMP_LEFT, "OpenWindows.KeyboardCommand.ScrollJumpLeft",
	"Left+Alt+Ctrl",
    ACTION_SCROLL_JUMP_RIGHT, "OpenWindows.KeyboardCommand.ScrollJumpRight",
	"Right+Alt+Ctrl",
    ACTION_SCROLL_ROW_START, "OpenWindows.KeyboardCommand.ScrollRowStart",
	"Home+Alt,R7+Alt",
    ACTION_SCROLL_ROW_END, "OpenWindows.KeyboardCommand.ScrollRowEnd",
	"End+Alt,R13+Alt",
    ACTION_SCROLL_PANE_UP, "OpenWindows.KeyboardCommand.ScrollPaneUp",
	"Prior+Alt,R9+Alt",
    ACTION_SCROLL_PANE_DOWN, "OpenWindows.KeyboardCommand.ScrollPaneDown",
	"Next+Alt,R15+Alt",
    ACTION_SCROLL_PANE_LEFT, "OpenWindows.KeyboardCommand.ScrollPaneLeft",
	"Prior+Alt+Ctrl,R9+Alt+Ctrl",
    ACTION_SCROLL_PANE_RIGHT, "OpenWindows.KeyboardCommand.ScrollPaneRight",
	"Next+Alt+Ctrl,R15+Alt+Ctrl",
    ACTION_SCROLL_DATA_START, "OpenWindows.KeyboardCommand.ScrollDataStart",
	"Home+Alt+Ctrl,R7+Alt+Ctrl",
    ACTION_SCROLL_DATA_END, "OpenWindows.KeyboardCommand.ScrollDataEnd",
	"End+Alt+Ctrl,R13+Alt+Ctrl",
    ACTION_ERASE_CHAR_BACKWARD, "OpenWindows.KeyboardCommand.EraseCharBackward",
	"Delete,BackSpace",
    ACTION_ERASE_CHAR_FORWARD, "OpenWindows.KeyboardCommand.EraseCharForward",
	"Delete+Shift,BackSpace+Shift",
    ACTION_ERASE_LINE, "OpenWindows.KeyboardCommand.EraseLine",
	"Delete+Meta,BackSpace+Meta",
    0, NULL, NULL
};

static Key_binding full_kbd_cmds[] = {
    /*
     * Mouseless Core Functions
     */
    ACTION_ADJUST, "OpenWindows.KeyboardCommand.Adjust", "Insert+Alt",
    ACTION_MENU, "OpenWindows.KeyboardCommand.Menu", "space+Alt",
    ACTION_INPUT_FOCUS_HELP, "OpenWindows.KeyboardCommand.InputFocusHelp",
	"question+Ctrl",
    ACTION_SUSPEND_MOUSELESS, "OpenWindows.KeyboardCommand.SuspendMouseless",
	"z+Alt",
    ACTION_RESUME_MOUSELESS, "OpenWindows.KeyboardCommand.ResumeMouseless",
	"Z+Alt",
    ACTION_JUMP_MOUSE_TO_INPUT_FOCUS,
	"OpenWindows.KeyboardCommand.JumpMouseToInputFocus", "j+Alt",
    /*
     * Global Navigation Commands
     */
    ACTION_NEXT_ELEMENT, "OpenWindows.KeyboardCommand.NextElement", "Tab+Ctrl",
    ACTION_PREVIOUS_ELEMENT, "OpenWindows.KeyboardCommand.PreviousElement",
	"Tab+Shift+Ctrl",
    ACTION_NEXT_PANE, "OpenWindows.KeyboardCommand.NextPane", "a+Alt",
    ACTION_PREVIOUS_PANE, "OpenWindows.KeyboardCommand.PreviousPane",
	"A+Alt",
    /*
     * Miscellaneous Navigation Commands
     */
    ACTION_PANEL_START, "OpenWindows.KeyboardCommand.PanelStart",
	"bracketleft+Ctrl",
    ACTION_PANEL_END, "OpenWindows.KeyboardCommand.PanelEnd",
	"bracketright+Ctrl",
    ACTION_VERTICAL_SCROLLBAR_MENU,
	"OpenWindows.KeyboardCommand.VerticalScrollbarMenu", "v+Alt",
    ACTION_HORIZONTAL_SCROLLBAR_MENU,
	"OpenWindows.KeyboardCommand.HorizontalScrollbarMenu", "h+Alt",
    ACTION_PANE_BACKGROUND, "OpenWindows.KeyboardCommand.PaneBackground",
	"b+Alt",
    0, NULL, NULL
};

#endif /* ~svr_keymap_data_DEFINED */
