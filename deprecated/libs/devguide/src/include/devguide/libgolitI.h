/*
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

/*
 * @(#)libgolitI.h	2.6 91/10/15 Copyright 1991 Sun Microsystems
 */
#ifndef	_libgolitI_h
#define _libgolitI_h

#include <libgolit.h>

/**************************
 *
 * GolitCallbackInfo
 *
 **************************/

typedef	struct	_golit_callback_info {
			String		callback_name;
			XtCallbackRec	callback_rec;
} GolitCallbackInfo, CallbackInfo, *CallbackInfoPtr, *GolitCallbackInfoPtr;

#define NullGolitCallbackInfoPtr		(GolitCallbackInfoPtr)NULL

#define	GolitCbICallbackName(cbi)		((cbi).callback_name)
#define	GolitCbIPtrCallbackName(cbi)		((cbi)->callback_name)

#define	GolitCbICallbackRec(cbi)		((cbi).callback_rec)
#define	GolitCbIPtrCallbackRec(cbi)		((cbi)->callback_rec)

#define	GolitCbICallbackRecProc(cbi)		((cbi).callback_rec.callback)
#define	GolitCbIPtrCallbackRecProc(cbi)		((cbi)->callback_rec.callback)

#define	GolitCbICallbackRecClosure(cbi)		((cbi).callback_rec.closure)
#define	GolitCbIPtrCallbackRecClosure(cbi)	((cbi)->callback_rec.closure)

#define	DefineCallbackInfo(cbn, cbp, clos)			\
				{ cbn, 				\
					{ (XtCallbackProc)cbp,  \
					  (XtPointer)clos }	\
},

#define	TerminateGCBI	{ (String)NULL, { (XtCallbackProc)NULL, NULL } },

#ifdef	__STDC__
#define	CallbackInfoListDesc(n)	n##_cb_info
#else
#define	CallbackInfoListDesc(n)	n/**/_cb_info
#endif

#define	BeginCallbackInfoList(n)	\
	static	GolitCallbackInfo	CallbackInfoListDesc(n)[] = {
#define	EndCallbackInfoList(n)		TerminateGCBI \
}

#define	DefineGolitCallbackInfoPtr(n)	static	GolitCallbackInfoPtr	n

#define	ExternGolitCallbackInfo(n)	\
	extern	GolitCallbackInfo	CallbackInfoListDesc(n)

#define	ExternGolitCallbackInfoPtr(n)	extern	GolitCallbackInfoPtr	n

	
/**************************
 *
 * GolitWidgetDesc
 *
 **************************/

#ifndef _VarargsI_h_
typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg;
#endif
#ifndef	_ResourceI_h
typedef	struct _XtTypedArg *XtTypedArgList;
#endif


#ifdef	__STDC__
#define	ArgListDesc(n)	n##_arg_list
#else
#define	ArgListDesc(n)	n/**/_arg_list
#endif

#define	NullArgList		(ArgList)NULL
#define	NullTypedArgList	(XtTypedArgList)NULL

#define	DefineArg(n , v)	{ (String)n, (XtArgVal)v },
#define	TerminateArgList	DefineArg(NULL, NULL)

#define	BeginArgList(n)	static	Arg	ArgListDesc(n)[] = {
#define	EndArgList(n)	TerminateArgList	\
}


#ifdef	__STDC__
#define	TypedArgListDesc(n)	n##_typed_arg_list
#else
#define	TypedArgListDesc(n)	n/**/_typed_arg_list
#endif

#define	DefineTypedArg(n, t, v, s) { (String)n , (String)t, (XtArgVal)v,  s },

#define	TerminateTypedArgList	DefineTypedArg( NULL, NULL, NULL, NULL)

#define	BeginTypedArgList(n)	static	XtTypedArg TypedArgListDesc(n)[] = {
#define	EndTypedArgList(n)		TerminateTypedArgList	\
}

#define	GolitVersion	1

typedef	Widget		(*_GolitWCProc) OL_ARGS ((String, ...));
typedef _GolitWCProc	*_GolitWCProcList;

#ifdef	__STDC__
#define	CreateProcListDesc(n)	n##_cp_list
#else
#define	CreateProcListDesc(n)	n/**/_cp_list
#endif

#define	NullCreateProcList	(_GolitWCProcList)NULL;

#define	DefineCreateProc(p)		(_GolitWCProc)(p),
#define	BeginCreateProcList(n)	static _GolitWCProc CreateProcListDesc(n)[] = {
#define	EndCreateProcList(n)	DefineCreateProc(NULL)	\
}

#define	BitsInCpIdx	4 /* power of 2 please .... */

typedef	struct	_golit_widget_desc {
		struct	{
			unsigned int	version:7;	/* back-end version */
			unsigned int	cp_idx:BitsInCpIdx;	
							/* create proc index */

			unsigned int	create_unmanaged:1;	
			unsigned int	is_shell:1;
			unsigned int	is_popup_shell:1;
			unsigned int	wd_is_dynamic:1;	/*reserved*/
			unsigned int	args_are_dynamic:1;	/*reserved*/
			unsigned int	targs_are_dynamic:1;	/*reserved*/
			unsigned int	cbs_are_dynamic:1;	/*reserved*/

			unsigned int	reserved:14;		/* see note */
		} flags;
	
		Widget			last_widget_created;	
		/* NOTE: these fields are all reserved for future expansion */

		GolitWidgetDescPtr	sibling;	/* next */

		GolitWidgetDescPtr	children;	/* null terminated */

		GolitWidgetDescPtr	popups;	 	/* null terminated */
		String			name;
		WidgetClass		*widget_class;

		ArgList			args;		/* null terminated */
		XtTypedArgList		typed_args;	/* null terminated */

		GolitCallbackInfoPtr	callbacks;	/* null terminated */

		_GolitWCProcList	create_procs;	/* null terminated */
} WidgetDesc, GolitWidgetDesc /* *WidgetDescPtr, *GolitWidgetDescPtr */ ;

#define	_DefineWidgetDesc(um,ish,isp,sib,child,pop,name,wc,args,targs,cbs,cps)\
{									\
	{ GolitVersion, 0, um, ish, isp, 0, 0, 0, 0, 0  },		\
	(Widget)NULL,							\
	sib,								\
	child,								\
	pop,								\
	name,								\
	&(wc),								\
	(args),								\
	(targs),							\
	(cbs),								\
	(_GolitWCProcList)(cps)						\
},

#define	DefineWidgetDesc(sib,child,pop,name,wc,args,targs,cbs,cps)	\
	_DefineWidgetDesc(0,0,0,sib,child,pop,name,wc,args,targs,cbs,cps)

#define	DefineShellWidgetDesc(sib,child,pop,name,wc,args,targs,cbs,cps)	\
	_DefineWidgetDesc(0,1,0,sib,child,pop,name,wc,args,targs,cbs,cps)

#define	DefinePopupWidgetDesc(sib,child,pop,name,wc,args,targs,cbs,cps)	\
	_DefineWidgetDesc(0,0,1,sib,child,pop,name,wc,args,targs,cbs,cps)

#ifdef	__STDC__
#define	WidgetDescDesc(n)	n##_desc
#else
#define	WidgetDescDesc(n)	n/**/_desc
#endif

#define	DefineWidgetDescPtr(n)	GolitWidgetDescPtr	n

#define	BeginWidgetDesc(n)  	GolitWidgetDesc	WidgetDescDesc(n)[] = {
#define	EndWidgetDesc(n)	};	\
			DefineWidgetDescPtr(n) = WidgetDescDesc(n)

#define	ExternWidgetDesc(n)	extern	GolitWidgetDesc	WidgetDescDesc(n)[]
#define	ExternWidgetDescPtr(n)	extern	GolitWidgetDescPtr	n


#define	NullGolitWidgetDescPtr		 (GolitWidgetDescPtr)NULL

#define	GolitWDVersion(wd)		 ((wd).flags.version)
#define	GolitWDPtrVersion(wdp)		 ((wdp)->flags.version)

#define	GolitWDCpIdx(wd)		 ((wd).flags.cp_idx)
#define	GolitWDPtrCpIdx(wdp)		 ((wdp)->flags.cp_idx)

#define	GolitWDCreateUnmanaged(wd)	 ((wd).flags.create_unmanaged)
#define	GolitWDPtrCreateUnmanaged(wdp)   ((wdp)->flags.create_unmanaged)

#define	GolitWDIsShell(wd)	 	 ((wd).flags.is_shell)
#define	GolitWDPtrIsShell(wdp)  	 ((wdp)->flags.is_shell)

#define	GolitWDIsPopupShell(wd)	 	 ((wd).flags.is_popup_shell)
#define	GolitWDPtrIsPopupShell(wdp)  	 ((wdp)->flags.is_popup_shell)

#define	GolitWDWDIsDynamic(wd)	 	 ((wd).flags.wd_is_dynamic)
#define	GolitWDPtrWDIsDynamic(wdp)  	 ((wdp)->flags.wd_is_dynamic)

#define	GolitWDArgsAreDynamic(wd)	 ((wd).flags.args_are_dynamic)
#define	GolitWDPtrArgsAreDynamic(wdp)  	 ((wdp)->flags.args_are_dynamic)

#define	GolitWDTArgsAreDynamic(wd)	 ((wd).flags.targs_are_dynamic)
#define	GolitWDPtrTArgsAreDynamic(wdp)	 ((wdp)->flags.s_popup_shell)

#define	GolitWDCBsAreDynamic(wd)	 ((wd).flags.cbs_are_dynamic)
#define	GolitWDPtrCBsAreDynamic(wdp)  	 ((wdp)->flags.cbs_are_dynamic)

#define	GolitWDLastWidgetCreated(wd)		 ((wd).last_widget_created)
#define	GolitWDPtrLastWidgetCreated(wdp)	 ((wdp)->last_widget_created)

#define	GolitWDSibling(wd)		 ((wd).sibling)
#define	GolitWDPtrSibling(wdp)		 ((wdp)->sibling)

#define	GolitWDChildren(wd)		 ((wd).children)
#define	GolitWDPtrChildren(wdp)		 ((wdp)->children)

#define	GolitWDPopups(wd)		 ((wd).popups)
#define	GolitWDPtrPopups(wdp)		 ((wdp)->popups)

#define	GolitWDName(wd)			 ((wd).name)
#define	GolitWDPtrName(wdp)		 ((wdp)->name)

#define	GolitWDWidgetClass(wd)		 ((wd).widget_class)
#define	GolitWDPtrWidgetClass(wdp)	 ((wdp)->widget_class)

#define	GolitWDArgs(wd)			 ((wd).args)
#define	GolitWDPtrArgs(wdp)		 ((wdp)->args)

#define	GolitWDTypedArgs(wd)		 ((wd).typed_args)
#define	GolitWDPtrTypedArgs(wdp)	 ((wdp)->typed_args)

#define	GolitWDCallbacks(wd)		 ((wd).callbacks)
#define	GolitWDPtrCallbacks(wdp)	 ((wdp)->callbacks)

#define	GolitWDCreateProcs(wd)		 (wd).create_procs
#define	GolitWDPtrCreateProcs(wdp)	 (wdp)->create_procs

#define	GolitWDShellCreateProcs(wd)	 (wd).create_procs
#define	GolitWDPtrShellCreateProcs(wdp)	 (wdp)->create_procs

#define	ForAllSiblings(cv, wdp)	/* includes itself */	\
		for (cv = wdp;				\
		     cv != (GolitWidgetDescPtr)NULL;	\
		     cv = GolitWDPtrSibling(cv))

#define	ForAllChildren(cv, wdp)				\
		for (cv = GolitWDPtrChildren(wdp);	\
		     cv != (GolitWidgetDescPtr)NULL;	\
		     cv = GolitWDPtrSibling(cv))

#define	ForAllPopups(cv, wdp)				\
		for (cv = GolitWDPtrPopups(wdp);	\
		     cv != (GolitWidgetDescPtr)NULL;	\
		     cv = GolitWDPtrSibling(cv))

#define	ForAllArgs(cv, wdp)		 		\
		for (cv = GolitWDPtrArgs(wdp);		\
		     cv != (ArgList)NULL &&		\
		     cv->name != (String)NULL;		\
		     cv++)

#define	ForAllTypedArgs(cv, wdp)	 		\
		for (cv = GolitWDPtrTypedArgs(wdp);     \
		     cv != (XtTypedArg *)NULL && 	\
		     cv->name != (String)NULL;		\
		     cv++)

#define	ForAllCallbacks(cv, wdp)					\
		for (cv = GolitWDPtrCallbacks(wdp);			\
		     cv != (GolitCallbackInfoPtr)NULL &&		\
		     GolitCbIPtrCallbackName(cv) != (String)NULL;	\
		     cv++)

#define	ForAllCreateProcs(cv, wdp)					    \
		for (cv = GolitWDPtrCreateProcs(wdp);			    \
		     cv != (_GolitWCProc *)NULL &&			    \
		     *cv != (_GolitWCProc)NULL  &&			    \
		     cv < GolitWDPtrCreateProcs(wdp) + (2 << BitsInCpIdx);  \
		     cv++)

#define	CreateProcListNULL(wdp)						\
	((GolitWidgetCreateProc *)GolitWDPtrCreateProcs(wdp) == 	\
		(GolitWidgetCreateProc *)NULL ||			\
	 (GolitWidgetCreateProc)(GolitWDPtrCreateProcs(wdp)		\
		[(int)GolitWDPtrCpIdx(wdp)]) == (GolitWidgetCreateProc)NULL)

#define	ShellCreateProcListNULL(wdp)					  \
	((GolitShellWidgetCreateProc *)GolitWDPtrShellCreateProcs(wdp) == \
		 (GolitShellWidgetCreateProc *)NULL || 			  \
	 (GolitShellWidgetCreateProc)(GolitWDPtrShellCreateProcs(wdp)    \
		[GolitWDPtrCpIdx(wdp)]) == (GolitShellWidgetCreateProc)NULL)

#define	DoRealCreate(wdp, func)						\
	 ((GolitWidgetCreateProc)(GolitWDPtrCreateProcs(wdp)		\
		[(int)GolitWDPtrCpIdx(wdp)]) == (func))
		
#define	DoRealShellCreate(wdp, func)					\
	 ((GolitShellWidgetCreateProc)(GolitWDPtrShellCreateProcs(wdp) \
		[(int)GolitWDPtrCpIdx(wdp)]) == (func))
		
#define	_DecrWDPtrCpIdx(wdp)						\
		(((unsigned int)GolitWDPtrCpIdx(wdp) > 0)		\
			? GolitWDPtrCpIdx(wdp)-- : 0 )

#define	_IncrWDPtrCpIdx(wdp)						   \
		(((unsigned int)GolitWDPtrCpIdx(wdp) < (2 << BitsInCpIdx)) \
			? GolitWDPtrCpIdx(wdp)++			   \
			: GolitWDPtrCpIdx(wdp))

#define	CreateProc(n, wdp, pw, oa, na, cl)				\
		(*((GolitWidgetCreateProc)(GolitWDPtrCreateProcs(wdp)	\
			[(int)_IncrWDPtrCpIdx(wdp)])))(n, wdp, pw, oa, na, cl)

#define	ShellCreateProc(n, c, wdp, dpy, oa, na, cl)			 \
	(*((GolitShellWidgetCreateProc)(GolitWDPtrShellCreateProcs(wdp) \
		[(int)_IncrWDPtrCpIdx(wdp)])))(n, c, wdp, dpy, oa, na, cl)

#define	NoneDynamic(wdp)			\
	!(GolitWDPtrArgsAreDynamic(wdp)  || 	\
	  GolitWDPtrTArgsAreDynamic(wdp) ||	\
	  GolitWDPtrCBsAreDynamic(wdp)   ||	\
	  GolitWDPtrCPsAreDynamic(wdp))

/**************************
 *
 * Some internal functions
 * for sophisticated 
 * create proc developers
 *
 **************************/

extern int _GolitWDNumSiblings OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumChildren OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumPopups OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumArgs OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumTypedArgs OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumCallbacks OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern int _GolitWDNumCreateProcs OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,	wdp */

extern Boolean _GolitWDIsSubclass OL_ARGS(( GolitWidgetDescPtr, WidgetClass ));
	/* GolitWidgetDescPtr,		wdp		*/
	/* register WidgetClass,	widget_class	*/

extern ArgList
_GolitFetchWDArgs OL_ARGS(( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,		wdp	*/

extern XtTypedArgList
_GolitFetchWDTypedArgs OL_ARGS (( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,		wdp	*/

extern WidgetClass
_GolitFetchWDWidgetClass OL_ARGS (( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,		wdp	*/

extern void
_GolitResetWDCpIdx OL_ARGS(( GolitWidgetDescPtr ));
        /* register GolitWidgetDescPtr,         wdp     */

extern int
_GolitIncrWDCpIdx OL_ARGS(( GolitWidgetDescPtr ));
        /* register GolitWidgetDescPtr,         wdp     */

extern int
_GolitDecrWDCpIdx OL_ARGS(( GolitWidgetDescPtr ));
        /* register GolitWidgetDescPtr,         wdp     */

extern void
_GolitInstallCBsOnWidget OL_ARGS(( Widget, GolitWidgetDescPtr, XtPointer ));
	/* register Widget,			widget		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* XtPointer,				default_closure */

extern Widget*
_GolitFetchWDSiblings OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList, 
			       Cardinal, GolitWidgetCreateProc, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* GolitWidgetCreateProc,		create_proc	*/
	/* XtPointer,				closure		*/

extern void
_GolitWarning OL_ARGS(( Widget, GolitWidgetDescPtr, String));
	/* Widget,				widget		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* String,				name		*/

extern void
_GolitError OL_ARGS(( Widget, GolitWidgetDescPtr, String));
	/* Widget,				widget		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* String,				name		*/

Boolean
_GolitInsertArgs OL_ARGS(( GolitWidgetDescPtr, ArgList, Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* ArgList,			args	*/
	/* Cardinal,			num_args*/


Boolean
_GolitDeleteArgs OL_ARGS(( GolitWidgetDescPtr, ArgList, Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* ArgList,			args	*/
	/* Cardinal,			num_args*/


Boolean
_GolitInsertTypedArgs OL_ARGS(( GolitWidgetDescPtr, XtTypedArgList, Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* XtTypedArgList,		targs	*/
	/* Cardinal,			num_args*/


Boolean
_GolitDeleteTypedArgs OL_ARGS(( GolitWidgetDescPtr, XtTypedArgList, Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* XtTypedArgList,		targs	*/
	/* Cardinal,			num_args*/


Boolean
_GolitInsertCallbackInfo OL_ARGS(( GolitWidgetDescPtr, GolitCallbackInfoPtr,
				   Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* GolitCallbackInfoPtr,	cbl	*/
	/* Cardinal,			num_cbs	*/


Boolean
_GolitDeleteCallbackInfo OL_ARGS(( GolitWidgetDescPtr, GolitCallbackInfoPtr,
				   Cardinal ));
	/* GolitWidgetDescPtr,		wdp	*/
	/* GolitCallbackInfoPtr,	cbl	*/
	/* Cardinal,			num_cbs	*/


Widget
GolitCallNextCreateProc OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList, 
			         Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

Boolean
GolitOKToDoRealCreate OL_ARGS((GolitWidgetDescPtr, GolitWidgetCreateProc,
			       OLVARGLIST));
	/* register GolitWidgetDescPtr,		wdp		*/
	/* GolitWidgetCreateProc,		this_cp		*/
	/* ... more create proc synonyms ...			*/
#endif /* _libgolitI_h */
