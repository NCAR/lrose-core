/*	@(#)seln_impl.h 20.38 93/06/28		*/

#ifndef	suntool_selection_impl_DEFINED
#define	suntool_selection_impl_DEFINED

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <errno.h>
#ifndef FILE
#if !defined(SVR4) && !defined(__linux) && !defined(__APPLE__)
#undef NULL
#endif /* SVR4 */
#include <stdio.h>
#endif /* FILE */
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <xview_private/i18n_impl.h>
#include <xview/notify.h>
#include <xview/pkg.h>
#include <xview/sel_pkg.h>
#include <xview/sel_svc.h>
#include <xview/sel_attrs.h>
#include <X11/Xlib.h>

extern int          errno;



/*
 * Procedure IDs for client-module procedures 
 */

#define SELN_CLNT_REQUEST	17
#define SELN_CLNT_DO_FUNCTION	18


/*	initializers		*/

#define SELN_NULL_ACCESS { 0, 0, {0}, {0}, 0}
#define SELN_NULL_HOLDER { SELN_UNKNOWN, SELN_NONE, SELN_NULL_ACCESS}
#define SELN_STD_TIMEOUT_SEC 	4
#define SELN_STD_TIMEOUT_USEC 	0	/* 4 sec timeout on connections  */


#define complain(str)	\
	(void)fprintf(stderr, XV_MSG("Selection library internal error:\n%s\n"), XV_MSG(str))

typedef struct {
    void	    (*do_function)();
    Seln_result	    (*do_request)();
}	Seln_client_ops;


typedef struct client_node    {
    Seln_client_ops	 ops;  /* How we pass requests to client  */
    char		*client_data;
    Seln_access		 access;
    struct client_node	*next;
    unsigned		client_num; /* this client is the (client_num)th
    				     * client for this selection library
				     */
}	Seln_client_node;


#define HIST_SIZE	50

#ifdef OW_I18N
typedef struct {
    unsigned char       first_time;
    unsigned char       event_sent;
    XID                 requestor;
    Atom                property;
    Atom                selection;
    Atom                target;
    Display             *display;
    int                 chars_remaining;
    Time                timestamp;
    unsigned char       format;
    CHAR                *buffer;
    int                 offset;
} Seln_agent_context;
#else
typedef struct {
    unsigned char     	first_time;
    unsigned char	event_sent;
    XID         	requestor;
    Atom		property;
    Atom		selection;
    Atom		target;
    Display		*display;
    int			bytes_remaining;
    Time		timestamp;
    unsigned char	format;
} Seln_agent_context;
#endif /* OW_I18N */

typedef struct {
    long		offset;
    Atom		property;/* Property returned after XConvertSelection*/
} Seln_agent_getprop;
#define	SELN_RANKS	((u_int)SELN_UNSPECIFIED)

typedef struct {
    Atom	length;
    Atom	contents_pieces;
    Atom	first;
    Atom	first_unit;
    Atom	last;
    Atom	last_unit;
    Atom	level;
    Atom	file_name;
    Atom	commit_pending_delete;
    Atom	delete;
    Atom	restore;
    Atom	yield;
    Atom	fake_level;
    Atom	set_level;
    Atom	end_request;
    Atom	targets;
    Atom	do_function;
    Atom	multiple;
    Atom	timestamp;
    Atom	string;
    Atom	is_readonly;
    Atom	func_key_state;
    Atom	selected_windows;
    Atom	object_content;
    Atom	object_size;
#ifdef OW_I18N
    Atom        length_chars;
    Atom	first_wc;
    Atom	last_wc;
    Atom        compound_text;
#endif
} Seln_target_atoms;

#define SELN_PROPERTY	100

typedef struct {
    Seln_agent_context	req_context;
    Seln_holder		client_holder[SELN_RANKS];
    int			held_file[SELN_RANKS];
    Seln_holder		agent_holder;
    Time		seln_acquired_time[SELN_RANKS];
    XID			xid;
    Seln_agent_getprop	get_prop;
    Seln_target_atoms	targets;
    Atom		property[SELN_PROPERTY];
    Atom		clipboard;
    Atom		caret;
    int			timeout;	/* Timeout in secs */
} Seln_agent_info;


#define CLIPBOARD(agent)	agent->clipboard
#define CARET(agent)		agent->caret
#define LENGTH(selection)	"LENGTH", SELN_REQ_BYTESIZE, selection.length
#define FIRST(selection)	"_SUN_SELN_FIRST", SELN_REQ_FIRST, \
								 selection.first
#define FIRST_UNIT(selection)	"_SUN_SELN_FIRST_UNIT", SELN_REQ_FIRST_UNIT, \
						            selection.first_unit
#define LAST(selection)		"_SUN_SELN_LAST", SELN_REQ_LAST, selection.last
#define LAST_UNIT(selection)	"_SUN_SELN_LAST_UNIT", SELN_REQ_LAST_UNIT, \
							     selection.last_unit
#define LEVEL(selection)	"_SUN_SELN_LEVEL", SELN_REQ_LEVEL, \
								 selection.level
#define FILE_NAME(selection)	"FILE_NAME", SELN_REQ_FILE_NAME, \
						             selection.file_name
#define DELETE(selection)	"_SUN_SELN_DELETE", SELN_REQ_DELETE, \
								selection.delete
#define RESTORE(selection)	"_SUN_SELN_RESTORE", SELN_REQ_RESTORE, \
							       selection.restore
#define YIELD(selection)	"_SUN_SELN_YIELD", SELN_REQ_YIELD, \
								 selection.yield
#define FAKE_LEVEL(selection)	"_SUN_SELN_FAKE_LEVEL", SELN_REQ_FAKE_LEVEL, \
							    selection.fake_level
#define SET_LEVEL(selection)	"_SUN_SELN_SET_LEVEL", SELN_REQ_SET_LEVEL, \
							     selection.set_level
#define END_REQUEST(selection)	"_SUN_SELN_END_REQUEST", SELN_REQ_END_REQUEST, \
							   selection.end_request
#define TARGETS(selection)	"TARGETS", ((Seln_attribute) 0), \
    	    	    	    	    	    	    	selection.targets
#define MULTIPLE(selection)	"MULTIPLE", ((Seln_attribute) 0), \
    	    	    	    	    	    	    	selection.multiple
#define TIMESTAMP(selection)	"TIMESTAMP", ((Seln_attribute) 0), \
    	    	    	    	    	    	    	selection.timestamp
#define DO_FUNCTION(selection)	"_SUN_SELN_DO_FUNCTION", ((Seln_attribute) 0), \
    	    	    	    	    	    	    	selection.do_function
#define STRING(selection)	"STRING", SELN_REQ_CONTENTS_ASCII, \
							selection.string

#ifdef OW_I18N
#define COMPOUND_TEXT(selection) "COMPOUND_TEXT", SELN_REQ_CONTENTS_CT, \
                                                        selection.compound_text
#define LENGTH_CHARS(selection) "LENGTH_CHARS", SELN_REQ_CHARSIZE, \
                                                        selection.length_chars
#define FIRST_WC(selection)	"_SUN_SELN_FIRST_WC", SELN_REQ_FIRST_WC, \
							selection.first_wc
#define LAST_WC(selection)	"_SUN_SELN_LAST_WC", SELN_REQ_LAST_WC, \
							selection.last_wc
#endif /* OW_I18N */

#define IS_READONLY(selection)	"_SUN_SELN_IS_READONLY", SELN_REQ_IS_READONLY, \
							   selection.is_readonly
#define OBJECT_SIZE(selection)	"_SUN_SELN_OBJECT_SIZE", SELN_REQ_OBJECT_SIZE, \
							   selection.object_size
#define CONTENTS_PIECES(selection)	"_SUN_SELN_CONTENTS_PIECES", \
                             SELN_REQ_CONTENTS_PIECES, selection.contents_pieces
#define COMMIT_PENDING_DELETE(selection) "_SUN_SELN_COMMIT_PENDING_DELETE", \
		 SELN_REQ_COMMIT_PENDING_DELETE, selection.commit_pending_delete
#define SELECTED_WINDOWS(selection)	"_SUN_SELN_SELECTED_WINDOWS", \
		           SELN_REQ_SELECTED_WINDOWS, selection.selected_windows
#define CONTENTS_OBJECT(selection)	"_SUN_SELN_CONTENTS_OBJECT", \
			      SELN_REQ_CONTENTS_OBJECT, selection.object_content
#define FUNC_KEY_STATE(selection)	"_SUN_SELN_FUNC_KEY_STATE", \
			       SELN_REQ_FUNC_KEY_STATE, selection.func_key_state
#define NUM_OF_TARGETS		22

#define SELN_REPORT(event)	seln_report_event(0, event)

/*	routines to manipulate the function-key state		*/

Pkg_private Seln_result seln_get_reply_buffer();
Pkg_private Seln_result seln_send_yield();
Pkg_private void 	seln_init_reply();
Pkg_private Seln_rank	selection_to_rank();
Pkg_private Atom	seln_rank_to_selection();
Pkg_private void	selection_agent_get_holder();
Pkg_private int		seln_equal_agent();
Pkg_private Xv_opaque	seln_agent_client();

#endif
