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
 * @file Kernels.hh 
 * @brief Kernels The set of Kernels for a Sweep
 * @class Kernels
 * @brief Kernels The set of Kernels for a Sweep
 */

#ifndef Kernels_H
#define Kernels_H

#include "KernelPair.hh"
#include <rapmath/MathUserData.hh>
#include <vector>

/*----------------------------------------------------------------*/
class Kernels : public MathUserData
{
public:
  Kernels();
  ~Kernels();

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return the next available i.d. value
   */
  inline int nextAvailableId(void) const {return _next_available_id;}

  /**
   * Append a kernel pair to the state
   * @param[in] k  Object to append
   */ 
  inline void append(const KernelPair &k) { _k.push_back(k); }

  /**
   * Increment the local available i.d. by a number
   * @param[in] num
   */
  inline void incrementNextAvailableId(int num)
  {
    _next_available_id += num;
  }
  
  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Return number of Kernel pairs
   */
  inline size_t size(void) const   {return _k.size(); }

  /**
   * @return reference to the 'ith' Kernel pair
   */
  inline const KernelPair &operator[](int i) const  {return _k[i]; }

  /**
   * @return reference to the 'ith' Kernel pair
   */
  inline KernelPair &operator[](int i) {return _k[i]; }

  /**
   * Compute attenuation for all kernel pairs and write values to the grid
   * @param[in] dx  Km per gate
   * @param[out] att  Attenuation output grid
   */
  void computeAttenuation(double dx, Grid2d &att) const;

  /**
   * Compute humidity for all kernel pairs and write values to the grid
   * @param[in] dx  Km per gate
   * @param[out] hum  Humidity output grid
   */
  void computeHumidity(double dx, Grid2d &hum) const;

  /**
   * @return ascii output for the Kernel pairs
   *
   * @param[in] vlevel vertical level degrees
   * @param[in] gp Grid projection
   */
  string asciiOutput(double vlevel, const MdvxProj &gp) const;

  /**
   * Filter to keep only kernel pairs for which both kernels passed all tests
   */
  void filterToGood(void);

  /**
   * Write genpoly representation of kernels to SPDB
   * @param[in] url   Where to write
   * @param[in] time  data time
   * @param[in] outside true to write the kernel pair 'outside' points
   * @param[in] proj  grid projection
   */
  bool writeGenpoly(const string &url, const time_t &time,
		    bool outside, const MdvxProj &proj) const;


protected:
private:

  int _next_available_id;       /**< Counter maintained to agree with _k */
  std::vector<KernelPair> _k;   /**< The kernel pairs */
};

#endif
 
