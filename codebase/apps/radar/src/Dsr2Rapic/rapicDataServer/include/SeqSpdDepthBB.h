
/*******************************************************************************
       SeqSpdDepthBB.h
       This header file is included by SeqSpdDepthBB.c

*******************************************************************************/

#ifndef	_SEQSPDDEPTHBB_INCLUDED
#define	_SEQSPDDEPTHBB_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Scale.h>
#include <Xm/BulletinB.h>

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
	Widget	UxSeqSpdDepthBB;
	Widget	UxSeqMemScale;
	Widget	UxSeqDepthScale;
	Widget	UxSeqSpdScale;
	Widget	UxSeqMemNow;
	Widget	UxSeqSizeNow;
	Widget	UxpushButton7;
} _UxCSeqSpdDepthBB;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSeqSpdDepthBB       *UxSeqSpdDepthBBContext;
#define SeqSpdDepthBB           UxSeqSpdDepthBBContext->UxSeqSpdDepthBB
#define SeqMemScale             UxSeqSpdDepthBBContext->UxSeqMemScale
#define SeqDepthScale           UxSeqSpdDepthBBContext->UxSeqDepthScale
#define SeqSpdScale             UxSeqSpdDepthBBContext->UxSeqSpdScale
#define SeqMemNow               UxSeqSpdDepthBBContext->UxSeqMemNow
#define SeqSizeNow              UxSeqSpdDepthBBContext->UxSeqSizeNow
#define pushButton7             UxSeqSpdDepthBBContext->UxpushButton7

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_SeqSpdDepthBB();

#endif	/* _SEQSPDDEPTHBB_INCLUDED */
