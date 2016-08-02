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
/************************************************************************
 * StratiformFilter.hh : header file for the StratiformFilter program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef StratiformFilter_HH
#define StratiformFilter_HH

/*
 **************************** includes **********************************
 */

#include <vector>
#include <sys/time.h>

#include <euclid/SimpleGrid.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxInput.hh>

#include "Args.hh"
#include "ConvPartition.hh"
#include "Params.hh"

/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class StratiformFilter
{
 public:

  ///////////////////////////////////////
  // Public constructors & destructors //
  ///////////////////////////////////////

  // Destructor

  ~StratiformFilter(void);
  
  // Get StratiformFilter singleton instance

  static StratiformFilter *Inst(int argc, char **argv);
  static StratiformFilter *Inst();
  

  ////////////////
  // Run method //
  ////////////////

  // Run the program.

  void run();
  

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static StratiformFilter *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Input file retriever object

  DsMdvxInput _fileRetriever;
  
  // Convective partitioner

  ConvPartition *_convPartition;
  
  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  StratiformFilter(int argc, char **argv);
  
  // Create the partitioning object

  bool _createPartitioner(void);
  
  // Construct the output file and write it to the appropriate directory.

  void _generateOutput(DsMdvx &input_file,
		       ui08 *partition,
		       fl32 *partitioned_data,
		       fl32 *mean_data);
  
  // Apply the given partition to the data.

  SimpleGrid<fl32> *_partitionData(const MdvxField &field,
				   ui08 *partition,
				   const ui08 partition_value);
  
  // Read the next file to be processed.  In realtime mode, blocks until
  // a new file is available.
  //
  // Returns true if successful, false otherwise.

  bool _readNextFile(DsMdvx &mdv_file);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("StratiformFilter");
  }
  
};


#endif
