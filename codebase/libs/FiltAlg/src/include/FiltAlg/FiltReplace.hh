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
 * @file FiltReplace.hh 
 * @brief filter to replace Data
 * @class FiltReplace
 * @brief filter to replace Data
 *
 * Replaces values in input Data whereever conditions are met on additional
 * input Data of the same type.
 * This filter can handle any kind of input data, as long it is consistent
 * amongst inputs, Data::GRID3d replaces at each grid point, Data::DATA2D
 * replaces the single value at each vertical level, and Data::DATA1D replaces
 * the single value that is input.  
 *
 * The main input is the data to filter, the additional inputs
 * are used as conditional test inputs. The single output is the result of
 * the replacements, unchanged from input where conditions not met.
 */

#ifndef FILT_REPLACE_H
#define FILT_REPLACE_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>
#include <FiltAlg/Comb.hh>
#include <FiltAlg/Find.hh>

//------------------------------------------------------------------
class FiltReplace : public Filter
{
public:
  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   *
   * @note uses params to initialize the Comb object _comb
   */
  FiltReplace(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltReplace(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  double _replacement_value;  /**< Value to replace with, when conditions met */
  bool _set_initial_value; /**< True to initialize output to a constant */
  double _initial_value;  /**< constant to use when _set_initial_value=true */
  Comb _comb;       /**< All the other inputs besides main one */
  Find _find;       /**< The find object that is built from logical string */

  /**
   * Do the filter when it is a vlevel slice
   * @param[in] in  The slice
   * @param[in] vlevel   Degrees radar scanning angle
   * @param[in] vlevel_index   Index, 0, 1, .. into the angles
   * @param[in] gp  Projection info
   * @param[out] out  Results are put in here.
   * @return true if successful
   */
  bool _filter_slice(const VlevelSlice &in, const double vlevel,
		     const GridProj &gp, Data &out);

  /** 
   * Do the filter when it is 1d single valued data
   * @param[in] in  The Data
   * @param[out] out  Results are put here
   * @return true if successful
   */
  bool _filter_data1d(const Data &in, Data &out);

  /**
   * Compare input data with output type and make sure everything is consistent.
   * @param[in] type  The output data type
   * @param[in] data  The input data
   * @return true if there is a match
   */
  bool _check_data(const Data::Data_t type, const VlevelSlice &data) const;
};

#endif
