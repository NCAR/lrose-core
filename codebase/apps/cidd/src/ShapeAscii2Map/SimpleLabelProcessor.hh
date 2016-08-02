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
 *   $Id: SimpleLabelProcessor.hh,v 1.2 2016/03/07 18:28:25 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SimpleLabelProcessor : Class for reading map information from an ASCII
 *                        shape file that just contains labels.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SimpleLabelProcessor_HH
#define SimpleLabelProcessor_HH

#include "InputProcessor.hh"

using namespace std;

class SimpleLabelProcessor : public InputProcessor
{
 public:

  /*********************************************************************
   * Constructors
   */

  SimpleLabelProcessor(const int header_lines = 1,
		       const bool debug_flag = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~SimpleLabelProcessor(void);
  

 protected:

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;

  typedef enum
  {
    LABEL_TOKEN_NUM,
    LON_TOKEN_NUM,
    LAT_TOKEN_NUM,
    NUM_TOKENS
  } token_positions_t;
  
  char **_tokens;
  
  /*********************************************************************
   * _readMap() - Read the input file and return the associated map.
   *
   * Returns 0 if there was an error generating the Map object.
   */

  virtual Map *_readMap(FILE *input_file);
  
};


#endif
