#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_selsvc.c 20.64 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Textsw interface to selection service.
 */

#include <errno.h>
#include <xview_private/portable.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#ifdef SVR4
#include <stdlib.h>
#endif /* SVR4 */

extern int      errno;

Pkg_private Es_status es_copy();
static Seln_result textsw_seln_yield();

Pkg_private Es_index
  textsw_input_partial(Textsw_view_handle view, CHAR *buf, long int buf_len);

static int
  textsw_should_ask_seln_svc(register Textsw_folio textsw);

static char           *shelf_name = "/tmp/textsw_shelf";

#define	SAME_HOLDER(folio, holder)				\
	seln_holder_same_client((Seln_holder *)holder,		\
				(char *)FOLIO_REP_TO_ABS(folio))

typedef struct continuation {
    int             in_use, type;
    Es_index        current, first, last_plus_one;
#ifdef OW_I18N    
    Es_index        first_mb, last_plus_one_mb;
#endif
    unsigned        span_level;
}               Continuation_object;
typedef Continuation_object *Continuation;
static Continuation_object fast_continuation;	/* Default init to zeros */

Pkg_private void
textsw_clear_secondary_selection(textsw, type)
    register Textsw_folio textsw;
    unsigned        type;
{
    if (type & EV_SEL_SECONDARY) {
	if (type & TFS_IS_OTHER) {
	    Seln_holder     holder;
	    holder = seln_inquire(SELN_SECONDARY);
	    if (holder.state != SELN_NONE)
		(void) seln_ask(&holder,
				SELN_REQ_YIELD, 0,
				NULL);
	} else {
	    textsw_set_selection(VIEW_REP_TO_ABS(textsw->first_view),
				 ES_INFINITY, ES_INFINITY, type);
	}
    }
}

static unsigned
holder_flag_from_seln_rank(rank)
    register Seln_rank rank;
{
    switch (rank) {
      case SELN_CARET:
	return (TXTSW_HOLDER_OF_CARET);
      case SELN_PRIMARY:
	return (TXTSW_HOLDER_OF_PSEL);
      case SELN_SECONDARY:
	return (TXTSW_HOLDER_OF_SSEL);
      case SELN_SHELF:
	return (TXTSW_HOLDER_OF_SHELF);
      case SELN_UNSPECIFIED:
	return (0);
      default:
	LINT_IGNORE(ASSUME(0));
	return (0);
    }
}

static unsigned
ev_sel_type_from_seln_rank(rank)
    register Seln_rank rank;
{
    switch (rank) {
      case SELN_CARET:
	return (EV_SEL_CARET);
      case SELN_PRIMARY:
	return (EV_SEL_PRIMARY);
      case SELN_SECONDARY:
	return (EV_SEL_SECONDARY);
      case SELN_SHELF:
	return (EV_SEL_SHELF);
      default:
	LINT_IGNORE(ASSUME(0));
	return (0);
    }
}

static unsigned
holder_flag_from_textsw_info(type)
    register int type;
{
    if (type & EV_SEL_CARET)
	return (TXTSW_HOLDER_OF_CARET);
    if (type & EV_SEL_PRIMARY)
	return (TXTSW_HOLDER_OF_PSEL);
    if (type & EV_SEL_SECONDARY)
	return (TXTSW_HOLDER_OF_SSEL);
    if (type & EV_SEL_SHELF)
	return (TXTSW_HOLDER_OF_SHELF);
    return (0);
}

Pkg_private     Seln_rank
seln_rank_from_textsw_info(type)
    register int type;
{
    if (type & EV_SEL_CARET)
	return (SELN_CARET);
    if (type & EV_SEL_PRIMARY)
	return (SELN_PRIMARY);
    if (type & EV_SEL_SECONDARY)
	return (SELN_SECONDARY);
    if (type & EV_SEL_SHELF)
	return (SELN_SHELF);
    return (SELN_UNKNOWN);
}

/*
 * The following acquires the selection of the specified rank unless the
 * textsw already holds it.
 */
Pkg_private     Seln_rank
textsw_acquire_seln(textsw, rank)
    register Textsw_folio textsw;
    register Seln_rank rank;
{
    unsigned        holder_flag;

    if (textsw_should_ask_seln_svc(textsw)) {
	if ((textsw->holder_state & holder_flag_from_seln_rank(rank))
	    == 0) {
	    rank = seln_acquire(textsw->selection_client, rank);
	}
    } else if (rank == SELN_UNSPECIFIED) {
#ifdef WAS_X11
	textsw->holder_state |= TXTSW_HOLDER_OF_ALL;
#endif
	return (SELN_UNKNOWN);
	/* Don't fall through as that will bump time-of-death */
    }
    holder_flag = holder_flag_from_seln_rank(rank);
    if (holder_flag) {
	textsw->holder_state |= holder_flag;
    } else {
	/*
	 * Assume svc temporarily dead, but make sure timer is set before
	 * calling textsw_post_error because it will result in a recursive
	 * call on this routine via textsw_may_win_exit if the timer is
	 * clear.
	 */
#ifdef WAS_X11
	gettimeofday(&textsw->selection_died, (struct timezone *) 0);
	textsw_post_error((Textsw_opaque) textsw, 0, 0,
	    XV_MSG("Cannot contact Selection Service - pressing\
 on but selections across windows may not work."), 0);
	textsw->holder_state |= TXTSW_HOLDER_OF_ALL;
#endif
    }
    return (rank);
}

/*
 * ==========================================================
 * 
 * Routines to support the use of selections.
 * 
 * ==========================================================
 */

#define	TXTSW_NEED_SELN_CLIENT	(Seln_client)1

static int
  textsw_should_ask_seln_svc(register Textsw_folio textsw)
{
    pkg_private void textsw_seln_svc_function();
    pkg_private Seln_result textsw_seln_svc_reply();

    if (textsw->state & TXTSW_DELAY_SEL_INQUIRE) {
	textsw->state &= ~TXTSW_DELAY_SEL_INQUIRE;
	return (textsw_sync_with_seln_svc(textsw));
    }
    if (textsw->selection_client == 0) {
	fprintf(stderr, 
		XV_MSG("textsw selection_client is null\n"));
	return (FALSE);
    }
#ifdef WAS_X11
    if (timerisset(&textsw->selection_died)) {
	if (!retry_okay)
	    return (FALSE);
	/* Retry if contact lost at least 60 seconds ago */
	gettimeofday(&now, (struct timezone *) 0);
	if (now.tv_sec < textsw->selection_died.tv_sec + 60)
	    return (FALSE);
	/*
	 * textsw_post_error will eventually call textsw_may_win_exit which
	 * will call textsw_should_ask_seln_svc (with retry_okay set to
	 * FALSE), but we must make sure that the timer is still set on that
	 * recursive call, so don't clear it until AFTER textsw_post_error
	 * returns!
	 */
	textsw_post_error((Textsw_opaque) textsw, 0, 0,
		XV_MSG("textsw: attempting to re-contact Selection Service\
 - selections may be temporarily incorrect."), 0);
	timerclear(&textsw->selection_died);
    }
#endif
    if (textsw->selection_client == TXTSW_NEED_SELN_CLIENT) {
	/*
	 * laf seln_use_test_service();
	 */
	/* Try to establish the initial connection with Seln. Svc. */
	textsw->selection_client = seln_create(
			    textsw_seln_svc_function, textsw_seln_svc_reply,
					  (char *)FOLIO_REP_TO_ABS(textsw));
	if (textsw->selection_client == 0) {
	    /* Set up to retry later and then tell user we lost. */
#ifdef WAS_X11
	    textsw->selection_client = TXTSW_NEED_SELN_CLIENT;
	    gettimeofday(&textsw->selection_died, (struct timezone *) 0);
	    textsw_post_error((Textsw_opaque) textsw, 0, 0,
		XV_MSG("textsw: cannot initiate contact with Selection\
 Service; pressing on and will retry later"), 0);
#endif
	    fprintf(stderr, 
	        XV_MSG("seln_client returned null"));
	    abort();
	}
    }
    return (TRUE);
}

Pkg_private unsigned
textsw_determine_selection_type(textsw, acquire)
    register Textsw_folio textsw;
    int             acquire;
{
    register unsigned result;
    register int    ask_svc =
	textsw_should_ask_seln_svc(textsw);
    register Seln_rank rank;
    Seln_holder     holder;

    if (textsw->holder_state & TXTSW_HOLDER_OF_SSEL) {
	if ((textsw->holder_state & TXTSW_HOLDER_OF_ALL) ==
	    TXTSW_HOLDER_OF_ALL) {
	    ask_svc = 0;
#ifdef notdef
	    BUG             ALERT ! Following or its equivalent must be re - instated when
	                    SunView 2 supports click - to - type.
	} else {
	    Firm_event      focus;
	    int             shift;
	    if              (!win_get_focus_event(WIN_FD_FOR_VIEW(textsw->first_view),
					                  &focus, &shift)) {
		ask_svc = (focus.id != LOC_WINENTER);
	    }
#endif
	}
	if              (!ask_svc)
	                    result = (textsw->track_state & TXTSW_TRACK_SECONDARY)
	    ?               EV_SEL_SECONDARY : EV_SEL_PRIMARY;
    }
#ifdef DEBUG_SVC
    fprintf(stderr, "Holders: %d%s", textsw->holder_state & TXTSW_HOLDER_OF_ALL,
	    (ask_svc) ? "\t" : "\n");
#endif
    if (ask_svc) {
	if (acquire == 0) {
	    holder = seln_inquire(SELN_UNSPECIFIED);
	    result = (holder.rank == SELN_SECONDARY)
		? EV_SEL_SECONDARY : EV_SEL_PRIMARY;
	} else {
	    rank = textsw_acquire_seln(textsw, SELN_UNSPECIFIED);
#ifdef DEBUG_SVC
	    fprintf(stderr, "textsw_acquire_seln returned rank = %d\n", (int) rank);
#endif
	    switch (rank) {
	      case SELN_PRIMARY:
		result = EV_SEL_PRIMARY;
		textsw->track_state &= ~TXTSW_TRACK_SECONDARY;
		if (!EV_MARK_IS_NULL(&textsw->save_insert)) {
		    ev_remove_finger(&textsw->views->fingers,
				     textsw->save_insert);
		    EV_INIT_MARK(textsw->save_insert);
		}
		break;
	      case SELN_SECONDARY:
		result = EV_SEL_SECONDARY;
		textsw->track_state |= TXTSW_TRACK_SECONDARY;
		ASSUME(EV_MARK_IS_NULL(&textsw->save_insert));
		EV_MARK_SET_MOVE_AT_INSERT(textsw->save_insert);
		ev_add_finger(&textsw->views->fingers,
		     EV_GET_INSERT(textsw->views), 0, &textsw->save_insert);
		if (textsw->func_state) {
		} else {
		    textsw->func_state |=
			TXTSW_FUNC_SVC_SAW(TXTSW_FUNC_GET);
		}
#ifdef WAS_X11
		if (seln_get_function_state(SELN_FN_DELETE)) {
		    /* Act as if it was local DELETE down */
		    textsw->func_state |=
			TXTSW_FUNC_DELETE |
			TXTSW_FUNC_SVC_SAW(TXTSW_FUNC_DELETE);
		    if (!TXTSW_IS_READ_ONLY(textsw))
			textsw->state |= TXTSW_PENDING_DELETE;
		} else if (seln_get_function_state(SELN_FN_PUT)) {
		    /* Act as if it was local PUT down */
		    textsw_begin_put(textsw->first_view, 0);
		    textsw->func_state |=
			TXTSW_FUNC_SVC_SAW(TXTSW_FUNC_PUT);
		}
#endif
		break;
	      default:		/* Assume svc temporarily dead */
		result = EV_SEL_PRIMARY;
		break;
	    }
	}
    } else if (textsw->selection_client == 0) {
	textsw->holder_state |= TXTSW_HOLDER_OF_ALL;
    }
    if (textsw->state & TXTSW_PENDING_DELETE) {
	if (textsw->track_state & TXTSW_TRACK_SECONDARY) {
	    if (textsw->track_state & TXTSW_TRACK_QUICK_MOVE)
		result |= EV_SEL_PD_SECONDARY;
	} else
	    result |= EV_SEL_PD_PRIMARY;
    }
    return (result);
}

/*
 * For function keys that deal with selections, the following cases must be
 * considered as plausible: No primary selection (See below for subcases)
 * Primary selection in window A (our window) FN key DOWN in window A No
 * secondary selection FN key UP in window A FN key UP in window B (some
 * other window) Secondary selection in window A FN key UP in window A FN key
 * UP in window B Secondary selection in window B FN key UP in window A FN
 * key UP in window B FN key DOWN in window B No secondary selection FN key
 * UP in window A FN key UP in window B Secondary selection in window A FN
 * key UP in window A FN key UP in window B Secondary selection in window B
 * FN key UP in window A FN key UP in window B Primary selection in window B
 * (See above for subcases)
 * 
 * On the FN key UP, the cases can be viewed as: DOWN in same window  => inform
 * service iff informed it about DOWN DOWN in other window => inform other
 * window via service
 * 
 * Window that is to process FN key (assumed to be holder of Primary selection)
 * has three UP cases to deal with: UP in this window, service not informed
 * => process directly/locally, any Secondary selection is local UP in this
 * window, service informed (by this window) || UP in other window =>
 * Secondary selection can be: local		=> process directly remote
 * => process indirectly non-existent	=> Primary selection can be: local
 * > process directly remote		=> process indirectly non-existent
 * => no-op
 */

Pkg_private void
textsw_give_shelf_to_svc(folio)
    register Textsw_folio folio;
{
    Pkg_private Es_handle es_file_create();
    register Es_handle output;
    Es_status       create_status, copy_status;
#ifdef OW_I18N 
    static CHAR	    *shelf_name_wc = 0;
#endif

    if (folio->trash == ES_NULL)
	return;
    if (!textsw_should_ask_seln_svc(folio))
	return;
    /* Write the contents to a file */
#ifdef OW_I18N
    if (!shelf_name_wc)	/* only first time */
	shelf_name_wc = _xv_mbstowcsdup(shelf_name);

    output = es_file_create(shelf_name_wc, ES_OPT_APPEND, &create_status);
#else
    output = es_file_create(shelf_name, ES_OPT_APPEND, &create_status);
#endif
    /* If create failed due to permission problem, retry */
    if ((output == 0) && (create_status == ES_CHECK_ERRNO) &&
	(errno == EACCES)) {
	unlink(shelf_name);
#ifdef OW_I18N
	output = es_file_create(shelf_name_wc, ES_OPT_APPEND, &create_status);
#else
	output = es_file_create(shelf_name, ES_OPT_APPEND, &create_status);
#endif
    }
    if (output) {
	copy_status = es_copy(folio->trash, output, FALSE);
	if (copy_status == ES_SUCCESS) {
	    seln_hold_file(SELN_SHELF, shelf_name);
	    folio->holder_state &= ~TXTSW_HOLDER_OF_SHELF;
	}
	es_destroy(output);
    }
    /* Now destroy the trashbin */
    if (folio->trash) {
	es_destroy(folio->trash);
	folio->trash = ES_NULL;
    }
}

Pkg_private int
textsw_sync_with_seln_svc(folio)
    register Textsw_folio folio;
{
    Seln_holders_all holders;
    int             result;

    if (result = textsw_should_ask_seln_svc(folio)) {
	holders = seln_inquire_all();
	if (SAME_HOLDER(folio, (caddr_t) & holders.caret)) {
	    folio->holder_state |= TXTSW_HOLDER_OF_CARET;
	} else {
	    folio->holder_state &= ~TXTSW_HOLDER_OF_CARET;
	}
	if (SAME_HOLDER(folio, (caddr_t) & holders.primary)) {
	    folio->holder_state |= TXTSW_HOLDER_OF_PSEL;
	} else {
	    folio->holder_state &= ~TXTSW_HOLDER_OF_PSEL;
	}
	if (SAME_HOLDER(folio, (caddr_t) & holders.secondary)) {
	    folio->holder_state |= TXTSW_HOLDER_OF_SSEL;
	} else {
	    folio->holder_state &= ~TXTSW_HOLDER_OF_SSEL;
	}
	if (SAME_HOLDER(folio, (caddr_t) & holders.shelf)) {
	    folio->holder_state |= TXTSW_HOLDER_OF_SHELF;
	} else {
	    folio->holder_state &= ~TXTSW_HOLDER_OF_SHELF;
	}
    } else {
	folio->holder_state |= TXTSW_HOLDER_OF_ALL;
    }
    return (result);
}

/*
 * Returns TRUE iff function is completely local to textsw, and all
 * selections are guaranteed to be within the textsw. The caller should
 * ignore the return value when is_down == TRUE.
 */
Pkg_private int
textsw_inform_seln_svc(textsw, function, is_down)
    register Textsw_folio textsw;
    register int    function;
    int             is_down;
{
#ifdef notdef
#define	ALL_SELECTIONS_ARE_LOCAL(textsw)	\
	(((textsw)->holder_state & TXTSW_HOLDER_OF_ALL) == TXTSW_HOLDER_OF_ALL)
#endif
#define	ALL_SELECTIONS_ARE_LOCAL(textsw)	\
	(FALSE)

    register Seln_function seln_function;

    if (!textsw_should_ask_seln_svc(textsw)) {
	return (TRUE);
    }
    switch (function) {
      case TXTSW_FUNC_DELETE:
	seln_function = SELN_FN_DELETE;
	break;
      case TXTSW_FUNC_FIND:
	seln_function = SELN_FN_FIND;
	break;
      case TXTSW_FUNC_GET:
	seln_function = SELN_FN_GET;
	break;
      case TXTSW_FUNC_PUT:
	seln_function = SELN_FN_PUT;
	break;
      case TXTSW_FUNC_FIELD:
	/* Seln. Svc. does not (yet) have the concept of FIELD search. */
      default:
	return (TRUE);
    }
#ifdef DEBUG_SVC
    fprintf(stderr, "Holders: %d	Func: %d(%s)	",
	    textsw->holder_state & TXTSW_HOLDER_OF_ALL, (int) function,
	    (is_down) ? "v" : "^");
#endif
    if (is_down) {
	if (!ALL_SELECTIONS_ARE_LOCAL(textsw)) {
	    textsw_acquire_seln(textsw, SELN_CARET);
	    textsw->selection_func =
		seln_inform(textsw->selection_client,
			    seln_function, is_down);
	    /*
	     * The Seln. Svc. resets the Secondary Selection to be
	     * non-existent on any inform, so make our state agree!
	     */
	    textsw->holder_state &= ~TXTSW_HOLDER_OF_SSEL;
	    ASSUME(textsw->selection_func.function == SELN_FN_ERROR);
	    textsw->func_state |= TXTSW_FUNC_SVC_SAW(function);
	}
	return (TRUE);
    } else if (textsw->func_state & TXTSW_FUNC_SVC_REQUEST) {
#ifdef DEBUG_SVC
	fprintf(stderr, "Request from service\n");
#endif
	if (textsw->selection_holder == 0 ||
	    textsw->selection_holder->state == SELN_NONE ||
	    ALL_SELECTIONS_ARE_LOCAL(textsw)) {
	    return (TRUE);
	}
	goto Service_Request;
    } else {
	if (ALL_SELECTIONS_ARE_LOCAL(textsw) &&
	    (textsw->func_state & function) &&
	    ((textsw->func_state & TXTSW_FUNC_SVC_SAW_ALL) == 0)) {
	    /*
	     * Don't tell svc about UP iff we also had DOWN and did not tell
	     * svc about it.  Test for local selections insures that we would
	     * have seen the DOWN.
	     */
	    return (TRUE);
	} else {

	    textsw->selection_func =
		seln_inform(textsw->selection_client,
			    seln_function, is_down);
#ifdef DEBUG_SVC
	    fprintf(stderr, "%d\n", textsw->selection_func.function);
#endif

	    if (textsw->selection_func.function != SELN_FN_ERROR) {
		pkg_private Seln_response textsw_setup_function();
		if (SELN_IGNORE !=
		    textsw_setup_function(textsw,
					  &textsw->selection_func))
		    goto Service_Request;
	    }
	    textsw->func_state &= ~TXTSW_FUNC_EXECUTE;
	    return (FALSE);
	}
    }
Service_Request:
#ifdef DEBUG_SVC
    fflush(stderr);
#endif
    switch (function) {
      case TXTSW_FUNC_GET:
      case TXTSW_FUNC_PUT:
	return ((textsw->selection_holder == 0) ||
		SAME_HOLDER(textsw,
			    (caddr_t) textsw->selection_holder));
      case TXTSW_FUNC_DELETE:
	/*
	 * Following is required to make DELETEs initiated from another
	 * window work.
	 */
	if ((textsw->holder_state & TXTSW_HOLDER_OF_CARET) == 0)
	    textsw->func_state |= TXTSW_FUNC_DELETE;
	/* Fall through */
      default:
	return (TRUE);
    }
#undef  ALL_SELECTIONS_ARE_LOCAL
}

Pkg_private int
textsw_abort(textsw)
    register Textsw_folio textsw;
{
    if (textsw_should_ask_seln_svc(textsw) &&
	(textsw->func_state & TXTSW_FUNC_SVC_SAW_ALL)) {
	seln_clear_functions();
    }
    if (textsw->track_state & TXTSW_TRACK_SECONDARY) {
	textsw_set_selection(VIEW_REP_TO_ABS(textsw->first_view),
			     ES_INFINITY, ES_INFINITY, EV_SEL_SECONDARY);
    }
    if (textsw->track_state & TXTSW_TRACK_MOVE)
	textsw_clear_move(VIEW_FROM_FOLIO_OR_VIEW(textsw));
    if (textsw->track_state & TXTSW_TRACK_DUPLICATE)
	textsw_clear_duplicate(VIEW_FROM_FOLIO_OR_VIEW(textsw));
    textsw_clear_pending_func_state(textsw);
    textsw->state &= ~TXTSW_PENDING_DELETE;
    textsw->track_state &= ~TXTSW_TRACK_ALL;
}

#ifdef DEBUG_SVC
static long     inform_service = 0x7FFFFFFF;
#endif
Pkg_private void
textsw_may_win_exit(textsw)
    Textsw_folio    textsw;
{
#define	PENDING_EVENT(textsw, func)				\
	(((textsw)->func_state & func) &&			\
	 (((textsw)->func_state & TXTSW_FUNC_SVC_SAW(func)) == 0))

    textsw_flush_caches(textsw->first_view, TFC_STD);
#ifdef DEBUG_SVC
    if (inform_service-- <= 0)
	return;
#endif
    if (textsw->state & TXTSW_DELAY_SEL_INQUIRE)
	return;			/* Textsw does not know about the state
				 * selection svc */


    /*
     * If selection service already got the request, then it should know
     * about the state.
     */
    if (((textsw->func_state & TXTSW_FUNC_SVC_REQUEST) == 0) &&
	textsw_should_ask_seln_svc(textsw)) {
	unsigned        holder_state;
	Es_index        first, last_plus_one;

	holder_state = textsw->holder_state & TXTSW_HOLDER_OF_ALL;
	(void) ev_get_selection(textsw->views, &first, &last_plus_one,
				EV_SEL_SECONDARY);
	/* Following makes sure cache actually tells the svc. */
	textsw->holder_state &= ~TXTSW_HOLDER_OF_ALL;
	if (PENDING_EVENT(textsw, TXTSW_FUNC_DELETE))
	    (void) textsw_inform_seln_svc(textsw, TXTSW_FUNC_DELETE, TRUE);
	if (PENDING_EVENT(textsw, TXTSW_FUNC_FIND))
	    (void) textsw_inform_seln_svc(textsw, TXTSW_FUNC_FIND, TRUE);
	if (PENDING_EVENT(textsw, TXTSW_FUNC_GET))
	    (void) textsw_inform_seln_svc(textsw, TXTSW_FUNC_GET, TRUE);
	if (PENDING_EVENT(textsw, TXTSW_FUNC_PUT))
	    (void) textsw_inform_seln_svc(textsw, TXTSW_FUNC_PUT, TRUE);
	/*
	 * Restore the cache's state, and sync Seln. Svc. - sticky part is we
	 * had not told Svc. about key down and may be part way into 2nd-ary
	 * Seln., which we also have not told Svc. about.
	 */
	textsw->holder_state |= holder_state;
	textsw->holder_state &= ~TXTSW_HOLDER_OF_SSEL;
	if (first < last_plus_one) {
	    textsw_acquire_seln(textsw, SELN_SECONDARY);
	}
    }
#undef	PENDING_EVENT
}

/*
 * Returns new value for buf_max_len. Can alter to_read and buf as
 * side_effects.
 */
Pkg_private int
textsw_prepare_buf_for_es_read(to_read, buf, buf_max_len, fixed_size)
    register int   *to_read;
    register CHAR **buf;
    register int    buf_max_len;
    int             fixed_size;
{
    if (*to_read > buf_max_len) {
	if (fixed_size) {
	    *to_read = buf_max_len;
	} else {
	    free(*buf);
	    buf_max_len = *to_read + 1;
	    *buf = MALLOC((u_int) buf_max_len);
	}
    }
    return (buf_max_len);
}

/*
 * Returns the actual number of entities read into the buffer. Caller is
 * responsible for making sure that the buffer is large enough.
 */
Pkg_private int
textsw_es_read(esh, buf, first, last_plus_one)
    register Es_handle esh;
    register CHAR  *buf;
    Es_index        first;
    register Es_index last_plus_one;
{
    register int    result;
    int             count;
    register Es_index current, next;

    result = 0;
    current = first;
    (void) es_set_position(esh, current);
    while (last_plus_one > current) {
	next = es_read(esh, last_plus_one - current, buf + result, &count);
	if READ_AT_EOF
	    (current, next, count)
		break;
	result += count;
	current = next;
    }
    return (result);
}

typedef struct tsfh_object {
    Textsw_view_handle view;
    Textsw_selection_handle selection;
    Seln_attribute  continued_attr;
    unsigned        flags;
    int             fill_result;
}               Tsfh_object;
typedef Tsfh_object *Tsfh_handle;

Pkg_private int
textsw_fill_selection_from_reply(context, reply)
    register Tsfh_handle context;
    register Seln_request *reply;
{
    unsigned int    result = 0;
    register Attr_attribute *attr;
    int             got_contents_ascii = FALSE, to_read;
    register        Textsw_selection_handle
                    selection = context->selection;

    if (context->continued_attr != SELN_REQ_END_REQUEST) {
	return (TFS_ERROR);
    }
    for (attr = (Attr_attribute *) reply->data; *attr;
	 attr = attr_next(attr)) {
	switch ((Seln_attribute) (*attr)) {
	  case SELN_REQ_BYTESIZE:
	    selection->first = 0;
	    selection->last_plus_one = (Es_index) attr[1];
	    break;
	  case SELN_REQ_FIRST:
	    selection->first = (Es_index) attr[1];
	    break;
	  case SELN_REQ_LAST:
	    selection->last_plus_one = 1 + (Es_index) attr[1];
	    break;
#ifdef OW_I18N
	  case SELN_REQ_CHARSIZE:
	    selection->first = 0;
	    selection->last_plus_one = (Es_index) attr[1];
	    break;    
	  case SELN_REQ_CONTENTS_ASCII: {
	    int		    temp_len = 0;
	    static char	   *extra_ptr;
	    int		    extra_bytes = 0;
	    char	   *temp_ptr = (char *) (attr + 1);

	    /* if the SELN_REQ_BYTESIZE hasn't been requested (because it
	       could have failed) then use the strlen() of the 
	       SELN_REQ_CONTENTS_ASCII for the length
	       */
	    if ((selection->first==ES_INFINITY) &&
		(selection->last_plus_one==ES_INFINITY)) {
		temp_len=strlen(temp_ptr);
		/* don't set the values unless there really is a string
		   because we want to preserve any previous values. */
		if (temp_len) {
		    selection->first = 0;
		    selection->last_plus_one = temp_len;
		}
	    } else {
		temp_len = selection->last_plus_one - selection->first;
	    }
	    
	    if (reply->status == SELN_CONTINUED) {
		context->continued_attr = (Seln_attribute) (*attr);
		to_read = temp_len = (strlen(temp_ptr));
		if (extra_ptr) { /* Prepend the unconverted char from last call */   
		    extra_bytes = strlen(extra_ptr);
		    to_read += extra_bytes;
		    temp_ptr = malloc(to_read + 1);
		    XV_BCOPY(extra_ptr, temp_ptr, extra_bytes);
		    XV_BCOPY((char *) (attr + 1), temp_ptr+extra_bytes, temp_len);
		    temp_ptr[to_read] = NULL;		    
		    temp_len = to_read;	    
		}		
	    } else {
		to_read = temp_len;
		if (extra_ptr) {
		    /* Clear out all the uncoverted bytes in the last pass */
		    xv_free(extra_ptr);
		    extra_ptr = NULL;
		}
	    }
	    selection->buf_max_len =
		textsw_prepare_buf_for_es_read(&to_read,
				    &selection->buf, selection->buf_max_len,
					       !selection->buf_is_dynamic);
	    if (to_read != temp_len) {
	       /* BIG BUG: Error case, need to put the (temp_len - to_read)
	        * number of char back.
	        */
	    }			       
	    /*
	     * Clients must be aware that either due to continuation or
	     * because the buffer was not big enough,
	     * selection->last_plus_one may be greater than selection->first
	     * + to_read
	     */
	    temp_len = to_read;
	    selection->buf_len = textsw_mbstowcs(selection->buf,
						 temp_ptr, &temp_len);
	    if (extra_ptr) {
	        xv_free(extra_ptr);
	        extra_ptr = NULL;
	    }
	    if (temp_len < to_read) {
	        /*  Some bytes are not converted */
	        
	        extra_ptr = STRDUP((char *)(temp_ptr + temp_len));
	    }
	    
	    if (extra_bytes != 0) {
	       /*  We just allocated temp_ptr in this pass */
	       xv_free(temp_ptr);	       
	    }
	        
	    got_contents_ascii = TRUE;
	    if (reply->status == SELN_CONTINUED)
		goto Return;
	    break;
	    }
	  case SELN_REQ_CONTENTS_WCS:
#else /* OW_I18N */
	  case SELN_REQ_CONTENTS_ASCII:
#endif /* OW_I18N */
	    /* if the SELN_REQ_BYTESIZE or CHARSIZE hasn't been requested
	       (because it could have failed) then use the strlen() of the 
	       SELN_REQ_CONTENTS_ASCII for the length
	       */
	    if ((selection->first==ES_INFINITY) &&
		(selection->last_plus_one==ES_INFINITY)) {
		to_read=STRLEN((CHAR *)(attr + 1));
		/* don't set the values unless there really is a string
		   because we want to preserve any previous values. */
		if (to_read) {
		    selection->first = 0;
		    selection->last_plus_one = to_read;
		}
	    }
	    if (reply->status == SELN_CONTINUED) {
		context->continued_attr = (Seln_attribute) (*attr);
		to_read = STRLEN((CHAR *) (attr + 1));
	    } else {
		to_read = selection->last_plus_one - selection->first;
	    }
	    selection->buf_max_len =
		textsw_prepare_buf_for_es_read(&to_read,
				    &selection->buf, selection->buf_max_len,
					       !selection->buf_is_dynamic);
	    selection->buf_len = to_read;
	    /*
	     * Clients must be aware that either due to continuation or
	     * because the buffer was not big enough,
	     * selection->last_plus_one may be greater than selection->first
	     * + to_read
	     */
	    BCOPY((CHAR *) (attr + 1), selection->buf, to_read);
	    got_contents_ascii = TRUE;
	    if (reply->status == SELN_CONTINUED)
		goto Return;
	    break;
	  case SELN_REQ_UNKNOWN:
	    result |= TFS_BAD_ATTR_WARNING;
	    break;
	  default:
	    if ((attr == (Attr_attribute *) reply->data) ||
		((context->flags & TFS_FILL_IF_OTHER) &&
		 (got_contents_ascii == FALSE)))
		goto Seln_Error;
	    result |= TFS_BAD_ATTR_WARNING;
	    break;
	}
    }
Return:
    return (result);

Seln_Error:
    return (TFS_SELN_SVC_ERROR);
}

static          Seln_result
only_one_buffer(request)
    Seln_request   *request;
{
    Tsfh_handle     context = (Tsfh_handle)
    request->requester.context;

    if (request->status == SELN_CONTINUED) {
	context->fill_result = TFS_ERROR;
	return (SELN_FAILED);
    } else {
	context->fill_result =
	    textsw_fill_selection_from_reply(context, request);
	return (SELN_SUCCESS);
    }
}

Pkg_private int
textsw_selection_from_holder(textsw, selection, holder, type, flags)
    register Textsw_folio textsw;
    register Textsw_selection_handle selection;
    Seln_holder    *holder;
    int		    type;
    int		    flags;

{
    Pkg_private int      ev_get_selection();
    unsigned        mode;
    register caddr_t *req_attr;
    int             result = type, to_read;
    caddr_t         req_for_ascii[3];
    Seln_result     query_result;
    Tsfh_object     context;

    if (holder) {
	if (holder->state == SELN_NONE)
	    goto Seln_Error;
	if (SAME_HOLDER(textsw, (caddr_t) holder)) {
	    textsw->holder_state |= holder_flag_from_seln_rank(holder->rank);
	    result = type = ev_sel_type_from_seln_rank(holder->rank);
	    if ((type == EV_SEL_PRIMARY) || (type == EV_SEL_SECONDARY))
		goto Get_Local;
	}
	/*
	 * Make the request to the selection holder
	 */
	if (selection->per_buffer) {
	    context.view = textsw->first_view;
	    context.selection = selection;
	    context.continued_attr = SELN_REQ_END_REQUEST;
	    context.flags = flags;
	} else {
	    LINT_IGNORE(ASSERT(0));
	    goto Seln_Error;
	}
	req_attr = req_for_ascii;
#ifdef OW_I18N
 	/* Test if it is a Sundae client */
	query_result =
	    seln_query(holder, selection->per_buffer, (caddr_t) & context,
	               SELN_REQ_CHARSIZE, ES_INFINITY,
	               NULL);
	if ((query_result != SELN_SUCCESS) ||
			(context.fill_result & TFS_BAD_ATTR_WARNING)) {
	    if (flags & TFS_FILL_IF_OTHER) {
	        *req_attr++ = (caddr_t) SELN_REQ_CONTENTS_ASCII;
	        *req_attr++ = (caddr_t) 0;	/* Null value */
	    }
	    *req_attr++ = (caddr_t) 0;	/* Null terminate request list */
	    query_result =
	        seln_query(holder, selection->per_buffer, (caddr_t) & context,
	               SELN_REQ_BYTESIZE, ES_INFINITY,
		       ATTR_LIST, req_for_ascii,
		       NULL);
	} else {
	    if (flags & TFS_FILL_IF_OTHER) {
	        *req_attr++ = (caddr_t) SELN_REQ_CONTENTS_WCS;
	        *req_attr++ = (caddr_t) 0;	/* Null value */
	    }
	    *req_attr++ = (caddr_t) 0;	/* Null terminate request list */
	    query_result =
	        seln_query(holder, selection->per_buffer, (caddr_t) & context,
	               SELN_REQ_CHARSIZE, ES_INFINITY,
		       ATTR_LIST, req_for_ascii,
		       NULL);
	}
#else /* OW_I18N */
	if (flags & TFS_FILL_IF_OTHER) {
	    *req_attr++ = (caddr_t) SELN_REQ_CONTENTS_ASCII;
	    *req_attr++ = (caddr_t) 0;	/* Null value */
	}
	*req_attr++ = (caddr_t) 0;	/* Null terminate request list */
	query_result =
	    seln_query(holder, selection->per_buffer, (caddr_t) & context,
		       SELN_REQ_BYTESIZE, ES_INFINITY,
		       SELN_REQ_FIRST, ES_INFINITY,
		       SELN_REQ_LAST, ES_INFINITY,
		       ATTR_LIST, req_for_ascii,
		       NULL);
#endif /* OW_I18N */
	if (query_result != SELN_SUCCESS)
	    goto Seln_Error;
	return (TFS_IS_ERROR(context.fill_result)
		? context.fill_result
		: (context.fill_result | result | TFS_IS_OTHER));
    } else {
Get_Local:
	mode = ev_get_selection(textsw->views, &selection->first,
				&selection->last_plus_one, type);
	result |= mode;
	if (selection->first >= selection->last_plus_one) {
	    return (TFS_ERROR);
	}
	if (flags & TFS_FILL_IF_SELF) {
	    to_read = selection->last_plus_one - selection->first;
	    selection->buf_max_len =
		textsw_prepare_buf_for_es_read(&to_read, &selection->buf,
			selection->buf_max_len, !selection->buf_is_dynamic);
	    selection->last_plus_one = selection->first + to_read;
	    selection->buf_len =
		textsw_es_read(
			       (type & EV_SEL_SHELF)
			       ? textsw->trash : textsw->views->esh,
			       selection->buf,
			       selection->first, selection->last_plus_one);
	}
	return (result | TFS_IS_SELF);
    }
Seln_Error:
    return (TFS_SELN_SVC_ERROR);
}

Pkg_private int
textsw_func_selection_internal(textsw, selection, type, flags)
    register Textsw_folio textsw;
    register Textsw_selection_handle selection;
    int		    type;
    int		    flags;

{
    Seln_holder     holder;
    int             result;

    if (((EV_SEL_BASE_TYPE(type) == EV_SEL_PRIMARY) ||
	 (EV_SEL_BASE_TYPE(type) == EV_SEL_SECONDARY)) &&
	(textsw->holder_state & holder_flag_from_textsw_info(type))) {
	result = textsw_selection_from_holder(
			 textsw, selection, (Seln_holder *) 0, type, flags);
    } else if (textsw_should_ask_seln_svc(textsw)) {
	if (textsw->selection_holder) {
	    holder = *textsw->selection_holder;
	} else {
	    holder = seln_inquire(seln_rank_from_textsw_info(type));
	}
	result = textsw_selection_from_holder(
				   textsw, selection, &holder, type, flags);
    } else {
	result = TFS_SELN_SVC_ERROR;
    }
    return (result);
}

Pkg_private int
textsw_func_selection(textsw, selection, flags)
    register Textsw_folio textsw;
    register Textsw_selection_handle selection;
    int        flags;

{
    int             result;

    if (textsw->selection_holder) {
	result = textsw_selection_from_holder(
				textsw, selection, textsw->selection_holder,
					      0, flags);
    } else {
	result = textsw_func_selection_internal(
				textsw, selection, EV_SEL_SECONDARY, flags);
	if (TFS_IS_ERROR(result)) {
	    result = textsw_func_selection_internal(
				  textsw, selection, EV_SEL_PRIMARY, flags);
	}
    }
    selection->type = result;
    return (result);
}

/* ARGSUSED */
Pkg_private void
textsw_init_selection_object(
			textsw, selection, buf, buf_max_len, buf_is_dynamic)
    Textsw_folio    textsw;	/* Currently unused */
    register Textsw_selection_handle selection;
    CHAR           *buf;
    int             buf_max_len;
    int             buf_is_dynamic;
{
    selection->type = 0;
    selection->first = selection->last_plus_one = ES_INFINITY;
    selection->per_buffer = only_one_buffer;
    if (buf) {
	selection->buf = buf;
	selection->buf_max_len = buf_max_len - 1;
	selection->buf_is_dynamic = buf_is_dynamic;
    } else {
	selection->buf = MALLOC(SELN_BUFSIZE + 1);
	selection->buf_max_len = SELN_BUFSIZE;
	selection->buf_is_dynamic = TRUE;
    }
    selection->buf_len = 0;
}

Pkg_private int
textsw_is_seln_nonzero(textsw, type)
    register Textsw_folio textsw;
    int        type;
{
    Textsw_selection_object selection;
    int             result;
#ifdef OW_I18N
    CHAR	    dummy[1];
    
    dummy[0] = NULL;
    textsw_init_selection_object(textsw, &selection, dummy, 1, FALSE);
#else
    textsw_init_selection_object(textsw, &selection, "", 1, FALSE);
#endif    
    result =
	textsw_func_selection_internal(textsw, &selection, type, 0);
    if (TFS_IS_ERROR(result) ||
	(selection.first >= selection.last_plus_one)) {
	return (0);
    } else {
	return ((result & TFS_IS_SELF) ? 2 : 1);
    }
}

#ifdef OW_I18N

static          Seln_result
textsw_stuff_all_buffers(request)
    register Seln_request *request;
{
    register Tsfh_handle context = (Tsfh_handle)
    request->requester.context;
    int             status;
    static char	*extra_ptr;

    if (context->continued_attr == SELN_REQ_END_REQUEST) {
        if (extra_ptr) { /* clean out all unconverted bytes in the last pass*/
            xv_free(extra_ptr);
            extra_ptr = NULL;           
        }
	context->fill_result =
	    textsw_fill_selection_from_reply(context, request);
	if (TFS_IS_ERROR(context->fill_result)) {
	    return (SELN_FAILED);
	} else {
	    status = textsw_input_partial(context->view,
					  context->selection->buf,
					  context->selection->buf_len);
	    return status;
	}
    } else if ((request->status == SELN_CONTINUED) || 
              (request->status == SELN_SUCCESS)) {
        CHAR		*buf;
        int		len;
        
        if (context->continued_attr == SELN_REQ_CONTENTS_ASCII) {
            int		to_read, temp_len;
            
	    int		extra_bytes = 0; 
	    char	*temp_ptr = (char *) (request->data);	              
            register Textsw_selection_handle
                    selection = context->selection;
                    
            to_read = temp_len = (strlen(temp_ptr));
            if (extra_ptr) {   /* Prepend the unconverted char from last call */
                    extra_bytes = strlen(extra_ptr);
		    to_read += extra_bytes;
		    temp_ptr = malloc(to_read + 1);
		    XV_BCOPY(extra_ptr, temp_ptr, extra_bytes);
		    XV_BCOPY((char *) (request->data), temp_ptr+extra_bytes, temp_len);
		    temp_ptr[to_read] = NULL;
		    temp_len = to_read;               
            }
            
            selection->buf_max_len =
		textsw_prepare_buf_for_es_read(&to_read,
				    &selection->buf, 
				    selection->buf_max_len,
				    !selection->buf_is_dynamic);
	    if (to_read != temp_len) {
	       /* Error case, need to put the (temp_len - to_read)
	        * number of char back.
	        */
	    }			       
	    temp_len = to_read;
	    /*
	     * Clients must be aware that either due to continuation or
	     * because the buffer was not big enough,
	     * selection->last_plus_one may be greater than selection->first
	     * + to_read
	     */
	    selection->buf_len = textsw_mbstowcs(selection->buf,
						 temp_ptr, &temp_len);
	    if (extra_ptr) {
	        xv_free(extra_ptr);
	        extra_ptr = NULL;
	        
	    }
	    if (temp_len < to_read) {
	        /*  Some bytes are not converted */        
	        extra_ptr = STRDUP((char *)(temp_ptr + temp_len));
	    }
	    
	    if (extra_bytes != 0) {
	       /*  We just allocated temp_ptr in this pass */
	       xv_free(temp_ptr);
	    }
	    buf = selection->buf;
	    len = selection->buf_len;
	    				    
        } else  {
           buf = (CHAR *)request->data; 
	   len = STRLEN((CHAR *)request->data);  
	}
	status = textsw_input_partial(context->view,
				      buf, (long int)len);
	return status;
    }  else {
	context->fill_result = TFS_SELN_SVC_ERROR;
	return (SELN_FAILED);
    }
}

#else /* OW_I18N */

static          Seln_result
textsw_stuff_all_buffers(request)
    register Seln_request *request;
{
    register Tsfh_handle context = (Tsfh_handle)
    request->requester.context;
    int             status;

    if (context->continued_attr == SELN_REQ_END_REQUEST) {
	context->fill_result =
	    textsw_fill_selection_from_reply(context, request);
	if (TFS_IS_ERROR(context->fill_result)) {
	    return (SELN_FAILED);
	} else {
	    status = textsw_input_partial(context->view,
					  context->selection->buf,
					  context->selection->buf_len);
	    return status;
	}
    } else if (request->status == SELN_CONTINUED) {
	status = textsw_input_partial(context->view,
				      request->data,
				      strlen((char *) (request->data)));
	return status;
    } else if (request->status == SELN_SUCCESS) {
	status = textsw_input_partial(context->view,
				      request->data,
				      strlen((char *) (request->data)));
	return status;
    } else {
	context->fill_result = TFS_SELN_SVC_ERROR;
	return (SELN_FAILED);
    }
}

#endif /* OW_I18N */

Pkg_private int
textsw_stuff_selection(view, type)
    Textsw_view_handle view;
    int        type;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    Textsw_selection_object selection;
    int             result;
    int             record;
    Es_index        delta;
    Es_index        old_insert_pos, old_length;

    textsw_init_selection_object(folio, &selection, NULL, 0, 0);
    selection.per_buffer = textsw_stuff_all_buffers;
    textsw_input_before(view, &old_insert_pos, &old_length);
    result = textsw_func_selection_internal(
				folio, &selection, type, TFS_FILL_IF_OTHER);
    if ((TFS_IS_ERROR(result) != 0) ||
	(selection.first >= selection.last_plus_one))
	goto Done;
    if (result & TFS_IS_SELF) {
	/* Selection is local, so copy pieces, not contents. */
	extern Es_handle textsw_esh_for_span();
	Es_handle       pieces;
	pieces = textsw_esh_for_span(view, selection.first,
				     selection.last_plus_one, ES_NULL);
	(void) textsw_insert_pieces(view, old_insert_pos, pieces);
    } else {
	record = (TXTSW_DO_AGAIN(folio) &&
		  ((folio->func_state & TXTSW_FUNC_AGAIN) == 0));
	delta = textsw_input_after(view, old_insert_pos, old_length,
				   record);
	if AN_ERROR
	    (delta == ES_CANNOT_SET) {
	    }
    }
Done:
    free(selection.buf);
    return (result);
}

/*
 * ==========================================================
 * 
 * Routines to support the use of the selection service.
 * 
 * ==========================================================
 */

Pkg_private int
textsw_seln_svc_had_secondary(textsw)
    register Textsw_folio textsw;
{
    return ((textsw->selection_func.function != SELN_FN_ERROR) &&
	    (textsw->selection_func.secondary.access.pid != 0) &&
	    (textsw->selection_func.secondary.state != SELN_NONE));
}

Pkg_private     Seln_response
textsw_setup_function(folio, function)
    register Textsw_folio folio;
    register Seln_function_buffer *function;
{
    register Seln_response response;

    response = seln_figure_response(function, &folio->selection_holder);
#ifdef DEBUG_SVC
    fprintf(stderr, "Response = %d\n", response);
#endif
    switch (response) {
      case SELN_IGNORE:
	/* Make sure we don't cause a yield to secondary. */
	function->function = SELN_FN_ERROR;
	return (response);
      case SELN_DELETE:
      case SELN_SHELVE:
	/* holder describes shelf, not sel., so ignore it */
	folio->selection_holder = (Seln_holder *) 0;
	/* Fall through */
      case SELN_FIND:
      case SELN_REQUEST:
	break;
      default:
	LINT_IGNORE(ASSUME(0));
	/* Make sure we don't cause a yield to secondary. */
	function->function = SELN_FN_ERROR;
	return (SELN_IGNORE);
    }
    /*
     * The holder information must be treated with special care, as the
     * following can otherwise happen: 1) Primary holder (or listener) gets
     * function request 2) Primary gets secondary, then requests secondary to
     * yield 3) Secondary yields, and its state is now correct 4)
     * (Ex)secondary gets same function request, and based on the (now out of
     * date) information, notes that it is the current secondary holder (and
     * its state is now incorrect). Although the above is a secondary
     * selection state problem, it also applies to the other holders, as each
     * holder gets a copy of the function request (multiple copies if a
     * single client is multiple holders).
     * 
     * The golden rule is that only the "driver" can trust the holder
     * information!
     */
    if (SAME_HOLDER(folio, (caddr_t) & function->caret))
	folio->holder_state |= TXTSW_HOLDER_OF_CARET;
    else
	folio->holder_state &= ~TXTSW_HOLDER_OF_CARET;
    if (SAME_HOLDER(folio, (caddr_t) & function->primary))
	folio->holder_state |= TXTSW_HOLDER_OF_PSEL;
    else
	folio->holder_state &= ~TXTSW_HOLDER_OF_PSEL;
    if (SAME_HOLDER(folio, (caddr_t) & function->secondary))
	folio->holder_state |= TXTSW_HOLDER_OF_SSEL;
    else
	folio->holder_state &= ~TXTSW_HOLDER_OF_SSEL;
    if (SAME_HOLDER(folio, (caddr_t) & function->shelf))
	folio->holder_state |= TXTSW_HOLDER_OF_SHELF;
    else
	folio->holder_state &= ~TXTSW_HOLDER_OF_SHELF;
    textsw_take_down_caret(folio);
    return (response);
}

Pkg_private void
textsw_end_selection_function(folio)
    register Textsw_folio folio;
{
    folio->selection_holder = (Seln_holder *) 0;
    if (folio->selection_func.function != SELN_FN_ERROR) {
	if (textsw_seln_svc_had_secondary(folio) &&
	    ((folio->holder_state & TXTSW_HOLDER_OF_SSEL) == 0)) {
	    (void) seln_ask(&folio->selection_func.secondary,
			    SELN_REQ_YIELD, 0,
			    NULL);
	}
	folio->selection_func.function = SELN_FN_ERROR;
    }
}

Pkg_private void
textsw_seln_svc_function(first_textsw_public, function)
    Textsw          first_textsw_public;
    register Seln_function_buffer *function;
{
    Seln_response   response;
    Textsw_view_handle first_view = VIEW_ABS_TO_REP(first_textsw_public);
    register Textsw_folio textsw = FOLIO_FOR_VIEW(first_view);
    register int    result = 0;
    pkg_private int textsw_end_quick_move();

    response = textsw_setup_function(textsw, function);
    if (!textsw->func_view) {
	textsw->func_view = first_view;
	textsw->func_x = textsw->func_y = 0;
    }
    textsw->selection_func = *function;
    textsw->func_state &= ~TXTSW_FUNC_SVC_SAW_ALL;
    textsw->func_state |= (TXTSW_FUNC_EXECUTE | TXTSW_FUNC_SVC_REQUEST);
    switch (response) {
      case SELN_REQUEST:
	switch (function->function) {
	  case SELN_FN_GET:
	    textsw->func_state |= TXTSW_FUNC_GET;
	    result |= textsw_end_get(textsw->func_view);
	    (void) textsw_set_cursor(FOLIO_REP_TO_ABS(textsw),
				     CURSOR_BASIC_PTR);
	    break;
	  case SELN_FN_PUT:
	    textsw->func_state |= TXTSW_FUNC_PUT;
	    result = textsw_end_put(textsw->func_view);
	    break;
	}
	break;
      case SELN_SHELVE:
	textsw->func_state |= TXTSW_FUNC_PUT;
	result = textsw_end_put(textsw->func_view);
	break;
      case SELN_FIND:
	textsw->func_state |= TXTSW_FUNC_FIND;
	textsw_end_find(textsw->func_view,
			textsw->func_x, textsw->func_y);
	break;
      case SELN_DELETE:
	if (textsw->track_state & TXTSW_TRACK_QUICK_MOVE) {
	    textsw->track_state &= ~TXTSW_TRACK_QUICK_MOVE;
	    (void) textsw_set_cursor(FOLIO_REP_TO_ABS(textsw),
				     CURSOR_BASIC_PTR);

	}
	break;

      default:
	goto Done;
    }
    if (result & TEXTSW_PE_READ_ONLY)
	textsw_read_only_msg((Textsw_folio) textsw->func_view,
			     textsw->func_x, textsw->func_y);
Done:
    textsw_clear_pending_func_state(textsw);
    textsw->func_state &= ~TXTSW_FUNC_SVC_ALL;
    textsw->func_view = (Textsw_view_handle) 0;
    textsw->state &= ~TXTSW_PENDING_DELETE;
    textsw->track_state &= ~TXTSW_TRACK_ALL;
    textsw_end_selection_function(textsw);
}

#define	TCAR_CONTINUED	0x40000000
#define	TCARF_ESH	0
#define	TCARF_STRING	1

static int
#ifdef OW_I18N
textsw_copy_wcs_reply(first, last_plus_one, response,
#else
textsw_copy_ascii_reply(first, last_plus_one, response,
#endif /* OW_I18N */
			max_length, flags, data)
    Es_index        first, last_plus_one;
    register caddr_t *response;
    int             max_length;		/* OW_I18N : This is in character */
    int	            flags;
    caddr_t         data;
{
    register CHAR  *dest = (CHAR *) response;
    int             count, continued;

    if (continued = (max_length < (last_plus_one - first))) {
	switch (flags) {
	  case TCARF_ESH:
	    count = textsw_es_read((Es_handle) data, dest,
				   first, first + max_length);
	    break;
	  case TCARF_STRING:
	    count = max_length;
	    BCOPY(((CHAR *) data) + first, dest, count);
	    break;
	  default:
	    LINT_IGNORE(ASSUME(0));
	    count = 0;
	    break;
	}
    } else {
	count = last_plus_one - first;
	if (count) {
	    switch (flags) {
	      case TCARF_ESH:
		count = textsw_es_read((Es_handle) data, dest,
				       first, last_plus_one);
		break;
	      case TCARF_STRING:
		BCOPY(((CHAR *) data) + first, dest, count);
		break;
	      default:
		LINT_IGNORE(ASSUME(0));
		break;
	    }
	}
	/*
	 * Null terminate string (and round count up.  If count is already
	 * rounded, then the null that terminates the value doubles as the
	 * string terminator.)
	 */
#ifdef OW_I18N
	while (((count * sizeof(CHAR)) % sizeof(*response)) != 0)
#else
	while ((count % sizeof(*response)) != 0)
#endif
	    dest[count++] = '\0';
    }
    return ((continued) ? count + TCAR_CONTINUED : count);
}

char          * 
textsw_base_name(full_name)
    char           *full_name;
{
    register char  *temp;

    if ((temp = XV_RINDEX(full_name, '/')) == NULL)
	return (full_name);
    else
	return (temp + 1);
}

Pkg_private     Es_index
textsw_check_valid_range(esh, first, ptr_last_plus_one)
    register Es_handle esh;
    register Es_index first;
    Es_index       *ptr_last_plus_one;	/* Can be NULL */
{
    register Es_index first_valid = first;
    int             count_read;
    CHAR            buf[200];	/* Must be bigger than wrap msg */

    if (first != ES_INFINITY &&
	((long) es_get(esh, ES_PS_SCRATCH_MAX_LEN) !=
	 ES_INFINITY)) {
	(void) es_set_position(esh, first);
	first_valid = es_read(esh, SIZEOF(buf) - 1, buf, &count_read);
	if (first + count_read == first_valid) {
	    first_valid = first;
	} else {
	    /* Hit hole, use es_read result, but check last_plus_one */
	    if (ptr_last_plus_one &&
		*ptr_last_plus_one < first_valid) {
		*ptr_last_plus_one = first_valid;
	    }
	}
    }
    return (first_valid);
}

#ifdef OW_I18N
static	Es_index
textsw_get_char_in_multibyte (folio, first, last_plus_one, buffer, buffer_size,
			      char_count, flags, data)
    Textsw_folio    folio;			      
    Es_index        first, last_plus_one;
    char	   *buffer;
    int		    buffer_size;	/* size in byte */
    int		   *char_count;
    int	            flags;
    caddr_t         data;
{
#define	    TEMP_BUF_SIZE	2048

    Es_handle       esh_to_use = (Es_handle)data;
    CHAR	    temp_wc[TEMP_BUF_SIZE + 1];
    CHAR	    *temp_wc_ptr = temp_wc;
    Es_index	    num_of_byte;
    int		    continued;
    int		    temp_num_char;

    if (buffer == NULL)
        return(0);
    
    /* 
     * temp_num_char is actual number of characters that can be put into buffer.
     * In non single byte locale, the byte size has to be divided by sizeof(CHAR)
     * to get the number of characters. 
     */
    temp_num_char = folio->locale_is_ale ? (buffer_size / (int)sizeof(CHAR)) : buffer_size;    
    if (temp_num_char > TEMP_BUF_SIZE) {
        temp_wc_ptr = MALLOC(temp_num_char + 1);
        
        if (temp_wc_ptr == NULL)
            return(0);
    }
    *char_count = textsw_copy_wcs_reply(first, last_plus_one, temp_wc_ptr,
			temp_num_char, flags, (caddr_t) esh_to_use);
			
    if (continued = (*char_count >= TCAR_CONTINUED))
	*char_count -= TCAR_CONTINUED;
	
    *(temp_wc_ptr+ *char_count) = NULL;

    num_of_byte = wcstombs(buffer, temp_wc_ptr, buffer_size);

    if (temp_num_char > TEMP_BUF_SIZE) {
       xv_free(temp_wc_ptr);
    }
    return((buffer && continued) ? (num_of_byte + TCAR_CONTINUED) : num_of_byte);
#undef 	 LARGE_BUF_SIZE
}


#endif /* OW_I18N */

Pkg_private     Seln_result
textsw_seln_svc_reply(attr, context, max_length)
    Seln_attribute  attr;
    register Seln_replier_data *context;
    int             max_length;
{
    register Textsw_folio textsw = FOLIO_ABS_TO_REP(context->client_data);
    register Continuation cont_data;
    register Seln_result result = SELN_SUCCESS;

    if (context->context) {
	cont_data = (Continuation) context->context;
    } else if (attr == SELN_REQ_END_REQUEST) {
	/*
	 * Don't set up state after having already flushed it due to previous
	 * error.
	 */
	cont_data = (Continuation) 0;
    } else {
	unsigned        holder_flag =
	holder_flag_from_seln_rank(context->rank);
	/* First attribute: set up for this set of replies. */
	if (fast_continuation.in_use) {
	    cont_data = NEW(Continuation_object);
	} else {
	    cont_data = &fast_continuation;
	}
	cont_data->in_use = TRUE;
	context->context = (char *) cont_data;
	if ((textsw->holder_state & holder_flag) == 0) {
	    result = SELN_DIDNT_HAVE;
	    goto Return;
	}
	switch (context->rank) {
	  case SELN_CARET:
	    cont_data->type = EV_SEL_CARET;
	    cont_data->first = cont_data->last_plus_one = ES_INFINITY;
	    break;
	  case SELN_PRIMARY:
	    cont_data->type = EV_SEL_PRIMARY;
	    goto Get_Info;
	  case SELN_SECONDARY:
	    cont_data->type = EV_SEL_SECONDARY;
    Get_Info:
	    cont_data->span_level = textsw->span_level;
	    ev_get_selection(textsw->views, &cont_data->first,
			     &cont_data->last_plus_one,
			     (unsigned) cont_data->type);
	    break;
	  case SELN_SHELF:
	    cont_data->type = EV_SEL_SHELF;
	    if (textsw->trash) {
		cont_data->first = es_set_position(textsw->trash, 0);
		cont_data->last_plus_one = es_get_length(textsw->trash);
	    } else {
		cont_data->first = cont_data->last_plus_one = ES_INFINITY;
	    }
	    break;
	  default:
	    result = SELN_FAILED;
	    goto Return;
	}
	cont_data->current = ES_INFINITY;
	/* Check for, and handle, wrap-around edit log */
	cont_data->first = textsw_check_valid_range(
					   (cont_data->type == EV_SEL_SHELF)
				       ? textsw->trash : textsw->views->esh,
						    cont_data->first,
						 &cont_data->last_plus_one);
#ifdef OW_I18N
	cont_data->first_mb = cont_data->last_plus_one_mb = 0;
#endif						 				
    }
    switch (attr) {
      case SELN_REQ_BYTESIZE:
#ifdef OW_I18N
        cont_data->first_mb = textsw_mbpos_from_wcpos(textsw, cont_data->first);
        cont_data->last_plus_one_mb = textsw_mbpos_from_wcpos(textsw, cont_data->last_plus_one);            
	*context->response_pointer++ =
	    (caddr_t) (cont_data->last_plus_one_mb - cont_data->first_mb);
	goto Return;
      case SELN_REQ_CHARSIZE:
#endif /* OW_I18N */
	*context->response_pointer++ =
	    (caddr_t) (cont_data->last_plus_one - cont_data->first);
	goto Return;

      case SELN_REQ_CONTENTS_ASCII: {
#ifdef OW_I18N
      	    int             continued;
	    int             char_count;
	    Es_index        current_first;
	    Es_handle       esh_to_use;
	    char   	   *dest;
	    int		    byte_count;
	    	
	    esh_to_use = (cont_data->type == EV_SEL_SHELF)
		? textsw->trash : textsw->views->esh;
	    current_first = (cont_data->current == ES_INFINITY)
		? cont_data->first : cont_data->current;
	    byte_count = textsw_get_char_in_multibyte(textsw, current_first, 
	    			    cont_data->last_plus_one, 
	    			    (char *)context->response_pointer, 
	    			    max_length,
	    			    &char_count, 
	    			    TCARF_ESH, (caddr_t) esh_to_use);
	    
	    if (continued = (byte_count >= TCAR_CONTINUED))
		byte_count -= TCAR_CONTINUED;
	    dest = (char  *) context->response_pointer;
	    while ((byte_count % sizeof(context->response_pointer)) != 0) {
	        dest[byte_count++] = '\0';
	    }
	    context->response_pointer +=
		(byte_count / sizeof(*context->response_pointer));
	    if (continued) {
	        cont_data->current = current_first + char_count;  
		result = SELN_CONTINUED;
	    } else {
		*context->response_pointer++ = 0; /* Null terminate value */
		cont_data->current = ES_INFINITY;
	    }
	    goto Return;
      }
      case SELN_REQ_CONTENTS_WCS:{
#endif /* OW_I18N */
	    int             continued, count;
	    Es_index        current_first;
	    Es_handle       esh_to_use;
	    esh_to_use = (cont_data->type == EV_SEL_SHELF)
		? textsw->trash : textsw->views->esh;
	    current_first = (cont_data->current == ES_INFINITY)
		? cont_data->first : cont_data->current;
#ifdef OW_I18N
	    count = textsw_copy_wcs_reply(
			   current_first, cont_data->last_plus_one,
			   context->response_pointer, max_length / sizeof(CHAR),
			   TCARF_ESH, (caddr_t) esh_to_use);
#else
	    count = textsw_copy_ascii_reply(
			    current_first, cont_data->last_plus_one,
			    context->response_pointer, max_length, TCARF_ESH,
			    (caddr_t) esh_to_use);
#endif /* OW_I18N */
	    if (continued = (count >= TCAR_CONTINUED))
		count -= TCAR_CONTINUED;
	    context->response_pointer +=
#ifdef OW_I18N
		((count * sizeof(CHAR)) / sizeof(*context->response_pointer));
#else
		(count / sizeof(*context->response_pointer));
#endif
	    if (continued) {
		cont_data->current = current_first + count;
		result = SELN_CONTINUED;
	    } else {
		*context->response_pointer++ = 0; /* Null terminate value */
		cont_data->current = ES_INFINITY;
	    }
	    goto Return;
	}
      case SELN_REQ_COMMIT_PENDING_DELETE:{
	    textsw_take_down_caret(textsw);
	    if (textsw_do_pending_delete(textsw->first_view,
					 EV_SEL_SECONDARY, 0))
		(void) textsw_possibly_edited_now_notify(textsw);
	    goto Return;
	}
      case SELN_REQ_DELETE:{
	    textsw_take_down_caret(textsw);
	    if (textsw_delete_span(textsw->first_view,
				 cont_data->first, cont_data->last_plus_one,
				   TXTSW_DS_ADJUST | TXTSW_DS_SHELVE))
		(void) textsw_possibly_edited_now_notify(textsw);
	    goto Return;
	}
      case SELN_REQ_FAKE_LEVEL:{
	    Seln_level      target_level = (Seln_level) * context->request_pointer;

	    switch (target_level) {
	      case TEXTSW_UNIT_IS_LINE:
		cont_data->span_level = EI_SPAN_LINE;
		ev_span(textsw->views, cont_data->first, &cont_data->first,
			&cont_data->last_plus_one, EI_SPAN_LINE);
		*context->response_pointer++ = (caddr_t) target_level;
		break;
	      default:
		LINT_IGNORE(ASSUME(0));	/* Let implementor look at this. */
		*context->response_pointer++ = (caddr_t) (0xFFFFFFFF);
		break;
	    }
	    goto Return;
	}
      case SELN_REQ_FILE_NAME:{
	    CHAR           *name;
	    int             continued, count;
	    int             str_first, str_last_plus_one;

	    if (context->rank != SELN_PRIMARY) {
	       result = SELN_UNRECOGNIZED;
	       goto Return;
	    }

	    if (textsw_file_name(textsw, &name) != 0) {
		*context->response_pointer++ = 0; /* Null terminate value */
		goto Return;
	    }
	    /*
	     * For dbxtool: return full name, rather than base name ... name
	     * = textsw_base_name(name);
	     */
	    str_first = (cont_data->current == ES_INFINITY)
		? 0 : cont_data->current;
	    str_last_plus_one = STRLEN(name);
#ifdef OW_I18N
	    count = textsw_copy_wcs_reply(str_first, str_last_plus_one,
				context->response_pointer,
				max_length / sizeof(CHAR), TCARF_STRING, name);
#else
	    count = textsw_copy_ascii_reply(str_first, str_last_plus_one,
					    context->response_pointer,
					    max_length, TCARF_STRING, name);
#endif /* OW_I18N */
	    if (continued = (count >= TCAR_CONTINUED))
		count -= TCAR_CONTINUED;
	    context->response_pointer +=
#ifdef OW_I18N
		((count * sizeof(CHAR)) / sizeof(*context->response_pointer));
#else
		(count / sizeof(*context->response_pointer));
#endif
	    *context->response_pointer++ = 0;	/* Null terminate value */
	    if (continued) {
		cont_data->current = str_first + count;
		result = SELN_CONTINUED;
	    } else {
		cont_data->current = ES_INFINITY;
	    }
	    goto Return;
	}
      case SELN_REQ_FIRST:
#ifdef OW_I18N

        cont_data->first_mb = textsw_mbpos_from_wcpos(textsw, cont_data->first);  
        *context->response_pointer++ = (caddr_t) cont_data->first_mb;
	goto Return;
      case SELN_REQ_FIRST_WC:
#endif /* OW_I18N */
	*context->response_pointer++ = (caddr_t) cont_data->first;
	goto Return;
      case SELN_REQ_FIRST_UNIT:
	switch (cont_data->span_level) {
	  case EI_SPAN_LINE:{
		int             line_number;
		if (cont_data->first == 0)
		    line_number = 0;
		else
		    line_number = ev_newlines_in_esh(textsw->views->esh,
						     0, cont_data->first);
		*context->response_pointer++ = (caddr_t) (long) line_number;
		break;
	    }
	  case EI_SPAN_CHAR:
	    /* Fall through */
	  default:		/* BUG ALERT: if in doubt, use char units. */
	    *context->response_pointer++ = (caddr_t) cont_data->first;
	}
	goto Return;
      case SELN_REQ_FUNC_KEY_STATE:{
	    int             func_key_state = 0;

	    if (textsw->func_state & TXTSW_FUNC_DELETE) {
		func_key_state |= (int) SELN_FN_DELETE;
	    }
	    if (textsw->func_state & TXTSW_FUNC_GET) {
		func_key_state |= (int) SELN_FN_GET;
	    }
	    if (textsw->func_state & TXTSW_FUNC_PUT) {
		func_key_state |= (int) SELN_FN_PUT;
	    }
	    *context->response_pointer++ =
		(caddr_t) (long) func_key_state;
	    goto Return;
	}
      case SELN_REQ_IS_READONLY:
	*context->response_pointer++ =
	    (caddr_t) TXTSW_IS_READ_ONLY(textsw);
	goto Return;

      case SELN_REQ_LAST:
#ifdef OW_I18N
        cont_data->last_plus_one_mb = textsw_mbpos_from_wcpos(textsw, cont_data->last_plus_one);              
        *context->response_pointer++ =
	    (caddr_t) (cont_data->last_plus_one_mb - 1);
	goto Return;
      case SELN_REQ_LAST_WC:
#endif /* OW_I18N */
	*context->response_pointer++ =
	    (caddr_t) (cont_data->last_plus_one - 1);
	goto Return;
      case SELN_REQ_LAST_UNIT:
	/* BUG ALERT: look at current level and adjust for it. */
	*context->response_pointer++ = (caddr_t) - 1;
	goto Return;
      case SELN_REQ_YIELD:{
	    result = textsw_seln_yield(textsw, context->rank);
	    *context->response_pointer++ = (caddr_t) result;
	    goto Return;
	}
      case SELN_REQ_END_REQUEST:
	result = SELN_FAILED;	/* Ignored by caller! */
	goto Return;
      default:
	result = SELN_UNRECOGNIZED;
	goto Return;
    }
Return:
    switch (result) {
      case SELN_SUCCESS:
      case SELN_CONTINUED:
      case SELN_UNRECOGNIZED:
	break;
      default:
	/* We are not going to get another chance, so clean up. */
	if (cont_data == &fast_continuation) {
	    cont_data->in_use = FALSE;
	} else {
	    free((caddr_t) cont_data);
	}
	context->context = NULL;/* Be paranoid! */
    }
    return (result);
}

static          Seln_result
textsw_seln_yield(folio, rank)
    register Textsw_folio folio;
    register Seln_rank rank;
{
    unsigned        holder_flag = holder_flag_from_seln_rank(rank);

    if (holder_flag) {
	switch (rank) {
	  case SELN_PRIMARY:
	    textsw_set_selection(FOLIO_REP_TO_ABS(folio),
				 ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
	    break;
	  case SELN_SECONDARY:
	    textsw_set_selection(FOLIO_REP_TO_ABS(folio),
				 ES_INFINITY, ES_INFINITY,
				 EV_SEL_SECONDARY);
	    folio->track_state &= ~TXTSW_TRACK_SECONDARY;
	    (void) textsw_set_cursor(FOLIO_REP_TO_ABS(folio),
				     CURSOR_BASIC_PTR);
	    if (!EV_MARK_IS_NULL(&folio->save_insert)) {
		ev_remove_finger(&folio->views->fingers,
				 folio->save_insert);
		EV_INIT_MARK(folio->save_insert);
	    }
	    break;
	  case SELN_SHELF:
	    if (folio->trash) {
		es_destroy(folio->trash);
		folio->trash = ES_NULL;
	    }
	    break;
	}
	folio->holder_state &= ~holder_flag;
	return (SELN_SUCCESS);
    } else {
	return (SELN_DIDNT_HAVE);
    }
}
