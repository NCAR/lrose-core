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
 *   $Date: 2016/03/04 02:22:13 $
 *   $Id: MdvVectorsTemporalSmooth.hh,v 1.2 2016/03/04 02:22:13 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvVectorsTemporalSmooth: MdvVectorsTemporalSmooth program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvVectorsTemporalSmooth_HH
#define MdvVectorsTemporalSmooth_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class MdvVectorsTemporalSmooth
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~MdvVectorsTemporalSmooth(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvVectorsTemporalSmooth *Inst(int argc, char **argv);
  static MdvVectorsTemporalSmooth *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvVectorsTemporalSmooth *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Input trigger object

  DsTrigger *_dataTrigger;
  
  // Speed/directions for the previous time period

  MdvxPjg _prevProj;
  time_t _prevGridTime;
  
  fl32 *_prevSpeed;
  fl32 *_prevDir;
  
  // Temporal limitations specified by the user in units needed
  // by the program for easiest calculations.

  double _speedDeltaMax;        // in m/s
  double _dirDeltaMax;          // in radians
  double _minOutputSpeed;       // in m/s
  double _minRecoverySpeed;     // in m/s
  
  
  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvVectorsTemporalSmooth(int argc, char **argv);
  

  /*********************************************************************
   * _calcAngleDiff() - Calculate the difference between the given angles.
   */

  static inline double _calcAngleDiff(double angle1, double angle2)
  {
    double angle_diff = fabs(angle1 - angle2);
	    
    if (angle_diff > 180.0)
      angle_diff = 360.0 - angle_diff;
	    
    return angle_diff;
  }


  /*********************************************************************
   * _calcSpeedDir() - Compute the speed and direction values for the
   *                   grid based on the current U/V values.
   */

  void _calcSpeedDir(const MdvxField &u_field,
		     const MdvxField &v_field,
		     fl32 *speed, fl32 *dir) const;
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(DateTime &trigger_time);
  

  /*********************************************************************
   * _readMdvFile() - Read the MDV file for the given time.
   */

  bool _readMdvFile(DsMdvx &input_mdv,
		    const string &input_url,
		    const DateTime &request_time,
		    const int search_margin = 0) const;
  
  bool _readMdvFile(DsMdvx &input_mdv,
		    const string &input_url,
		    const time_t request_time,
		    const int search_margin = 0) const;
  

  /*********************************************************************
   * _readPrevData() - If we don't currently have previous data (usually
   *                   because we just started or restarted), try to read
   *                   some in.
   */

  void _readPrevData(const time_t curr_grid_time);
  

  /*********************************************************************
   * _smoothSpeedDir() - Smooth the speed and direction values based on
   *                     the previous values and on the limitations specified
   *                     in the parameter file.
   */

  void _smoothSpeedDir(const MdvxPjg &proj, const time_t curr_grid_time,
		       fl32 *curr_speed, fl32 *curr_dir) const;
  

  /*********************************************************************
   * _updateUV() - Update the U and V field values based on the smoothed
   *               speed and direction values.  Don't update grid squares
   *               where the original U or V value was missing.
   */

  void _updateUV(MdvxField &u_field,
		 MdvxField &v_field,
		 const fl32 *speed,
		 const fl32 *dir) const;
  

};


#endif
