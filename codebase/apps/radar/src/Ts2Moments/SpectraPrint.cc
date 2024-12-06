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
/*********************************************************************
 * SpectraPrint: SpectraPrint program object.
 *
 *********************************************************************/

#include <cerrno>
#include <cstring>
#include <toolsa/file_io.h>
#include <toolsa/udatetime.h>
#include "SpectraPrint.hh"

SpectraPrint *SpectraPrint::_instance = (SpectraPrint *)NULL;

//////////////////
// Constructor

SpectraPrint::SpectraPrint(const Params &params) :
        _params(params)
{
  
  _out = NULL;
  _count = 0;

  // open combined spectra file if required
  
  if (_params.write_combined_spectra_file) {
    if (ta_makedir_recurse(_params.spectra_dir)) {
      cerr << "ERROR - Ts2Moments" << endl;
      cerr << "  Cannot make spectra output dir: "
	   << _params.spectra_dir << endl;
      return;
    }
    _filePath = _params.spectra_dir;
    _filePath += PATH_DELIM;
    _filePath += "combined_spectra.out";
    if ((_out = fopen(_filePath.c_str(), "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - Ts2Moments" << endl;
      cerr << "  Cannot open combined spectra file: "
	   << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return;
    }
  }

}

//////////////////
// Destructor


SpectraPrint::~SpectraPrint()
{
  if (_out != NULL) {
    fclose(_out);
  }
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

SpectraPrint *SpectraPrint::Inst(const Params &params)
{

  if (_instance == NULL) {
    new SpectraPrint(params);
  }
  return(_instance);
}

SpectraPrint *SpectraPrint::Inst()
{
  return(_instance);
}


////////////////////////////////////////////////////////////////////////
// Add spectrum to combined spectra output file

void SpectraPrint::addSpectrumToFile(time_t beamTime,
                                     double el, double az, int gateNum,
                                     double snr, double vel, double width,
                                     int nSamples, const RadarComplex_t *iq)

{

  date_time_t btime;
  btime.unix_time = beamTime;
  uconvert_from_utime(&btime);
  
  fprintf(_out,
	  "%d %d %d %d %d %d %d %g %g %d %g %g %g %d",
	  _count,
	  btime.year, btime.month, btime.day,
	  btime.hour, btime.min, btime.sec,
	  el, az, gateNum,
	  snr, vel, width, nSamples);
  
  for (int ii = 0; ii < nSamples; ii++) {
    fprintf(_out, " %g %g", iq[ii].re, iq[ii].im);
  }
  
  fprintf(_out, "\n");

  _count++;

}

