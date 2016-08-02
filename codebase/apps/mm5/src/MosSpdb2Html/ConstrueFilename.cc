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
//
// Construes the filename, which is returned in FileName.
//

#include <cstdio>
#include <toolsa/umisc.h>
#include <Spdb/Spdb.hh>

#include "ConstrueFilename.hh"
using namespace std;

void ConstrueFilename(const time_t validTime, 
		      const time_t leadTime, 
		      const int dataType,
		      const string outDir,
		      const bool archiveMode,
		      string  &FileName){

  date_time_t valid, generate;

  valid.unix_time = validTime;
  uconvert_from_utime( &valid );

  generate.unix_time = validTime - leadTime;
  uconvert_from_utime( &generate );


  string stationID = Spdb::dehashInt32To4Chars(dataType);


  char fName[MAX_PATH_LEN];

 if (archiveMode){
    sprintf(fName,"%4d%02d%02d_%02d%02d/%s_%4d%02d%02d_%02d%02d.html",
	    generate.year, generate.month, generate.day,
	    generate.hour, generate.min,
	    stationID.c_str(),
	    valid.year, valid.month, valid.day,
	    valid.hour, valid.min);

 } else {

   sprintf(fName,"%4d%02d%02d/%s_%02d%02d%02d.html",
	   valid.year, valid.month, valid.day,
	   stationID.c_str(),
	   valid.hour, valid.min, valid.sec);


 }

 FileName = outDir + PATH_DELIM + fName;


}


