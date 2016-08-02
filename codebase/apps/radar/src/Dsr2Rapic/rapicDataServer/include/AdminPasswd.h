
/*******************************************************************************
       AdminPasswd.h
       This header file is included by AdminPasswd.c

*******************************************************************************/

#ifndef	_ADMINPASSWD_INCLUDED
#define	_ADMINPASSWD_INCLUDED


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
	Widget	UxAdminPasswd;
	Widget	Uxlabel29;
	Widget	UxtextField4;
	Widget	UxpushButton25;
	char	Uxpwordchar;
	char	Uxpwordstr[64];
	swidget	UxUxParent;
} _UxCAdminPasswd;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCAdminPasswd         *UxAdminPasswdContext;
#define AdminPasswd             UxAdminPasswdContext->UxAdminPasswd
#define label29                 UxAdminPasswdContext->Uxlabel29
#define textField4              UxAdminPasswdContext->UxtextField4
#define pushButton25            UxAdminPasswdContext->UxpushButton25
#define pwordchar               UxAdminPasswdContext->Uxpwordchar
#define pwordstr                UxAdminPasswdContext->Uxpwordstr
#define UxParent                UxAdminPasswdContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_AdminPassword();

#endif	/* _ADMINPASSWD_INCLUDED */
