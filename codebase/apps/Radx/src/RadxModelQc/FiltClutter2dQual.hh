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
 * @file FiltClutter2dQual.hh 
 * @brief Apply the spectrum width normalization equations
 * @class FiltClutter2dQual
 * @brief Apply the spectrum width normalization equations
 *
 */

#ifndef FILT_CLUTTER_2D_QUAL_HH
#define FILT_CLUTTER_2D_QUAL_HH

#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class FiltClutter2dQual : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltClutter2dQual(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltClutter2dQual(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  // std::string _dbzFieldName;
  std::string _velFieldName;        /**< Input velocity data field name */
  std::string _widthFieldName;      /**< Input width data field name */
  std::string _cmdFlagFieldName;    /**< Input command flag data field name */

  double _swShapeFactor;            /**< equation parameter */
  double _vrShapeFactor;            /**< equation parameter */
  // double _scrIntercept;
  // double _scrDenom;
  // double _notchNumCoeff;
  // double _smallClut;
  // double _meanPrt;
  // double _meanNsamples;
  // double _meanNyquist;
  // bool _meanValuesSet;
};

#endif
