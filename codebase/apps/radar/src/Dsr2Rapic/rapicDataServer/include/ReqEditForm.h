
/*******************************************************************************
       ReqEditForm.h
       This header file is included by ReqEditForm.c

*******************************************************************************/

#ifndef	_REQEDITFORM_INCLUDED
#define	_REQEDITFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>

/*******************************************************************************
       The definition of the context structure:
       If you create multiple copies of your interface, the context
       structure ensures that your callbacks use the variables for the
       correct copy.

       For each swidget in the interface, each argument to the Interface
       function, and each variable in the Interface Specific section of the
       Declarations Editor, there is an entry in the context structure.
       and a #define.  The #define makes the variable name refer to the
       corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	Widget	UxReqEditForm;
	Widget	UxReqEditClose;
	Widget	UxscrolledWindowList2;
	Widget	UxReqEditList;
	Widget	Uxlabel23;
	Widget	UxReqEditDel;
	Widget	Uxlabel24;
	Widget	UxpushButton20;
	char	UxSelReqStr[64];
	void	*UxCallingMng;
	swidget	UxUxParent;
	void	*Uxcallingmng;
} _UxCReqEditForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCReqEditForm         *UxReqEditFormContext;
#define ReqEditForm             UxReqEditFormContext->UxReqEditForm
#define ReqEditClose            UxReqEditFormContext->UxReqEditClose
#define scrolledWindowList2     UxReqEditFormContext->UxscrolledWindowList2
#define ReqEditList             UxReqEditFormContext->UxReqEditList
#define label23                 UxReqEditFormContext->Uxlabel23
#define ReqEditDel              UxReqEditFormContext->UxReqEditDel
#define label24                 UxReqEditFormContext->Uxlabel24
#define pushButton20            UxReqEditFormContext->UxpushButton20
#define SelReqStr               UxReqEditFormContext->UxSelReqStr
#define CallingMng              UxReqEditFormContext->UxCallingMng
#define UxParent                UxReqEditFormContext->UxUxParent
#define callingmng              UxReqEditFormContext->Uxcallingmng

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_ReqEditForm(swidget, void*);
#ifdef __cplusplus
}
#endif

#endif	/* _REQEDITFORM_INCLUDED */
