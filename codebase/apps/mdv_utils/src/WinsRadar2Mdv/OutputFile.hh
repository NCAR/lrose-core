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
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include "Params.hh"
#include <Mdv/DsMdvx.hh>
#include <rapformats/WinsRadar.hh>
#include <string>
using namespace std;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(const string &prog_name,
	     const Params &params);
  
  // destructor
  
  virtual ~OutputFile();
  
  // return reference to DsMdvx object
  DsMdvx &getDsMdvx() { return (_mdvx); }

  // load up the data
  void loadData(const WinsRadar &radar);
  
  // write out merged volume
  int writeVol();

protected:
  
private:
  
  const string &_progName;
  const Params &_params;

  DsMdvx _mdvx;
  
};

#endif
