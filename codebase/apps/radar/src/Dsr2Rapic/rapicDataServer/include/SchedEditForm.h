
/*******************************************************************************
       SchedEditForm.h
       This header file is included by SchedEditForm.c

*******************************************************************************/

#ifndef	_SCHEDEDITFORM_INCLUDED
#define	_SCHEDEDITFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/Scale.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
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
	Widget	UxSchedEditForm;
	Widget	Uxlabel22;
	Widget	UxSchedEditClose;
	Widget	UxSchedPeriodScale1;
	Widget	UxSchedOffsetScale1;
	Widget	UxSchedEditSetNew;
	Widget	UxSchedEditDel;
	Widget	UxscrolledWindowList3;
	Widget	UxSchedEditList;
	Widget	UxpushButton19;
	void	*UxCallingMng;
	swidget	UxUxParent;
	void	*Uxcallingmng;
} _UxCSchedEditForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSchedEditForm       *UxSchedEditFormContext;
#define SchedEditForm           UxSchedEditFormContext->UxSchedEditForm
#define label22                 UxSchedEditFormContext->Uxlabel22
#define SchedEditClose          UxSchedEditFormContext->UxSchedEditClose
#define SchedPeriodScale1       UxSchedEditFormContext->UxSchedPeriodScale1
#define SchedOffsetScale1       UxSchedEditFormContext->UxSchedOffsetScale1
#define SchedEditSetNew         UxSchedEditFormContext->UxSchedEditSetNew
#define SchedEditDel            UxSchedEditFormContext->UxSchedEditDel
#define scrolledWindowList3     UxSchedEditFormContext->UxscrolledWindowList3
#define SchedEditList           UxSchedEditFormContext->UxSchedEditList
#define pushButton19            UxSchedEditFormContext->UxpushButton19
#define CallingMng              UxSchedEditFormContext->UxCallingMng
#define UxParent                UxSchedEditFormContext->UxUxParent
#define callingmng              UxSchedEditFormContext->Uxcallingmng

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_SchedEditForm(swidget, void*);
#ifdef __cplusplus
}
#endif

#endif	/* _SCHEDEDITFORM_INCLUDED */
