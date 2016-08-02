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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:28:25 $
//   $Id: InputProcessor.cc,v 1.3 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InputProcessor : Base class for classes that process the input file.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "InputProcessor.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

InputProcessor::InputProcessor(const int header_lines,
			       const bool debug_flag) :
  _debug(debug_flag),
  _headerLines(header_lines)
{
  _inputLine = new char[BUFSIZ];
}


/*********************************************************************
 * Destructor
 */

InputProcessor::~InputProcessor()
{
  delete [] _inputLine;
}


/*********************************************************************
 * readMap() - Read the input file and return the associated map.
 *
 * Returns 0 if there was an error generating the Map object.
 */

Map *InputProcessor::readMap(const string &input_file_path)
{
  static const string method_name = "InputProcessor::readMap()";
  
  // Open the input file

  FILE *input_file;
  
  if ((input_file = fopen(input_file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file" << endl;
    perror(input_file_path.c_str());
    
    return 0;
  }
  
  // Skip the header lines

  for (int i = 0; i < _headerLines; ++i)
    fgets(_inputLine, BUFSIZ, input_file);
  
  // Create the Map object and update it from the contents of the
  // input file.

  Map *map = _readMap(input_file);
  
  // Close the input file

  fclose(input_file);
  
  return map;
}
