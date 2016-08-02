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
 *  $Id: SpdbMultSim.cc,v 1.5 2016/03/07 01:39:56 dixon Exp $
 *  $Revision: 1.5 $
 *  $State: Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: SpdbMultSim.cc,v 1.5 2016/03/07 01:39:56 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Module:	SpdbMultSim

Author:	Dave Albo

Date:	Wed Jan 30 17:33:01 2002

Description: 
             Simulating multiple spdb writes from a set of inputs

************************************************************************/



/* System include files / Local include files */
#include <stdio.h>
#include <unistd.h>
#include <SpdbMultSim.hh>
#include <toolsa/DateTime.hh>

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
# ifndef    lint
void SpdbMultSim_rcsprint(void)
{
    printf("rcsid=%s\n", RCSid);
}
# endif     /* not lint */
  
/*----------------------------------------------------------------*/
// default constructor
SpdbMultSim::SpdbMultSim()
{
    __S = new vector<SpdbSimIO>();
    __t0 = __t1 = __last_output_time = -1;
}


/*----------------------------------------------------------------*/
  // main constructor
SpdbMultSim::SpdbMultSim(int argc, char **argv)
{
    int i;
    SpdbSimIO *s;
    DateTime *time0, *time1;

    __S = new vector<SpdbSimIO>();
    __t0 = __t1 = __last_output_time = -1;
  
    if (!_parse_args(argc, argv))
        return;

    // set up time range
    time0 = new DateTime(__P._time0[0], __P._time0[1],
			 __P._time0[2], __P._time0[3], 
			 __P._time0[4], __P._time0[5]);
    time1 = new DateTime(__P._time1[0], __P._time1[1],
			 __P._time1[2], __P._time1[3], 
			 __P._time1[4], __P._time1[5]);
    __t0 = time0->utime();
    __t1 = time1->utime();
    delete time0;
    delete time1;

    // Add an item for each parameterized url pair.
    for (i=0; i<__P.element_n; ++i)
    {
        s = new SpdbSimIO(__P._element[i].in_url,
			  __P._element[i].out_url,
			  __t0, __t1);
	__S->push_back(*s);
	delete s;
    }
}

/*----------------------------------------------------------------*/
// destructor
SpdbMultSim::~SpdbMultSim()
{
    delete __S;
}


/*----------------------------------------------------------------*/
// return true if there is any more processing to do.
bool SpdbMultSim::not_done(void) const
{
    vector <SpdbSimIO>::iterator i;

    for (i=__S->begin(); i!= __S->end(); ++i)
    {
        if (i->not_done())
	    return true;
    }
    return false;
}


/*----------------------------------------------------------------*/
// do the next processing step.
void SpdbMultSim::update(void)
{
    time_t oldest_t;
    int sleepsec;
  
    // determine time of oldest possible next read.
    if ((oldest_t = _oldest_time(true)) == -1)
        return;
    
    // read in new Spdb data for all items at this oldest time.
    _read(oldest_t);
    
    // get oldest time of all current data
    if ((oldest_t = _oldest_time(false)) == -1)
        return;

    // sleep to simulate real-time
    if (__last_output_time != -1)
    {
        sleepsec = oldest_t - __last_output_time;
        sleepsec = (int)( (double)sleepsec/__P.speedup );
	printf("Sleeping %d seconds\n", sleepsec);
	sleep(sleepsec);
    }

    // write Spdb data at oldest time.
    _write(oldest_t);
    __last_output_time = oldest_t;
}


/*----------------------------------------------------------------*/
bool SpdbMultSim::_parse_args(int argc, char **argv)
{
    tdrp_override_t override;
    char *name;
    int i;
    for (i=1; i<argc; ++i)
    {
	if (!strcmp(argv[i], "--")    || !strcmp(argv[i], "-h") ||
	    !strcmp(argv[i], "-help") || !strcmp(argv[i], "-usage") ||
	    !strcmp(argv[i], "-man"))
	{
	    TDRP_usage(stdout);
	    return false;
	}
    }

    TDRP_init_override(&override);

    // read algorithm parms.
    if (__P.loadFromArgs(argc, argv, override.list, &name) == -1)
    {
        fprintf(stderr, "Problems with params file\n");
	return false;
    }
    
    TDRP_free_override(&override);
    return true;
}

/*----------------------------------------------------------------*/
// figure out which spdb(s) have oldest time last read in,
// or last written.
time_t SpdbMultSim::_oldest_time(bool next_read) const
{
    time_t oldest_t, t;
    bool first;
    vector <SpdbSimIO>::iterator i;
    
    oldest_t = -1;
    first = true;
    for (i=__S->begin(); i!= __S->end(); ++i)
    {
        if (i->not_done())
	{
	    if (next_read)
	        t = i->get_next_read_time();
	    else
	    {
	        t = i->get_current_data_time();
		if (t == -1)
		    continue;
	    }
	    if (first)
	    {
	        oldest_t = t;
		first = false;
	    }
	    else
	    {
	        if (t < oldest_t)
		    oldest_t = t;
	    }
	}
    }
    return oldest_t;
}

/*----------------------------------------------------------------*/
// read spdb's that have current_time = t.
void  SpdbMultSim::_read(time_t t)
{
    vector <SpdbSimIO>::iterator i;

    // for all spdbs at oldest time, read in something new.
    for (i=__S->begin(); i!= __S->end(); ++i)
    {
        if (i->not_done())
	{
	    if (i->get_next_read_time() != t)
	        continue;

	    // get the next data.
	    i->read();
	}
    }
}

/*----------------------------------------------------------------*/
// write spdb's that have current_time = t.
void  SpdbMultSim::_write(time_t t)
{
    vector <SpdbSimIO>::iterator i;
    
    // for all spdbs at oldest time, read in something new.
    for (i=__S->begin(); i!= __S->end(); ++i)
    {
        if (i->not_done())
	{
	    if (i->get_current_data_time() != t)
	        continue;

	    // write the current data.
	    i->write();
	}
    }
}

