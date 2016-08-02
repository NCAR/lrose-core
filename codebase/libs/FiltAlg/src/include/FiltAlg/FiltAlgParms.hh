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
 * @file FiltAlgParms.hh 
 * @brief base class params shared by all Filter objects, some virtual methods
 * @class FiltAlgParms
 * @brief base class params shared by all Filter objects, some virtual methods
 * 
 */

#ifndef FILT_ALG_PARMS_H
#define FILT_ALG_PARMS_H
#include <FiltAlg/FiltAlgParams.hh>

//------------------------------------------------------------------
class FiltAlgParms  : public FiltAlgParams
{
public:
  /**
   * Constructor empty.
   */
  FiltAlgParms(void);

  /**
   * Constructor..sets base class to input
   * @param[in] p  Base class values to use
   */
  FiltAlgParms(const FiltAlgParams &p);

  /**
   * Destructor
   */
  virtual ~FiltAlgParms(void);

  /**
   * @return maximum fuzzy function index
   */
  int max_fuzzy_function_index(void) const;


  /**
   * @return pointer to array of fuzzy parameters for index, or an empty pointer
   * for index out of range, or empty
   * @param[in] index   The index
   * @param[out] nf  Number of fuzzy params for this fuzzy function
   */
  const FiltAlgParams::fuzzy_t *fuzzy_params(const int index, int &nf) const;


  /** 
   * @return maximum number of filter param sets for the input
   * @param[in] f  A filter type, assumed to be an 'app' filter (type=APPFILTER)
   * @note returns 0 if f is not an 'app' filter with type=APPFILTER
   */
  virtual
  int app_max_elem_for_filter(const FiltAlgParams::data_filter_t f) const = 0;

  /** 
   * @return pointer to the apps' params, cast to void
   *
   * Each app will have Params class, this method returns a pointer to that.
   * It is up to the app derived class to keep a Params object around to
   * point to here.
   */
  virtual const void *app_parms(void) const = 0;
  
protected:
private:

};

#endif
