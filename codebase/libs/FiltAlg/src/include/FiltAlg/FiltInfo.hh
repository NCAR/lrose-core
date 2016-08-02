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
 * @file FiltInfo.hh 
 * @brief The information needed to put filter results into output
 * @class FiltInfo
 * @brief The information needed to put filter results into output
 */

#ifndef FILT_INFO_HH
#define FILT_INFO_HH
#include <FiltAlg/FiltInfoInput.hh>
#include <FiltAlg/FiltInfoOutput.hh>

//------------------------------------------------------------------
class FiltInfo
{
public:

  /** 
   * Constructor
   */
  FiltInfo(void);

  /**
   * Destructor
   */
  virtual ~FiltInfo(void);

  inline void setInput(const FiltInfoInput &i) { _input = i; }
  inline FiltInfoInput &getInput(void) { return _input; }
  inline void setOutput(const FiltInfoOutput &i) { _output = i;}
  inline FiltInfoOutput &getOutput(void) { return _output;}

  /**
   * Filter passing inputs to the filter, and setting outputs
   * @param[in] debug  True for extra debugging
   */
  void doFilter(const bool debug);

  /**
   * Should be called when vertical levels, stores _output data to 
   * 'out' using _input_data values
   *
   * @param[in,out] out Data to store to
   *
   * @return true if successful
   */
  bool storeSlice(Data &out) const;

  /**
   * Should be called when no vertical levels, stores _output data to 
   * 'out'
   *
   * @param[in,out] out Data to store to
   *
   * @return true if successful
   */
  bool store1dValue(Data &out) const;

protected:
private:

  FiltInfoInput _input;  /**< Input filter information */
  FiltInfoOutput _output; /**< Output filter information */

};

#endif
