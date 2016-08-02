
/*******************************************************************************
       GetDBName.h
       This header file is included by GetDBName.c

*******************************************************************************/

#ifndef	_GETDBNAME_INCLUDED
#define	_GETDBNAME_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/FileSB.h>

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
	Widget	UxGetDBName;
	swidget	Uxnamew;
	char	*UxDBName;
	swidget	UxTextW;
	int	UxSimpleDir;
	char	*Uxdbname;
} _UxCGetDBName;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCGetDBName           *UxGetDBNameContext;
#define GetDBName               UxGetDBNameContext->UxGetDBName
#define namew                   UxGetDBNameContext->Uxnamew
#define DBName                  UxGetDBNameContext->UxDBName
#define TextW                   UxGetDBNameContext->UxTextW
#define SimpleDir               UxGetDBNameContext->UxSimpleDir
#define dbname                  UxGetDBNameContext->Uxdbname

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_GetDBName();

#endif	/* _GETDBNAME_INCLUDED */
