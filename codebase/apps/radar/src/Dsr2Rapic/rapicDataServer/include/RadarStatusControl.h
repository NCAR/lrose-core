
/*******************************************************************************
       RadarStatusControl.h
       This header file is included by RadarStatusControl.c

*******************************************************************************/

#ifndef	_RADARSTATUSCONTROL_INCLUDED
#define	_RADARSTATUSCONTROL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
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
	Widget	UxRadarStatusControl;
	Widget	UxRdrStCtlRdrNameTitle;
	Widget	UxRdrStCtlRadarName;
	Widget	UxRdrStatusTitle;
	Widget	UxRdrStCtlConnStTitle;
	Widget	UxRdrStCtlRdrStTitle;
	Widget	UxRdrStCtlNextScanTitle;
	Widget	UxRdrStCtlConnSt;
	Widget	UxRdrStCtlRdrSt;
	Widget	UxRdrStCtlNextScan;
	Widget	Uxseparator1;
	Widget	Uxseparator2;
	Widget	UxRdrControlTitle;
	Widget	Uxframe1;
	Widget	UxTransmitterCtl;
	Widget	UxRadarTxOn;
	Widget	UxRadarTxOff;
	Widget	Uxframe2;
	Widget	UxServoCtl;
	Widget	UxRadarServoOn;
	Widget	UxRadarServoOff;
	Widget	UxDemandPPITitle;
	Widget	UxDemandPPIEl;
	Widget	UxDemandPPIDoIt;
	Widget	UxDemandRHITitle;
	Widget	UxDemandRHIAz;
	Widget	UxDemandRHIDoIt;
	Widget	UxVolScanSettings;
	Widget	UxDemandRHIDoIt1;
	Widget	Uxframe3;
	Widget	UxVolCtl;
	Widget	UxRadarVolOn;
	Widget	UxRadarVolOff;
	Widget	Uxframe4;
	Widget	UxRngRes;
	Widget	UxRngRes2000;
	Widget	UxRngRes1000;
	Widget	UxRngRes500;
	Widget	UxSetAzScanRate;
	Widget	UxAzScanRateScale;
	Widget	UxRdrStCtlHandlerTitle;
	Widget	UxRdrStCtlHandler;
	struct RdrStCtlStruct	*UxRdrStCtlVar;
	void	*Uxcallingwin;
	swidget	UxVolScanWid;
	swidget	UxUxParent;
	void	*UxCallingHndl;
	struct RdrStCtlStruct	*UxStCtlStruct;
} _UxCRadarStatusControl;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRadarStatusControl  *UxRadarStatusControlContext;
#define RadarStatusControl      UxRadarStatusControlContext->UxRadarStatusControl
#define RdrStCtlRdrNameTitle    UxRadarStatusControlContext->UxRdrStCtlRdrNameTitle
#define RdrStCtlRadarName       UxRadarStatusControlContext->UxRdrStCtlRadarName
#define RdrStatusTitle          UxRadarStatusControlContext->UxRdrStatusTitle
#define RdrStCtlConnStTitle     UxRadarStatusControlContext->UxRdrStCtlConnStTitle
#define RdrStCtlRdrStTitle      UxRadarStatusControlContext->UxRdrStCtlRdrStTitle
#define RdrStCtlNextScanTitle   UxRadarStatusControlContext->UxRdrStCtlNextScanTitle
#define RdrStCtlConnSt          UxRadarStatusControlContext->UxRdrStCtlConnSt
#define RdrStCtlRdrSt           UxRadarStatusControlContext->UxRdrStCtlRdrSt
#define RdrStCtlNextScan        UxRadarStatusControlContext->UxRdrStCtlNextScan
#define separator1              UxRadarStatusControlContext->Uxseparator1
#define separator2              UxRadarStatusControlContext->Uxseparator2
#define RdrControlTitle         UxRadarStatusControlContext->UxRdrControlTitle
#define frame1                  UxRadarStatusControlContext->Uxframe1
#define TransmitterCtl          UxRadarStatusControlContext->UxTransmitterCtl
#define RadarTxOn               UxRadarStatusControlContext->UxRadarTxOn
#define RadarTxOff              UxRadarStatusControlContext->UxRadarTxOff
#define frame2                  UxRadarStatusControlContext->Uxframe2
#define ServoCtl                UxRadarStatusControlContext->UxServoCtl
#define RadarServoOn            UxRadarStatusControlContext->UxRadarServoOn
#define RadarServoOff           UxRadarStatusControlContext->UxRadarServoOff
#define DemandPPITitle          UxRadarStatusControlContext->UxDemandPPITitle
#define DemandPPIEl             UxRadarStatusControlContext->UxDemandPPIEl
#define DemandPPIDoIt           UxRadarStatusControlContext->UxDemandPPIDoIt
#define DemandRHITitle          UxRadarStatusControlContext->UxDemandRHITitle
#define DemandRHIAz             UxRadarStatusControlContext->UxDemandRHIAz
#define DemandRHIDoIt           UxRadarStatusControlContext->UxDemandRHIDoIt
#define VolScanSettings         UxRadarStatusControlContext->UxVolScanSettings
#define DemandRHIDoIt1          UxRadarStatusControlContext->UxDemandRHIDoIt1
#define frame3                  UxRadarStatusControlContext->Uxframe3
#define VolCtl                  UxRadarStatusControlContext->UxVolCtl
#define RadarVolOn              UxRadarStatusControlContext->UxRadarVolOn
#define RadarVolOff             UxRadarStatusControlContext->UxRadarVolOff
#define frame4                  UxRadarStatusControlContext->Uxframe4
#define RngRes                  UxRadarStatusControlContext->UxRngRes
#define RngRes2000              UxRadarStatusControlContext->UxRngRes2000
#define RngRes1000              UxRadarStatusControlContext->UxRngRes1000
#define RngRes500               UxRadarStatusControlContext->UxRngRes500
#define SetAzScanRate           UxRadarStatusControlContext->UxSetAzScanRate
#define AzScanRateScale         UxRadarStatusControlContext->UxAzScanRateScale
#define RdrStCtlHandlerTitle    UxRadarStatusControlContext->UxRdrStCtlHandlerTitle
#define RdrStCtlHandler         UxRadarStatusControlContext->UxRdrStCtlHandler
#define RdrStCtlVar             UxRadarStatusControlContext->UxRdrStCtlVar
#define callingwin              UxRadarStatusControlContext->Uxcallingwin
#define VolScanWid              UxRadarStatusControlContext->UxVolScanWid
#define UxParent                UxRadarStatusControlContext->UxUxParent
#define CallingHndl             UxRadarStatusControlContext->UxCallingHndl
#define StCtlStruct             UxRadarStatusControlContext->UxStCtlStruct

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
Widget	popup_RadarStatusControl(swidget, void*, struct RdrStCtlStruct*);
#ifdef __cplusplus
}
#endif

#endif	/* _RADARSTATUSCONTROL_INCLUDED */
