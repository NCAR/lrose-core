
/*******************************************************************************
       RadarStatus1.h
       This header file is included by RadarStatus1.c

*******************************************************************************/

#ifndef	_RADARSTATUS1_INCLUDED
#define	_RADARSTATUS1_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/Separator.h>
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
	Widget	UxRadarStatus1;
	Widget	UxRdrStatusTitle1;
	Widget	UxRdrStCtlHandlerTitle1;
	Widget	UxRdrStCtlRdrNameTitle1;
	Widget	UxRdrStCtlConnStTitle1;
	Widget	UxRdrStCtlRdrStTitle1;
	Widget	UxRdrStCtlNextScanTitle1;
	Widget	UxRdrStCtlHandler1;
	Widget	UxRdrStCtlRadarName1;
	Widget	UxRdrStCtlConnSt1;
	Widget	UxRdrStCtlRdrSt1;
	Widget	UxRdrStCtlNextScan1;
	Widget	Uxseparator3;
	Widget	UxDemandRHIDoIt2;
	struct RdrStCtlStruct	*UxRdrStCtlVar;
	void	*Uxcallingwin;
	swidget	UxUxParent;
	void	*UxCallingHndl;
	struct RdrStCtlStruct	*UxStCtlStruct;
} _UxCRadarStatus1;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRadarStatus1        *UxRadarStatus1Context;
#define RadarStatus1            UxRadarStatus1Context->UxRadarStatus1
#define RdrStatusTitle1         UxRadarStatus1Context->UxRdrStatusTitle1
#define RdrStCtlHandlerTitle1   UxRadarStatus1Context->UxRdrStCtlHandlerTitle1
#define RdrStCtlRdrNameTitle1   UxRadarStatus1Context->UxRdrStCtlRdrNameTitle1
#define RdrStCtlConnStTitle1    UxRadarStatus1Context->UxRdrStCtlConnStTitle1
#define RdrStCtlRdrStTitle1     UxRadarStatus1Context->UxRdrStCtlRdrStTitle1
#define RdrStCtlNextScanTitle1  UxRadarStatus1Context->UxRdrStCtlNextScanTitle1
#define RdrStCtlHandler1        UxRadarStatus1Context->UxRdrStCtlHandler1
#define RdrStCtlRadarName1      UxRadarStatus1Context->UxRdrStCtlRadarName1
#define RdrStCtlConnSt1         UxRadarStatus1Context->UxRdrStCtlConnSt1
#define RdrStCtlRdrSt1          UxRadarStatus1Context->UxRdrStCtlRdrSt1
#define RdrStCtlNextScan1       UxRadarStatus1Context->UxRdrStCtlNextScan1
#define separator3              UxRadarStatus1Context->Uxseparator3
#define DemandRHIDoIt2          UxRadarStatus1Context->UxDemandRHIDoIt2
#define RdrStCtlVar             UxRadarStatus1Context->UxRdrStCtlVar
#define callingwin              UxRadarStatus1Context->Uxcallingwin
#define UxParent                UxRadarStatus1Context->UxUxParent
#define CallingHndl             UxRadarStatus1Context->UxCallingHndl
#define StCtlStruct             UxRadarStatus1Context->UxStCtlStruct

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_RadarStatus(Widget, void *, struct RdrStCtlStruct *);
#ifdef __cplusplus
}
#endif

#endif	/* _RADARSTATUS1_INCLUDED */
