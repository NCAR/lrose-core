
/*******************************************************************************
       SetMinValue.h
       This header file is included by SetMinValue.c

*******************************************************************************/

#ifndef	_SETMINVALUE_INCLUDED
#define	_SETMINVALUE_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/ScrollBar.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
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
	Widget	UxSetMinValue;
	Widget	UxSetMinValueTextW;
	Widget	Uxlabel12;
	Widget	UxscrollBarH1;
	char	UxMinValueTextString[32];
	swidget	UxUxParent;
} _UxCSetMinValue;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSetMinValue         *UxSetMinValueContext;
#define SetMinValue             UxSetMinValueContext->UxSetMinValue
#define SetMinValueTextW        UxSetMinValueContext->UxSetMinValueTextW
#define label12                 UxSetMinValueContext->Uxlabel12
#define scrollBarH1             UxSetMinValueContext->UxscrollBarH1
#define MinValueTextString      UxSetMinValueContext->UxMinValueTextString
#define UxParent                UxSetMinValueContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_SetMinValue();

#endif	/* _SETMINVALUE_INCLUDED */
