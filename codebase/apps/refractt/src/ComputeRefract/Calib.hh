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
 * @file Calib.hh
 *
 * @class Calib
 *
 * Calib program object.
 *  
 * @date 12/1/2008
 *
 */

#ifndef Calib_HH
#define Calib_HH

#include <Refract/FieldDataPair.hh>
#include <Refract/FieldWithData.hh>
#include <Mdv/DsMdvx.hh>

/** 
 * @class Calib
 */

class Calib
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  Calib(void);
  

  /**
   * @brief Destructor
   */

  virtual ~Calib(void);
  
  bool initialize(const std::string &ref_file_name);

  inline const FieldDataPair *avIqPtr(void) const
  {
    return &_averageIQ;
  }

  inline const FieldWithData *phaseErPtr(void) const
  {
    return &_phaseEr;
  }

  inline double refN(void) const {return _refN;}

 private:

  /**
   * @brief The field name of the average I field in the calibration file.
   */

  static const std::string CALIB_AV_I_FIELD_NAME;
  
  /**
   * @brief The field name of the average Q field in the calibration file.
   */

  static const std::string CALIB_AV_Q_FIELD_NAME;
  
  /**
   * @brief The field name of the phase error field in the calibration file.
   */

  static const std::string CALIB_PHASE_ER_FIELD_NAME;

  /**
   * @brief The calibration file for daytime.
   */

  DsMdvx _calibFile;
  FieldDataPair _averageIQ;
  FieldWithData _phaseEr;
  double _refN;
};


#endif
