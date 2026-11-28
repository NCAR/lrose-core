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
 * @file InputData.hh
 * @brief Handles one input data
 * @class InputData
 * @brief Handles one input data 
 */

# ifndef    INPUT_DATA_H
# define    INPUT_DATA_H

#include "Data.hh"
#include <Mdv/DsMdvx.hh>

//----------------------------------------------------------------
class InputData : public Data
{
public:

  typedef enum
  {
    BAD=-1,
    UNCHANGED=0,
    FIRST_DATA=1,
    NEW_DATA=2,
    TIMEOUT=3
  } Status_t;

  /**
   * @param[in] dparm  Params for this input
   * @param[in] parm  Overall params
   */
  InputData(const ParmInput &dparm, const Parms &parm);

  /**
   *  Destructor
   */
  virtual ~InputData(void);

  /**
   * Update for input triggering time
   * @param[in] t  Time
   * @return  status
   */
  Status_t update(const time_t &t);

protected:

private:  

  bool _first;       /**< True for no data yet */
  Status_t _status;  /**< current status */

};

# endif 
