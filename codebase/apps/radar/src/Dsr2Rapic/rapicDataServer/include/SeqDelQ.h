
/*******************************************************************************
       SeqDelQ.h
       This header file is included by SeqDelQ.c

*******************************************************************************/

#ifndef	_SEQDELQ_INCLUDED
#define	_SEQDELQ_INCLUDED


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
	Widget	UxSeqDelQ;
} _UxCSeqDelQ;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCSeqDelQ             *UxSeqDelQContext;
#define SeqDelQ                 UxSeqDelQContext->UxSeqDelQ

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	SeqDel_popup();

#endif	/* _SEQDELQ_INCLUDED */
