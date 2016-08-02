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
 * @file RadxQpe.hh
 * @brief  The algorithm
 * @class RadxQpe
 * @brief  The algorithm
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * April 2014
 *
 * RadxQpe reads an input beam blockage volume, and then reads in
 * PID volumes, producing output volumes in the same space as the PID,
 * with precip information.
 */

#ifndef RADX_PRECIP_HH
#define RADX_PRECIP_HH

#include "Args.hh"
#include "Parms.hh"
#include <toolsa/TaThreadDoubleQue.hh>

class Data;
class OutputData;
class BeamBlock;
class QpeInfo;

//---------------------------------------------------------------------------
class RadxQpe
{
  
public:
  /**
   * @param[in] parms   Alg parameters
   */
  RadxQpe (const Parms &parms);

  /**
   * Destructor
   */
  ~RadxQpe(void);

  /**
   * Run the algorithm, 
   * @return 1 for failure, 0 for succes
   */
  void process(Data &data, const BeamBlock &block, const time_t &t,
	       OutputData &out);

protected:
private:

  Parms _params;                /**< Alg parameters */

  /**
   * @class QpeThreads
   * @brief Instantiate TaThreadDoubleQue by implementing clone() method
   */
  class QpeThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Trivial constructor
     */
    inline QpeThreads(void) : TaThreadDoubleQue() {}
    /**
     * Trivial destructor
     */
    inline virtual ~QpeThreads(void) {}

    /**
     * @return pointer to TaThread created in the method
     * @param[in] index  Index value to maybe use
     */
    TaThread *clone(int index);
  };

  /**
   * Threading
   */
  QpeThreads _thread;
  
  /**
   * Static method called within each thread
   * @param[in] thread_data  Pointer to a QpeInfo object, to be freed by
   *                         this method
   */
  static void _computeInThread(void *thread_data);

  /**
   * The method that can be threaded
   */
  void _processAzimuth(QpeInfo &info);
};

#endif

