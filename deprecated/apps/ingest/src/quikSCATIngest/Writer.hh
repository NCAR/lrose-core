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
 * Writer: Base class for classes that write observation information.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef Writer_H
#define Writer_H

#include <cstdio>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/Path.hh>

#include "quikSCATObs.hh"

using namespace std;


class Writer
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  Writer(const string &output_url,
	 const int expire_secs,
	 const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~Writer();


  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  virtual bool init() = 0;


  /*********************************************************************
   * addInfo() - Add observation information.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  virtual bool addInfo(const quikSCATObs &obs) = 0;


  /*********************************************************************
   * writeInfo() - Write the observation information.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  virtual bool writeInfo() = 0;



protected:

  bool _debug;
  string _outputUrl;
  int _expireSecs;
  
};

#endif
