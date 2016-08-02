// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/umisc.h>

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 * RCS info
 *  $Author: dixon $
 *  $Locker:  $
 *  $Date: 2016/03/07 01:39:56 $
 *  $Id: Driver.cc,v 1.4 2016/03/07 01:39:56 dixon Exp $
 *  $Revision: 1.4 $
 *  $State: Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: Driver.cc,v 1.4 2016/03/07 01:39:56 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Module:	Driver

Author:	Dave Albo

Date:	Wed Jan 30 17:33:01 2002

Description: 
             Driver for Simulating multiple spdb writes from a set of inputs

************************************************************************/



/* System include files / Local include files */
#include <vector>
#include <stdio.h>
#include <SpdbMultSim.hh>

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
# ifndef    lint
void SpdbSimDriver_rcsprint(void)
{
    printf("rcsid=%s\n", RCSid);
}
# endif     /* not lint */
  
/*----------------------------------------------------------------*/
int main(int argc, char **argv)
{
    SpdbMultSim *s;
  
    // create the object
    s = new SpdbMultSim(argc, argv);

    // do the processing.
    while (s->not_done())
        s->update();

    // done.
    delete s;
    printf("Normal exit\n");
    exit(0);
}
