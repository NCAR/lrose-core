
/*******************************************************************************
       VILLimitForm.h
       This header file is included by VILLimitForm.c

*******************************************************************************/

#ifndef	_VILLIMITFORM_INCLUDED
#define	_VILLIMITFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/TextF.h>
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
	Widget	UxVILLimitForm;
	Widget	Uxlabel10;
	Widget	UxVILLimitText;
	Widget	UxVILLimitClose;
	Widget	Uxlabel34;
	Widget	UxdBZHailLimitText;
	void	*Uxcallingwin;
	swidget	UxUxParent;
	void	*UxCallingWin;
} _UxCVILLimitForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCVILLimitForm        *UxVILLimitFormContext;
#define VILLimitForm            UxVILLimitFormContext->UxVILLimitForm
#define label10                 UxVILLimitFormContext->Uxlabel10
#define VILLimitText            UxVILLimitFormContext->UxVILLimitText
#define VILLimitClose           UxVILLimitFormContext->UxVILLimitClose
#define label34                 UxVILLimitFormContext->Uxlabel34
#define dBZHailLimitText        UxVILLimitFormContext->UxdBZHailLimitText
#define callingwin              UxVILLimitFormContext->Uxcallingwin
#define UxParent                UxVILLimitFormContext->UxUxParent
#define CallingWin              UxVILLimitFormContext->UxCallingWin

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_VILLimitForm(swidget, void*);
#ifdef __cplusplus
}
#endif

#endif	/* _VILLIMITFORM_INCLUDED */
