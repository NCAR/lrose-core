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
 *  $Id: SpdbMultSim.hh,v 1.3 2016/03/07 01:39:56 dixon Exp $
 *  $Revision: 1.3 $
 *  $State: Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: SpdbMultSim

Author: Dave Albo

Date:	Thu Jan 31 09:11:32 2002

Description:	Simulation of real time Spdb data writing, multiple Spdbs

*************************************************************************/

# ifndef    SPDBMULTSIM_H
# define    SPDBMULTSIM_H

/* System include files / Local include files */
#include <vector>
#include <Params.hh>
#include <SpdbSimIO.hh>

/* Constant definitions / Macro definitions / Type definitions */

class SpdbMultSim
{
public:

  // default constructor
  SpdbMultSim();

  // main constructor
  SpdbMultSim(int argc, char **argv);

  // destructor
  virtual ~SpdbMultSim();

  // return true if there is any more processing to do.
  bool not_done(void) const;
  
  // do the next processing step.
  void update(void);
  
private:  

  Params __P;                  // parameters
  vector <SpdbSimIO> *__S;     // input/output Spdb simulators.
  time_t __last_output_time;   // time of last Spdb output.
  time_t __t0, __t1;           // full range of times to simulate.

  bool _parse_args(int argc, char **argv);
  time_t _oldest_time(bool read) const;
  void _read(time_t t);
  void _write(time_t t);
};


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

# endif     /* SPDBMULTSIM_H */
