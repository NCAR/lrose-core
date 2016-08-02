
/*******************************************************************************
       RadarStatus.h
       This header file is included by RadarStatus.c

*******************************************************************************/

#ifndef	_RADARSTATUS_INCLUDED
#define	_RADARSTATUS_INCLUDED


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
	Widget	UxRadarStatus;
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
} _UxCRadarStatus;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRadarStatus         *UxRadarStatusContext;
#define RadarStatus             UxRadarStatusContext->UxRadarStatus
#define RdrStatusTitle1         UxRadarStatusContext->UxRdrStatusTitle1
#define RdrStCtlHandlerTitle1   UxRadarStatusContext->UxRdrStCtlHandlerTitle1
#define RdrStCtlRdrNameTitle1   UxRadarStatusContext->UxRdrStCtlRdrNameTitle1
#define RdrStCtlConnStTitle1    UxRadarStatusContext->UxRdrStCtlConnStTitle1
#define RdrStCtlRdrStTitle1     UxRadarStatusContext->UxRdrStCtlRdrStTitle1
#define RdrStCtlNextScanTitle1  UxRadarStatusContext->UxRdrStCtlNextScanTitle1
#define RdrStCtlHandler1        UxRadarStatusContext->UxRdrStCtlHandler1
#define RdrStCtlRadarName1      UxRadarStatusContext->UxRdrStCtlRadarName1
#define RdrStCtlConnSt1         UxRadarStatusContext->UxRdrStCtlConnSt1
#define RdrStCtlRdrSt1          UxRadarStatusContext->UxRdrStCtlRdrSt1
#define RdrStCtlNextScan1       UxRadarStatusContext->UxRdrStCtlNextScan1
#define separator3              UxRadarStatusContext->Uxseparator3
#define DemandRHIDoIt2          UxRadarStatusContext->UxDemandRHIDoIt2
#define RdrStCtlVar             UxRadarStatusContext->UxRdrStCtlVar
#define callingwin              UxRadarStatusContext->Uxcallingwin
#define UxParent                UxRadarStatusContext->UxUxParent
#define CallingHndl             UxRadarStatusContext->UxCallingHndl
#define StCtlStruct             UxRadarStatusContext->UxStCtlStruct

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_RadarStatus(swidget, void*, RdrStCtlStruct*);
#ifdef __cplusplus
}
#endif

#endif	/* _RADARSTATUS_INCLUDED */




























