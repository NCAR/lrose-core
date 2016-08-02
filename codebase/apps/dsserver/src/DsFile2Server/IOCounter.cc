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
/////////////////////////////////////////////////////////////
// IOCOUNTER.cc
//
//
///////////////////////////////////////////////////////////////


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <didss/DsMessage.hh>

#include <dsserver/DsFileCopyMsg.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/DsSpdb.hh>
#include <dsserver/DsFileCopy.hh>
#include <didss/DsInputPath.hh>

#include "IOCounter.hh"

// Constructor
IOCounter::IOCounter()
{
	// Zero out the counter accumulators.
	for (int i = 0; i < 1440; i++) {
		 clear_minute(i);
		 ioc[i].last_update = 0;
	}
}

// Count a message
void IOCounter::count(int msg_type, size_t m_size)
{
  time_t now = time(0);
  int minute = (now / 60) % 1440;
	
  if (ioc[minute].last_update <= now - 86400) { // more than 24hrs ago
	   ioc[minute].last_update = now;
	   clear_minute(minute);
  }

  switch ( msg_type){
    case DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY:
		 ioc[minute].mdv_files++;
		 ioc[minute].mdv_bytes += m_size; 
	break;

	case DsSpdbMsg::DS_MESSAGE_TYPE_SPDB:
		 ioc[minute].spdb_files++;
		 ioc[minute].spdb_bytes += m_size; 
	break;
  }


}

// Count errors
void IOCounter::count_error(void)
{
  time_t now = time(0);
  int minute = (now / 60) % 1440;

   ioc[minute].spdb_files++;
	
}
   
// Write an output summary.
void IOCounter::write_stats(FILE *outfile)
{
	time_t now = time(0);
	int minute = (now / 60) % 1440;

	int lmin = minute -1;
	if(lmin < 0) lmin += 1440;

	//
	// Header
	//
	fprintf(outfile,"\nStatistics for DsServer2File at %s\n",ctime(&now));
	fprintf(outfile,"               MDV Files   Bytes   SPDB Files  Bytes  Total   Errors\n");
	//
	// Prev one minute  stats
	//
	fprintf(outfile,"Prev Minute:   %8d %8d", ioc[lmin].mdv_files, ioc[lmin].mdv_bytes);
	fprintf(outfile," %8d %8d", ioc[lmin].spdb_files, ioc[lmin].spdb_bytes);
	fprintf(outfile," %8d %8d\n",
			ioc[lmin].spdb_bytes + ioc[lmin].mdv_bytes,
			ioc[lmin].tot_errors);
	//
	// Prev five minutes  stats
	// 
	int indx = 0;
	size_t tot_mfiles = 0;
	size_t tot_mbytes = 0;
	size_t tot_sfiles = 0;
	size_t tot_sbytes = 0;
	size_t tot_errors = 0;
	for(int i =0; i < 5; i++) {
		indx = lmin - i;
		if(indx < 0) indx += 1440;
		tot_mbytes +=  ioc[indx].mdv_bytes;
		tot_sbytes +=  ioc[indx].spdb_bytes;
		tot_mfiles +=  ioc[indx].mdv_files;
		tot_sfiles +=  ioc[indx].spdb_files;
		tot_errors +=  ioc[indx].tot_errors;
	} 
	double mbytes1 = tot_mbytes / 1048576.0;
	double mbytes2 = tot_sbytes / 1048576.0;

	if(mbytes1 < 1.0  && mbytes2 < 1.0) {
	    fprintf(outfile,"Prev 5 Minutes:%8d %8d", tot_mfiles, tot_mbytes);
	    fprintf(outfile," %8d %8d", tot_sfiles, tot_sbytes);
	    fprintf(outfile," %8d %8d\n", tot_mbytes + tot_sbytes, tot_errors);
	} else {
	    fprintf(outfile,"Prev 5 Minutes:%8d %6.1f M", tot_mfiles, mbytes1);
	    fprintf(outfile," %8d %6.1f M", tot_sfiles, mbytes2);
	    fprintf(outfile," %6.1f M %8d\n", mbytes1 + mbytes2, tot_errors);
	}
	
	//
	// Last 24Hrs stats
	// 
	indx = 0;
	tot_mfiles = 0;
	tot_mbytes = 0;
	tot_sfiles = 0;
	tot_sbytes = 0;
	tot_errors = 0;
	for(int i =0; i < 1440; i++) {
		tot_mbytes +=  ioc[i].mdv_bytes;
		tot_sbytes +=  ioc[i].spdb_bytes;
		tot_mfiles +=  ioc[i].mdv_files;
		tot_sfiles +=  ioc[i].spdb_files;
		tot_errors +=  ioc[i].tot_errors;
	} 
	mbytes1 = tot_mbytes / 1048576.0;
	mbytes2 = tot_sbytes / 1048576.0;


	if(mbytes1 < 1.0  && mbytes2 < 1.0) {
	    fprintf(outfile,"Last 24 Hours: %8d %8d", tot_mfiles, tot_mbytes);
	    fprintf(outfile," %8d %8d", tot_sfiles, tot_sbytes);
	    fprintf(outfile," %8d %8d\n", tot_mbytes + tot_sbytes, tot_errors);
	} else {
	    fprintf(outfile,"Last 24 Hours: %8d %6.1f M", tot_mfiles, mbytes1);
	    fprintf(outfile," %8d %6.1f M", tot_sfiles, mbytes2);
	    fprintf(outfile," %6.1f M %8d\n", mbytes1 + mbytes2, tot_errors);
	}
}
   
// Zero out one accumulator
void IOCounter::clear_minute(int m)
{
	  // Reset the accumulators for this minute.
	   ioc[m].spdb_bytes = 0;
	   ioc[m].spdb_files = 0;
	   ioc[m].mdv_bytes = 0;
	   ioc[m].mdv_files = 0;
	   ioc[m].tot_errors = 0;
}
// Destructor.

IOCounter::~IOCounter()
{
}
