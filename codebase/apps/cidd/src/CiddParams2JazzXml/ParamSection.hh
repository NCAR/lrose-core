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
 * @file ParamSection.hh
 *
 * @class ParamSection
 *
 * Base class for the different sections of the CIDD parameter file.
 *  
 * @date 9/24/2010
 *
 */

#ifndef ParamSection_HH
#define ParamSection_HH

#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;


class ParamSection
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  // Constructor

  ParamSection();
  
  // Destructor

  virtual ~ParamSection(void);
  
 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const size_t TAG_BUF_LEN;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Search a null terminated string for the text between tags.
   *
   * Searches through input_buf for text between tags of the form
   * <TAG>...Text...</TAG>.  Returns a pointer to the beginning of the text
   * and its length if found. text_line_no is used on input to begin counting
   * and is set on output to the starting line number of the tagged text
   */

  const char *_findTagText(const char *input_buf,
			   const char * tag,
			   long *text_len,
			   long *text_line_no) const;
  

};


#endif
