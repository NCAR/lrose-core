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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:17:03 $
 *   $Id: RefFile.hh,v 1.2 2016/03/03 19:17:03 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * RefFile: Class controlling access to a refractivity calibration reference
 *          file.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2009
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef RefFile_H
#define RefFile_H

#include <iostream>
#include <string>

using namespace std;


class RefFile
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  typedef struct
  {
    float strength ;                /* Strength (SNR or NIQ-based) of reference
                                       target */
    float av_i ;                    /* Average I of target for N = Nref */
    float av_q ;                    /* Average Q of target for N = Nref */
    float phase_er ;                /* And its expected error */
    float dr ;                      /* Range difference between true target 
				       and bin center */
    float dNdz_sensitivity ;        /* How does phase vary with changes in
                                       dN/dz */
  } R_info_t;


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  RefFile();
  

  /**
   * @brief Destructor.
   */

  virtual ~RefFile();


  bool loadFile(const string &ref_file_name,
		const int num_beams,
		const int num_gates);
  

  ////////////////////
  // Access methods //
  ////////////////////

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
    if (debug_flag)
      _verbose = debug_flag;
  }
  
  void setVerboseFlag(const bool verbose_flag)
  {
    _verbose = verbose_flag;
  }
  

  float getRefN() const
  {
    return _refN;
  }
  

  R_info_t *getCalibRef() const
  {
    return _calibRef;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  

  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  
  int _numBeams;
  int _numGates;
  
  float _refN;
  R_info_t *_calibRef; // _numBeams * _numGates
};

#endif
