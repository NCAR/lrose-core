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
// IOCOUNTER.hh
//
// (C) 2005 RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA 
//
// November 2005
// Frank Hage 
//
///////////////////////////////////////////////////////////////

#ifndef IOCounter_H
#define IOCounter_H

typedef struct  {
	size_t	   spdb_bytes;
	size_t    spdb_files;
	size_t	   mdv_bytes;
	size_t    mdv_files;
	size_t    tot_errors;
    time_t last_update;
} IOC_t;


class IOCounter {

public:
  
  // constructor
   IOCounter();

   // Count this message
   void count(int msg_type, size_t m_size);

   // Count an error condition.
   void count_error();

   // Output status and statistics to this file
  void  write_stats(FILE *outfile);

   // Statistics accumulation happens here.
   IOC_t ioc[1440];  // One record for each minute of the day.

 // Destructor
   ~IOCounter();
protected:
private:
   // Clear a minute accumulator
   void clear_minute(int m);

};


#endif
