
/*******************************************************************************
       CommConnectBB.h
       This header file is included by CommConnectBB.c

*******************************************************************************/

#ifndef	_COMMCONNECTBB_INCLUDED
#define	_COMMCONNECTBB_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Text.h>
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
	Widget	UxCommConnectBB;
	Widget	Uxlabel6;
	Widget	UxConnStrText;
} _UxCCommConnectBB;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCCommConnectBB       *UxCommConnectBBContext;
#define CommConnectBB           UxCommConnectBBContext->UxCommConnectBB
#define label6                  UxCommConnectBBContext->Uxlabel6
#define ConnStrText             UxCommConnectBBContext->UxConnStrText

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_CommConnBB();

#endif	/* _COMMCONNECTBB_INCLUDED */
