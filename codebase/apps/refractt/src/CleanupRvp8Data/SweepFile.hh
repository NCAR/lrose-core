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
 *   $Id: SweepFile.hh,v 1.2 2016/03/07 18:17:27 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SweepFile: Class for controlling access to the netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SweepFile_H
#define SweepFile_H

#include <string>
#include <map>
#include <netcdf.hh>

#include "NegateVar.hh"

using namespace std;


class SweepFile
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  SweepFile(const string &sweep_file_path,
	    const map< string, NegateVar > &negate_var_list,
	    const int num_begin_delete_gates,
	    const string &gates_dim_name,
	    const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  ~SweepFile();


  /*********************************************************************
   * initialize() - Initialize the SweepFile.  This method MUST be called
   *                before any other methods are called.
   *
   * Returns true on success, false on failure
   */

  bool initialize();
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * writeCleanFile() - Write out a new sweep file that is cleaned up.
   *
   * Returns true on success, false on failure
   */

  bool writeCleanFile(const string &output_dir,
		      const string &file_name);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * setDebugFlag() - Set the debug flag to the given value.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

  /*********************************************************************
   * setFilePath() - Set the sweep file path.
   */

  void setFilePath(const string &sweep_file_path)
  {
    _filePath = sweep_file_path;
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _filePath;
  NcFile *_sweepFile;
  
  map< string, NegateVar > _negateVarList;
  
  int _numBeginDeleteGates;
  string _gatesDimName;
  
  bool _objectInitialized;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _addDimensions() - Add the dimensions to the given output file.
   *
   * Returns true on success, false on failure.
   */

  bool _addDimensions(NcFile &output_file) const;
  

  /*********************************************************************
   * _addGlobalAttributes() - Add the global attributes to the given
   *                          output file.
   *
   * Returns true on success, false on failure.
   */

  bool _addGlobalAttributes(NcFile &output_file) const;
  

  /*********************************************************************
   * _addVariableAttributes() - Add the attributes of the input variable 
   *                            to the output variable.
   *
   * Returns true on success, false on failure.
   */

  bool _addVariableAttributes(const NcVar &input_var,
			      NcVar &output_var) const;
  

  /*********************************************************************
   * _addVariables() - Add the variables to the given output file.
   *
   * Returns true on success, false on failure.
   */

  bool _addVariables(NcFile &output_file) const;
  

  /*********************************************************************
   * _copyData() - Copies data from the input variable into the output
   *               variable.
   *
   * Returns true on success, false on failure.
   */

  bool _copyData(NcVar &input_var,
		 NcVar &output_var,
		 const int num_dims) const;


  /*********************************************************************
   * _negateVariable() - Negate the data values for this variable.
   */

  bool _negateVariable(NcVar &output_var,
		       const string &negate_fill_attr_name) const;
  

  /////////////////////////
  // Protected templates //
  /////////////////////////

  template< class T > bool _copyDataT(NcVar &input_var,
				      NcVar &output_var,
				      T *vals,
				      long *counts,
				      long *cur,
				      const string &method_name) const
  {
    if (_debug)
    {
      cerr << "Inside _copyDataT:" << endl;
      
      for (int dim = 0; dim < input_var.num_dims(); ++dim)
	cerr << "   counts = " << counts[dim]
	     << ", cur = " << cur[dim] << endl;
    }
    
    // Get the data values from the input variable

    if (!input_var.set_cur(cur))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error setting current position on input variable" << endl;
      return false;
    }
    
    if (!input_var.get(vals, counts))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting data values for input variable: "
	   << input_var.name() << endl;
      return false;
    }
      
    // Write the data values to the output variable

    if (!output_var.put(vals, counts))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error setting data values for output variable: "
	   << output_var.name() << endl;
      return false;
    }
      
    return true;
  }
  
  template< class T > void _negateDataT(NcVar &output_var,
					const long *counts,
					const T fill_value,
					const int num_vals) const
  {
    T *vals = new T[num_vals];

    output_var.get(vals, counts);
    
    for (int i = 0; i < num_vals; ++i)
      if (vals[i] != fill_value)
	vals[i] = -vals[i];
    
    output_var.put(vals, counts);
  }
  
};

#endif
