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
 *   $Id: Csv2Map.hh,v 1.2 2016/03/07 18:28:25 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Csv2Map : Csv2Map program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2014
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Csv2Map_HH
#define Csv2Map_HH

#include <rapformats/Map.hh>
#include <rapformats/MapIconDef.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class Csv2Map
{
 public:

  // Destructor

  ~Csv2Map(void);
  
  // Get Csv2Map singleton instance

  static Csv2Map *Inst(int argc, char **argv);
  static Csv2Map *Inst();
  
  // Initialize the program.  Must be called before run().

  bool init();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int BUFFER_SIZE;
  
  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Csv2Map *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // The objects we need for parsing the tokens on each line.  We want
  // these to be global just so we don't keep reallocating the space.

  int _maxTokens;
  char **_tokens;
  
  // The icon to use for the icons map

  MapIconDef *_iconDef;

  // The null icon to use for the labels map so that just the labels
  // appear in that map

  MapIconDef *_nullIconDef;

  // Flag indicating whether to create the labels map

  bool _createLabelsMap;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  Csv2Map(int argc, char **argv);
  
  // Process the given line from the input file

  bool _processLine(const char *line_buffer,
		    Map &icons_map, Map &labels_map);
  
  // Write a map to the given file path

  bool _writeMap(const Map &map, const string &file_path);
  
};


#endif
