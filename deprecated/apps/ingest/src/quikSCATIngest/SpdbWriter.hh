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

/************************************************************************
 * SpdbWriter: Base class for classes that write obs information to an
 *             SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef SpdbWriter_H
#define SpdbWriter_H

#include "Writer.hh"
#include "quikSCATObs.hh"

#include <cstdio>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>

using namespace std;


class SpdbWriter : public Writer
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  SpdbWriter(const string &output_url,
	     const int expire_secs,
	     const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~SpdbWriter();


  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  virtual bool init();


  /*********************************************************************
   * writeInfo() - Write the observation information.
   *
   */

  virtual bool writeInfo();


protected:

  DsSpdb _outputSpdb;
  
};

#endif
