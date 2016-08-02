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
 *   $Date: 2016/03/06 23:28:58 $
 *   $Id: PrevTemporalSmoother.hh,v 1.5 2016/03/06 23:28:58 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * PrevTemporalSmoother: Temporal smoother that limits vector changes
 *                       to a percentage of the previous vector U and
 *                       V values.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PrevTemporalSmoother_HH
#define PrevTemporalSmoother_HH


#include "TemporalSmoother.hh"
using namespace std;


class PrevTemporalSmoother : public TemporalSmoother
{
 public:

  /**********************************************************************
   * Constructor
   */

  PrevTemporalSmoother(const string &prev_url,
		       const string &prev_u_field_name,
		       const string &prev_v_field_name,
		       const double u_max_percent,
		       const double v_max_percent,
		       const double prev_u_min,
		       const double prev_v_min,
		       const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~PrevTemporalSmoother(void);
  

  ///////////////////////
  // Smoothing methods //
  ///////////////////////

  /**********************************************************************
   * smoothData() - Smooth the current vectors based on the previous vectors.
   *
   * Updates the current U and V grids with the smoothed values.
   */

  virtual void smoothData(const time_t prev_data_time,
			  const int searchMargin,
			  fl32 *current_u_grid, fl32 *current_v_grid,
			  int grid_size, fl32 bad_data_value);

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _prevUFieldName;
  string _prevVFieldName;
  
  double _uMaxPercent;
  double _vMaxPercent;
  
  double _prevUMin;
  double _prevVMin;
  
};


#endif
