
/*******************************************************************************
       NavigationDataForm.h
       This header file is included by NavigationDataForm.c

*******************************************************************************/

#ifndef	_NAVIGATIONDATAFORM_INCLUDED
#define	_NAVIGATIONDATAFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
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
	Widget	UxCursorDataForm;
	Widget	Uxlabel30;
	Widget	UxdBZLabel;
	Widget	UxdBZText;
	Widget	UxRLabel;
	Widget	UxRText;
	Widget	UxLatLongLabel;
	Widget	UxHtLabel;
	Widget	UxHtUnitLabel;
	Widget	UxLatLongText;
	Widget	UxhtText;
	Widget	UxRUnitLabel;
	Widget	UxrngbrngLabel;
	Widget	UxDistNELabel;
	Widget	UxDistNEText;
	Widget	UxRngBrngText;
	Widget	UxRngBrgUnitLabel;
	Widget	UxHtUnitLabel2;
	Widget	Uxseparator11;
	Widget	UxUnitMode;
	Widget	UxUnitsMetric;
	Widget	UxUnitsImperial;
	Widget	UxUnitsNautical;
	Widget	UxoptionMenu1;
	Widget	UxCursDistMode1;
	Widget	UxDistFromRadar;
	Widget	UxDistFromRef;
	Widget	UxoptionMenu2;
	Widget	Uxseparator12;
	Widget	UxpushButton26;
	int	Uxvalmode;
	int	Uxdistmode;
	struct CursorDataStruct	*UxCursorData;
	void	*UxCallingWin;
	swidget	UxUxParent;
	void	*Uxcallingwin;
	struct CursorDataStruct	*Uxcursordata;
} _UxCCursorDataForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCCursorDataForm      *UxCursorDataFormContext;
#define CursorDataForm          UxCursorDataFormContext->UxCursorDataForm
#define label30                 UxCursorDataFormContext->Uxlabel30
#define dBZLabel                UxCursorDataFormContext->UxdBZLabel
#define dBZText                 UxCursorDataFormContext->UxdBZText
#define RLabel                  UxCursorDataFormContext->UxRLabel
#define RText                   UxCursorDataFormContext->UxRText
#define LatLongLabel            UxCursorDataFormContext->UxLatLongLabel
#define HtLabel                 UxCursorDataFormContext->UxHtLabel
#define HtUnitLabel             UxCursorDataFormContext->UxHtUnitLabel
#define LatLongText             UxCursorDataFormContext->UxLatLongText
#define htText                  UxCursorDataFormContext->UxhtText
#define RUnitLabel              UxCursorDataFormContext->UxRUnitLabel
#define rngbrngLabel            UxCursorDataFormContext->UxrngbrngLabel
#define DistNELabel             UxCursorDataFormContext->UxDistNELabel
#define DistNEText              UxCursorDataFormContext->UxDistNEText
#define RngBrngText             UxCursorDataFormContext->UxRngBrngText
#define RngBrgUnitLabel         UxCursorDataFormContext->UxRngBrgUnitLabel
#define HtUnitLabel2            UxCursorDataFormContext->UxHtUnitLabel2
#define separator11             UxCursorDataFormContext->Uxseparator11
#define UnitMode                UxCursorDataFormContext->UxUnitMode
#define UnitsMetric             UxCursorDataFormContext->UxUnitsMetric
#define UnitsImperial           UxCursorDataFormContext->UxUnitsImperial
#define UnitsNautical           UxCursorDataFormContext->UxUnitsNautical
#define optionMenu1             UxCursorDataFormContext->UxoptionMenu1
#define CursDistMode1           UxCursorDataFormContext->UxCursDistMode1
#define DistFromRadar           UxCursorDataFormContext->UxDistFromRadar
#define DistFromRef             UxCursorDataFormContext->UxDistFromRef
#define optionMenu2             UxCursorDataFormContext->UxoptionMenu2
#define separator12             UxCursorDataFormContext->Uxseparator12
#define pushButton26            UxCursorDataFormContext->UxpushButton26
#define valmode                 UxCursorDataFormContext->Uxvalmode
#define distmode                UxCursorDataFormContext->Uxdistmode
#define CursorData              UxCursorDataFormContext->UxCursorData
#define CallingWin              UxCursorDataFormContext->UxCallingWin
#define UxParent                UxCursorDataFormContext->UxUxParent
#define callingwin              UxCursorDataFormContext->Uxcallingwin
#define cursordata              UxCursorDataFormContext->Uxcursordata

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_CursorDataForm();

#endif	/* _NAVIGATIONDATAFORM_INCLUDED */
