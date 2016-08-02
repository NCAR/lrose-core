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
 * @file FiltInfoInput.hh 
 * @brief The information needed to run a filter
 * @class FiltInfoInput
 * @brief The information needed to run a filter
 */

#ifndef FILT_INFO_INPUT_HH
#define FILT_INFO_INPUT_HH

#include <FiltAlg/GridProj.hh>
class Data;
class Filter;
class VlevelSlice;

//------------------------------------------------------------------
class FiltInfoInput 
{
public:

  /** 
   * Constructor
   */
  FiltInfoInput(void);

  /** 
   * Constructor for vlevels
   *
   * @param[in] gi  Pointer to the VlevelSlice
   * @param[in] f  Pointer to the Filter
   * @param[in] g  Pointer to output data
   * @param[in] index  Vlevel index
   * @param[in] vlevel  Vlevel value
   * @param[in] gp  Grid projection
   *
   * All inputs are stored as members
   */
  FiltInfoInput(const VlevelSlice *gi, const Filter *f, const Data *g,
		const int index, const double vlevel, const GridProj &gp);

  /** 
   * Constructor for no vlevels
   *
   * @param[in] in  Pointer to input Data object
   * @param[in] out  Pointer to output Data object
   */
  FiltInfoInput(const Data *in, const Data *out);

  /**
   * Destructor
   */
  virtual ~FiltInfoInput(void);


  /**
   * Print the pointer values
   */
  void printPtrs(void) const;

  /**
   * Print the filtering activity
   * @param[in] isStart true if filter is starting up, false if now done
   * @param[in] debug True for thread extra debugging
   */
  void printFilter(const bool isStart, const bool debug) const;

  inline const VlevelSlice *getSlice(void) const {return _gin;}
  inline const Filter *getFilter(void) const {return _filter;}
  inline const Data *getDataOut(void) const {return _gout;}
  inline const int getVlevelIndex(void) const {return _vIndex;}
  inline const double getVlevel(void) const {return _vlevel;}
  inline const GridProj &getGridProj(void) const {return _gp;}
  inline bool hasVlevels(void) const {return _has_vlevels;}
  inline const Data *getInput1d(void) const {return _in;}
  inline const Data *getOutput1d(void) const {return _out;}

protected:
private:

  /**
   * True if the input info is for data with vertical levels
   */
  bool _has_vlevels;

  const VlevelSlice *_gin;   /**< Used only when _has_vlevels = true */
  const Filter *_filter;     /**< Used only when _has_vlevels = true */
  const Data *_gout;         /**< Used only when _has_vlevels = true */
  int _vIndex;               /**< Used only when _has_vlevels = true */
  double _vlevel;            /**< Used only when _has_vlevels = true */
  GridProj _gp;              /**< Used only when _has_vlevels = true */

  const Data *_in;    /**< Used only when _has_vlevels = false */
  const Data *_out;   /**< Used only when _has_vlevels = false */
};

#endif
