/*	@(#)xv_list.h 20.12 93/06/28 SMI	*/

#ifndef _xv_list_h_already_included
#define _xv_list_h_already_included

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/pkg.h>

#ifdef notdef

These data structures, types, and functions implement singly-linked lists
with first list node as list head.

The expected usage of the functions is:
  void xv_sl_init(head)
	Initialize the list that 'head' specifies.
  Xv_sl_link xv_sl_add_after(head, link, new)
	Add 'new' to list that 'head' specifies after 'link'.  If 'link' is
	null, make 'new' the first node (caller must update its notion of
	head).  Returns 'new' to allow call to be part of expression.
  Xv_sl_link xv_sl_remove_after(head, link)
	Remove link->next from list that 'head' specifies, unless 'link' is
	null in which case remove 'head' (caller must update its notion of
	head).  NOTE: Caller must do actual deallocation, so return value
	is removed node.
  Xv_sl_link xv_sl_remove(head, link)
	Similar to xv_sl_remove_after, but affects link, not link->next.

Sample code fragments:
	typedef struct foo {xv_sl_link next; ... } *Foo;
	
	Foo_info	a_foo, next_foo, head_foo = (Foo)malloc(struct foo);
	XV_SL_INIT(head_foo);
	... fill in head_foo and allocate and fill in a_foo,
	    then link a_foo after head_foo via ...
	XV_SL_ADD_AFTER(head_foo, head_foo, a_foo);
	... finish building list then enumerate via ...
	XV_SL_TYPED_FOR_ALL(head_foo, a_foo, Foo) { ... process a_foo ... }
	... and when done with list ...
	XV_SL_SAFE_TYPED_FOR_ALL(head_foo, a_foo, next_foo, Foo) {
	    free((char *)a_foo);
	}
#endif

struct xv_sl_node {
    struct xv_sl_node		*next;
};

typedef struct xv_sl_node	*Xv_sl_link;
typedef Xv_sl_link		 Xv_sl_head;
#define XV_SL_NULL		((Xv_sl_link)0)

#define XV_SL_FOR_ALL(_head, _this)				\
	for (_this = _head; _this; _this = (_this)->next)
#define XV_SL_SAFE_NEXT(_this)	(_this) ? (_this)->next : XV_SL_NULL
#define XV_SL_SAFE_FOR_ALL(_head, _this, _next)		\
	for (_this = _head, _next = XV_SL_SAFE_NEXT(_this); \
		 _this; _this = _next, _next = XV_SL_SAFE_NEXT(_this))

Xv_private void
xv_sl_init();

Xv_private Xv_sl_link
xv_sl_add_after();

Xv_private Xv_sl_link
xv_sl_remove_after();

Xv_private Xv_sl_link
xv_sl_remove();

/* Following assume xv_sl_link is aligned at start of embedding struct. */
#define XV_SL_TO_LINK(_formal)	((Xv_sl_link)(_formal))
#define XV_SL_TO_HEAD(_formal)	((Xv_sl_head)(_formal))
#define XV_SL_INIT(_head)	xv_sl_init(XV_SL_TO_HEAD(_head))
#define XV_SL_ADD_AFTER(_head, _link, _new)	\
	xv_sl_add_after(XV_SL_TO_HEAD(_head),	\
			XV_SL_TO_LINK(_link), XV_SL_TO_LINK(_new))
#define XV_SL_REMOVE_AFTER(_head, _link)		\
	xv_sl_remove_after(XV_SL_TO_HEAD(_head), XV_SL_TO_LINK(_link))
#define XV_SL_REMOVE(_head, _link)		\
	xv_sl_remove(XV_SL_TO_HEAD(_head), XV_SL_TO_LINK(_link))
#define XV_SL_TYPED_FOR_ALL(_head, _this, _type)			\
	for (_this = (_type)_head; _this; _this = (_type)_this->next)
#define XV_SL_SAFE_TYPED_FOR_ALL(_head, _this, _next, _type)		\
	for (_this = (_type)_head, _next = XV_SL_SAFE_NEXT(_this);	\
	     _this; _this = (_type)_next, _next = XV_SL_SAFE_NEXT(_this))

#endif /* _xv_list_h_already_included */
