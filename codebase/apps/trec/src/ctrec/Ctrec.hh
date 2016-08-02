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
 *   $Date: 2016/03/06 23:28:57 $
 *   $Id: Ctrec.hh,v 1.18 2016/03/06 23:28:57 dixon Exp $
 *   $Revision: 1.18 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Ctrec.hh : header file for the Ctrec program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Ctrec_HH
#define Ctrec_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <toolsa/ldata_info.h>

#include "Args.hh"
#include "ClutterRemover.hh"
#include "CtrecAlg.hh"
#include "DataDetrender.hh"
#include "NoiseGenerator.hh"
#include "Params.hh"
#include "TemporalSmoother.hh"
using namespace std;


class Ctrec
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

  // Destructor

  ~Ctrec(void);
  
  // Get Ctrec singleton instance

  static Ctrec *Inst(int argc, char **argv);
  static Ctrec *Inst();
  
  // Run the program.

  void run();
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Ctrec *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Trigger object

  DsTrigger *_trigger;
  
  // Lookup table used when remapping the data into the desired
  // subgrid.

  MdvxRemapLut _subgridRemapLut;
  
  // Data detrender algorithm used by the clutter remover algorithm.

  DataDetrender *_detrender;
  
  // Clutter removal algorithm

  ClutterRemover *_clutterRemover;
  
  // Ctrec algorithm object

  CtrecAlg *_ctrecAlg;
  
  // Noise generator algorithm object

  NoiseGenerator _noiseGenerator;
  
  // Object that performs the temporal smoothing

  TemporalSmoother *_smoother;
  
  // Flag indicatine whether the thr_dbz value has been reset based
  // on the data.

  bool _thrDbzReset;


  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  Ctrec(int argc, char **argv);
  
  // Disallow the copy constructor and assignment operator

  Ctrec(const Ctrec&);
  const Ctrec& operator=(const Ctrec&);
  
  // Add the given field to the output file.
  
  void _addDebugOutputField(DsMdvx &output_mdv_file,
			    const MdvxField &field,
			    const string &transform) const;
  
  // Add the given vector field to the output file.

  void _addVectorOutputField(DsMdvx &output_mdv_file,
			     const Mdvx::field_header_t data_field_hdr,
			     const Mdvx::vlevel_header_t data_vlevel_hdr,
			     const fl32 *data,
			     const int field_code,
			     const string &field_name_long,
			     const string &field_name_short) const;
  
  // Cleanup the data so it is ready for processing.

  void _cleanupData(MdvxField &field, DsMdvx *output_mdv_file);
  
  // Create the output file.

  DsMdvx *_createOutputFile(const Mdvx::master_header_t input_master_hdr,
			    const int nx, const int ny,
			    const string dataset_source) const;
  
  // Run the ctrec algorithm on the 2 specified files and create the
  // output file.
  //
  // Returns true if the algorithm ran successfully, false otherwise.
  //
  // Note that the curr_field_orig pointer is added to the output Mdvx
  // object so the object is freed at the end of this method.

  bool _processFiles(const DsMdvx &prev_mdv_file,
		     const DsMdvx &curr_mdv_file,
		     const int image_delta_secs,
		     DsMdvx &output_mdv_file);
  
  // Random number generator of Park and Miller with Bays-Durham
  // shuffle and added safeguards.

  double _ranf(void);
  
  /**********************************************************************
   * _readNextFile() - Read the next file to be processed.  In realtime
   *                   mode, blocks until a new file is available.
   *
   * Returns true if successful, false otherwise.
   */

  bool _readNextFile(DsMdvx &mdv_file,
		     const time_t data_time);
  

  /**********************************************************************
   * _readPrevFile() - Read the previous file to be processed.
   *
   * Returns true if successful, false otherwise.
   */

  bool _readPrevFile(DsMdvx &mdv_file,
		     const time_t data_time);
  

  // Round the given value to the closest integer value

  static int _round(double x)
  {
    if ((x - floor(x)) >= .5) 
      return ((int)ceil(x));
    else
      return ((int)floor(x));
  }
  
  // Replace data below the threshold (i.e. missing or weak data)
  // with noise.

  void _trecFloor(MdvxField &field);

};


#endif
