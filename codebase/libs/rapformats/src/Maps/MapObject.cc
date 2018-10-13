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
//   $Date: 2018/10/13 23:22:11 $
//   $Id: MapObject.cc,v 1.5 2018/10/13 23:22:11 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MapObject.cc: abstract base class representing a map object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include <rapformats/MapObject.hh>

using namespace std;


const int MapObject::MAX_TOKENS = 10;
const int MapObject::MAX_TOKEN_LEN = 80;


/**********************************************************************
 * Constructor
 */

MapObject::MapObject(void) :
  _tokens(0)
{
  _freeTokens();
}


/**********************************************************************
 * Destructor
 */

MapObject::~MapObject(void)
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _allocateTokens() - Allocate space for the tokens needed for parsing
 *                     input lines when reading the polyline from a
 *                     file.
 */

void MapObject::_allocateTokens()
{
  // If we've already allocated the tokens, we don't need to do anything

  if (_tokens != 0)
    return;
  
  // Allocate the tokens

  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
}


/**********************************************************************
 * _freeTokens() - Free the space for the tokens.
 */

void MapObject::_freeTokens()
{
  // If the tokens haven't been allocated, we don't need to do anything.

  if (_tokens == 0)
    return;
  
  // Free the tokens

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] _tokens[i];
  
  delete [] _tokens;
  
  _tokens = 0;
}
