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
 *   $Date: 2016/03/04 02:01:42 $
 *   $Id: Output.hh,v 1.2 2016/03/04 02:01:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Output: Base class for classes that create the output.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Output_H
#define Output_H

#include <iostream>
#include <string>

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>

using namespace std;


class Output
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  Output(const string &output_dir,
	 const string &output_ext,
	 const bool debug_flag = false);
  
  Output(const string &accum_path,
	 const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~Output();


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /*********************************************************************
   * addOutput() - Add the given information to the output buffer.
   *
   * Returns true on success, false on failure.
   */

  virtual bool addOutput(const DateTime &data_time,
			 const int simple_track_num,
			 const int complex_track_num,
			 const double vert_level,
			 const vector< double > &statistics) = 0;


  /*********************************************************************
   * writeOutput() - Write the information to the output file.
   *
   * Returns true on success, false on failure.
   */

  virtual bool writeOutput(const DateTime &file_time) = 0;


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _outputDir;
  string _outputExt;
  
  bool _accumulateData;
  string _accumPath;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _generateOutputPath() - Generate the output path to use for this
   *                         plot.
   *
   * Returns the generated output path.
   *
   * Note that this method also makes sure that the output directory
   * exists.
   */

  virtual Path _generateOutputPath(const DateTime &data_time) const;
  

};

#endif
