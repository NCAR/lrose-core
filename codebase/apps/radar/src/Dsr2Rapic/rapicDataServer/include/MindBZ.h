
/*******************************************************************************
       MindBZ.h
       This header file is included by MindBZ.c

*******************************************************************************/

#ifndef	_MINDBZ_INCLUDED
#define	_MINDBZ_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/Scale.h>
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
	Widget	UxMindBZ;
	Widget	Uxlabel9;
	Widget	UxMindBZScale;
	Widget	UxCommReqClose1;
	void	*Uxcallingwin;
	swidget	UxUxParent;
	void	*UxCallingWin;
} _UxCMindBZ;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCMindBZ              *UxMindBZContext;
#define MindBZ                  UxMindBZContext->UxMindBZ
#define label9                  UxMindBZContext->Uxlabel9
#define MindBZScale             UxMindBZContext->UxMindBZScale
#define CommReqClose1           UxMindBZContext->UxCommReqClose1
#define callingwin              UxMindBZContext->Uxcallingwin
#define UxParent                UxMindBZContext->UxUxParent
#define CallingWin              UxMindBZContext->UxCallingWin

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_MindBZ(swidget, void*);
#ifdef __cplusplus
}
#endif

#endif	/* _MINDBZ_INCLUDED */
