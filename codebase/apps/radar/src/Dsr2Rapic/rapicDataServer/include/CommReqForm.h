
/*******************************************************************************
       CommReqForm.h
       This header file is included by CommReqForm.c

*******************************************************************************/

#ifndef	_COMMREQFORM_INCLUDED
#define	_COMMREQFORM_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
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
	Widget	UxCommReqForm;
	Widget	UxCommReqStnList;
	Widget	UxCommReqStnScrollList;
	Widget	UxCommReqStnLabel;
	Widget	UxCommReqStnLabel1;
	Widget	UxCommReqProdList;
	Widget	UxCommReqProdScrollList;
	Widget	UxCommReqSend;
	Widget	UxCommReqClose;
	Widget	UxCommReqReqString;
	Widget	UxCommReqTitle;
	Widget	UxpushButton16;
	Widget	UxSchedPeriodScale;
	Widget	UxpushButton17;
	Widget	Uxseparator4;
	Widget	Uxseparator5;
	Widget	UxCommReqSend1;
	Widget	UxSchedOffsetScale;
	int	UxReqStn;
	int	UxReqType;
	swidget	UxUxParent;
} _UxCCommReqForm;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCCommReqForm         *UxCommReqFormContext;
#define CommReqForm             UxCommReqFormContext->UxCommReqForm
#define CommReqStnList          UxCommReqFormContext->UxCommReqStnList
#define CommReqStnScrollList    UxCommReqFormContext->UxCommReqStnScrollList
#define CommReqStnLabel         UxCommReqFormContext->UxCommReqStnLabel
#define CommReqStnLabel1        UxCommReqFormContext->UxCommReqStnLabel1
#define CommReqProdList         UxCommReqFormContext->UxCommReqProdList
#define CommReqProdScrollList   UxCommReqFormContext->UxCommReqProdScrollList
#define CommReqSend             UxCommReqFormContext->UxCommReqSend
#define CommReqClose            UxCommReqFormContext->UxCommReqClose
#define CommReqReqString        UxCommReqFormContext->UxCommReqReqString
#define CommReqTitle            UxCommReqFormContext->UxCommReqTitle
#define pushButton16            UxCommReqFormContext->UxpushButton16
#define SchedPeriodScale        UxCommReqFormContext->UxSchedPeriodScale
#define pushButton17            UxCommReqFormContext->UxpushButton17
#define separator4              UxCommReqFormContext->Uxseparator4
#define separator5              UxCommReqFormContext->Uxseparator5
#define CommReqSend1            UxCommReqFormContext->UxCommReqSend1
#define SchedOffsetScale        UxCommReqFormContext->UxSchedOffsetScale
#define ReqStn                  UxCommReqFormContext->UxReqStn
#define ReqType                 UxCommReqFormContext->UxReqType
#define UxParent                UxCommReqFormContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_CommReqForm();

#endif	/* _COMMREQFORM_INCLUDED */
