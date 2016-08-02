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
 *   $Date: 2016/03/07 18:17:27 $
 *   $Id: RefractParams2Tdrp.hh,v 1.5 2016/03/07 18:17:27 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * RefractParams2Tdrp: RefractParams2Tdrp program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef RefractParams2Tdrp_HH
#define RefractParams2Tdrp_HH

#include <iostream>
#include <string>

#include <tdrp/tdrp.h>
#include <toolsa/str.h>

#include "Args.hh"

using namespace std;


class RefractParams2Tdrp
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

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~RefractParams2Tdrp(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static RefractParams2Tdrp *Inst(int argc, char **argv);
  static RefractParams2Tdrp *Inst();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static RefractParams2Tdrp *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  RefractParams2Tdrp(int argc, char **argv);
  

  /*********************************************************************
   * _getKeyVal() - Get the keyword value from the parameter file line.
   */

  static void _getKeyVal(char *line, string &keyword, string &value);
  

  /*********************************************************************
   * _printUnusedParamWarning() - Print the unused parameter warning
   *                              message.
   */

  void _printUnusedParamWarning(const string &keyword) const
  {
    if (_args->debug)
    {
      cerr << "WARNING: " << keyword
	   << " specified in original param file" << endl;
      cerr << "This parameter is not used in the TDRP param file" << endl;
      cerr << "Parameter skipped" << endl << endl;
    }
  }


  /*********************************************************************
   * _processParams() - Process the parameters in the given parameter
   *                    file.
   *
   * Returns true on success, false on failure.
   */

  bool _processParams(const string &params_file);
  

  /*********************************************************************
   * _parseBoolean() - Parse a boolean value from the input parameter file.
   *
   * Returns true if the flag is true, false otherwise.
   */

  static bool _parseBoolean(const string &value)
  {
    if (value == "yes" || value == "Yes" || value == "YES" ||
	value == "y" || value == "Y" ||
	value == "true" || value == "True" || value == "TRUE" ||
	value == "t" || value == "T")
      return true;
    
    return false;
  }

  
  /*********************************************************************
   * _writeTdrpFile() - Write the given parameters to the given TDRP file.
   *
   * Returns true on success, false on failure.
   */

  template <class T>
  bool _writeTdrpFile(T tdrp_params,
		      const string &tdrp_buffer,
		      const string &tdrp_file_path)
  {
    static const string method_name = "RefractParams2Tdrp::_writeTdrpFile()";
  
    tdrp_params.loadFromBuf("RefractParams2Tdrp",
			    0, (char *)tdrp_buffer.c_str(),
			    tdrp_buffer.length(),
			    0, false, false);
  
    /********** UPDATE ********************************************
     * Create output directories.
     */

    FILE *params_file;
  
    if ((params_file = fopen(tdrp_file_path.c_str(), "w")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening Refract TDRP file for output: "
	   << tdrp_file_path << endl;
	
      return false;
    }
  
    tdrp_params.print(params_file);
    fclose(params_file);
  
    return true;
  }


};

#endif
