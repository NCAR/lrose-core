
/*******************************************************************************
       VolumetricSettings.h
       This header file is included by VolumetricSettings.c

*******************************************************************************/

#ifndef	_VOLUMETRICSETTINGS_INCLUDED
#define	_VOLUMETRICSETTINGS_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
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
	Widget	UxVolumetricSettings;
	Widget	UxtextField2;
	Widget	UxVolScanList;
	Widget	UxscrolledList1;
	Widget	UxVolScanListTitle;
	Widget	UxVolSettingsClose;
	Widget	UxscaleH1;
	Widget	UxDemandPPITitle1;
	Widget	UxtextField3;
	Widget	UxVolSettingsApply;
	Widget	UxVolSettingsOK;
	swidget	UxUxParent;
} _UxCVolumetricSettings;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCVolumetricSettings  *UxVolumetricSettingsContext;
#define VolumetricSettings      UxVolumetricSettingsContext->UxVolumetricSettings
#define textField2              UxVolumetricSettingsContext->UxtextField2
#define VolScanList             UxVolumetricSettingsContext->UxVolScanList
#define scrolledList1           UxVolumetricSettingsContext->UxscrolledList1
#define VolScanListTitle        UxVolumetricSettingsContext->UxVolScanListTitle
#define VolSettingsClose        UxVolumetricSettingsContext->UxVolSettingsClose
#define scaleH1                 UxVolumetricSettingsContext->UxscaleH1
#define DemandPPITitle1         UxVolumetricSettingsContext->UxDemandPPITitle1
#define textField3              UxVolumetricSettingsContext->UxtextField3
#define VolSettingsApply        UxVolumetricSettingsContext->UxVolSettingsApply
#define VolSettingsOK           UxVolumetricSettingsContext->UxVolSettingsOK
#define UxParent                UxVolumetricSettingsContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_VolumetricSettings();

#endif	/* _VOLUMETRICSETTINGS_INCLUDED */
