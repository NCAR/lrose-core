
/*******************************************************************************
	warningDialog1.h
	This header file is included by warningDialog1.c

*******************************************************************************/

#ifndef	_WARNINGDIALOG1_INCLUDED
#define	_WARNINGDIALOG1_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/MessageB.h>

/*******************************************************************************
	The definition of the context structure:
	If you create multiple instances of your interface, the context
	structure ensures that your callbacks use the variables for the
	correct instance.

	For each swidget in the interface, each argument to the Interface
	function, and each variable in the Instance Specific section of the
	Declarations Editor, there is an entry in the context structure.
	and a #define.  The #define makes the variable name refer to the
	corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	Widget	UxExitWarn;
} _UxCExitWarn;

#define ExitWarn                UxExitWarnContext->UxExitWarn

static _UxCExitWarn	*UxExitWarnContext;


#endif	/* _WARNINGDIALOG1_INCLUDED */
