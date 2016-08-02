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
 * @file VlevelSlice.hh 
 * @brief a 2d slice into a 3d grid (Grid2dW), or a single number (Data1d)
 * @class VlevelSlice
 * @brief a 2d slice into a 3d grid (Grid2dW), or a single number (Data1d)
 */

#ifndef VLEVEL_SLICE_H
#define VLEVEL_SLICE_H
#include <string>
#include <euclid/GridAlgs.hh>
#include <FiltAlg/Data1d.hh>
#include <FiltAlg/GridProj.hh>

//------------------------------------------------------------------
class VlevelSlice : public GridAlgs, public Data1d
{
public:

  /**
   * Empty constructor
   */
  VlevelSlice(void);

  /**
   * Constructor for a Grid2d slice
   * @param[in] name  Name of the slice
   * @param[in] g     The Grid2d base class values to use
   * @param[in] vlevel  The vertical level (degrees)
   * @param[in] vlevel_index  The vertical level index 0,1,..
   * @param[in] gp  Grid projection information
   */
  VlevelSlice(const string &name, const Grid2d &g, const double vlevel,
	      const int vlevel_index, const GridProj &gp);

  /**
   * Constructor for a Data1d slice
   * @param[in] name  Name of the slice
   * @param[in] value  The single value to use
   * @param[in] vlevel  The vertical level (degrees)
   * @param[in] vlevel_index  The vertical level index 0,1,..
   */
  VlevelSlice(const string &name, const double value, const double vlevel,
	      const int vlevel_index);

  /**
   * Destructor
   */
  virtual ~VlevelSlice(void);

  /**
   * Debug print the vertical level information, and the slice itself
   */
  void print_slice(void) const;

  /**
   * Debug print the vertical level information.
   */
  void print_vlevel(void) const;

  /**
   * @return the vertical level (degrees)
   */
  inline double get_vlevel(void) const {return _vlevel;}

  /**
   * @return the vertical level index (0, 1, ..)
   */
  inline double get_vlevel_index(void) const {return _vlevel_index;}

  /**
   * @return true if the object is a 2d slice, false if a single value
   */
  inline bool is_grid(void) const {return !_is_data;}

  /**
   * Set name of local object to input
   * @param[in] name  Name to use
   */
  void set_name(const string &name);

  /**
   * Take max of input and local into local.
   * @param[in] v  Object to use 
   * @return true if ok.
   *
   * @note if both input and local objects are a value, take the max
   *       if both input and local objects are Grid2d, take max at each point
   *       if input and local object not same type, return false
   */
  bool max_slice(const VlevelSlice &v);

  /**
   * Take product of input and local into local.
   * @param[in] v  Object to use 
   * @return true if ok.
   *
   * @note if both input and local objects are a value, multiply values
   *       if both input and local objects are Grid2d, multiply at each point
   *       if input and local object not same type, return false
   */
  bool product_slice(const VlevelSlice &v);

  /**
   * Divide local object by input object
   * @param[in] v  Object to use 
   * @return true if ok.
   *
   * @note if both input and local objects are a value, divide values
   *       if both input and local objects are Grid2d, divide at each point
   *       if input and local object not same type, return false
   */
  bool divide_slice(const VlevelSlice &v);

  /**
   * Add input slice to local slice
   * @param[in] v  Object to use 
   * @return true if ok.
   *
   * @note if both input and local objects are a value, add values
   *       if both input and local objects are Grid2d, add at each point
   *       if input and local object not same type, return false
   */
  bool add_slice(const VlevelSlice &v);

  /**
   * Set entire slice to value
   * @param[in] value  
   */
  void set_slice(const double value);

  /**
   * Multiply entire slice by value
   * @param[in] value  
   */
  void multiply_slice(const double value);

  /**
   * Add value to entire slice
   * @param[in] value  
   */
  void add_value_to_slice(const double value);

  /**
   * Set entire slice to empty value
   * @note if 1d data, set value to 0.0, if grid2d, set everything to bad
   */
  void set_slice_empty(void);

  /**
   * Initialize for computing average.
   * @param[out] counts  Slice object in which to put counts
   * @note  Local object has the accumulation of what was its original state,
   *        counts object has counts from this one accumulation
   * @note  The local object can call accum_average() and finish_average()
   *        with counts as input
   */
  void init_average(VlevelSlice &counts);

  /**
   * increment into local object using input data, adjust count
   * @param[in] data  Data with values to be added to average
   * @param[in,out] counts  Counts of how much has been added.
   */
  bool accum_average(const VlevelSlice &data, VlevelSlice &counts);

  /**
   * Finish computing average, with local object set to final average.
   * @param[in] counts  Counts of how much has been added.
   *
   * Divides local values by counts
   */
  void finish_average(const VlevelSlice &counts);

  /**
   * Return grid related information from local state
   * @param[out] vlevel Vertical level (degrees)
   * @param[out] gp  Grid projection 
   */
  inline void get_grid_info(double &vlevel, GridProj &gp) const
  {
    vlevel = _vlevel;
    gp = _grid_proj;
  }

protected:
private:

  string _name;        /**< Object name */
  double _vlevel;      /**< Vertical level degrees */
  int _vlevel_index;   /**< Vertical level index 0,1,.. */
  GridProj _grid_proj; /**< Grid projection information */
  bool _is_data;       /**< true if local object is a Data1d, false if Grid2d*/
};

#endif
