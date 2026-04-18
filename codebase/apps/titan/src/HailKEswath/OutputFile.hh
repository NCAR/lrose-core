/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1992, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// OutputFile.hh
//
// OutputFile class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include "Params.hh"
#include <string>
#include <Mdv/DsMdvx.hh>
using namespace std;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(const string &prog_name, const Params &params);
  
  // destructor
  
  virtual ~OutputFile();
  
  // write
  
  int write(time_t start_time,
	    time_t end_time,
	    time_t centroid_time,
	    const Mdvx::coord_t &grid,
	    fl32 *max_hailke_flux,
	    fl32 *max_hailmass_flux);

protected:
  
  void _setFieldName(const Mdvx::master_header_t &mhdr,
		     Mdvx::field_header_t &fhdr,
		     const char *name,
		     const char *name_long,
		     const char *units);
  
private:
  
  const string &_progName;
  const Params &_params;
  
  int _nFields;
  int _maxHailKeFluxFieldNum;
  int _maxHailMassFluxFieldNum;

};

#endif

