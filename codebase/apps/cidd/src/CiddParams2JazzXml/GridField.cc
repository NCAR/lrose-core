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
/**
 *
 * @file GridField.cc
 *
 * @class GridField
 *
 * Class representing a grid field in a CIDD parameter file.
 *  
 * @date 9/24/2010
 *
 */

#include <iostream>

#include "GridField.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

GridField::GridField () :
  compositeMode(false),
  autoScale(false),
  displayInMenu(false),
  backgroundRender(false)
{
}


/**********************************************************************
 * Destructor
 */

GridField::~GridField(void)
{
}
  

/**********************************************************************
 * replaceUnderscores()
 */

void GridField::replaceUnderscores()
{
  for (size_t i = 0; i < legendName.length(); ++i)
  {
    if (legendName[i] == '_')
      legendName[i] = ' ';
  }
  
  for (size_t i = 0; i < buttonName.length(); ++i)
  {
    if (buttonName[i] == '_')
      buttonName[i] = '-';
  }
  
}


/**********************************************************************
 * setUrlAndFieldName()
 */

bool GridField::setUrlAndFieldName(const string &token)
{
  static const string method_name = "GridField::setUrlAndFieldName()";

  // Find the '&' character in the string.  If there isn't an '&', then the
  // string can't be parsed.

  size_t amp_pos;
  
  if ((amp_pos = token.find("&")) == string::npos)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Can't parse URL/field name from CIDD parameter file" << endl;
    cerr << "No '&' character found in token: " << token << endl;
    
    return false;
  }
  
  // Now separate the URL from the field name and set the class members

  url = token.substr(0, amp_pos);
  fieldName = token.substr(amp_pos + 1);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
