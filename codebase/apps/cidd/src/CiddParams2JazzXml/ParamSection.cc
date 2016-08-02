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
 * @file ParamSection.cc
 *
 * @class ParamSection
 *
 * Base class for the different sections of the CIDD parameter file.
 *  
 * @date 9/24/2010
 *
 */

#include <string.h>

#include "ParamSection.hh"

using namespace std;

// Globals

const size_t ParamSection::TAG_BUF_LEN = 256;

/**********************************************************************
 * Constructor
 */

ParamSection::ParamSection ()
{
}


/**********************************************************************
 * Destructor
 */

ParamSection::~ParamSection(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _findTagText()
 */

const char *ParamSection::_findTagText(const char *input_buf,
				       const char * tag,
				       long *text_len,
				       long *text_line_no) const
{
  static const string method_name = "ParamSection::_findTagText()";
  
  int start_line_no;
  const char *start_ptr;
  const char *end_ptr;
  const char *ptr;
  char end_tag[TAG_BUF_LEN];
  char start_tag[TAG_BUF_LEN];

  // Reasonable tag check - Give up

  if (strlen(tag) > TAG_BUF_LEN - 5)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Unreasonable tag: " << tag << " - TOO long!"<< endl;

    *text_len = 0;
    return 0;
  }

  // Clear the string buffers

  memset(start_tag, 0, TAG_BUF_LEN);
  memset(end_tag, 0, TAG_BUF_LEN);
    
  start_line_no = *text_line_no;

  sprintf(start_tag, "<%s>", tag);
  sprintf(end_tag, "</%s>", tag);

  // Search for Start tag

  if ((start_ptr = strstr(input_buf, start_tag)) == 0)
  {
    *text_len = 0;
    *text_line_no = start_line_no;
    return 0;
  }
  start_ptr +=  strlen(start_tag); // Skip over tag to get to the text

  // Search for end tag after the start tag

  if ((end_ptr = strstr(start_ptr, end_tag)) == 0)
  {
    *text_len = 0;
    *text_line_no = start_line_no;
    return 0;
  }
  end_ptr--;  // Skip back one character to last text character

  // Compute the length of the text_tag

  *text_len = (long) (end_ptr - start_ptr);

  // Count the lines before the starting tag

  ptr = input_buf;
  while (((ptr = strchr(ptr,'\n')) != 0) && (ptr < start_ptr))
  {
    ptr++; // Move past the found NL
    start_line_no++;
  }

  *text_line_no = start_line_no;
  return start_ptr;
}
