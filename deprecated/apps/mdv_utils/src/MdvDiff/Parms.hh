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
#include <toolsa/copyright.h>

//   File: $RCSfile: Parms.hh,v $
//   Version: $Revision: 1.9 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Parms.hh
 * @brief all parms.
 * @class Parms
 * @brief all parms.
 * @note
 * @todo
 */

# ifndef    Parms_HH
# define    Parms_HH

#include <vector>
#include <string>
#include "Params.hh"

/*----------------------------------------------------------------*/
class Parms : public Params
{
public:

  /**
   * Empty
   */
  Parms();

  /**
   * default constructor
   */
  Parms(int argc, char **argv);
  
  /**
   *  destructor
   */
  virtual ~Parms();

  /**
   * @return true if input is a field that should be looked at
   */
  bool wanted_field(const string &s) const;

  bool _all_fields;
  std::vector<std::string> _fields;

protected:
private:  
};

# endif     /* Parms_HH */
