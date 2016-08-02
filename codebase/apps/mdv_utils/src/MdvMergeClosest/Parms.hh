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
#include <toolsa/copyright.h>
/**
 * @file Parms.hh
 * @brief All the algorithm parameters for the app, derived from Params
 * @class Parms
 * @brief All the algorithm parameters for the app, derived from Params
 *
 * The parameters are intentionally public as it is a stateless 'struct-like'
 * class.
 */

# ifndef    PARMS_HH
# define    PARMS_HH

#include "Params.hh"
#include <string>
#include <vector>
class ParmInput;

//*----------------------------------------------------------------
class Parms
{
public:

  /**
   * Empty constructor, sets members to default values
   */
  Parms(void);

  /**
   * Default constructor, gives parameters values by reading in a parameter
   * file, using input command line arguments
   * 
   * @param[in] argc  Number of command line arguments is generally three for 
   *                  real-time or six for archive mode.
   * @param[in] argv  Typical realtime and archive mode command lines are: 
   *                   ' -params X.params' or 
   *                   ' -params X.params -interval 
   *                   yyyymmddhhmmdss yyyymmddhhmmdss'
   */
  Parms(int argc, char **argv);
  
  /**
   * Destructor
   */
  virtual ~Parms(void);

  /**
   * @return True if object is well formed
   */
  inline bool isOk(void) const {return _ok;}


  ////// General params /////////////////////////////////////////////////

  bool _debug;            /**< True for debugging */
  bool _debugVerbose;     /**< True for verbose debugging */
  // int _numThreads;     /**< Number of threads when there is threading */
  // bool _threadDebug;   /**< True for thread debugging */
  string _instance;       /**< Process instance */
  int _registerSeconds;   /**< Process registration interval seconds */

  bool _isArchive;     /**< True if archive mode */
  time_t _archiveT0;   /**< If _isArchive, the earliest time */
  time_t _archiveT1;   /**< If _isArchive, the last time */


  ////// Inputs params //////////////////////////////////////////////////

  std::vector<ParmInput> _input;         /**< Input sources */
  std::vector<std::string> _field;       /**< Fields to merge */

  int _triggerInterval;        /**< Seconds between triggers */
  int _triggerOffset;          /**< Offset from hms=000 in triggering */
  int _triggerMargin;          /**< Max difference between trigger and data */

  ///// Alg params //////////////////////////////////////////////////////
  
  int _timeoutSeconds;        /**< Maximum age of data before it times out */

  ////// Outputs params //////////////////////////////////////////////////

  std::string _outputUrl;     /**< Merge output URL */

protected:
private:  

  /**
   * True if this object is well formed
   */
  bool _ok;

};

/**
 * @class ParmInput
 * @brief  Params for one input
 */
class ParmInput
{
public:
  /**
   * Constructor
   */
  inline ParmInput(void) :
    _url("Unknown"),
    _latitude(0),
    _longitude(0) {}
  /**
   * Constructor
   * @param[in] i  Input params
   */
  inline ParmInput(const Params::input_t &i) :
    _url(i.url),
    _latitude(i.origin_latitude),
    _longitude(i.origin_longitude) {}

  /**
   * Destructor
   */
  inline ~ParmInput(void) {}

  string _url;        /**< Input URL */
  double _latitude;   /**< Origin */
  double _longitude;  /**< Origin */
};


# endif  

