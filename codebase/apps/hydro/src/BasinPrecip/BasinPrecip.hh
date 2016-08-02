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
// BasinPrecip.hh
//
// BasinPrecip object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
// Modified by Alex Baia, August 2000
//
///////////////////////////////////////////////////////////////

#ifndef BasinPrecip_H
#define BasinPrecip_H

#include <vector>

#include <rapformats/GenPt.hh>
#include <euclid/WorldPoint2D.hh>
#include <hydro/BasinList.hh>
#include <hydro/Basin.hh>
#include <Spdb/DsSpdb.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvxInput.hh>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <fstream>

#include "Args.hh"
#include "Params.hh"
using namespace std;

// Define output file for writing statistics

#define out_file "BasinPrecip.stats"


class BasinPrecip
{
  
public:

  // constructor

  BasinPrecip(int argc, char **argv);

  // destructor
  
  ~BasinPrecip();

  // Run the BasinPrecip program.
  //
  // Returns the return code for the program (0 if successful, error code
  // otherwise).

  int Run();

  // data members

  bool OK;

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  // Program parameters

  char *_progName;
  char *_paramsPath;
  Args *_args;
  Params *_params;

  // Masks representing the basins

  //  vector< Basin* > _basins;
  
  // Input file retriever object

  DsMdvxInput _fileRetriever;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Read the next file to be processed.  In realtime mode, blocks until
  // a new file is available.
  //
  // Returns true if successful, false otherwise.

  bool _readNextFile(DsMdvx &mdv_file);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("BasinPrecip");
  }


  // Returns the average value, within the basin, of the array 
  // passed in, or -9999 if there are no values in the basin.
  //
  // BasinAverage functions is passed an array of data, a
  // mask defining the basin, max and min x and y coordinates
  // which define a rectangular basin region, and a grid width, nx.

  double BasinAverage(fl32 *data,
		      const fl32 bad_data_value,
		      const fl32 missing_data_value,
		      unsigned char *mask, 
		      int min_x, int max_x,
		      int min_y, int max_y, int nx);


  // Returns the standard deviation, within the basin, of the array 
  // passed in, or -9999 if there are either no values or one value
  // in the basin.
  //
  // BasinSDeviation functions is passed an array of data, a
  // mask defining the basin, max and min x and y coordinates
  // which define a rectangular basin region, and a grid width, nx.

  double BasinSDeviation(fl32 *data,
			 const fl32 bad_data_value,
			 const fl32 missing_data_value,
			 unsigned char *mask, 
			 int min_x, int max_x,
			 int min_y, int max_y, int nx);


  // Calls average, standard deviation functions.
  // Results are printed to an output file called  
  // "BasinTest.stats", which is defined in the
  // BasinTest header file.

  void PrintStats(fl32 *data,
		  const fl32 bad_data_value, const fl32 missing_data_value,
		  unsigned char *mask, ofstream &sds,
       		  int min_x, int max_x, 
		  int min_y, int max_y, int nx, 
		  Mdvx input_file, int basin_num);
  
};

#endif
