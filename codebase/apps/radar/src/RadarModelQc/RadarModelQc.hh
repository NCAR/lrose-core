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
 * @file RadarModelQc.hh
 * @brief main algorithm
 * @class RadarModelQc
 * @brief main algorithm
 */

#ifndef RADARMODELQC_H
#define RADARMODELQC_H

#include "Params.hh"
#include <RadxFiltAlg/RadxFiltAlg.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/TaThreadDoubleQue.hh>

class Filter;

class RadarModelQc
{
  
public:

  /**
   * Constructor
   * @param[in] argc
   * @param[in] argv
   * @param[in] cleanup  Method to call when exiting
   * @param[in] outOfStore Method to call when out of memory
   */  
  RadarModelQc (int argc, char **argv, void cleanup(int),
		void outOfStore(void));

  /**
   * Destructor
   */
  ~RadarModelQc(void);

  /**
   * Main method to run the algorithm
   * @return 0 for success 1 for not
   */
  int Run(void);

  /**
   * compute method for threading a beam
   *
   * @param[in] info  Pointer to Info passed in and out of algorithm
   */
  static void compute(void *info);

  bool OK;    /**< True if object is good */

protected:
private:

  /**
   * @class RadarThreads
   * @brief implement clone() method to have a TaThreadDoubleQue to use
   *        for threading.
   */
  class RadarThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Trivial constructor
     */
    inline RadarThreads(void) : TaThreadDoubleQue() {}
    /**
     * Trivial destructor
     */
    inline virtual ~RadarThreads(void) {}

    /**
     * @return pointer to a TaThread created by the method
     * @param[in] index  Index value that might be used
     */
    TaThread *clone(int index);
  };


  RadxFiltAlg _alg;           /**< The algorithm object used to run filters */
  Params _params;             /**< The params */
  vector<Filter *> _filters;  /**< The filters, in order, pointers because
			       *   they are derived classes */
  RadarThreads pThread;       /**< Threading object */
  bool pBeamStatus;           /**< status value shared amongst threads */


  bool _create_filter(const Params::data_filter_t &p);
  bool _process(const time_t t, RadxVol &vol);
  void _processRay(const time_t &t, RadxRay *ray);
  bool _filter(const time_t &t, Filter *f, RadxRay &ray,
	       vector<RayData> &data, const bool first);

};

#endif
