
/*******************************************************************************
       CommMngForm.h
       This header file is included by CommMngForm.c

*******************************************************************************/

#ifndef	_COMMMNGFORM_INCLUDED
#define	_COMMMNGFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
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
	Widget	UxCommMngForm;
	Widget	Uxlabel15;
	Widget	UxCommMngWindowList;
	Widget	UxCommMngHandlerList;
	Widget	Uxlabel18;
	Widget	UxCommMngFormOpenButton;
	Widget	UxCommMngFormDoneButton;
	Widget	UxpushButton11;
	Widget	UxCommMngLinkStatusLabel;
	Widget	UxCommMngLinkStatus;
	Widget	Uxseparator6;
	Widget	Uxseparator7;
	Widget	Uxlabel25;
	Widget	Uxseparator10;
	Widget	UxpushButton21;
	Widget	UxCommReqSend2;
	Widget	UxpushButton22;
	Widget	UxpushButton23;
	void	*UxCallingMng;
	int	UxSelectedID;
	swidget	UxUxParent;
	void	*Uxcallingmng;
} _UxCCommMngForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCCommMngForm         *UxCommMngFormContext;
#define CommMngForm             UxCommMngFormContext->UxCommMngForm
#define label15                 UxCommMngFormContext->Uxlabel15
#define CommMngWindowList       UxCommMngFormContext->UxCommMngWindowList
#define CommMngHandlerList      UxCommMngFormContext->UxCommMngHandlerList
#define label18                 UxCommMngFormContext->Uxlabel18
#define CommMngFormOpenButton   UxCommMngFormContext->UxCommMngFormOpenButton
#define CommMngFormDoneButton   UxCommMngFormContext->UxCommMngFormDoneButton
#define pushButton11            UxCommMngFormContext->UxpushButton11
#define CommMngLinkStatusLabel  UxCommMngFormContext->UxCommMngLinkStatusLabel
#define CommMngLinkStatus       UxCommMngFormContext->UxCommMngLinkStatus
#define separator6              UxCommMngFormContext->Uxseparator6
#define separator7              UxCommMngFormContext->Uxseparator7
#define label25                 UxCommMngFormContext->Uxlabel25
#define separator10             UxCommMngFormContext->Uxseparator10
#define pushButton21            UxCommMngFormContext->UxpushButton21
#define CommReqSend2            UxCommMngFormContext->UxCommReqSend2
#define pushButton22            UxCommMngFormContext->UxpushButton22
#define pushButton23            UxCommMngFormContext->UxpushButton23
#define CallingMng              UxCommMngFormContext->UxCallingMng
#define SelectedID              UxCommMngFormContext->UxSelectedID
#define UxParent                UxCommMngFormContext->UxUxParent
#define callingmng              UxCommMngFormContext->Uxcallingmng

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_CommMngForm(swidget, void*);
#ifdef __cplusplus
}
#endif

#endif	/* _COMMMNGFORM_INCLUDED */
