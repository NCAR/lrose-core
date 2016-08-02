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
 * @file Parms.hh
 * @brief all parms.
 * @class Parms
 * @brief all parms.
 */

# ifndef    Parms_HH
# define    Parms_HH

#include <FiltAlg/FiltAlgParms.hh>
#include "Params.hh"

/*----------------------------------------------------------------*/
class Parms : public FiltAlgParms
{
public:

  /**
   * default constructor
   */
  Parms();
  Parms(int argc, char **argv);
  
  /**
   *  destructor
   */
  virtual ~Parms();


  /**
   * Public members
   */
  Params _main;

  static bool name_is_humidity(const string &name);

  virtual int 
  app_max_elem_for_filter(const FiltAlgParams::data_filter_t f) const;
  virtual const void *app_parms(void) const;

protected:
private:  
};

# endif     /* Parms_HH */
