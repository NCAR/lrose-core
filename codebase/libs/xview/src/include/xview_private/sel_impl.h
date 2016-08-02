#ifndef lint
#ifdef SCCS
static char     sccsid[] = "@(#)sel_impl.h 1.10 91/03/01";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef sel_impl_defined
#define sel_impl_defined

#include <xview/sel_pkg.h>
#include <xview_private/portable.h>


#define SEL_PUBLIC(object)	XV_PUBLIC(object)
#define SEL_PRIVATE(object)	XV_PRIVATE(Sel_info, Xv_sel, object)

#define SEL_OWNER_PUBLIC(object) XV_PUBLIC(object)
#define SEL_OWNER_PRIVATE(object) \
    XV_PRIVATE(Sel_owner_info, Xv_sel_owner, object)

#define SEL_REQUESTOR_PUBLIC(object) XV_PUBLIC(object)
#define SEL_REQUESTOR_PRIVATE(object) \
    XV_PRIVATE(Sel_req_info, Xv_sel_requestor, object)

#define SEL_ITEM_PUBLIC(object)	XV_PUBLIC(object)
#define SEL_ITEM_PRIVATE(object) \
    XV_PRIVATE(Sel_item_info, Xv_sel_item, object)


#define SEL_ADD_CLIENT     0
#define SEL_DELETE_CLIENT  1



typedef struct sel_type_tbl {
    Atom	       type;	
    char	       *type_name;	
    int            status;
    Sel_prop_info  *propInfo;
} Sel_type_tbl;


/*
 * Selection object private data
 */
typedef struct sel_info {
    Selection	    public_self;  /* back pointer to public object */
    Atom	    rank;
    char	    *rank_name;
    struct timeval  time;
    u_int	    timeout;
    Display         *dpy;
} Sel_info;



/*
 * Selection_requestor object private data
 */
typedef struct sel_req_info {
    Selection_requestor	    public_self;  /* back pointer to public object */
    int		    nbr_types;	/* number of types and type names */
    void	    (*reply_proc)();
    Sel_type_tbl    *typeTbl;
    int             typeIndex;
} Sel_req_info;


/*
 * Selection_owner object private data
 */
typedef struct requestor {
    XID        requestor;
    Atom       property;
    Atom       target;
    Atom       type;
    int        format;
    char       *data;
    int        bytelength;
    int        offset;
    int        timeout;
    Time       time;
    int        incr;              /* reply in increments */
    int        numIncr;           /* number of incrs in a request */
    int        multiple;  
    void       (*reply_proc)();
    Atom       *incrPropList;
    struct sel_owner_info  *owner;
} Requestor;


typedef struct  sel_prop_list {
    Atom     prop;
    int      avail;
    struct  sel_prop_list *next;
} Sel_prop_list;


typedef struct  sel_atom_list {
    Atom         multiple;
    Atom         targets;
    Atom         timestamp;
    Atom         file_name;
    Atom         string;
    Atom         incr;
    Atom         integer;
#ifdef OW_I18N
    Atom	 ctext;
#endif /* OW_I18N */
} Sel_atom_list;


typedef struct {
    Atom  target;
    Atom  property;
} atom_pair;


typedef struct sel_owner_info {
    Selection_owner      public_self;  /* back pointer to public object */
    Bool	        (*convert_proc)();
    void	        (*done_proc)();
    void	        (*lose_proc)();
    Bool	        own;	/* True: acquired, False: lost */
    struct sel_item_info *first_item;
    struct sel_item_info *last_item;
    Display         *dpy;
    Time	    time;
    XID             xid;
    Atom            property;
    Atom            selection;
    int             status;
    Sel_atom_list   *atomList;
    Sel_prop_info   *propInfo;
    Sel_req_info    *req_info;
    Requestor       *req;
} Sel_owner_info;


/*
 * Selection_item object private data
 */
typedef struct sel_item_info {
    Selection_item  public_self;  /* back pointer to public object */
    Bool	    copy;	/* True: malloc & copy data */
    Xv_opaque	    data;
    int		    format;	/* data element size: 8, 16 or 32 bits */
    unsigned long   length;	/* nbr of elements in data */
    struct sel_item_info *next;
    struct sel_owner_info *owner;
    struct sel_item_info *previous;
    Atom	    type;
    char	   *type_name;
} Sel_item_info;


/*
 *  Reply data
 */
typedef struct {
    Window         requestor;
    Atom           *target;
    Atom           property;
    int            format;
    Xv_opaque      data;
    unsigned long  length;
    int            timeout;
    int            multiple;
    atom_pair      *atomPair;
    Time           time;
    int            status;
    int            incr;
    Sel_owner_info *seln;
    Sel_req_info   *req_info;
} Sel_reply_info;
    

typedef struct sel_req_tbl {
    int            done;
    Sel_reply_info   *reply;
    struct  sel_req_tbl   *next;
} Sel_req_tbl;


typedef struct sel_client_info {
    Sel_owner_info          *client;
    struct sel_client_info  *next;
} Sel_client_info;



Pkg_private int xv_sel_add_prop_notify_mask();
Pkg_private Atom xv_sel_get_property();
Pkg_private void xv_sel_free_property();
Pkg_private int xv_sel_predicate();
Pkg_private int xv_sel_check_property_event();
Pkg_private int xv_sel_handle_incr();
Pkg_private struct timeval *xv_sel_cvt_xtime_to_timeval();
Pkg_private Time xv_sel_cvt_timeval_to_xtime();
Pkg_private Sel_atom_list *xv_sel_find_atom_list();
Pkg_private Sel_prop_list *xv_sel_get_prop_list();
Pkg_private Sel_req_tbl *xv_sel_add_new_req();
Pkg_private Sel_req_tbl *xv_sel_set_reply();
Pkg_private Sel_reply_info *xv_sel_get_reply();
Pkg_private Sel_cmpat_info  *xv_sel_get_compat_data();

Pkg_private int SelOwnerIsLocal();
Pkg_private void xv_sel_send_old_pkg_sel_clear();
Pkg_private void xv_sel_free_compat_data();

Xv_private int  xv_seln_handle_req();
Xv_private void xv_sel_send_old_owner_sel_clear();
Xv_private void xv_sel_set_compat_data();

#if defined (__APPLE__) && defined(__DEFINE_SEL_IMPL_VARS)
XContext  selCtx = 0;
XContext  reqCtx = 0;
XContext  targetCtx = 0;
XContext  propCtx = 0;
XContext  replyCtx = 0;
XContext  cmpatCtx = 0;
#elif !defined(__linux) || defined(__DEFINE_SEL_IMPL_VARS)
XContext  selCtx;
XContext  reqCtx;
XContext  targetCtx;
XContext  propCtx;
XContext  replyCtx;
XContext  cmpatCtx;
#else
extern XContext  selCtx;
extern XContext  reqCtx;
extern XContext  targetCtx;
extern XContext  propCtx;
extern XContext  replyCtx;
extern XContext  cmpatCtx;
#endif

#endif /* sel_impl_defined */


