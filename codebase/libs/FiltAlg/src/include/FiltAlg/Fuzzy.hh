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
 * @file Fuzzy.hh 
 * @brief extend FuzzyF class to construct off of params.
 * @class Fuzzy
 * @brief extend FuzzyF class to construct off of params.
 */

#ifndef FUZZY_H
#define FUZZY_H
#include <FiltAlg/FiltAlgParams.hh>
#include <rapmath/FuzzyF.hh>

//------------------------------------------------------------------
class Fuzzy : public FuzzyF
{
public:
  /**
   * Empty constructor
   */
  Fuzzy(void);

  /**
   * Construct from params
   * @param[in] p Algorithm params
   * @param[in] index  Index to a particular fuzzy params array
   */
  Fuzzy(const FiltAlgParams &p, const int index);

  /**
   * Destructor
   */
  virtual ~Fuzzy(void);

  /**
   * @return true if well formed
   */
  inline bool ok(void) const {return _ok;}

  
protected:
private:

  bool _ok;    /**< True if well formed */

  /**
   * Construct using params
   * @param[in] n  Length of f array
   * @param[in] f  Parameters for fuzzy function
   */
  void _build(const int n, const FiltAlgParams::fuzzy_t *f);
};

#endif
