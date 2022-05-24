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
 * @file TargetVector.hh
 *
 * @class TargetVector
 *
 * Class representing a target data point for the entire scan.
 *  
 */

#ifndef TargetVector_H
#define TargetVector_H


#include "TargetInfo.hh"
#include <Refract/VectorData.hh>
#include <Refract/VectorIQ.hh>

#include <vector>

class FieldDataPair;
class FieldWithData;
// class VectorData;

/** 
 * @class TargetVector
 */
class TargetVector
{

public:

  // TargetVector(void);
  TargetVector(int scanSize=0);
  ~TargetVector();
  void compute_phase_diff(const FieldDataPair &difPrevScan,
			  const std::vector<double> &norm,
			  const FieldDataPair &difFromRef);
  void setIQ(const FieldDataPair &difFromRef);
  void setStrengthAndPhaseErr(const FieldWithData &snr,
			      const FieldDataPair &rawPhase,
			      const std::vector<double> &phase_er);
  void copyPhaseDiff(VectorData &p) const;
  void copyPhase(VectorData &p) const;

  
  /**
   * operator[]  
   * @param[in] i  Index
   * @return reference to i'th TargetInfo
   */
  // inline const TargetInfo& operator[](size_t i) const {return _target[i];}

  /**
   * operator[]  
   * @param[in] i  Index
   * @return reference to i'th TargetInfo
   */
  // inline TargetInfo& operator[](size_t i) {return _target[i];}

  bool phase_dif_er_is_valid(int offset) const;
  double iqNorm(int offset) const;

  inline const VectorData & getPhaseDiffErr(void) const {return _phase_diff_er;}
  inline const VectorIQ & getIQ(void) const {return _iq;}

private:

  int _scanSize;  /**< Shared by all */
  // std::vector<TargetInfo> _target;

  /**
   * @brief Strength (SNR-based) of target.
   */
  VectorData _strength;

  /**
   * @brief Scan-to-scan difference.
   */
  VectorData _phase_diff;

  /**
   * @brief Expected error in the scan-to-scan difference (phase_diff).
   */
  VectorData _phase_diff_er;

  /**
   * @brief scan-to-scan difference (phase_diff).
   */
  VectorIQ _dif_iq;

  /**
   * @brief Scan-to-reference difference.
   */
  VectorData _phase;

  /**
   * @brief Expected error in the scan-to-reference difference (phase).
   */
  VectorData _phase_er;

  /**
   * @brief Scan-to-reference difference (phase) corrected for dN/dz,
   *        sub-range.
   */
  VectorData _phase_cor;

  /**
   * @brief IQ phase_cor.
   */
   VectorIQ _iq;
};

#endif
