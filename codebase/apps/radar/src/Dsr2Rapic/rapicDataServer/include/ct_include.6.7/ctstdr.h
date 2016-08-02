#ifndef ctSTDRH
#define ctSTDRH		

#ifndef ctOPT2H		/* Special ctoptn.h & ctopt2.h include controls */
#define ctOPT2H 	/* Prevent ctopt2.h header in initial ctoptn.h */
#include "ctoptn.h"	/* Compiler Options Header */
#undef	ctOPT2H    	/* Allow ctopt2.h for traditianal ctoptn.h inclusion */
#endif

#include "ctcmpl.h" /* Compiler-Operating System Specific definitions */
#include "ctport.h" /* System independent type definitions */

#ifdef ctPortFAIRCOM_THREADS	/* FairCom's Proprietary Thread Manager */
#include "ctthrd.h"
#endif

#endif /* ctSTDRH */
