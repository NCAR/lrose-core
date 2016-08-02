
/*******************************************************************************
       SelectStn.h
       This header file is included by SelectStn.c

*******************************************************************************/

#ifndef	_SELECTSTN_INCLUDED
#define	_SELECTSTN_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
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
	Widget	UxSelectStn;
	Widget	Uxlabel19;
	Widget	UxrowColumn2;
	Widget	UxtoggleButton1;
	Widget	UxtoggleButton2;
	Widget	Uxlabel20;
	Widget	UxpushButton14;
	Widget	UxpushButton15;
	Widget	UxscrolledWindowList1;
	Widget	UxSelectStnList;
	Widget	UxpushButton18;
	void	*UxCallingWin;
	char	UxNewTitle[64];
	int	UxKnownOnly;
	int	UxSelectedStn;
	int	*UxStnPnt;
	swidget	UxUxParent;
	void	*Uxcallingwin;
	char	*Uxnewtitle;
	int	*Uxstn;
	int	Uxknownonly;
} _UxCSelectStn;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSelectStn           *UxSelectStnContext;
#define SelectStn               UxSelectStnContext->UxSelectStn
#define label19                 UxSelectStnContext->Uxlabel19
#define rowColumn2              UxSelectStnContext->UxrowColumn2
#define toggleButton1           UxSelectStnContext->UxtoggleButton1
#define toggleButton2           UxSelectStnContext->UxtoggleButton2
#define label20                 UxSelectStnContext->Uxlabel20
#define pushButton14            UxSelectStnContext->UxpushButton14
#define pushButton15            UxSelectStnContext->UxpushButton15
#define scrolledWindowList1     UxSelectStnContext->UxscrolledWindowList1
#define SelectStnList           UxSelectStnContext->UxSelectStnList
#define pushButton18            UxSelectStnContext->UxpushButton18
#define CallingWin              UxSelectStnContext->UxCallingWin
#define NewTitle                UxSelectStnContext->UxNewTitle
#define KnownOnly               UxSelectStnContext->UxKnownOnly
#define SelectedStn             UxSelectStnContext->UxSelectedStn
#define StnPnt                  UxSelectStnContext->UxStnPnt
#define UxParent                UxSelectStnContext->UxUxParent
#define callingwin              UxSelectStnContext->Uxcallingwin
#define newtitle                UxSelectStnContext->Uxnewtitle
#define stn                     UxSelectStnContext->Uxstn
#define knownonly               UxSelectStnContext->Uxknownonly

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_SelectStn();

#endif	/* _SELECTSTN_INCLUDED */
