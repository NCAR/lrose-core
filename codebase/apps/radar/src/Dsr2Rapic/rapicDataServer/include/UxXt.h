/*---------------------------------------------------------------------
 * $Date: 2007/10/25 03:37:27 $             $Revision: 1.1 $
 *---------------------------------------------------------------------
 * 
 *
 *             Copyright (c) 1991, Visual Edge Software Ltd.
 *
 * ALL  RIGHTS  RESERVED.  Permission  to  use,  copy,  modify,  and
 * distribute  this  software  and its documentation for any purpose
 * and  without  fee  is  hereby  granted,  provided  that the above
 * copyright  notice  appear  in  all  copies  and  that  both  that
 * copyright  notice and this permission notice appear in supporting
 * documentation,  and that  the name of Visual Edge Software not be
 * used  in advertising  or publicity  pertaining to distribution of
 * the software without specific, written prior permission. The year
 * included in the notice is the year of the creation of the work.
 *-------------------------------------------------------------------*/

/*****************************************************************************/
/*				UxXt.h				             */
/*****************************************************************************/

#ifndef	_UX_XT_H_
#define	_UX_XT_H_

#include <stdlib.h>
#include <Xm/Xm.h>

#ifdef UIL_CODE
#include <Mrm/MrmPublic.h>
#endif /* UIL_CODE */

#if defined(__STDC__) && !defined(SOLARIS) && !defined(sun4) && !defined(univel) && !defined(motor88) && !defined(__sgi) && !defined(ncrix86) && !defined(sco) && !defined(linux)
typedef char *caddr_t;
#endif

/*-----------------------------------------------------
 * UXORB_HEADER, if defined, is the include form for
 * the header that defines the CORBA Environment type
 * and exception type codes.
 *
 * You can specify a file with a compile option like
 * 	-DUXORB_HEADER='<SomeOrb.h>'
 *-----------------------------------------------------*/
#ifdef UXORB_HEADER
#include UXORB_HEADER
#else
	/*
	 * In the absence of an ORB implementation,
	 * these minimal definitions satisfy our method dispatch code.
	 */
	typedef enum {
		NO_EXCEPTION,
		USER_EXCEPTION,
		SYSTEM_EXCEPTION
	} exception_type;

	typedef struct Environment {
		exception_type	_major;
	} Environment;
#endif  /* UXORB_HEADER */

/*
 * UxEnv is provided as a convenience for use in interface methods.
 */
extern	Environment	UxEnv;


/* The following macros are used in converting string values to the form
   required by the widgets */

#define	RES_CONVERT( res_name, res_value) \
	XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1

#define	UxPutStrRes( wgt, res_name, res_value ) \
	XtVaSetValues( wgt, RES_CONVERT( res_name, res_value ), NULL )


#ifndef UX_INTERPRETER	/* Omit this section when interpreting the code */

/* The following macros are supplied for compatibility with swidget code */
#define	swidget			Widget
#define	UxWidgetToSwidget(w)	(w)
#define	UxGetWidget(sw)		(sw)
#define	UxIsValidSwidget(sw)	((sw) != NULL)
#define NO_PARENT             	((Widget) NULL)
#define UxThisWidget		(UxWidget)

/* Macros needed for the method support code */
#define	UxMalloc(a)		(malloc(a))
#define	UxRealloc(a,b)		(realloc((a), (b)))
#define	UxCalloc(a,b)		(calloc((a), (b)))
#define UxStrEqual(a,b)		(!strcmp((a),(b)))
#define UxGetParent(a)		(XtParent((a)))

#define	no_grab			XtGrabNone
#define	nonexclusive_grab	XtGrabNonexclusive
#define	exclusive_grab		XtGrabExclusive


/* The following global variables are defined in the main() function */
extern  XtAppContext	UxAppContext;
extern  Widget		UxTopLevel;
extern  Display		*UxDisplay;
extern  int		UxScreen;


/* The following are error codes returned by the functions in UxXt.c */
#define UX_ERROR           -1
#define UX_NO_ERROR        0

#ifdef UIL_CODE
#ifdef _NO_PROTO
extern	void    	UxMrmFetchError();
extern	MrmHierarchy    UxMrmOpenHierarchy();
extern	void    	UxMrmRegisterClass();
#else
extern	void    	UxMrmFetchError(MrmHierarchy, char *, Widget, Cardinal);
extern	MrmHierarchy    UxMrmOpenHierarchy( char *);
extern	void    	UxMrmRegisterClass( char *, Widget (*)(Widget, String, Arg *, Cardinal));
#endif /* _NO_PROTO */
#endif /* UIL_CODE */



/* The following are declarations of the functions in UxXt.c */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _NO_PROTO

extern  int		UxPopupInterface();
extern  int		UxPopdownInterface();
extern  int		UxDestroyInterface();
extern  int		UxPutContext();
extern  caddr_t		UxGetContext();
extern  void		UxFreeClientDataCB();
extern  void		UxLoadResources();
extern  XmFontList	UxConvertFontList();
extern  Pixmap		UxConvertPixmap();
extern  Pixmap		UxConvert_bitmap();
extern	wchar_t *	UxConvertValueWcs();
extern	void            UxDestroyContextCB();
extern	void    	UxDeleteContextCB();
extern	XtArgVal	UxRemoveValueFromArgList();
extern	Widget		UxChildSite();
extern	Widget  	UxRealWidget( );

#else

extern  int		UxPopupInterface( Widget wgt, XtGrabKind grab_flag );
extern  int		UxPopdownInterface( Widget wgt );
extern  int		UxDestroyInterface( Widget wgt);
extern  int		UxPutContext( Widget wgt, caddr_t context );
extern  caddr_t		UxGetContext( Widget wgt );
extern  void		UxFreeClientDataCB( Widget wgt, XtPointer client_data,
						 XtPointer call_data );
extern  void		UxLoadResources( char *fname );
extern  XmFontList	UxConvertFontList( char *fontlist_str );
extern  Pixmap		UxConvertPixmap( char *file_name );
extern  Pixmap		UxConvert_bitmap( char *file_name );
extern	wchar_t *	UxConvertValueWcs( char *value_str );

extern  void            UxDestroyContextCB(Widget, XtPointer, XtPointer);
extern	void    	UxDeleteContextCB( Widget, XtPointer, XtPointer);
extern	XtArgVal	UxRemoveValueFromArgList( Arg *args,
						Cardinal *ptr_num_args,
						String res_name );
extern	Widget		UxChildSite( Widget );
extern	Widget  	UxRealWidget( Widget );

#endif /* _NO_PROTO */
#ifdef __cplusplus
        }
#endif

#ifdef __cplusplus
class _UxCInterface {

public:

	virtual swidget childSite (Environment * pEnv);
	virtual swidget UxChildSite (swidget sw);

protected:
	swidget	UxThis;
};

#define CPLUS_ADAPT_CONTEXT(CLASS) \
        static inline \
                CLASS* UxGetContext(CLASS*self) {return self;} \
        static inline\
                void* UxGetContext(swidget any) {return ::UxGetContext(any);}

#endif /* _cplusplus */

#endif /* ! UX_INTERPRETER */

#endif /* ! _UX_XT_H_ */

