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
 *   $Author: hardt $
 *   $Locker:  $
 *   $Date: 2018/05/22 18:24:45 $
 *   $Id: Affirm.hh,v 1.8 2018/05/22 18:24:45 hardt Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Affirm.hh : header file for the Affirm program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2000
 *
 * Dan Megenhardt
 *
 ************************************************************************/

#ifndef Affirm_HH
#define Affirm_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <Mdv/DsMdvx.hh>
#include <dsdata/DsTrigger.hh>

#include "Args.hh"
#include "Params.hh"
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

class Affirm
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

  // Destructor

  ~Affirm(void);
  
  // Get Affirm singleton instance

  static Affirm *Inst(int argc, char **argv);
  static Affirm *Inst(void);
  
  // Run the program.

  void run(void);
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const double GRID_DELTA_TOL;
  
  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Affirm *_instance;
  
  // Constant value used to signal missing data.

  static const double MISSING_DATA_VALUE;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Latest read input file in realtime mode.

  time_t _lastRealtimeFileTime;

  // trigger
  DsTrigger *_trigger;

  // grid delta tolerance
  double _gridDeltaTol;

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  Affirm(int argc, char **argv);
  
  
  // Create the convergence field object based on the given field object.
  //
  // Returns a pointer to the created field.

  MdvxField *_createAffirmField(const MdvxField& field) const;
  
  // Process the given MDV file.  Note that because of the way the data
  // is read in, we know that the U components are found in field 0 and
  // the V components are found in field 1.

    void _processFile(const DsMdvx& input_file) const;
    void _processFile(const DsMdvx& first_input_file,
		      const DsMdvx& sec_input_file) const;
  
  // Read the indicated MDV file.
  //
  // Returns a pointer to the read Mdv file on success, 0 on failure.

  DsMdvx *_readFile(const time_t file_time,
		    const char* input_url,
		    const int field_num,
		    const char * field_name,
		    const int level_num,
		    const int search_margin = 0) const;
  
  // Run the algorithm in ARCHIVE mode.

  void _runArchive(void);
  
  // Run the algorithm in REALTIME mode.

  void _runRealtime(void);

  void _remap(DsMdvx *mdvx) const;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Affirm");
  }
  
};


#endif
