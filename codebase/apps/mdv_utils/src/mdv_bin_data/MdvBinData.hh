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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:15 $
 *   $Id: MdvBinData.hh,v 1.3 2016/03/04 02:22:15 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvBinData.hh: Program to convert "continuous" float data into
 *                binned byte data.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvBinData_HH
#define MdvBinData_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include "Args.hh"
#include "Params.hh"
#include "FileRetriever.hh"
using namespace std;


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

class MdvBinData
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  ////////////////////
  // Public methods //
  ////////////////////

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  // Destructor

  ~MdvBinData(void);
  
  // Get MdvBinData singleton instance

  static MdvBinData *Inst(int argc, char **argv);
  static MdvBinData *Inst();
  

  ///////////////////////////
  // Miscellaneous methods //
  ///////////////////////////

  // Run the program.

  void run();
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvBinData *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Object for retrieving files to process

  FileRetriever *_fileRetriever;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  MdvBinData(int argc, char **argv);
  

  // Create an output field based on the given input field.

  MdvxField *_createOutputField(const MdvxField& input_field);
  

  // Process the given input file.

  void _processFile(const DsMdvx& input_file);
  

  // Set the master header fields in the output file based on those in
  // the input file.

  void _setMasterHeaderFields(const Mdvx::master_header_t& input_hdr,
			      const string& data_set_source,
			      DsMdvx& output_file);

  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MdvBinData");
  }
  
};


#endif
