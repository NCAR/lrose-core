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
#include <string>
#include <vector>

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>

#include "Bin.hh"
#include "Params.hh"

using namespace std;

class Process {

public:

  //
  // Constructor
  //
  Process(Params *params);


  //
  // Main method - run.
  //
  int Derive(Params *params, time_t T, int leadTime = 0);


  //
  // Read the input field
  //
  MdvxField *readField(const Params *params,
		       const time_t trigger_time,
		       const int leadTime,
		       time_t &gen_time);
  

  //
  // Get ith total count.
  //
  int getCount(int index);


  //
  // Output total counts.
  //
  void outputTotalCounts(Params *params,
			 DateTime startTime,
			 DateTime endTime);


  //
  // Output individual counts.
  //
  bool outputIndividualCountsPlain(const string &output_path,
				   const DateTime &start_time,
				   const int nGood,
				   const int numOutsideBins,
				   vector< Bin > &bin);
  
  bool outputIndividualCountsFormatted(const string &output_path,
				       const DateTime &start_time,
				       const int nGood,
				       const int numOutsideBins,
				       vector< Bin > &bin);
  

  //
  // Destructor
  //
  ~Process();

  private :

  vector< Bin >_totalCounts;
  vector< Bin >_individualCounts;
  
  string _constructIndividualPath(const Params *params,
				  const time_t genTimeUnix,
				  const int leadTime,
				  const date_time_t startTime);
  
  void _initCounts(const Params *params,
		   vector< Bin > &counts);
  
};


