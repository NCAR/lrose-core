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
 *  $Id: SpdbSimIO.hh,v 1.3 2016/03/07 01:39:56 dixon Exp $
 *  $Revision: 1.3 $
 *  $State: Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: SpdbSim

Author: Dave Albo

Date:	Wed Jan 30 17:46:49 2002

Description:	One input/output simulation element

*************************************************************************/

# ifndef    SPDBSIMIO_H
# define    SPDBSIMIO_H

/* System include files / Local include files */
#include <string>
#include <Spdb/DsSpdb.hh>


/* Constant definitions / Macro definitions / Type definitions */

class SpdbSimIO
{
  public:
    // default constructor
    SpdbSimIO();

    SpdbSimIO(string in_url, string out_url, time_t mintime, time_t maxtime);

    // copy constructor
    SpdbSimIO(const SpdbSimIO &s);
  
    // destructor
    virtual ~SpdbSimIO();

    // operator overloading
    void operator=(const SpdbSimIO &s);
    bool operator==(const SpdbSimIO &s) const;

    // return true if more processing to do.
    bool not_done(void) const;

    // return earliest time of next read.
    time_t get_next_read_time(void) const;

    // return time of current data, -1 for none.
    time_t get_current_data_time(void) const;

    // read next data 
    void read(void);

    // write current data.
    void write(void);
  
  private:  
    DsSpdb __input;
    DsSpdb __output;
    string __input_url;
    string __output_url;
    time_t __next_read_min_time;
    time_t __current_data_time;
    time_t __max_time;

    void _set_done(void);
};


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */


# endif     /* SPDBSIM_H */
