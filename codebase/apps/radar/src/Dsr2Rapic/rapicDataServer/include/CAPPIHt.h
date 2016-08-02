
/*******************************************************************************
       CAPPIHt.h
       This header file is included by CAPPIHt.c

*******************************************************************************/

#ifndef	_CAPPIHT_INCLUDED
#define	_CAPPIHT_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
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
	Widget	UxCAPPIHt;
	Widget	UxCAPPIHtScale;
	Widget	Uxlabel11;
	Widget	UxCAPPIUnitsRowCol;
	Widget	UxCAPPIUnitsKft;
	Widget	UxCAPPIUnitsKm;
	Widget	UxCAPPIModeRowCol;
	Widget	UxCAPPINearestButton;
	Widget	UxCAPPIInterpButton;
	Widget	UxCAPPIHtClose1;
	Widget	Uxlabel12;
	Widget	Uxlabel13;
	Widget	UxrowColumn1;
	Widget	UxCAPPIButton80kft;
	Widget	UxCAPPIButton70kft;
	Widget	UxCAPPIButton60kft;
	Widget	UxCAPPIButton50kft;
	Widget	UxCAPPIButton40kft;
	Widget	UxCAPPIButton30kft;
	Widget	UxCAPPIButton20kft;
	Widget	UxCAPPIButton10kft;
	Widget	UxCAPPIButton0kft;
	void	*Uxcallingwin;
	swidget	UxUxParent;
	void	*UxCallingWin;
        int     UxCAPPIModeKm;
} _UxCCAPPIHt;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCCAPPIHt             *UxCAPPIHtContext;
#define CAPPIHt                 UxCAPPIHtContext->UxCAPPIHt
#define CAPPIHtScale            UxCAPPIHtContext->UxCAPPIHtScale
#define label11                 UxCAPPIHtContext->Uxlabel11
#define CAPPIUnitsRowCol        UxCAPPIHtContext->UxCAPPIUnitsRowCol
#define CAPPIUnitsKft           UxCAPPIHtContext->UxCAPPIUnitsKft
#define CAPPIUnitsKm            UxCAPPIHtContext->UxCAPPIUnitsKm
#define CAPPIModeRowCol         UxCAPPIHtContext->UxCAPPIModeRowCol
#define CAPPINearestButton      UxCAPPIHtContext->UxCAPPINearestButton
#define CAPPIInterpButton       UxCAPPIHtContext->UxCAPPIInterpButton
#define CAPPIHtClose1           UxCAPPIHtContext->UxCAPPIHtClose1
#define label12                 UxCAPPIHtContext->Uxlabel12
#define label13                 UxCAPPIHtContext->Uxlabel13
#define rowColumn1              UxCAPPIHtContext->UxrowColumn1
#define CAPPIButton80kft        UxCAPPIHtContext->UxCAPPIButton80kft
#define CAPPIButton70kft        UxCAPPIHtContext->UxCAPPIButton70kft
#define CAPPIButton60kft        UxCAPPIHtContext->UxCAPPIButton60kft
#define CAPPIButton50kft        UxCAPPIHtContext->UxCAPPIButton50kft
#define CAPPIButton40kft        UxCAPPIHtContext->UxCAPPIButton40kft
#define CAPPIButton30kft        UxCAPPIHtContext->UxCAPPIButton30kft
#define CAPPIButton20kft        UxCAPPIHtContext->UxCAPPIButton20kft
#define CAPPIButton10kft        UxCAPPIHtContext->UxCAPPIButton10kft
#define CAPPIButton0kft         UxCAPPIHtContext->UxCAPPIButton0kft
#define CAPPIModeKm             UxCAPPIHtContext->UxCAPPIModeKm
#define callingwin              UxCAPPIHtContext->Uxcallingwin
#define UxParent                UxCAPPIHtContext->UxUxParent
#define CallingWin              UxCAPPIHtContext->UxCallingWin

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_CAPPIHt(swidget, void*);
#ifdef __cplusplus
}
#endif


#endif	/* _CAPPIHT_INCLUDED */
