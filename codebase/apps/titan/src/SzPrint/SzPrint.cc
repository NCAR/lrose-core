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
///////////////////////////////////////////////////////////////
// SzPrint.cc
//
// SzPrint object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2004
//
///////////////////////////////////////////////////////////////
//
// SzPrint prints out MDV data in table format for performing
// SZ comparisons. Output is to stdout.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "SzPrint.hh"
using namespace std;

// Constructor

SzPrint::SzPrint(int argc, char **argv)

{

  isOK = true;
  _doNcarLongPp = false;
  _doNcarShortPp = false;
  _doNcarShortFft = false;

  // set programe name

  _progName = "SzPrint";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SzPrint::~SzPrint()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SzPrint::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx objects for each file, read in
  
  DsMdvx rph;
  if (_readFile(rph, _params.rph_mdv_path)) {
    cerr << "Cannot read in rph file: " << _params.rph_mdv_path << endl;
    return -1;
  }

  DsMdvx msz;
  if (_readFile(msz, _params.msz_mdv_path)) {
    cerr << "Cannot read in msz file: " << _params.msz_mdv_path << endl;
    return -1;
  }

  DsMdvx longPrt;
  if (_readFile(longPrt, _params.long_mdv_path)) {
    cerr << "Cannot read in long prt file: " << _params.long_mdv_path << endl;
    return -1;
  }

  DsMdvx ncarLongPp;
  _doNcarLongPp = false;
  if (strcmp(_params.ncar_long_pp_mdv_path, "none")) {
    if (_readFile(ncarLongPp, _params.ncar_long_pp_mdv_path)) {
      cerr << "Cannot read in ncar long pp file: "
	   << _params.ncar_long_pp_mdv_path << endl;
      return -1;
    }
    _doNcarLongPp = true;
  }

  DsMdvx ncarShortPp;
  _doNcarShortPp = false;
  if (strcmp(_params.ncar_short_pp_mdv_path, "none")) {
    if (_readFile(ncarShortPp, _params.ncar_short_pp_mdv_path)) {
      cerr << "Cannot read in ncar short pp file: "
	   << _params.ncar_short_pp_mdv_path << endl;
      return -1;
    }
    _doNcarShortPp = true;
  }

  DsMdvx ncarShortFft;
  _doNcarShortFft = false;
  if (strcmp(_params.ncar_short_fft_mdv_path, "none")) {
    if (_readFile(ncarShortFft, _params.ncar_short_fft_mdv_path)) {
      cerr << "Cannot read in ncar short fft file: "
	   << _params.ncar_short_fft_mdv_path << endl;
      return -1;
    }
    _doNcarShortFft = true;
  }

  _doPrint(rph, msz, longPrt, ncarLongPp, ncarShortPp, ncarShortFft);

  return 0;

}

///////////////////
// print

int SzPrint::_doPrint(const DsMdvx &rph,
		      const DsMdvx &msz,
		      const DsMdvx &longPrt,
		      const DsMdvx &ncarLongPp,
		      const DsMdvx &ncarShortPp,
		      const DsMdvx &ncarShortFft)


{

  // get pointers to dbz fields and data

  MdvxField *rphDbzFld = rph.getField("DBZ");
  if (rphDbzFld == NULL) {
    cerr << "Cannot find DBZ field, RPH file" << endl;
    return -1;
  }
  fl32 *rphDbz = (fl32 *) rphDbzFld->getVol();

  MdvxField *mszDbzFld = msz.getField("DBZ");
  if (mszDbzFld == NULL) {
    cerr << "Cannot find DBZ field, MSZ file" << endl;
    return -1;
  }
  fl32 *mszDbz = (fl32 *) mszDbzFld->getVol();

  MdvxField *longDbzFld = longPrt.getField("DBZ");
  if (longDbzFld == NULL) {
    cerr << "Cannot find DBZ field, long prt file" << endl;
    return -1;
  }
  fl32 *longDbz = (fl32 *) longDbzFld->getVol();

  MdvxField *ncarLongPpDbzFld = NULL;
  fl32 *ncarLongPpDbz = NULL;
  if (_doNcarLongPp) {
    ncarLongPpDbzFld = ncarLongPp.getField("DBZ");
    if (ncarLongPpDbzFld == NULL) {
      cerr << "Cannot find DBZ field, ncarLongPp file" << endl;
      return -1;
    }
    ncarLongPpDbz = (fl32 *) ncarLongPpDbzFld->getVol();
  }

  MdvxField *ncarShortPpDbzFld = NULL;
  fl32 *ncarShortPpDbz = NULL;
  if (_doNcarShortPp) {
    ncarShortPpDbzFld = ncarShortPp.getField("DBZ");
    if (ncarShortPpDbzFld == NULL) {
      cerr << "Cannot find DBZ field, ncarShortPp file" << endl;
      return -1;
    }
    ncarShortPpDbz = (fl32 *) ncarShortPpDbzFld->getVol();
  }

  MdvxField *ncarShortFftDbzFld = NULL;
  fl32 *ncarShortFftDbz = NULL;
  if (_doNcarShortFft) {
    ncarShortFftDbzFld = ncarShortFft.getField("DBZ");
    if (ncarShortFftDbzFld == NULL) {
      cerr << "Cannot find DBZ field, ncarShortFft file" << endl;
      return -1;
    }
    ncarShortFftDbz = (fl32 *) ncarShortFftDbzFld->getVol();
  }

  // get pointers to vel fields and data

  MdvxField *rphVelFld = rph.getField("VEL");
  if (rphVelFld == NULL) {
    cerr << "Cannot find VEL field, RPH file" << endl;
    return -1;
  }
  fl32 *rphVel = (fl32 *) rphVelFld->getVol();

  MdvxField *mszVelFld = msz.getField("VEL");
  if (mszVelFld == NULL) {
    cerr << "Cannot find VEL field, MSZ file" << endl;
    return -1;
  }
  fl32 *mszVel = (fl32 *) mszVelFld->getVol();

  MdvxField *longVelFld = longPrt.getField("VEL");
  if (longVelFld == NULL) {
    cerr << "Cannot find VEL field, long prt file" << endl;
    return -1;
  }
  fl32 *longVel = (fl32 *) longVelFld->getVol();

  MdvxField *ncarLongPpVelFld = NULL;
  fl32 *ncarLongPpVel = NULL;
  if (_doNcarLongPp) {
    ncarLongPpVelFld = ncarLongPp.getField("VEL");
    if (ncarLongPpVelFld == NULL) {
      cerr << "Cannot find VEL field, ncarLongPp file" << endl;
      return -1;
    }
    ncarLongPpVel = (fl32 *) ncarLongPpVelFld->getVol();
  }

  MdvxField *ncarShortPpVelFld = NULL;
  fl32 *ncarShortPpVel = NULL;
  if (_doNcarShortPp) {
    ncarShortPpVelFld = ncarShortPp.getField("VEL");
    if (ncarShortPpVelFld == NULL) {
      cerr << "Cannot find VEL field, ncarShortPp file" << endl;
      return -1;
    }
    ncarShortPpVel = (fl32 *) ncarShortPpVelFld->getVol();
  }

  MdvxField *ncarShortFftVelFld = NULL;
  fl32 *ncarShortFftVel = NULL;
  if (_doNcarShortFft) {
    ncarShortFftVelFld = ncarShortFft.getField("VEL");
    if (ncarShortFftVelFld == NULL) {
      cerr << "Cannot find VEL field, ncarShortFft file" << endl;
      return -1;
    }
    ncarShortFftVel = (fl32 *) ncarShortFftVelFld->getVol();
  }

  // get pointers to spw fields and data

  MdvxField *rphSpwFld = rph.getField("SPW");
  if (rphSpwFld == NULL) {
    cerr << "Cannot find SPW field, RPH file" << endl;
    return -1;
  }
  fl32 *rphSpw = (fl32 *) rphSpwFld->getVol();

  MdvxField *mszSpwFld = msz.getField("SPW");
  if (mszSpwFld == NULL) {
    cerr << "Cannot find SPW field, MSZ file" << endl;
    return -1;
  }
  fl32 *mszSpw = (fl32 *) mszSpwFld->getVol();

  MdvxField *longSpwFld = longPrt.getField("SPW");
  if (longSpwFld == NULL) {
    cerr << "Cannot find SPW field, long prt file" << endl;
    return -1;
  }
  fl32 *longSpw = (fl32 *) longSpwFld->getVol();

  MdvxField *ncarLongPpSpwFld = NULL;
  fl32 *ncarLongPpSpw = NULL;
  if (_doNcarLongPp) {
    ncarLongPpSpwFld = ncarLongPp.getField("SPW");
    if (ncarLongPpSpwFld == NULL) {
      cerr << "Cannot find SPW field, ncarLongPp file" << endl;
      return -1;
    }
    ncarLongPpSpw = (fl32 *) ncarLongPpSpwFld->getVol();
  }

  MdvxField *ncarShortPpSpwFld = NULL;
  fl32 *ncarShortPpSpw = NULL;
  if (_doNcarShortPp) {
    ncarShortPpSpwFld = ncarShortPp.getField("SPW");
    if (ncarShortPpSpwFld == NULL) {
      cerr << "Cannot find SPW field, ncarShortPp file" << endl;
      return -1;
    }
    ncarShortPpSpw = (fl32 *) ncarShortPpSpwFld->getVol();
  }

  MdvxField *ncarShortFftSpwFld = NULL;
  fl32 *ncarShortFftSpw = NULL;
  if (_doNcarShortFft) {
    ncarShortFftSpwFld = ncarShortFft.getField("SPW");
    if (ncarShortFftSpwFld == NULL) {
      cerr << "Cannot find SPW field, ncarShortFft file" << endl;
      return -1;
    }
    ncarShortFftSpw = (fl32 *) ncarShortFftSpwFld->getVol();
  }

  // radar params

  MdvxRadar longPrtRadar;
  longPrtRadar.loadFromMdvx(longPrt);
  double longPrtNyquist = longPrtRadar.getRadarParams().unambigVelocity;

  MdvxRadar rphRadar;
  rphRadar.loadFromMdvx(rph);
  double rphNyquist = rphRadar.getRadarParams().unambigVelocity;

  MdvxRadar mszRadar;
  mszRadar.loadFromMdvx(msz);
  double mszNyquist = mszRadar.getRadarParams().unambigVelocity;

  // censor if needed

  if (_params.censor_rph) {
    const Mdvx::field_header_t &fhdr = rphVelFld->getFieldHeader();
    int ngates = fhdr.nx;
    int censor[ngates];
    for (int iaz = 0; iaz < fhdr.ny; iaz++) {
      fl32 *velBeam = rphVel + (iaz * ngates);
      _computeVelSpeckleCensoring(velBeam, ngates, fhdr.missing_data_value,
				  rphNyquist, censor);
      fl32 *dbzBeam = rphDbz + (iaz * ngates);
      fl32 *spwBeam = rphSpw + (iaz * ngates);
      for (int ii = 0; ii < ngates; ii++) {
	if (censor[ii]) {
	  dbzBeam[ii] = rphDbzFld->getFieldHeader().missing_data_value;
	  velBeam[ii] = rphVelFld->getFieldHeader().missing_data_value;
	  spwBeam[ii] = rphSpwFld->getFieldHeader().missing_data_value;
	}
      }
    }
  }

  if (_params.censor_msz) {
    const Mdvx::field_header_t &fhdr = mszVelFld->getFieldHeader();
    int ngates = fhdr.nx;
    int censor[ngates];
    for (int iaz = 0; iaz < fhdr.ny; iaz++) {
      fl32 *velBeam = mszVel + (iaz * ngates);
      _computeVelSpeckleCensoring(velBeam, ngates, fhdr.missing_data_value,
				  mszNyquist, censor);
      fl32 *dbzBeam = mszDbz + (iaz * ngates);
      fl32 *spwBeam = mszSpw + (iaz * ngates);
      for (int ii = 0; ii < ngates; ii++) {
	if (censor[ii]) {
	  dbzBeam[ii] = mszDbzFld->getFieldHeader().missing_data_value;
	  velBeam[ii] = mszVelFld->getFieldHeader().missing_data_value;
	  spwBeam[ii] = mszSpwFld->getFieldHeader().missing_data_value;
	}
      }
    }
  }

  // get field headers

  const Mdvx::field_header_t &rphParams = rphDbzFld->getFieldHeader();
  const Mdvx::field_header_t &mszParams = mszDbzFld->getFieldHeader();
  const Mdvx::field_header_t &longParams = longDbzFld->getFieldHeader();

  fl32 rphDbzMiss = rphDbzFld->getFieldHeader().missing_data_value;
  fl32 mszDbzMiss = mszDbzFld->getFieldHeader().missing_data_value;
  fl32 longDbzMiss = longDbzFld->getFieldHeader().missing_data_value;
  fl32 rphVelMiss = rphVelFld->getFieldHeader().missing_data_value;
  fl32 mszVelMiss = mszVelFld->getFieldHeader().missing_data_value;
  fl32 longVelMiss = longVelFld->getFieldHeader().missing_data_value;
  fl32 rphSpwMiss = rphSpwFld->getFieldHeader().missing_data_value;
  fl32 mszSpwMiss = mszSpwFld->getFieldHeader().missing_data_value;
  fl32 longSpwMiss = longSpwFld->getFieldHeader().missing_data_value;

  int nGatesFirstTrip = rphParams.nx / 2;

  // loop through all azimuths and ranges

  for (int iy = 0; iy < rphParams.ny; iy++) {  // az

    for (int ix = 0; ix < nGatesFirstTrip; ix++) { // range

      int iRangeShort, iRangeLong;
      int tripNum;
      double range, dbz1, dbz2, pow1, pow2;
      _computeIndices(rphDbzFld, longDbzFld,
		      ix, iy, nGatesFirstTrip,
		      iRangeShort, iRangeLong, tripNum,
		      range, dbz1, dbz2, pow1, pow2);
      
      // get values for this az and range
      
      fl32 rphDbzVal = rphDbz[iy * rphParams.nx + iRangeShort];
      fl32 mszDbzVal = mszDbz[iy * mszParams.nx + iRangeShort];
      fl32 longDbzVal = longDbz[iy * longParams.nx + iRangeLong];

      fl32 rphVelVal = rphVel[iy * rphParams.nx + iRangeShort];
      fl32 mszVelVal = mszVel[iy * mszParams.nx + iRangeShort];
      fl32 longVelVal = longVel[iy * longParams.nx + iRangeLong];

      fl32 rphSpwVal = rphSpw[iy * rphParams.nx + iRangeShort];
      fl32 mszSpwVal = mszSpw[iy * mszParams.nx + iRangeShort];
      fl32 longSpwVal = longSpw[iy * longParams.nx + iRangeLong];

      fl32 ncarLongPpDbzVal = 0;
      fl32 ncarShortPpDbzVal = 0;
      fl32 ncarShortFftDbzVal = 0;
      fl32 ncarLongPpVelVal = 0;
      fl32 ncarShortPpVelVal = 0;
      fl32 ncarShortFftVelVal = 0;
      fl32 ncarLongPpSpwVal = 0;
      fl32 ncarShortPpSpwVal = 0;
      fl32 ncarShortFftSpwVal = 0;

      if (_doNcarLongPp) {
	const Mdvx::field_header_t &fParams = ncarLongPpDbzFld->getFieldHeader();
	ncarLongPpDbzVal = ncarLongPpDbz[iy * fParams.nx + iRangeLong];
	ncarLongPpVelVal = ncarLongPpVel[iy * fParams.nx + iRangeLong];
	ncarLongPpSpwVal = ncarLongPpSpw[iy * fParams.nx + iRangeLong];
      }
      if (_doNcarShortPp) {
	const Mdvx::field_header_t &fParams = ncarShortPpDbzFld->getFieldHeader();
	ncarShortPpDbzVal = ncarShortPpDbz[iy * fParams.nx + iRangeShort];
	ncarShortPpVelVal = ncarShortPpVel[iy * fParams.nx + iRangeShort];
	ncarShortPpSpwVal = ncarShortPpSpw[iy * fParams.nx + iRangeShort];
      }
      if (_doNcarShortFft) {
	const Mdvx::field_header_t &fParams = ncarShortFftDbzFld->getFieldHeader();
	ncarShortFftDbzVal = ncarShortFftDbz[iy * fParams.nx + iRangeShort];
	ncarShortFftVelVal = ncarShortFftVel[iy * fParams.nx + iRangeShort];
	ncarShortFftSpwVal = ncarShortFftSpw[iy * fParams.nx + iRangeShort];
      }
      
      // fold the short prt data to match long prt

      double foldedRphVel = rphVelVal;
      while (foldedRphVel > longPrtNyquist) {
	foldedRphVel -= longPrtNyquist * 2;
      }
      while (foldedRphVel < -longPrtNyquist) {
	foldedRphVel += longPrtNyquist * 2;
      }

      double foldedMszVel = mszVelVal;
      while (foldedMszVel > longPrtNyquist) {
	foldedMszVel -= longPrtNyquist * 2;
      }
      while (foldedMszVel < -longPrtNyquist) {
	foldedMszVel += longPrtNyquist * 2;
      }

      // check for missing

      int nMiss = 0;
      if (rphDbzVal == rphDbzMiss) {
	nMiss++;
      }
      if (mszDbzVal == mszDbzMiss) {
	nMiss++;
      }
      if (longDbzVal == longDbzMiss) {
	nMiss++;
      }
      if (rphVelVal == rphVelMiss) {
	nMiss++;
      }
      if (mszVelVal == mszVelMiss) {
	nMiss++;
      }
      if (longVelVal == longVelMiss) {
	nMiss++;
      }
      if (rphSpwVal == rphSpwMiss) {
	nMiss++;
      }
      if (mszSpwVal == mszSpwMiss) {
	nMiss++;
      }
      if (longSpwVal == longSpwMiss) {
	nMiss++;
      }

      if (_params.include_missing) {
	if (nMiss > 3) {
	  continue;
	}
      } else {
	if (nMiss > 0) {
	  continue;
	}
      }

      double rangeCorr = -42 + 20 * log10(range);
      double longSnr = longDbzVal - rangeCorr;
      double rphSnr = rphDbzVal - rangeCorr;
      double mszSnr = mszDbzVal - rangeCorr;
      
      cout << range << " ";
      cout << tripNum << " ";
      cout << pow1 << " ";
      cout << pow2 << " ";
      cout << dbz1 << " ";
      cout << dbz2 << " ";

      if (longDbzVal == longDbzMiss) {
	cout << "NaN ";
      } else {
	cout << longDbzVal << " ";
      }
      if (longVelVal == longVelMiss) {
	cout << "NaN ";
      } else {
	cout << longVelVal << " ";
      }
      if (longSpwVal == longSpwMiss) {
	cout << "NaN ";
      } else {
	cout << longSpwVal << " ";
      }

      if (rphDbzVal == rphDbzMiss) {
	cout << "NaN ";
      } else {
	cout << rphDbzVal << " ";
      }
      if (rphVelVal == rphVelMiss) {
	cout << "NaN NaN ";
      } else {
	cout << rphVelVal << " ";
	cout << foldedRphVel << " ";
      }
      if (rphSpwVal == rphSpwMiss) {
	cout << "NaN ";
      } else {
	cout << rphSpwVal << " ";
      }

      if (mszDbzVal == mszDbzMiss) {
	cout << "NaN ";
      } else {
	cout << mszDbzVal << " ";
      }
      if (mszVelVal == mszVelMiss) {
	cout << "NaN NaN ";
      } else {
	cout << mszVelVal << " ";
	cout << foldedMszVel << " ";
      }
      if (mszSpwVal == mszSpwMiss) {
	cout << "NaN ";
      } else {
	cout << mszSpwVal << " ";
      }

      cout << longSnr << " ";
      cout << rphSnr << " ";
      cout << mszSnr << " ";

      if (_doNcarLongPp) {
	cout << ncarLongPpDbzVal << " ";
	cout << ncarLongPpVelVal << " ";
	cout << ncarLongPpSpwVal << " ";
      }
      if (_doNcarShortPp) {
	cout << ncarShortPpDbzVal << " ";
	cout << ncarShortPpVelVal << " ";
	cout << ncarShortPpSpwVal << " ";
      }
      if (_doNcarShortFft) {
	cout << ncarShortFftDbzVal << " ";
	cout << ncarShortFftVelVal << " ";
	cout << ncarShortFftSpwVal << " ";
      }

      cout << endl;

    } // ix
  } // iy

  return 0;

}
  
///////////////////
// read in MDV file

int SzPrint::_readFile(DsMdvx &mdvx, const char *path)

{
  
  mdvx.clearRead();
  mdvx.setReadPath(path);
  mdvx.setDebug(_params.debug);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.addReadField("DBZ");
  mdvx.addReadField("VEL");
  mdvx.addReadField("SPW");
  if (mdvx.readVolume()) {
    cerr << "ERROR - SzPrint::_readFile" << endl;
    cerr << "  Cannot read in file: " << path << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////
// compute indices in range

void SzPrint::_computeIndices(MdvxField *shortDbzFld,
			      MdvxField *longDbzFld,
			      int ix, int iy,
			      int nGatesFirstTrip,
			      int &iRangeShort,
			      int &iRangeLong,
			      int &tripNum,
			      double &range,
			      double &dbz1,
			      double &dbz2,
			      double &pow1,
			      double &pow2)

{

  const Mdvx::field_header_t &shortParams = shortDbzFld->getFieldHeader();
  const Mdvx::field_header_t &longParams = longDbzFld->getFieldHeader();
  fl32 *longDbz = (fl32 *) longDbzFld->getVol();

  int ixShort1 = ix;
  int ixShort2 = ix + nGatesFirstTrip;
  double range1 = shortParams.grid_minx + shortParams.grid_dx * ixShort1;
  double range2 = shortParams.grid_minx + shortParams.grid_dx * ixShort2;
  int ixLong1 =
    (int) ((range1 - longParams.grid_minx) / longParams.grid_dx + 0.5);
  int ixLong2 =
    (int) ((range2 - longParams.grid_minx) / longParams.grid_dx + 0.5);
  double corr1 = 20 * log10(range1);
  double corr2 = 20 * log10(range2);
  dbz1 = longDbz[iy * longParams.nx + ixLong1];
  dbz2 = longDbz[iy * longParams.nx + ixLong2];
  pow1 = dbz1 - corr1;
  pow2 = dbz2 - corr2;

  switch (_params.trip) {

  case Params::STRONG_TRIP:
  case Params::WEAK_TRIP:
    {
      bool trip1 = true;
      if (pow1 > pow2) {
	if (_params.trip == Params::WEAK_TRIP) {
	  trip1 = false;
	}
      } else {
	if (_params.trip == Params::STRONG_TRIP) {
	  trip1 = false;
	}
      }
      if (trip1) {
	tripNum = 1;
	range = range1;
	iRangeShort = ixShort1;
	iRangeLong = ixLong1;
      } else {
	tripNum = 2;
	range = range2;
	iRangeShort = ixShort2;
	iRangeLong = ixLong2;
      }
    }
    break;

  case Params::FIRST_TRIP:
    tripNum = 1;
    range = range1;
    iRangeShort = ixShort1;
    iRangeLong = ixLong1;
    break;

  case Params::SECOND_TRIP:
    tripNum = 2;
    range = range2;
    iRangeShort = ixShort2;
    iRangeLong = ixLong2;
    break;

  } // switch

}


////////////////////////////////////////////////////////////////////////
// Compute vel censoring interest value

void SzPrint::_computeVelCensoring(const fl32 *vel,
				   int ngates,
				   fl32 missing_val,
				   double nyquist,
				   int *vcensor)
  
{
  
  // compute vel diff array
   
  double veld[ngates];
  veld[0] = 0.0;

  for (int ii = 1; ii < ngates; ii++) {
    double diff;
    if (vel[ii] == missing_val || vel[ii -1] == missing_val) {
      diff = nyquist / 2.0;
    } else {
      diff = vel[ii] - vel[ii -1];
      if (diff > nyquist) {
	diff -= 2.0 * nyquist;
      } else if (diff < -nyquist) {
	diff += 2.0 * nyquist;
      }
    }
    veld[ii] = diff;
  } // ii
  
  int nGatesUsed = 5;
  int nGatesHalf = nGatesUsed / 2;
  
  double vtexture[ngates];
  memset(vtexture, 0, ngates * sizeof(double));

  for (int ii = nGatesHalf; ii < ngates - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf; jj <= ii + nGatesHalf; jj++) {
      sum += veld[jj] * veld[jj];
      count++;
    } // jj
    double texture = sqrt(sum / count) / nyquist;
    // double interest = _computeInterest(texture, 40.0, 70.0);
    double interest = _computeInterest(texture, 0.0, 0.75);
    // double interest = texture;
    vtexture[ii] = interest;
  } // ii
  
  double dtest = nyquist / 4.0;
  double vspin[ngates];
  memset(vspin, 0, ngates * sizeof(double));
  for (int ii = nGatesHalf; ii < ngates - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf + 1; jj <= ii + nGatesHalf; jj++) {
      if (vel[ii] == missing_val || vel[ii -1] == missing_val) {
	sum++;
      } else {
	double mult = veld[jj-1] * veld[jj];
	if (mult < 0 && (fabs(veld[jj-1]) > dtest || fabs(veld[jj]) > dtest)) {
	  sum++;
	}
      }
      count++;
    } // jj
    double spin = sum / count;
    double interest = _computeInterest(spin, 0.0, 1.0);
    // double interest = spin;
    vspin[ii] = interest;
  } // ii
  
  memset(vcensor, 0, ngates * sizeof(int));
  for (int ii = 0; ii < ngates; ii++) {
    if (vtexture[ii] * vspin[ii] > 0.3) {
      vcensor[ii] = 1;
    }
  } // ii

}

////////////////////////////////////////////////////////////////////////
// Compute interest value

double SzPrint::_computeInterest(double xx,
				 double x0, double x1)
  
{

  if (xx <= x0) {
    return 0.01;
  }
  
  if (xx >= x1) {
    return 0.99;
  }
  
  double xbar = (x0 + x1) / 2.0;
  
  if (xx <= xbar) {
    double yy = (xx - x0) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 2.0 * yy2;
    return interest;
  } else {
    double yy = (x1 - xx) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 1.0 - 2.0 * yy2;
    return interest;
  }

}

////////////////////////////////////////////////////////////////////////
// Compute vel speckle censoring

void SzPrint::_computeVelSpeckleCensoring(const fl32 *vel,
					  int ngates,
					  fl32 missing_val,
					  double nyquist,
					  int *vcensor)
  
{

  memset(vcensor, 0, ngates * sizeof(int));

  for (int ii = 1; ii < ngates - 1; ii++) {
    if (vel[ii - 1] == missing_val && vel[ii + 1] == missing_val) {

      vcensor[ii] = 1;

    } else if (vel[ii - 1] != missing_val && vel[ii + 1] != missing_val) {

      double diff1 = vel[ii] - vel[ii -1];
      if (diff1 > nyquist) {
	diff1 -= 2.0 * nyquist;
      } else if (diff1 < -nyquist) {
	diff1 += 2.0 * nyquist;
      }

      double diff2 = vel[ii] - vel[ii -1];
      if (diff2 > nyquist) {
	diff2 -= 2.0 * nyquist;
      } else if (diff2 < -nyquist) {
	diff2 += 2.0 * nyquist;
      }

      double meanDiff = (fabs(diff1) + fabs(diff2)) / 2;
      if (meanDiff > nyquist / 3.0) {
	vcensor[ii] = 1;
      }

    }

  } // ii
}

