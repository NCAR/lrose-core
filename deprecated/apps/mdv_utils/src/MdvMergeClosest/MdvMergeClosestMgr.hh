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
 * @file MdvMergeClosestMgr.hh
 * @brief The manager layer for the algorithm.
 * @class MdvMergeClosestMgr
 * @brief The manager layer for the algorithm.
 *        
 */

# ifndef    MdvMergeClosestMgr_hh
# define    MdvMergeClosestMgr_hh

#include "Parms.hh"
#include "MdvMergeClosest.hh"

class Trigger;

//----------------------------------------------------------------
class MdvMergeClosestMgr
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   * @param[in] appName  Name of app
   * @param[in] tidyAndExit  Cleanup method with return status arg
   *
   * Prepare the class for a call to the run() method
   */
  MdvMergeClosestMgr(const Parms &p, const std::string &appName,
		     void tidyAndExit(int));
  
  /**
   *  Destructor
   */
  virtual ~MdvMergeClosestMgr(void);

  /**
   * Run the app for all times
   * @return 1 for bad, 0 for good
   */
  int run(void);

protected:
private:  


  /**
   * The Algorithm parameters, kept as internal state
   */
  Parms _parms;

  /**
   * Triggering mechanism
   */
  Trigger *_trigger;


  /**
   * Algorithm
   */
  MdvMergeClosest _alg;

};

# endif 
