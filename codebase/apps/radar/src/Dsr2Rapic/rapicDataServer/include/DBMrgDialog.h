
/*******************************************************************************
       DBMrgDialog.h
       This header file is included by DBMrgDialog.c

*******************************************************************************/

#ifndef	_DBMRGDIALOG_INCLUDED
#define	_DBMRGDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/MessageB.h>

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
	Widget	UxDBMrgDialog;
} _UxCDBMrgDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCDBMrgDialog         *UxDBMrgDialogContext;
#define DBMrgDialog             UxDBMrgDialogContext->UxDBMrgDialog

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_MrgWarn();

#endif	/* _DBMRGDIALOG_INCLUDED */
