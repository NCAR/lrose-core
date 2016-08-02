
/*******************************************************************************
       ZRCoeff.h
       This header file is included by ZRCoeff.c

*******************************************************************************/

#ifndef	_ZRCOEFF_INCLUDED
#define	_ZRCOEFF_INCLUDED


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
	Widget	UxZRCoeff;
	Widget	Uxlabel14;
	Widget	Uxlabel16;
	Widget	UxZRaTextField;
	Widget	UxZRbTextField;
	Widget	Uxlabel17;
	Widget	UxpushButton12;
	Widget	UxpushButton13;
	swidget	UxUxParent;
} _UxCZRCoeff;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCZRCoeff             *UxZRCoeffContext;
#define ZRCoeff                 UxZRCoeffContext->UxZRCoeff
#define label14                 UxZRCoeffContext->Uxlabel14
#define label16                 UxZRCoeffContext->Uxlabel16
#define ZRaTextField            UxZRCoeffContext->UxZRaTextField
#define ZRbTextField            UxZRCoeffContext->UxZRbTextField
#define label17                 UxZRCoeffContext->Uxlabel17
#define pushButton12            UxZRCoeffContext->UxpushButton12
#define pushButton13            UxZRCoeffContext->UxpushButton13
#define UxParent                UxZRCoeffContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_ZRCoeff(Widget);

#endif	/* _ZRCOEFF_INCLUDED */
