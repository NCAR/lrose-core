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
 * @file VlevelData.hh 
 * @brief VlevelData is a vector of VlevelSlice, one per vlevel.
 * @class VlevelData
 * @brief VlevelData is a vector of VlevelSlice, one per vlevel.
 *
 * This represents an entire volume of data, or one value per vlevel
 */

#ifndef VLEVEL_DATA_H
#define VLEVEL_DATA_H
#include <string>
#include <vector>
#include <FiltAlg/VlevelSlice.hh>
#include <FiltAlg/Data2d.hh>
#include <FiltAlg/Info.hh>

//------------------------------------------------------------------
class VlevelData
{
public:

  /**
   * Constructor
   *
   * @param[in] name  Name for the object
   * @param[in] disabled   True if there are no vlevels
   * @param[in] grid   True if gridded vlevel data, false if single values
   */
  VlevelData(const string &name, const bool disabled, const bool grid);

  /**
   * Destructor
   */
  virtual ~VlevelData(void);

  /**
   * Print out each slice (debug) to stdout
   */
  void print_vlevel_data(void) const;

  /**
   * @return number of vertical levels
   */
  int num_vlevel(void) const;

  /**
   * @return pointer to a particular slice
   * @param[in] i  Index
   */
  inline const VlevelSlice *ith_vlevel_slice(const int i) const
  {
    return (const VlevelSlice *)(&_g[i]);
  }

  /**
   * @return the vertical levels (degrees)
   */
  vector<double> extract_vlevel(void) const;

  /**
   * @return Pointer to the slice at a particular vlevel, NULL for none
   * @param[in] vlevel  The vlevel to match 
   * @param[in] tolerance  Allowed diff between local and input
   *
   * If no vlevels are within tolerance of input, returns NULL
   */
  const VlevelSlice *matching_vlevel(const double vlevel,
				     const double tolerance) const;

  /**
   * create a 3d array with order x,y,z which is returned
   * @param[out] bad  Bad data value 
   * @param[out] missing  Missing data value 
   *
   * @returns NULL if no vertical levels or the data is not grids
   *
   * @note caller must free the array
   */
  fl32 *volume(fl32 &bad, fl32 &missing) const;

  /**
   * @return true if input args not same as local 3d grid values,
   *
   * @param[in] nx  Number of grid values x
   * @param[in] ny  Number of grid values y
   * @param[in] nz  Number of grid values z (vertical levels)
   * @param[in] print  True to print out what is going on when bad
   *
   * @note returns false if data is not a 3d grid
   */
  bool dim_bad(const int nx, const int ny, const int nz, 
	       bool print=false) const;

  /**
   * Clear so nothing (no data)
   */
  void clear(void);

  /**
   * Add a new 2d grid to the 3d object. Works only when gridded
   *
   * @param[in] g  A 2d grid
   * @param[in] vlevel  Vertical level (degrees)
   * @param[in] vlevel_index  Vertical level index 0,1,..
   * @param[in] gp  Grid projection info.
   *
   * @return true if successful, false if inputs inconsistent with state,
   *         or data is not gridded.
   *
   */
  bool add(const Grid2d &g, const double vlevel, const int vlevel_index,
	   const GridProj &gp);

  /**
   * Add a new single value of data. Works only when single valued
   * @param[in] vlevel  Vertical level (degrees)
   * @param[in] vlevel_index  Vertical level index 0,1,..
   * @param[in] value  Value to use
   *
   * @return true if successful, false if inputs inconsistent with state,
   *         or data is gridded.
   */
  bool add(const double vlevel, const int vlevel_index, const double value);

  /**
   * Store vertical levels and data values into info. Store data values into
   * a Data2d object
   * @param[in,out] d  Object that will have data values
   * @param[in,out] info  Object with vlevel and data values on exit
   *
   * @return true if successful
   */
  bool store_2d_info(Data2d &d, Info &info) const;

  /**
   * Create Data2d objects with data values and with vertical levels
   * @param[out] d  Object that will have data values
   * @param[out] vlevels  Object that will have vertical levels
   *
   * @return true if successful
   */
  bool construct_data2d(Data2d &d, Data2d &vlevels) const;

protected:
private:

  
  string _name;           /**< name for this data */
  vector<VlevelSlice> _g; /**< slices of data */
  bool _disabled;         /**< True if no vertical levels */
  bool _gridded;          /**< true if each level is 2d data */
};

#endif
