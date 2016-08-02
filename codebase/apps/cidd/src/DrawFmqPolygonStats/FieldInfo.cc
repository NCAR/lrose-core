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
 * @file FieldInfo.cc
 *
 * @class FieldInfo
 *
 * Information about a gridded field that will be used to generate statistics.
 *  
 * @date 9/1/2011
 *
 */

#include "FieldInfo.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

FieldInfo::FieldInfo() :
  _fieldName(""),
  _fieldNum(0),
  _isLog(false),
  _field(0)
{
}


FieldInfo::FieldInfo(const string &field_name,
		     const int field_num,
		     const bool is_log) :
  _fieldName(field_name),
  _fieldNum(field_num),
  _isLog(is_log),
  _field(0)
{
}


/*********************************************************************
 * Destructor
 */

FieldInfo::~FieldInfo()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
