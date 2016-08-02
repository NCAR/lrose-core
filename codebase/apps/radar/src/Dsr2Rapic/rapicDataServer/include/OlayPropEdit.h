
/*******************************************************************************
       OlayPropEdit.h
       This header file is included by OlayPropEdit.c

*******************************************************************************/

#ifndef	_OLAYPROPEDIT_INCLUDED
#define	_OLAYPROPEDIT_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
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
	Widget	UxOlayPropEdit;
	Widget	Uxlabel28;
	Widget	UxOlayBGButton;
	Widget	UxRngRingButton;
	Widget	UxTextButton;
	Widget	UxRngRingButton1;
	Widget	UxpushButton24;
	Widget	Uxlabel30;
	Widget	Uxlabel33;
	Widget	UxOlayLineThicknessText;
	Widget	UxOlayFontSizeText;
	Widget	UxTextButton1;
	void	*UxCallingObj;
	struct olayproperties	*UxOlayProps;
	swidget	UxUxParent;
	void	*Uxcallingobj;
	struct olayproperties	*Uxolayprops;
} _UxCOlayPropEdit;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCOlayPropEdit        *UxOlayPropEditContext;
#define OlayPropEdit            UxOlayPropEditContext->UxOlayPropEdit
#define label28                 UxOlayPropEditContext->Uxlabel28
#define OlayBGButton            UxOlayPropEditContext->UxOlayBGButton
#define RngRingButton           UxOlayPropEditContext->UxRngRingButton
#define TextButton              UxOlayPropEditContext->UxTextButton
#define RngRingButton1          UxOlayPropEditContext->UxRngRingButton1
#define pushButton24            UxOlayPropEditContext->UxpushButton24
#define label30                 UxOlayPropEditContext->Uxlabel30
#define label33                 UxOlayPropEditContext->Uxlabel33
#define OlayLineThicknessText   UxOlayPropEditContext->UxOlayLineThicknessText
#define OlayFontSizeText        UxOlayPropEditContext->UxOlayFontSizeText
#define TextButton1             UxOlayPropEditContext->UxTextButton1
#define CallingObj              UxOlayPropEditContext->UxCallingObj
#define OlayProps               UxOlayPropEditContext->UxOlayProps
#define UxParent                UxOlayPropEditContext->UxUxParent
#define callingobj              UxOlayPropEditContext->Uxcallingobj
#define olayprops               UxOlayPropEditContext->Uxolayprops

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	create_OlayPropEdit(swidget, void*, struct olayproperties*);
#ifdef __cplusplus
}
#endif

#endif	/* _OLAYPROPEDIT_INCLUDED */
