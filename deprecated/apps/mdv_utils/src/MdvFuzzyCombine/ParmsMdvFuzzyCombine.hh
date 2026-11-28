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
#include <copyright.h>

/**
 * @file ParmsMdvFuzzyCombine.hh
 * @brief All the algorithm parameters for MdvFuzzyCombine
 * @class ParmsMdvFuzzyCombine
 * @brief All the algorithm parameters for MdvFuzzyCombine
 *
 * The parameters are intentionally public as it is a stateless 'struct-like'
 * class.
 */

# ifndef    PARMS_MDV_FUZZY_COMBINE_HH
# define    PARMS_MDV_FUZZY_COMBINE_HH

#include <vector>
#include <string>
#include "Params.hh"
#include "Parm1.hh"
#include <rapmath/FuzzyF.hh>

//------------------------------------------------------------------
class ParmsMdvFuzzyCombine
{
public:

  /**
   * Default constructor, gives parameters values by reading in a parameter
   * file, using input command line arguments
   * 
   * @param[in] argc  Number of command line arguments
   * @param[in] argv  args.
   */
  ParmsMdvFuzzyCombine(int argc, char **argv);
  
  /**
   * Destructor
   */
  virtual ~ParmsMdvFuzzyCombine(void);

  bool _debug;                 /**< Debugging */
  std::string _instance;       /** For PMU calls */
  std::string _inputUrl;       /**< Input */
  std::string _inputMaskUrl;   /**< Input */
  std::string _inputMaskFieldName;  /** Name */
  bool _staticMask;            /**< True for static input mask data */
  std::string _outputUrl;      /**< Output */
  bool _isForecastData;        /**< True if data is forecast directory */
  bool _isForecastMaskData;        /**< True if data is forecast directory */
  std::vector<Parm1> _input;   /**< Each input to be combined in */
  std::vector<std::string> _fields;  /**< The fields to read in */

  std::string _outputFieldName;  /**< Output name */
  std::string _outputFieldUnits; /**< Output units */
  bool _isArchiveMode;          /**< True if in archive mode */
  time_t _archiveTime0;         /**< If in archive mode, earliest wanted time */
  time_t _archiveTime1;         /**< If in archive mode, last wanted time */

  int _erodeMax;       /**< distance (# of gridpts) to erode */
  int _smoothNpt;      /**< Number of points in x and y to use in smoothing */
  int _distFillMax;    /**< distance (# of gridpts) to fill */
  int _distFillScale;  /**< scale (# of gridpoints) during fill */

  FuzzyF _taper;      

protected:
private:  

  void _addInput(const Params &p, const Params::Input_t &s);
  void _addInput(const Params::Input_t &s, const int n,
		 const Params::Fuzzy_t *f);
  void _setTaper(const int n, const Params::Fuzzy_t *f);
};

# endif
