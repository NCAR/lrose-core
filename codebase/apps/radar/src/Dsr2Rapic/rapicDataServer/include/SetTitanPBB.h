
/*******************************************************************************
       SetTitanPBB.h
       This header file is included by SetTitanPBB.c

*******************************************************************************/

#ifndef	_SETTITANPBB_INCLUDED
#define	_SETTITANPBB_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Scale.h>
#include <Xm/BulletinB.h>
#include "UxXt.h"
#include "titanTime.h"

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
	Widget	UxSetTitanPBB;

        titanOptions *pastStormOpt;
        titanOptions *currentStormOpt;
        titanOptions *futureStormOpt;
        titanOptions *forecastStormOpt;
        titanOptions *pastForecastStormOpt;
        titanOptions *plotForecast;
        titanOptions *plotPast;
	titanOptions *plotPastForecast;
        titanOptions *plotFuture;
        titanOptions *plotTracks;
        titanOptions *annotateTracks;
        titanOptions *trackDisplay;
        titanOptions *trackAnnotate;
	titanOptions *nForecastSteps;
	titanOptions *nTitanMode;
	titanTime    *nTitanStarttime;
/**
	Widget	     labTitanStarttime;
	Widget	     txtTitanStarttime;
**/
        Widget  UxpbClose;
} _UxCSetTitanPBB;

#ifdef CONTEXT_MACRO_ACCESS

static _UxCSetTitanPBB       *UxSetTitanPBBContext;

#define SetTitanPBB             UxSetTitanPBBContext->UxSetTitanPBB
#define PBClose                 UxSetTitanPBBContext->UxpbClose

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Widget	popup_SetTitanPBB();
#ifdef __cplusplus
        }
#endif

#endif	/* _SETTITANPBB_INCLUDED */
