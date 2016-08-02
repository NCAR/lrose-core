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
 * @file PpiInterpInfo.hh
 * @brief The Information (in and out) of the algorithm
 * @class PpiInterpInfo
 * @brief The Information (in and out) of the algorithm
 */

# ifndef    PPI_INTERP_INFO_HH
# define    PPI_INTERP_INFO_HH

#include <ctime>
class PpiInterp;


//----------------------------------------------------------------
class PpiInterpInfo
{
public:

  typedef enum {
    GRID_LOC, INTERP
  } ppi_task_t;

  /**
   * Default constructor, values are not set
   */
  PpiInterpInfo(void);
  
  /**
   * Constructor that sets all member values
   * @param[in] t  Time of data
   * @param[in] ray  Pointer to ray
   * @param[in] alg  Pointer to algorithm
   */
  PpiInterpInfo(ppi_task_t task, int y, int z, PpiInterp *alg);

  /**
   * Set all member values
   * @param[in] t  Time of data
   * @param[in] ray  Pointer to ray
   * @param[in] alg  Pointer to algorithm
   */
  void set(ppi_task_t task, int y, int z, PpiInterp *alg);

  /**
   * Destructor
   */
  virtual ~PpiInterpInfo(void);

  ppi_task_t _task; /**< What is the task */
  int _y;          /**< Index into data */
  int _z;          /**< Index into data */
  PpiInterp *_alg; /**< Pointer to main algorithm */

protected:
private:

};

#endif
