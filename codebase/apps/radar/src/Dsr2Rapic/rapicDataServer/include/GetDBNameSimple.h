
/*******************************************************************************
       GetDBNameSimple.h
       This header file is included by GetDBNameSimple.c

*******************************************************************************/

#ifndef	_GETDBNAMESIMPLE_INCLUDED
#define	_GETDBNAMESIMPLE_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/BulletinB.h>

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
	Widget	UxGetDBNameSimple;
	Widget	Uxlabel2;
	Widget	UxGetDBNameSimpleList;
	Widget	UxGetDBNameSimpleSel;
	Widget	UxpushButton8;
	Widget	UxpushButton9;
	Widget	UxpushButton10;
	Widget	Uxlabel8;
} _UxCGetDBNameSimple;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCGetDBNameSimple     *UxGetDBNameSimpleContext;
#define GetDBNameSimple         UxGetDBNameSimpleContext->UxGetDBNameSimple
#define label2                  UxGetDBNameSimpleContext->Uxlabel2
#define GetDBNameSimpleList     UxGetDBNameSimpleContext->UxGetDBNameSimpleList
#define GetDBNameSimpleSel      UxGetDBNameSimpleContext->UxGetDBNameSimpleSel
#define pushButton8             UxGetDBNameSimpleContext->UxpushButton8
#define pushButton9             UxGetDBNameSimpleContext->UxpushButton9
#define pushButton10            UxGetDBNameSimpleContext->UxpushButton10
#define label8                  UxGetDBNameSimpleContext->Uxlabel8

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_GetDBNameSimple();

#endif	/* _GETDBNAMESIMPLE_INCLUDED */
