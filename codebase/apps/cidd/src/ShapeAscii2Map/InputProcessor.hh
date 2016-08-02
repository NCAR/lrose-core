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
 *   $Date: 2016/03/07 18:28:25 $
 *   $Id: InputProcessor.hh,v 1.3 2016/03/07 18:28:25 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InputProcessor : Base class for classes that process the input file.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InputProcessor_HH
#define InputProcessor_HH

#include <cstdio>
#include <iostream>
#include <string>

#include <rapformats/Map.hh>

using namespace std;

class InputProcessor
{
 public:

  /*********************************************************************
   * Constructors
   */

  InputProcessor(const int header_lines = 0,
		 const bool debug_flag = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~InputProcessor(void);
  

  /*********************************************************************
   * readMap() - Read the input file and return the associated map.
   *
   * Returns 0 if there was an error generating the Map object.
   */

  virtual Map *readMap(const string &input_file_path);
  
  
 protected:

  bool _debug;
  int _headerLines;
  char *_inputLine;
  
  /*********************************************************************
   * _readMap() - Read the input file and return the associated map.
   *
   * Returns 0 if there was an error generating the Map object.
   */

  virtual Map *_readMap(FILE *input_file) = 0;
  
  
};


#endif
