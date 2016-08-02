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
 * @file FiltCreate.hh
 * @brief Class for filter creation
 *
 * @note Each app that uses this library should implement appFiltCreate
 *       if it has filters unique to the app.
 */

#ifndef FILTCREATE_H
#define FILTCREATE_H

#include <FiltAlg/FiltAlgParams.hh>

class FiltAlgParms;
class Filter;

class FiltCreate
{
public:

  /**
   * Empty constructor
   */
  inline FiltCreate(void) {}

  /**
   * Destructor
   */
  inline virtual ~FiltCreate(void) {}

  /**
   * Create a filter from input params
   *
   * @param[in] f  Filter specification params
   * @param[in] p  General params
   *
   * @return pointer to the Filter which is owned by caller
   */
  Filter *create(const FiltAlgParams::data_filter_t f,
		 const FiltAlgParms &p) const;

  /**
   * Create a filter from input params
   * @param[in] f  Filter specification params
   * @param[in] p  General params
   *
   * @return pointer to the Filter which is owned by caller
   */
  virtual Filter *appFiltCreate(const FiltAlgParams::data_filter_t f,
				const FiltAlgParms &p) const;
protected:
private:

};

#endif
