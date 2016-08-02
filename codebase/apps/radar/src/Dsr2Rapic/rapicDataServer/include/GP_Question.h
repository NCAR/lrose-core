
/*******************************************************************************
       GP_Question.h
       This header file is included by GP_Question.c

*******************************************************************************/

#ifndef	_GP_QUESTION_INCLUDED
#define	_GP_QUESTION_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/MessageB.h>

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
	Widget	UxGP_Question;
	void	(*UxOKCallBack)();
	char	UxTitleStr[32];
	char	UxMssgStr[64];
	swidget	UxUxParent;
	char	*Uxtitlestr;
	char	*Uxmssgstr;
	void	(*Uxokcallback)();
} _UxCGP_Question;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCGP_Question         *UxGP_QuestionContext;
#define GP_Question             UxGP_QuestionContext->UxGP_Question
#define OKCallBack              UxGP_QuestionContext->UxOKCallBack
#define TitleStr                UxGP_QuestionContext->UxTitleStr
#define MssgStr                 UxGP_QuestionContext->UxMssgStr
#define UxParent                UxGP_QuestionContext->UxUxParent
#define titlestr                UxGP_QuestionContext->Uxtitlestr
#define mssgstr                 UxGP_QuestionContext->Uxmssgstr
#define okcallback              UxGP_QuestionContext->Uxokcallback

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_GP_Question();

#endif	/* _GP_QUESTION_INCLUDED */
