
/*******************************************************************************
       SaveLoadLayout.h
       This header file is included by SaveLoadLayout.c

*******************************************************************************/

#ifndef	_SAVELOADLAYOUT_INCLUDED
#define	_SAVELOADLAYOUT_INCLUDED


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
	Widget	UxSaveLoadLayout;
	int	UxMode;
} _UxCSaveLoadLayout;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSaveLoadLayout      *UxSaveLoadLayoutContext;
#define SaveLoadLayout          UxSaveLoadLayoutContext->UxSaveLoadLayout
#define Mode                    UxSaveLoadLayoutContext->UxMode

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_SaveLoadLayout();

#endif	/* _SAVELOADLAYOUT_INCLUDED */
