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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:01:42 $
//   $Id: Output.cc,v 1.2 2016/03/04 02:01:42 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Output: Base class for classes that create the output.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "Output.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Output::Output(const string &output_dir,
	       const string &output_ext,
	       const bool debug_flag) :
  _debug(debug_flag),
  _outputDir(output_dir),
  _outputExt(output_ext),
  _accumulateData(false),
  _accumPath("")
{
}

Output::Output(const string &accum_path,
	       const bool debug_flag) :
  _debug(debug_flag),
  _outputDir(""),
  _outputExt(""),
  _accumulateData(true),
  _accumPath(accum_path)
{
}

  
/*********************************************************************
 * Destructor
 */

Output::~Output()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _generateOutputPath() - Generate the output path to use for this
 *                         plot.
 *
 * Returns the generated output path.
 *
 * Note that this method also makes sure that the output directory
 * exists.
 */

Path Output::_generateOutputPath(const DateTime &data_time) const
{
  static const string method_name = "Output::_generateOutputPath()";
  
  Path output_path;
  
  output_path.setDirectory(_outputDir, data_time.getYear(),
			   data_time.getMonth(), data_time.getDay());
  output_path.setFile(data_time, _outputExt);
  
  if (output_path.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: "
	 << output_path.getDirectory() << endl;
  }
  
  return output_path;
}

  

