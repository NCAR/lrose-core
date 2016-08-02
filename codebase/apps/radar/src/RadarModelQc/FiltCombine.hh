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
 * @file FiltCombine.hh 
 * @brief Combine any number of inputs
 * @class FiltCombine
 * @brief Combine any number of inputs
 */

#ifndef FILT_COMBINE_H
#define FILT_COMBINE_H
#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class FiltCombine : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltCombine(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltCombine(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  double _main_input_weight;  /**< Weight of main input, for AVERAGE only */

  /**
   * The non-main inputs weight/name pairs (weight for AVERAGE only)
   */
  std::vector<std::pair<double, std::string> > _weight_and_names;
  
  /**
   * @enum Combine_t
   * @brief The types of combining to to
   */
  typedef enum
  {
    AVERAGE, /**< take a weighted average */
    MIN,     /**< the minimum of all inputs */
    MAX,     /**< the maximum of all inputs */
    PRODUCT  /**< The produce of all inputs */
  } Combine_t;

  Combine_t _type;  /**< The type of combining to do */
  bool _normalize;  /**< True to divide by sum of weights (AVERAGE only) */
  bool _missing_ok; /**< True to combine all non-missing inputs, false to
		     *   set output to missing if any input is missing */

  void _build(const Params::Combine_t *c, const int n);
};

#endif
