
/*******************************************************************************
       RGBEdit.h
       This header file is included by RGBEdit.c

*******************************************************************************/

#ifndef	_RGBEDIT_INCLUDED
#define	_RGBEDIT_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/TextF.h>
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
	Widget	UxRGBEdit;
	Widget	Uxlabel26;
	Widget	UxBlueScale;
	Widget	UxGreenScale;
	Widget	UxRedScale;
	Widget	Uxlabel27;
	Widget	UxColorSample;
	swidget	UxUxParent;
} _UxCRGBEdit;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRGBEdit             *UxRGBEditContext;
#define RGBEdit                 UxRGBEditContext->UxRGBEdit
#define label26                 UxRGBEditContext->Uxlabel26
#define BlueScale               UxRGBEditContext->UxBlueScale
#define GreenScale              UxRGBEditContext->UxGreenScale
#define RedScale                UxRGBEditContext->UxRedScale
#define label27                 UxRGBEditContext->Uxlabel27
#define ColorSample             UxRGBEditContext->UxColorSample
#define UxParent                UxRGBEditContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_RGBEdit();

#endif	/* _RGBEDIT_INCLUDED */
