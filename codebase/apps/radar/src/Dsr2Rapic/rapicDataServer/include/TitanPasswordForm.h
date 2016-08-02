
/*******************************************************************************
       TitanPasswordForm.h
       This header file is included by TitanPasswordForm.c

*******************************************************************************/

#ifndef	_TITANPASSWORDFORM_INCLUDED
#define	_TITANPASSWORDFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
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
	Widget	UxTitanPasswordForm;
	Widget	Uxlabel35;
	Widget	Uxlabel36;
	Widget	UxTitanPasswordText;
	Widget	Uxlabel37;
	Widget	UxpushButton1;
	int	UxtitanPasswordOk;
	char	UxTitanPasswordString[64];
	swidget	UxUxParent;
} _UxCTitanPasswordForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCTitanPasswordForm   *UxTitanPasswordFormContext;
#define TitanPasswordForm       UxTitanPasswordFormContext->UxTitanPasswordForm
#define label35                 UxTitanPasswordFormContext->Uxlabel35
#define label36                 UxTitanPasswordFormContext->Uxlabel36
#define TitanPasswordText       UxTitanPasswordFormContext->UxTitanPasswordText
#define label37                 UxTitanPasswordFormContext->Uxlabel37
#define pushButton1             UxTitanPasswordFormContext->UxpushButton1
#define titanPasswordOk         UxTitanPasswordFormContext->UxtitanPasswordOk
#define TitanPasswordString     UxTitanPasswordFormContext->UxTitanPasswordString
#define UxParent                UxTitanPasswordFormContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_TitanPasswordForm();

#endif	/* _TITANPASSWORDFORM_INCLUDED */
