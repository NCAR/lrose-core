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
 *   $Id: AsciiOutput.hh,v 1.2 2016/03/04 02:01:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AsciiOutput: Class for creating ASCII output.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef AsciiOutput_H
#define AsciiOutput_H

#include <iostream>
#include <string>

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>

#include "Output.hh"

using namespace std;


class AsciiOutput : public Output
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  AsciiOutput(const string &output_dir,
	      const string &output_ext,
	      const string &delimiter,
	      const vector< string > &statistic_names,
	      const bool include_header,
	      const bool debug_flag = false);
  
  AsciiOutput(const string &accum_path,
	      const string &delimiter,
	      const vector< string > &statistic_names,
	      const bool include_header,
	      const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~AsciiOutput();


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
			 const vector< double > &statistics);


  /*********************************************************************
   * writeOutput() - Write the information to the output file.
   *
   * Returns true on success, false on failure.
   */

  virtual bool writeOutput(const DateTime &file_time);


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  string _delimiter;
  bool _includeHeader;
  bool _headerWritten;
  
  vector< string > _statNames;
  
  string _outputBuffer;
  
};

#endif
