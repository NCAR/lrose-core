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
 *
 * @file RefDebug.hh
 *
 * @class RefDebug
 *
 */

#ifndef RefDebug_HH
#define RefDebug_HH

#include <vector>

class MdvxPjg;

/** 
 * @class RefDebug
 */

class RefDebug
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * Empty
   */
  RefDebug(void);

  /**
   * @param[in] lat
   * @param[in] lon
   * @param[in] npt
   */
  RefDebug(double lat, double lon, int npt);

  /**
   *
   */
  ~RefDebug(void);

  /**
   * Fill in indices using input projection
   * @param[in] proj
   */
  void setDebug(const MdvxPjg &proj);

  /**
   * @return true if index is a debug point
   * @param[in] i
   */
  bool isDebugPt(int i) const;

 private:

  double _lat;  /**< Debug latitude */
  double _lon;  /**< Debug longitude */
  int _debugX;  /**< Debug grid index */
  int _debugY;  /**< Debug grid index */
  int _npt;     /**< Number of debug points around the center */
  std::vector<int> _debugIpt;  /**< index into grid at debug pts*/
};


#endif
