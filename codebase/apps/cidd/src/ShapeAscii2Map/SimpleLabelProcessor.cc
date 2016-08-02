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
//   $Id: SimpleLabelProcessor.cc,v 1.2 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SimpleLabelProcessor : Class for reading map information from an ASCII
 *                        shape file that just contains labels.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdlib.h>

#include <rapformats/MapSimpleLabel.hh>
#include <toolsa/str.h>

#include "SimpleLabelProcessor.hh"

using namespace std;

const int SimpleLabelProcessor::MAX_TOKENS = 10;
const int SimpleLabelProcessor::MAX_TOKEN_LEN = 1024;

/*********************************************************************
 * Constructor
 */

SimpleLabelProcessor::SimpleLabelProcessor(const int header_lines,
					   const bool debug_flag) :
  InputProcessor(header_lines, debug_flag)
{
  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
}


/*********************************************************************
 * Destructor
 */

SimpleLabelProcessor::~SimpleLabelProcessor()
{
  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] _tokens[i];
  delete [] _tokens;
}


/*********************************************************************
 * _readMap() - Read the input file and return the associated map.
 *
 * Returns 0 if there was an error generating the Map object.
 */

Map *SimpleLabelProcessor::_readMap(FILE *input_file)
{
  static const string method_name = "SimpleLabelProcessor::_readMap()";
  
  Map *map = new Map();
  
  while (fgets(_inputLine, BUFSIZ, input_file)!= 0)
  {
    if (STRparse_delim(_inputLine, _tokens, BUFSIZ, ",", 
		       MAX_TOKENS, MAX_TOKEN_LEN) != NUM_TOKENS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing input line <" << _inputLine << ">" << endl;
      cerr << "--- SKIPPING LINE ---" << endl;
      
      continue;
    }
    
    if (strlen(_tokens[LABEL_TOKEN_NUM]) == 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "No label on input line <" << _inputLine << ">" << endl;
      cerr << "--- SKIPPING LINE ---" << endl;
      
      continue;
    }
    
    MapPoint location(atof(_tokens[LAT_TOKEN_NUM]),
		      atof(_tokens[LON_TOKEN_NUM]));
    
    // Replace all spaces in the label with underscores because Rview
    // can't handle labels with spaces.

    char label[MAX_TOKEN_LEN];
    STRcopy(label, _tokens[LABEL_TOKEN_NUM], MAX_TOKEN_LEN);
    
    for (int i = 0; i < MAX_TOKEN_LEN; ++i)
    {
      if (label[i] == '\0')
	break;
      
      if (label[i] == ' ')
	label[i] = '_';
    } /* enfor - i */
    
    MapSimpleLabel *simple_label =
      new MapSimpleLabel(location, label);
    
    map->addObject(simple_label);
  }
  
  return map;
}
