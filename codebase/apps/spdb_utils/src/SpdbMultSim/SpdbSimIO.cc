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
 *  $Id: SpdbSimIO.cc,v 1.4 2016/03/07 01:39:56 dixon Exp $
 *  $Revision: 1.4 $
 *  $State: Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: SpdbSimIO.cc,v 1.4 2016/03/07 01:39:56 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Module:	SpdbSimIO

Author:	Dave Albo

Date:	Wed Jan 30 17:51:01 2002

Description: 
             One element Spdb simulation input/output.

************************************************************************/



/* System include files / Local include files */
#include <stdio.h>
#include <SpdbSimIO.hh>
#include <toolsa/DateTime.hh>

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
# ifndef    lint
void SpdbSimIO_rcsprint(void)
{
    printf("rcsid=%s\n", RCSid);
}
# endif     /* not lint */
  

/*----------------------------------------------------------------*/
// default constructor
SpdbSimIO::SpdbSimIO()
{
    __input.clearUrls();
    __output.clearUrls();
    __output.setPutMode(Spdb::putModeAdd);
    __next_read_min_time = -1;
    __current_data_time = -1;
    __max_time = -1;
}

/*----------------------------------------------------------------*/
// main constructor
SpdbSimIO::SpdbSimIO(string in_url, string out_url, time_t mint,
		     time_t maxt)
{
    __input.clearUrls();
    __output.clearUrls();
    __output.setPutMode(Spdb::putModeAdd);
    __input_url = in_url;
    __output_url = out_url;
    __input.addUrl(in_url.c_str());
    __output.addUrl(out_url.c_str());
    __next_read_min_time = mint;
    __current_data_time = -1;
    __max_time = maxt;
}


/*----------------------------------------------------------------*/
// copy constructor
SpdbSimIO::SpdbSimIO(const SpdbSimIO &s)
{
    __input = s.__input;
    __output = s.__output;
    __input_url = s.__input_url;
    __output_url = s.__output_url;
    __next_read_min_time = s.__next_read_min_time;
    __current_data_time = s.__current_data_time;
    __max_time = s.__max_time;
}

/*----------------------------------------------------------------*/
// destructor
SpdbSimIO::~SpdbSimIO()
{
}

/*----------------------------------------------------------------*/
// operator overloading
void SpdbSimIO::operator=(const SpdbSimIO &s)
{
    __input = s.__input;
    __output = s.__output;
    __input_url = s.__input_url;
    __output_url = s.__output_url;
    __next_read_min_time = s.__next_read_min_time;
    __current_data_time = s.__current_data_time;
    __max_time = s.__max_time;
}

/*----------------------------------------------------------------*/
bool SpdbSimIO::operator==(const SpdbSimIO &s) const
{
    return (__input_url == s.__input_url &&
	    __output_url == s.__output_url &&
	    __next_read_min_time == s.__next_read_min_time &&
	    __current_data_time == s.__current_data_time &&
	    __max_time == s.__max_time);
}

/*----------------------------------------------------------------*/
// return true if more processing to do.
bool SpdbSimIO::not_done(void) const
{
    return (__next_read_min_time <= __max_time) && (__max_time != -1);
}

/*----------------------------------------------------------------*/
// return earliest time of next read.
time_t SpdbSimIO::get_next_read_time(void) const
{
    return __next_read_min_time;
}

/*----------------------------------------------------------------*/
// return time of current data, -1 for none.
time_t SpdbSimIO::get_current_data_time(void) const
{
    return __current_data_time;
}

/*----------------------------------------------------------------*/
// read next data 
void SpdbSimIO::read(void)
{
   Spdb::chunk_ref_t *Hdrs;
   int n;
   
   // get first possible data >= __next read min time.
   if (__input.getFirstAfter(__input_url, __next_read_min_time, 
			     __max_time - __next_read_min_time,
			     0, 0))
   {
       // no more data.
       _set_done();
       return;
   }
   
   n = __input.getNChunks();
   if (n <= 0)
   {
       // a problems.
       _set_done();
       return;
   }
      
   Hdrs = __input.getChunkRefs();
   __current_data_time = Hdrs[0].valid_time;
   __next_read_min_time = __current_data_time + 1;
}

/*----------------------------------------------------------------*/
// write current data 
void SpdbSimIO::write(void)
{
    int n, id;
    Spdb::chunk_ref_t *Hdrs;
    char *Buffer;
    string label;
    DateTime *t;
    
    // make sure have some chunk data to write.
    if (__current_data_time == -1)
        return;
    n = __input.getNChunks();
    if (n <= 0)
        return;

    t = new DateTime(__current_data_time);
    printf("%s---Adding %d chunks to %s\n", t->dtime(), n, 
	   __output_url.c_str());
    delete t;

    // copy the input chunks to output.
    id = __input.getProdId();
    label = __input.getProdLabel();
    Hdrs = __input.getChunkRefs();
    Buffer = (char *)__input.getChunkData();

    __output.clearPutChunks();
    __output.addPutChunks(n, Hdrs, Buffer);
    __output.put(__output_url, id, label);

    // now that it has been written, don't want to write again.
    __current_data_time = -1;
}


/*----------------------------------------------------------------*/
void SpdbSimIO::_set_done(void)
{
    __next_read_min_time = __max_time + 1;
    __current_data_time = -1;
}

