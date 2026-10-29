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
//////////////////////////////////////////////////////////
// MM5DataV2.cc
//
// Read in MM5 V2 file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
//////////////////////////////////////////////////////////


#include <mm5/MM5DataV2.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
#include <physics/thermo.h>
#include <sys/stat.h>
#include <toolsa/toolsa_macros.h>
#include <iostream>
#include <cerrno>
using namespace std;

int MM5DataV2::MM5_HEADER_LEN = 3360000;

//////////////
// Constructor

MM5DataV2::MM5DataV2 (const string &prog_name,
		      const string &path,
		      bool debug /* = false */,
		      const heartbeat_t heartbeat_func /* = NULL */,
		      bool dbzConstantIntercepts /* = true */ ) :

  MM5Data(prog_name, path, debug, heartbeat_func, dbzConstantIntercepts)

{

  _version = 2;

  _mif = (si32 **) umalloc2(20, 1000, sizeof(si32));
  _mrf = (fl32 **) umalloc2(20, 1000, sizeof(fl32));

  MEM_zero(_mifc);
  MEM_zero(_mrfc);

  _nForecasts = 0;
  _dataSetNum = -1;
  _dataSetOffset = 0;
 
}

/////////////
// Destructor

MM5DataV2::~MM5DataV2()

{
  _freeLabels();
}

////////////////
// read()
//
// Read the data in the set
//
// returns 0 on success, -1 on failure

int MM5DataV2::read()

{

  _dataSetNum++;
  _dataSetOffset = _dataSetNum * _dataSetLen;

  if (_debug) {
    fprintf(stderr, "Reading file %s\n", _path.c_str());
    fprintf(stderr, "  Data set num: %d\n", _dataSetNum);
  }

  if (_readMainHeaders()) {
    return (-1);
  }
  if (_readTimes()) {
    return (-1);
  }
  if (_readDataSet()) {
    return (-1);
  }

  return (0);

}

// get the FDDA start and end times - minutes

double MM5DataV2::getFddaStartTime() const

{
  return _mrf[5][315];
}

double MM5DataV2::getFddaEndTime() const

{
  return _mrf[5][316];
}

///////////////////
// findHdrRecords()
//
// Find the fortran record markers around the header
//

void MM5DataV2::findHdrRecords()

{

  si32 iii;
  int dataSet = 0;
  int count = 0;
  int prev_end = 0;
  int inHeader = FALSE;
  int start = 0, end;

  fprintf(stderr, "\n");
  fprintf(stderr, "%s: looking for FORTRAN headers in file:\n   %s\n",
	  _progName.c_str(), _path.c_str());
  fprintf(stderr, "\n");
  fprintf(stderr, "Addresses and lengths are 4-byte values\n\n");

  fseek(_in, 0, SEEK_SET);
  while (!feof(_in)) {
    if (ufread(&iii, sizeof(si32), 1, _in) != 1) {
      fprintf(stderr,
	      "Data   found: start %10d, end %10d, len %10d\n\n",
	      prev_end + 1, count - 1, count - prev_end - 1);
      fprintf(stderr, "End of file %s\n", _path.c_str());
      break;
    }
    BE_from_array_32(&iii, 4);
    if (iii == MM5_HEADER_LEN) {
      if (!inHeader) {
	start = count;
	inHeader = TRUE;
	dataSet++;
      } else {
	end = count;
	inHeader = FALSE;
	if (prev_end != 0) {
	  fprintf(stderr,
		  "Data   found: start %10d, end %10d, len %10d\n",
		  prev_end + 1, start - 1, start - prev_end - 1);
	}
	fprintf(stderr,
		"Data set %2d ----------------------------"
		"----------------------\n", dataSet);
	fprintf(stderr,
		"Header found: start %10d, end %10d, len %10d\n",
		start, end, end - start + 1);
	prev_end = end;
      }
    }
    count++;
  }

}

///////////////////////////////////////////////////
// _readMainHeaders()
//
// Reads the first header to set class members
//
// returns 0 on success, -1 on failure

int MM5DataV2::_readMainHeaders()

{

  // compute offset and go there
  
  fseek(_in, _dataSetOffset, SEEK_SET);

  // fortran record length
  _readFortRecLen();

  // MIF array
  
  if (ufread(*_mif, sizeof(si32), 20000, _in) != 20000) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot read mif array\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(*_mif, 20000 * sizeof(si32));

  // MRF array

  if (ufread(*_mrf, sizeof(fl32), 20000, _in) != 20000) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot read mrf array\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(*_mrf, 20000 * sizeof(fl32));

  if (_mif[0][0] != 6) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n",
	    _progName.c_str());
    fprintf(stderr, "This is not an MM5 output file.\n");
    return (-1);
  }
  unsigned int iset = _mif[0][0]-1;
  
  // set grid info, sizes
  
  nSigma = (int) (_mrf[4][100] + 0.5);

  nyDot = _mif[0][103];
  nxDot = _mif[0][104];
  nyDotCoarse = _mif[0][1];
  nxDotCoarse = _mif[0][2];
  nPtsDotPlane = nxDot * nyDot;

  nLon = nxDot - 1;
  nLat = nyDot - 1;

  _n3d = _mif[iset][200];
  _n2d = _mif[iset][201];

  // 3d field array has extra level for reading w field
  if (_field3d == NULL) {
    _field3d = (fl32 ***) umalloc3(nSigma+1, nxDot, nyDot, sizeof(fl32));
  }

  if (_field2d == NULL) {
    _field2d = (fl32 **) umalloc2(nxDot, nyDot, sizeof(fl32));
  }

  if (_halfSigma == NULL) {
    _halfSigma = (fl32 *) umalloc(nSigma * sizeof(fl32));
  }

  // load up half sigma levels

  for (int i = 0; i < nSigma; i++) {
    _halfSigma[nSigma - i - 1] = _mrf[5][i+101];
  }

  if (_debug) {
    fprintf(stderr, "nsigma: %d\n", nSigma);
    fprintf(stderr, "nxDot, nyDot: %d, %d\n", nxDot, nyDot);
    fprintf(stderr, "n3d, n2d: %d, %d\n", _n3d, _n2d);
    for (int i = 0; i < nSigma; i++) {
      fprintf(stderr, "half sigma[%d]: %g\n", i, _halfSigma[i]);
    }
  }

  // read in labels

  if (_readLabels()) {
    return -1;
  }

  // read the field names

  long field_name_start =
    sizeof(si32) +            // fortran rec len
    40000 * sizeof(si32) +    // mif and mrf
    5204 * 80;                // mifc strings

  char mifc[81];
  
  // initialize field number flags

  uFieldNum = -1;
  vFieldNum = -1;
  tFieldNum = -1;
  qFieldNum = -1;
  clwFieldNum = -1;
  rnwFieldNum = -1;
  iceFieldNum = -1;
  snowFieldNum = -1;
  graupelFieldNum = -1;
  nciFieldNum = -1;
  radTendFieldNum = -1;
  wFieldNum = -1;
  ppFieldNum = -1;
  pstarFieldNum = -1;
  groundTFieldNum = -1;
  rainConFieldNum = -1;
  rainNonFieldNum = -1;
  terrainFieldNum = -1;
  coriolisFieldNum = -1;
  resTempFieldNum = -1;
  latFieldNum = -1;
  lonFieldNum = -1;
  landUseFieldNum = -1;
  snowcovrFieldNum = -1;
  tseasfcFieldNum = -1;
  pblHgtFieldNum = -1;
  regimeFieldNum = -1;
  shfluxFieldNum = -1;
  lhfluxFieldNum = -1;
  ustFieldNum = -1;
  swdownFieldNum = -1;
  lwdownFieldNum = -1;
  soilT1FieldNum = -1;
  soilT2FieldNum = -1;
  soilT3FieldNum = -1;
  soilT4FieldNum = -1;
  soilT5FieldNum = -1;
  soilT6FieldNum = -1;
  sfcrnoffFieldNum = -1;
  t2FieldNum = -1;
  q2FieldNum = -1;
  u10FieldNum = -1;
  v10FieldNum = -1;
  mapfXFieldNum = -1;
  mapfDotFieldNum = -1;
  weasdFieldNum = -1;
  snowhFieldNum = -1;
  hc_rainFieldNum = -1;
  hn_rainFieldNum = -1;

  swfracFieldNum = -1;
  sunaltFieldNum = -1;
  sunazmFieldNum = -1;
  moonaltFieldNum = -1;
  moonazmFieldNum = -1;
  sunillFieldNum = -1;
  moonillFieldNum = -1;
  totalillFieldNum = -1;

  clwiFieldNum = -1;
  rnwiFieldNum = -1;
  iceiFieldNum = -1;
  snowiFieldNum = -1;
  pwvFieldNum = -1;

  sunBtwFieldNum = -1;
  sunEtwFieldNum = -1;
  sunAbtwFieldNum = -1;
  sunAetwFieldNum = -1;
  sunRiseFieldNum = -1;
  sunSetFieldNum = -1;
  sunArisFieldNum = -1;
  sunAsetFieldNum = -1;
  moonRisFieldNum = -1;
  moonSetFieldNum = -1;
  moonAriFieldNum = -1;
  moonAseFieldNum = -1;

  fseek(_in, field_name_start, SEEK_SET);

  for (int i = 0; i < _n3d; i++) {
    if (ufread(mifc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot read 3d field from mifc\n");
      perror(_path.c_str());
      return (-1);
    }
    mifc[8] = '\0';
    if (_debug) {
      fprintf(stderr, "3d field %d: %s\n", i, mifc);
    }
    fieldNames.push_back(mifc);
    if (!strcmp(mifc, "U       ")) {
      uFieldNum = i;
      fieldUnits.push_back("m/s");
    } else if (!strcmp(mifc, "V       ")) {
      vFieldNum = i;
      fieldUnits.push_back("m/s");
    } else if (!strcmp(mifc, "T       ")) {
      tFieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "Q       ")) {
      qFieldNum = i;
      fieldUnits.push_back("kg/kg");
    } else if (!strcmp(mifc, "CLW     ")) {
      clwFieldNum = i;
      fieldUnits.push_back("kg/kg");
    } else if (!strcmp(mifc, "RNW     ")) {
      rnwFieldNum = i;
      fieldUnits.push_back("kg/kg");
    } else if (!strcmp(mifc, "ICE     ")) {
      iceFieldNum = i;
      fieldUnits.push_back("kg/kg");
    } else if (!strcmp(mifc, "SNOW    ")) {
      snowFieldNum = i;
      fieldUnits.push_back("kg/kg");
    } else if (!strcmp(mifc, "RAD_TEND")) {
      radTendFieldNum = i;
      fieldUnits.push_back("K/day");
    } else if (!strcmp(mifc, "W       ")) {
      wFieldNum = i;
      fieldUnits.push_back("m/s");
    } else if (!strcmp(mifc, "PP      ")) {
      ppFieldNum = i;
      fieldUnits.push_back("Pa");
    } else {
      fieldUnits.push_back("");
    }
  }

  if (_debug) {
    fprintf(stderr, "uField: %d\n", uFieldNum);
    fprintf(stderr, "vField: %d\n", vFieldNum);
    fprintf(stderr, "tField: %d\n", tFieldNum);
    fprintf(stderr, "qField: %d\n", qFieldNum);
    fprintf(stderr, "clwField: %d\n", clwFieldNum);
    fprintf(stderr, "rnwField: %d\n", rnwFieldNum);
    fprintf(stderr, "iceField: %d\n", iceFieldNum);
    fprintf(stderr, "snowField: %d\n", snowFieldNum);
    fprintf(stderr, "radTendField: %d\n", radTendFieldNum);
    fprintf(stderr, "wField: %d\n", wFieldNum);
    fprintf(stderr, "ppField: %d\n", ppFieldNum);
  }
    
  for (int i = 0; i < _n2d; i++) {
    if (ufread(mifc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n", _progName.c_str());
      fprintf(stderr, "Cannot read 2d field from mifc\n");
      perror(_path.c_str());
      return (-1);
    }
    mifc[8] = '\0';
    if (_debug) {
      fprintf(stderr, "2d field %d: %s\n", i, mifc);
    }
    fieldNames.push_back(mifc);

    // You have to pad the names out to 8 characters long.

    if (!strcmp(mifc, "TERRAIN ")) {
      terrainFieldNum = i;
      fieldUnits.push_back("m");
    } else if (!strcmp(mifc, "PSTARCRS")) {
      pstarFieldNum = i;
      fieldUnits.push_back("Pa");
    } else if (!strcmp(mifc, "LATITCRS")) {
      latFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "LONGICRS")) {
      lonFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "PSTARCRS")) {
      pstarFieldNum = i;
      fieldUnits.push_back("m");

    } else if (!strcmp(mifc, "GROUND T")) {
      groundTFieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "RAIN CON")) {
      rainConFieldNum = i;
      fieldUnits.push_back("cm");
    } else if (!strcmp(mifc, "RAIN NON")) {
      rainNonFieldNum = i;
      fieldUnits.push_back("cm");
    } else if (!strcmp(mifc, "TERRAIN ")) {
      terrainFieldNum = i;
      fieldUnits.push_back("m");
    } else if (!strcmp(mifc, "MAPFACCR")) {
      mapfXFieldNum = i;
      fieldUnits.push_back("");
    } else if (!strcmp(mifc, "MAPFACDT")) {
      mapfDotFieldNum = i;
      fieldUnits.push_back("");
    } else if (!strcmp(mifc, "CORIOLIS")) {
      coriolisFieldNum = i;
      fieldUnits.push_back("/s");
    } else if (!strcmp(mifc, "RES TEMP")) {
      resTempFieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "LATITCRS")) {
      latFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "LONGICRS")) {
      lonFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "LAND USE")) {
      landUseFieldNum = i;
      fieldUnits.push_back("category");
    } else if (!strcmp(mifc, "SNOWCOVR")) {
      snowcovrFieldNum = i;
      fieldUnits.push_back("");
    } else if (!strcmp(mifc, "TSEASFC ")) {
      tseasfcFieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "PBL HGT ")) {
      pblHgtFieldNum = i;
      fieldUnits.push_back("m");
    } else if (!strcmp(mifc, "REGIME  ")) {
      regimeFieldNum = i;
      fieldUnits.push_back("");
    } else if (!strcmp(mifc, "SHFLUX  ")) {
      shfluxFieldNum = i;
      fieldUnits.push_back("w/m2");
    } else if (!strcmp(mifc, "LHFLUX  ")) {
      lhfluxFieldNum = i;
      fieldUnits.push_back("w/m2");
    } else if (!strcmp(mifc, "UST     ")) {
      ustFieldNum = i;
      fieldUnits.push_back("m/s");
    } else if (!strcmp(mifc, "SWDOWN  ")) {
      swdownFieldNum = i;
      fieldUnits.push_back("w/m2");
    } else if (!strcmp(mifc, "LWDOWN  ")) {
      lwdownFieldNum = i;
      fieldUnits.push_back("w/m2");
    } else if (!strcmp(mifc, "SOIL T 1")) {
      soilT1FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SOIL T 2")) {
      soilT2FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SOIL T 3")) {
      soilT3FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SOIL T 4")) {
      soilT4FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SOIL T 5")) {
      soilT5FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SOIL T 6")) {
      soilT6FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "SFCRNOFF")) {
      sfcrnoffFieldNum = i;
      fieldUnits.push_back("mm");
    } else if (!strcmp(mifc, "T2      ")) {
      t2FieldNum = i;
      fieldUnits.push_back("K");
    } else if (!strcmp(mifc, "Q2      ")) {
      q2FieldNum = i;
      fieldUnits.push_back("kg kg{-1}");
    } else if (!strcmp(mifc, "U10     ")) {
      u10FieldNum = i;
      fieldUnits.push_back("m s{-1}");
    } else if (!strcmp(mifc, "V10     ")) {
      v10FieldNum = i;
      fieldUnits.push_back("m s{-1}");
    } else if (!strcmp(mifc, "WEASD   ")) {
      weasdFieldNum = i;
      fieldUnits.push_back("mm");
    } else if (!strcmp(mifc, "SNOWH   ")) {
      snowhFieldNum = i;
      fieldUnits.push_back("m");
    } else if (!strcmp(mifc, "HRAINCON   ")) {
      hc_rainFieldNum = i;
      fieldUnits.push_back("cm");
    } else if (!strcmp(mifc, "HRAINNON  ")) {
      hn_rainFieldNum = i;
      fieldUnits.push_back("cm");

    } else if (!strcmp(mifc, "SWFRAC  ")) {
      swfracFieldNum = i;
      fieldUnits.push_back("");
    } else if (!strcmp(mifc, "SUNALT  ")) {
      sunaltFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "SUNAZM  ")) {
      sunazmFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "MOONALT ")) {
      moonaltFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "MOONAZM ")) {
      moonazmFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "SUNILL  ")) {
      sunillFieldNum = i;
      fieldUnits.push_back("lux");
    } else if (!strcmp(mifc, "MOONILL ")) {
      moonillFieldNum = i;
      fieldUnits.push_back("lux");
    } else if (!strcmp(mifc, "TOTALILL")) {
      totalillFieldNum = i;
      fieldUnits.push_back("lux");

    } else if (!strcmp(mifc, "CLWI    ")) {
      clwiFieldNum = i;
      fieldUnits.push_back("kg/m2");
    } else if (!strcmp(mifc, "RNWI    ")) {
      rnwiFieldNum = i;
      fieldUnits.push_back("kg/m2");
    } else if (!strcmp(mifc, "ICEI    ")) {
      iceiFieldNum = i;
      fieldUnits.push_back("kg/m2");
    } else if (!strcmp(mifc, "SNOWI   ")) {
      snowiFieldNum = i;
      fieldUnits.push_back("kg/m2");
    } else if (!strcmp(mifc, "PWV     ")) {
      pwvFieldNum = i;
      fieldUnits.push_back("kg/m2");

    } else if (!strcmp(mifc, "SUN-BTW ")) {
      sunBtwFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "SUN-ETW ")) {
      sunEtwFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "SUN-ABTW")) {
      sunAbtwFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "SUN-AETW")) {
      sunAetwFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "SUN-RISE")) {
      sunRiseFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "SUN-SET ")) {
      sunSetFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "SUN-ARIS")) {
      sunArisFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "SUN-ASET")) {
      sunAsetFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "MOON-RIS")) {
      moonRisFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "MOON-SET")) {
      moonSetFieldNum = i;
      fieldUnits.push_back("hr");
    } else if (!strcmp(mifc, "MOON-ARI")) {
      moonAriFieldNum = i;
      fieldUnits.push_back("deg");
    } else if (!strcmp(mifc, "MOON-ASE")) {
      moonAseFieldNum = i;
      fieldUnits.push_back("deg");

    } else {
      fieldUnits.push_back("");
    }

  } // i
    
  if (_debug) {
    fprintf(stderr, "pstarField: %d\n", pstarFieldNum);
    fprintf(stderr, "groundTField: %d\n", groundTFieldNum);
    fprintf(stderr, "rainConField: %d\n", rainConFieldNum);
    fprintf(stderr, "rainNonField: %d\n", rainNonFieldNum);
    fprintf(stderr, "terrainField: %d\n", terrainFieldNum);
    fprintf(stderr, "coriolisField: %d\n", coriolisFieldNum);
    fprintf(stderr, "resTempField: %d\n", resTempFieldNum);
    fprintf(stderr, "latField: %d\n", latFieldNum);
    fprintf(stderr, "lonField: %d\n", lonFieldNum);
    fprintf(stderr, "landUseField: %d\n", landUseFieldNum);
    fprintf(stderr, "snowcovrField: %d\n", snowcovrFieldNum);
    fprintf(stderr, "tseasfcField: %d\n", tseasfcFieldNum);
    fprintf(stderr, "pblHgtField: %d\n", pblHgtFieldNum);
    fprintf(stderr, "regimeField: %d\n", regimeFieldNum);
    fprintf(stderr, "shfluxField: %d\n", shfluxFieldNum);
    fprintf(stderr, "lhfluxField: %d\n", lhfluxFieldNum);
    fprintf(stderr, "ustField: %d\n", ustFieldNum);
    fprintf(stderr, "swdownField: %d\n", swdownFieldNum);
    fprintf(stderr, "mapfXField: %d\n", mapfXFieldNum);
    fprintf(stderr, "mapfDotField: %d\n", mapfDotFieldNum);
    fprintf(stderr, "soilT1Field: %d\n", soilT1FieldNum);
    fprintf(stderr, "soilT2Field: %d\n", soilT2FieldNum);
    fprintf(stderr, "soilT3Field: %d\n", soilT3FieldNum);
    fprintf(stderr, "soilT4Field: %d\n", soilT4FieldNum);
    fprintf(stderr, "soilT5Field: %d\n", soilT5FieldNum);
    fprintf(stderr, "soilT6Field: %d\n", soilT6FieldNum);
    fprintf(stderr, "sfcrnoffField: %d\n", sfcrnoffFieldNum);
    fprintf(stderr, "t2Field: %d\n", t2FieldNum);
    fprintf(stderr, "q2Field: %d\n", q2FieldNum);
    fprintf(stderr, "u10Field: %d\n", u10FieldNum);
    fprintf(stderr, "v10Field: %d\n", v10FieldNum);
    fprintf(stderr, "weasdField: %d\n", weasdFieldNum);
    fprintf(stderr, "snowhField: %d\n", snowhFieldNum);
    fprintf(stderr, "hc_rainField: %d\n", hc_rainFieldNum);
    fprintf(stderr, "hn_rainField: %d\n", hn_rainFieldNum);

    fprintf(stderr, "swfracField: %d\n", swfracFieldNum);
    fprintf(stderr, "sunaltField: %d\n", sunaltFieldNum);
    fprintf(stderr, "sunazmField: %d\n", sunazmFieldNum);
    fprintf(stderr, "moonaltField: %d\n", moonaltFieldNum);
    fprintf(stderr, "moonazmField: %d\n", moonazmFieldNum);
    fprintf(stderr, "sunillField: %d\n", sunillFieldNum);
    fprintf(stderr, "moonillField: %d\n", moonillFieldNum);
    fprintf(stderr, "totalillField: %d\n", totalillFieldNum);

    fprintf(stderr, "clwiField: %d\n", clwiFieldNum);
    fprintf(stderr, "rnwiField: %d\n", rnwiFieldNum);
    fprintf(stderr, "iceiField: %d\n", iceiFieldNum);
    fprintf(stderr, "snowiField: %d\n", snowiFieldNum);
    fprintf(stderr, "pwvField: %d\n", pwvFieldNum);

    fprintf(stderr, "sunBtwField: %d\n", sunBtwFieldNum);
    fprintf(stderr, "sunEtwField: %d\n", sunEtwFieldNum);
    fprintf(stderr, "sunAbtwField: %d\n", sunAbtwFieldNum);
    fprintf(stderr, "sunAetwField: %d\n", sunAetwFieldNum);
    fprintf(stderr, "sunRiseField: %d\n", sunRiseFieldNum);
    fprintf(stderr, "sunSetField: %d\n", sunSetFieldNum);
    fprintf(stderr, "sunArisField: %d\n", sunArisFieldNum);
    fprintf(stderr, "sunAsetField: %d\n", sunAsetFieldNum);
    fprintf(stderr, "moonRisField: %d\n", moonRisFieldNum);
    fprintf(stderr, "moonSetField: %d\n", moonSetFieldNum);
    fprintf(stderr, "moonAriField: %d\n", moonAriFieldNum);
    fprintf(stderr, "moonAseField: %d\n", moonAseFieldNum);
  }

  // compute field data length
  // includes 2*sizeof(si32) for each record to account for the
  // fortran record length entries

  _fieldDataLen =
    (_n3d * (nxDot * nyDot * nSigma * sizeof(fl32) + 2 * sizeof(si32))) +
    (_n2d * (nxDot * nyDot * sizeof(fl32) + 2 * sizeof(si32)));

  // wfield has one extra level

  if (wFieldNum >= 0) {
    _fieldDataLen += nxDot * nyDot * sizeof(fl32);
  }

  if (_debug) {
    fprintf(stderr, "fieldDataLen: %d bytes\n", _fieldDataLen);
  }

  // pTop etc

  pTop = _mrf[1][0];
  pos = _mrf[5][1] / 100.0;
  tso = _mrf[5][2];
  tlp = _mrf[5][3];

  if (_debug) {
    fprintf(stderr, "pTop, pos, tos, tlp: %g, %g, %g, %g\n",
	    pTop, pos, tso, tlp);
  }
  
  // projection info

  if (_mif[0][3] == 1) {
    proj_type = LAMBERT_CONF;
  } else if (_mif[0][3] == 2) {
    proj_type = STEREOGRAPHIC;
  } else if (_mif[0][3] == 3) {
    proj_type = MERCATOR;
  } else {
    proj_type = UNKNOWN;
  }
  center_lat = _mrf[0][1];
  center_lon = _mrf[0][2];
  cone_factor =  _mrf[0][3];
  true_lat1 = _mrf[0][4];
  true_lat2 = _mrf[0][5];
  grid_distance = _mrf[0][100];
  domain_scale_coarse = _mif[0][107];
  y1_in_coarse_domain = _mrf[0][101];
  x1_in_coarse_domain = _mrf[0][102];
  x1_in_mother_domain = _mif[0][106];
  y1_in_mother_domain = _mif[0][105];

  grid_distance_coarse = grid_distance * domain_scale_coarse;
  minx_dot_coarse = -1.0 * ((nxDotCoarse - 1.0) / 2.0) * grid_distance_coarse;
  miny_dot_coarse = -1.0 * ((nyDotCoarse - 1.0) / 2.0) * grid_distance_coarse;
  minx_dot =
    minx_dot_coarse + (x1_in_coarse_domain - 1.0) * grid_distance_coarse;
  miny_dot =
    miny_dot_coarse + (y1_in_coarse_domain -1.0) * grid_distance_coarse;
  minx_cross = minx_dot + grid_distance / 2.0;
  miny_cross = miny_dot + grid_distance / 2.0;

  // compute single forecast output len and number of
  // forecasts in the file

  _dataSetLen = MM5_HEADER_LEN + 2 * sizeof(si32) + _fieldDataLen;

  struct stat file_stat;

  if (stat(_path.c_str(), &file_stat)) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readMainHeaders\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot stat file %s\n", _path.c_str());
    perror(_path.c_str());
    return (-1);
  }

  _nForecasts = file_stat.st_size / _dataSetLen;

  if (_debug) {
    fprintf(stderr, "dataSetLen: %d bytes\n", _dataSetLen);
    fprintf(stderr, "_nForecasts: %d\n", _nForecasts);
  }

  return (0);

}

///////////////
// _readTimes()
//
// Reads the times
//
// returns 0 on success, -1 on failure

int MM5DataV2::_readTimes()

{
  
  // initalize class members
  
  modelTime = 0;
  outputTime = 0;
  forecastLeadTime = 0;
  forecastDelta = 0;

  // set times
  
  time_t model_run_time, forecast_time;
  int lead_time;
  
  if (_readDataSetTimes(_dataSetNum, model_run_time,
			forecast_time, lead_time) == 0) {
    modelTime = model_run_time;
    outputTime = forecast_time;
    forecastLeadTime = lead_time;
  } else {
    return -1;
  }
  
  // set forecast delta
  
  if (_nForecasts > 1) {
    time_t modelRunTimeStart, modelRunTimeEnd;
    time_t forecastTimeStart, forecastTimeEnd;
    int leadTimeStart, leadTimeEnd;
    if ((_readDataSetTimes(0, modelRunTimeStart,
			   forecastTimeStart,leadTimeStart) == 0) &&
	(_readDataSetTimes(_nForecasts - 1, modelRunTimeEnd,
			   forecastTimeEnd,leadTimeEnd) == 0)) {
      forecastDelta = (int)
	((double) (forecastTimeEnd - forecastTimeStart) /
	 (double) (_nForecasts - 1) + 0.5);
    }
  }

  if (_debug) {
    fprintf(stderr, "modelTime: %s\n", utimstr(modelTime));
    fprintf(stderr, "outputTime: %s\n", utimstr(outputTime));
    fprintf(stderr, "forecastLeadTime: %d\n", forecastLeadTime);
    fprintf(stderr, "forecastDelta: %d\n", forecastDelta);
  }
  
  return (0);

}

///////////////////////
// _readDataSetTimes()
//
// Reads the times for a data set
//
// returns 0 on success, -1 on failure

int MM5DataV2::_readDataSetTimes(int data_set_num,
				 time_t &model_run_time,
				 time_t &forecast_time,
				 int &lead_time)

{
  
  long start = _dataSetOffset + sizeof(si32) + 5000 * sizeof(si32);
  fseek(_in, start, SEEK_SET);
  
  si32 forecastHdr[20];
  if (ufread(forecastHdr, sizeof(si32), 20, _in) != 20) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readTimes\n", _progName.c_str());
    fprintf(stderr, "Cannot read forecastHdr\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(forecastHdr, 20 * sizeof(si32));

  //    if (_debug) {
  //      fprintf(stderr, "----> Forecast number %d\n", data_set_num);
  //      fprintf(stderr, "forecastTime: %d\n", forecastHdr[0]);
  //      fprintf(stderr, "modelTime: %d\n", forecastHdr[1]);
  //      fprintf(stderr, "min: %d\n", forecastHdr[10]);
  //      fprintf(stderr, "hour %d\n", forecastHdr[11]);
  //      fprintf(stderr, "day: %d\n", forecastHdr[12]);
  //      fprintf(stderr, "month: %d\n", forecastHdr[13]);
  //      fprintf(stderr, "year: %d\n", forecastHdr[14]);
  //      fprintf(stderr, "cent: %d\n", forecastHdr[15]);
  //    }
  
  date_time_t ftime;
  ftime.year = forecastHdr[15] * 100 + forecastHdr[14];
  ftime.month = forecastHdr[13];
  ftime.day = forecastHdr[12];
  ftime.hour = forecastHdr[11];
  ftime.min = forecastHdr[10];
  ftime.sec = 0;
  uconvert_to_utime(&ftime);
  forecast_time = ftime.unix_time;

  // compute model time 
  
  date_time_t time0, time1;
  MEM_zero(time0);
  MEM_zero(time1);
    
  time0.month = forecastHdr[0] / 10000;
  time0.day = (forecastHdr[0] % 10000) / 100;
  time0.hour = forecastHdr[0] % 100;
  uconvert_to_utime(&time0);
  
  time1.month = forecastHdr[1] / 10000;
  time1.day = (forecastHdr[1] % 10000) / 100;
  time1.hour = forecastHdr[1] % 100;
  uconvert_to_utime(&time1);
    
  lead_time = time0.unix_time - time1.unix_time;
  model_run_time = forecast_time - lead_time;
  
  return 0;

}

////////////////////
// read a data set()

int MM5DataV2::_readDataSet()

{

  if (_debug) {
    fprintf(stderr, "Reading dataSet num %d\n", _dataSetNum);
  }

  // 2D fields

  _read2dField(pstarFieldNum, "pstar", &pstar);
  _read2dField(groundTFieldNum, "ground temp", &ground_t);
  _read2dField(rainConFieldNum, "rainCon", &rain_con);
  _read2dField(rainNonFieldNum, "rainNon", &rain_non);
  _read2dField(terrainFieldNum, "terrain", &terrain);
  _read2dField(coriolisFieldNum, "coriolis", &coriolis);
  _read2dField2dot(coriolisFieldNum, "coriolis_dot", &coriolis_dot);
  _read2dField(resTempFieldNum, "resTemp", &res_temp);
  _read2dField(latFieldNum, "lat", &lat);
  _read2dField(lonFieldNum, "lon", &lon);
  _read2dField(landUseFieldNum, "landUse", &land_use);
  _read2dField(snowcovrFieldNum, "snowcovr", &snowcovr);
  _read2dField(tseasfcFieldNum, "tseasfc", &tseasfc);
  _read2dField(pblHgtFieldNum, "pblHgt", &pbl_hgt);
  _read2dField(regimeFieldNum, "regime", &regime);
  _read2dField(shfluxFieldNum, "shflux", &shflux);
  _read2dField(lhfluxFieldNum, "lhflux", &lhflux);
  _read2dField(ustFieldNum, "ust", &ust);
  _read2dField(swdownFieldNum, "swdown", &swdown);
  _read2dField(lwdownFieldNum, "lwdown", &lwdown);
  _read2dField(soilT1FieldNum, "soilT1", &soil_t_1);
  _read2dField(soilT2FieldNum, "soilT2", &soil_t_2);
  _read2dField(soilT3FieldNum, "soilT3", &soil_t_3);
  _read2dField(soilT4FieldNum, "soilT4", &soil_t_4);
  _read2dField(soilT5FieldNum, "soilT5", &soil_t_5);
  _read2dField(soilT6FieldNum, "soilT6", &soil_t_6);
  _read2dField(sfcrnoffFieldNum, "sfcrnoff", &sfcrnoff);
  _read2dField(t2FieldNum, "T2", &t2);
  _read2dField(q2FieldNum, "Q2", &q2);
  _read2dField(u10FieldNum, "U10", &u10);
  _read2dField(v10FieldNum, "V10", &v10);
  _read2dField(mapfXFieldNum, "mapfX", &mapf_x);
  _read2dField2dot(mapfDotFieldNum, "mapf_dot", &mapf_dot);
  _read2dField(weasdFieldNum, "weasd", &weasd);
  _read2dField(snowhFieldNum, "snowh", &snowh);
  _read2dField(hc_rainFieldNum, "hraincon", &snowh);
  _read2dField(hn_rainFieldNum, "hrainnon", &snowh);

  _read2dField(swfracFieldNum, "swfrac", &swfrac);
  _read2dField(sunaltFieldNum, "sunalt", &sunalt);
  _read2dField(sunazmFieldNum, "sunazm", &sunazm);
  _read2dField(moonaltFieldNum, "moonalt", &moonalt);
  _read2dField(moonazmFieldNum, "moonazm", &moonazm);
  _read2dField(sunillFieldNum, "sunill", &sunill);
  _read2dField(moonillFieldNum, "moonill", &moonill);
  _read2dField(totalillFieldNum, "totalill", &totalill);

  _read2dField(clwiFieldNum, "clwi", &clwi);
  _read2dField(rnwiFieldNum, "rnwi", &rnwi);
  _read2dField(iceiFieldNum, "icei", &icei);
  _read2dField(snowiFieldNum, "snowi", &snowi);
  _read2dField(pwvFieldNum, "pwv", &pwv);

  _read2dField(sunBtwFieldNum, "sun_btw", &sun_btw);
  _read2dField(sunEtwFieldNum, "sun_etw", &sun_etw);
  _read2dField(sunAbtwFieldNum, "sun_abtw", &sun_abtw);
  _read2dField(sunAetwFieldNum, "sun_aetw", &sun_aetw);
  _read2dField(sunRiseFieldNum, "sun_rise", &sun_rise);
  _read2dField(sunSetFieldNum, "sun_set", &sun_set);
  _read2dField(sunArisFieldNum, "sun_aris", &sun_aris);
  _read2dField(sunAsetFieldNum, "sun_aset", &sun_aset);
  _read2dField(moonRisFieldNum, "moon_ris", &moon_ris);
  _read2dField(moonSetFieldNum, "moon_set", &moon_set);
  _read2dField(moonAriFieldNum, "moon_ari", &moon_ari);
  _read2dField(moonAseFieldNum, "moon_ase", &moon_ase);

  // 3D fields

  _read3dField(uFieldNum, "U", &uu);
  _readUv2dot(uFieldNum, "U", &uu_dot);
  _read3dField(vFieldNum, "V", &vv);
  _readUv2dot(vFieldNum, "V", &vv_dot);
  _read3dField(tFieldNum, "temperature", &tk);
  _read3dField(qFieldNum, "Mixing ratio", &qq);
  _read3dField(ppFieldNum, "Pres pert", &pp);
  _read3dField(clwFieldNum, "clw", &clw);
  _read3dField(rnwFieldNum, "rnw", &rnw);
  _read3dField(iceFieldNum, "ice", &ice);
  _read3dField(snowFieldNum, "snow", &snow);
  _read3dField(radTendFieldNum, "radTend", &rad_tend);
  _readwField();

  // convert pstar to Pa
  _convertPstar();
  
  // set file state markers to next data set
  
  long nextOffset = _dataSetOffset + _dataSetLen;
  if(_fileSize - nextOffset > 100) {
    _more = true;
  } else {
    _more = false;
  }

  return (0);

}

//////////////
// _offset3d()
//
// Compute the offset of a 3d field
//

long MM5DataV2::_offset3d(int field_num_3d)

{

  // compute offset - Header + fortran rec lengths

  long offset = _dataSetOffset + MM5_HEADER_LEN + 2 * sizeof(si32);

  // previous 3d fields

  offset += field_num_3d *
    (nxDot * nyDot * nSigma * sizeof(fl32) + 2 * sizeof(si32));

  // w field has one extra level

  if (wFieldNum >= 0 && field_num_3d > wFieldNum) {
    offset += nxDot * nyDot * sizeof(fl32);
  }

  // fortran record length

  offset += sizeof(si32);

  return (offset);

}

//////////////////
// printHeaders()
//
// Print out header labels etc.

void MM5DataV2::printHeaders(FILE *out) const

{

  _printHeaders(out);

  // MIFC

  fprintf(out, "\n================== MIF ===================\n\n");
  for (int i = 0; i < 20000; i++) {
    int ii = i % 1000;
    int jj = i / 1000;
    // if (_mif[jj][ii] != -999) {
    if (_mifc[i] != NULL) {
      fprintf(out, "[%2d][%4d] = %8d: %s\n",
	      jj+1, ii+1, _mif[jj][ii], _mifc[i]);
    }
  }

  // MRFC

  fprintf(out, "\n================== MRF ===================\n\n");
  for (int i = 0; i < 20000; i++) {
    int ii = i % 1000;
    int jj = i / 1000;
    // if (_mrf[jj][ii] > -998.0 || _mrf[jj][ii] < -1000.0) {
    if (_mrfc[i] != NULL) {
      fprintf(out, "[%2d][%4d] = %8g: %s\n",
	      jj+1, ii+1, _mrf[jj][ii], _mrfc[i]);
    }
  }
    
}

//////////////////
// _freeLabels
//
// Free the MIF and MRF labels

void MM5DataV2::_freeLabels()

{

  for (int i = 0; i < 20000; i++) {
    if (_mifc[i]) {
      delete _mifc[i];
      _mifc[i] = NULL;
    }
    if (_mrfc[i]) {
      delete _mrfc[i];
      _mrfc[i] = NULL;
    }
  }

}

//////////////////
// _readLabels
//
// Read the MIF and MRF labels

int MM5DataV2::_readLabels()

{

  // free up the arrays

  _freeLabels();

  // read mifc

  long mifc_start = sizeof(si32) + // fortran rec len
    40000 * sizeof(si32);          // mif and mrf
  fseek(_in, mifc_start, SEEK_SET);
  
  char mifc[81];
  for (int i = 0; i < 20000; i++) {
    if (ufread(mifc, 80, 1, _in) != 1) {
      int errNum = errno;
      cerr << "ERROR - " << _progName << "::MM5DataV2::_readLabels" << endl;
      cerr << "  Cannot read mifc entry" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    // set nulls
    mifc[80] = '\0';
    for (int j = 79; j >= 0; j--) {
      if (mifc[j] != ' ') {
	break;
      }
      mifc[j] = '\0';
    }
    if (strlen(mifc) > 0) {
      _mifc[i] = new char[strlen(mifc)+1];
      strcpy(_mifc[i], mifc);
    }
  }

  // read mrfc

  char mrfc[81];
  for (int i = 0; i < 20000; i++) {
    if (ufread(mrfc, 80, 1, _in) != 1) {
      int errNum = errno;
      cerr << "ERROR - " << _progName << "::MM5DataV2::_readLabels" << endl;
      cerr << "  Cannot read mrfc entry" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    // set nulls
    mrfc[80] = '\0';
    for (int j = 79; j >= 0; j--) {
      if (mrfc[j] != ' ') {
	break;
      }
      mrfc[j] = '\0';
    }
    if (strlen(mrfc) > 0) {
      _mrfc[i] = new char[strlen(mrfc)+1];
      strcpy(_mrfc[i], mrfc);
    }
  }

  return 0;

}

///////////////////////////////////////////
// get MIF or MRF values, given the label

int MM5DataV2::getMifVal(const char *label) const

{

  for (int i = 0; i < 20000; i++) {
    if (_mifc[i] == NULL) {
      continue;
    }
    size_t nlabel = strlen(label);
    if (nlabel > strlen(_mifc[i])) {
      continue;
    }
    if (strncmp(_mifc[i], label, nlabel) == 0) {
      int ii = i % 1000;
      int jj = i / 1000;
      return _mif[jj][ii];
    }
  }

  return -999;

}

double MM5DataV2::getMrfVal(const char *label) const

{

  for (int i = 0; i < 20000; i++) {
    if (_mrfc[i] == NULL) {
      continue;
    }
    size_t nlabel = strlen(label);
    if (nlabel > strlen(_mrfc[i])) {
      continue;
    }
    if (strncmp(_mrfc[i], label, nlabel) == 0) {
      int ii = i % 1000;
      int jj = i / 1000;
      return _mrf[jj][ii];
    }
  }

  return -999.0;

}

//////////////
// _offset2d()
//
// Compute the offset of a 2d field
//

long MM5DataV2::_offset2d(int field_num_2d)

{
  
  // 3d fields

  long offset = _offset3d(_n3d);
  
  // prev 2d fields

  offset += field_num_2d *
    (nxDot * nyDot * sizeof(fl32) + 2 * sizeof(si32));
  
  return (offset);

}

/////////////////
// _read3dField()
//
// Read in a 3D field
//

int MM5DataV2::_read3dField(int field_num_3d,
			    const char *field_name, fl32 ****field_p)

{
  
  if (field_num_3d < 0) {
    return (-1);
  }

  if (*field_p == NULL) {
    *field_p = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  // zero out field
  memset(***field_p, 0, nSigma * nLat * nLon * sizeof(fl32));

  long offset = _offset3d(field_num_3d);
  fseek(_in, offset, SEEK_SET);

  int npts = nPtsDotPlane * nSigma;
  int nread = ufread(**_field3d, sizeof(fl32), npts, _in);
  if (nread != npts) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_read3dField\n", _progName.c_str());
    fprintf(stderr, "  Cannot read 3d field %d for %s\n", field_num_3d,
	    field_name);
    fprintf(stderr, "  Field  offset: %ld\n", offset);
    fprintf(stderr, "  nread: %d, nexpected: %d\n", nread, npts);
    perror(_path.c_str());
    return (-1);
  }
  
  BE_from_array_32(**_field3d, npts * sizeof(fl32));

  int idot = _mif[5][204+field_num_3d] / 10;
    
  if (idot == 0) {

    // cross field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  (*field_p)[jsig][ilat][ilon] =
	    _field3d[isig][ilon][ilat] / pstar[ilat][ilon];
	}
      }
    }

  } else {

    // dot field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  (*field_p)[jsig][ilat][ilon] =
	    ((_field3d[isig][ilon][ilat] +
	      _field3d[isig][ilon][ilat+1] +
	      _field3d[isig][ilon+1][ilat] +
	      _field3d[isig][ilon+1][ilat+1]) /
	     (pstar[ilat][ilon] * 4.0));
	}
      }
    }

  }

  return (0);

}
   
///////////////////////////////////////////////////////////////////
// read 3d field dot
// Reads in U and V fields, and leaves on dot grid,
// does not average data to cross points
//

int MM5DataV2::_readUv2dot(int field_num_3d,
			   const char *field_name, fl32 ****field_p)

{
  
  if (field_num_3d < 0) {
    return (-1);
  }

  long offset = _offset3d(field_num_3d);
  
  fseek(_in, offset, SEEK_SET);

  int npts = nPtsDotPlane * nSigma;
  
  if (ufread(**_field3d, sizeof(fl32), npts, _in) != npts) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readUv2dot\n", _progName.c_str());
    fprintf(stderr, "Cannot read 3d field %d for %s\n", field_num_3d,
	    field_name);
    perror(_path.c_str());
    return (-1);
  }
  
  BE_from_array_32(**_field3d, npts * sizeof(fl32));
  
  // Recall u and v are defined on the dot grid. 
  // Since the model u and v are coupled with pstar, 
  // and pstar is defined on the cross grid, we divide the values 
  // of u and v by the average of surrounding values of pstar.
  
  if (*field_p == NULL) {
    *field_p = (fl32 ***) umalloc3(nSigma, nyDot, nxDot, sizeof(fl32));
  }
    
  // zero out field
  memset(***field_p, 0, nSigma * nyDot * nxDot * sizeof(fl32));
  
  // assign corners of horizontal grid:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    (*field_p)[jsig][0][0] = _field3d[isig][0][0]/pstar[0][0];
    (*field_p)[jsig][0][nxDot - 1 ] =
      _field3d[isig][0][nxDot -1]/pstar[0][nxDot-2];
    (*field_p)[jsig][nyDot - 1][0] =
      _field3d[isig][nyDot-1][0]/pstar[nyDot-2][0];
    (*field_p)[jsig][nyDot - 1][nxDot - 1 ] = 
      _field3d[isig][nyDot - 1][nxDot - 1 ]/pstar[nyDot-2][nxDot -2];
  }
  
  //assign grid edges:
  //west edge:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilat = 1; ilat < nyDot - 1; ilat++) {
      fl32 ave_pstar = (pstar[ilat][0] + pstar[ilat-1][0])/2;
      (*field_p)[jsig][ilat][0] = _field3d[isig][ilat][0]/ave_pstar;
    }
  }
  
  //east edge:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilat = 1; ilat < nyDot - 1; ilat++) {
      fl32 ave_pstar = (pstar[ilat][nxDot-2] + pstar[ilat-1][nxDot-2])/2;
      (*field_p)[jsig][ilat][nxDot - 1] =
	_field3d[isig][ilat][nxDot - 1]/ave_pstar;
    }
  }
  
  //south edge:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilon = 1; ilon < nxDot - 1; ilon++) {
      fl32 ave_pstar = (pstar[0][ilon] + pstar[0][ilon - 1])/2;
      (*field_p)[jsig][0][ilon] = _field3d[isig][0][ilon]/ave_pstar;
    }
  }
  
  // north edge:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilon = 1; ilon < nxDot - 1; ilon++) {
      fl32 ave_pstar =
	(pstar[nyDot - 2][ilon] + pstar[nyDot - 2][ilon - 1])/2;
      (*field_p)[jsig][nyDot -1][ilon] =
	_field3d[isig][nyDot -1][ilon]/ave_pstar;
    }
  }
  
  // assign interior of grid:
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilat = 1; ilat < nyDot - 1; ilat++) {
      for (int ilon = 1; ilon < nxDot - 1; ilon++) {
	fl32 ave_pstar = (pstar[ilat-1][ilon-1] + pstar[ilat-1][ilon] +
			  pstar[ilat][ilon-1] + pstar[ilat][ilon])/4;
	(*field_p)[jsig][ilat][ilon] =
	  _field3d[isig][ilon][ilat] / ave_pstar;
      }
    }
  }
  
  return (0);

}  

/////////////////
// _read2dField()
//
// Read in a 2D field
//

int MM5DataV2::_read2dField(int field_num_2d,
			    const char *field_name, fl32 ***field_p)

{

  if (field_num_2d < 0) {
    return (-1);
  }

  long offset = _offset2d(field_num_2d);
  fseek(_in, offset, SEEK_SET);

  if (ufread(*_field2d, sizeof(fl32), nPtsDotPlane, _in) != nPtsDotPlane) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_read2dField\n", _progName.c_str());
    fprintf(stderr, "Cannot read 2d field %d for %s\n", field_num_2d,
	    field_name);
    perror(_path.c_str());
    return (-1);
  }

  BE_from_array_32(*_field2d, nPtsDotPlane * sizeof(fl32));

  if (*field_p == NULL) {
    *field_p = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }

  int idot = _mif[5][204+_n3d+field_num_2d] / 10;

  if (idot == 0) {

    // cross field

    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	(*field_p)[ilat][ilon] = _field2d[ilon][ilat];
      }
    }

  } else {

    // dot field

    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	(*field_p)[ilat][ilon] =
	  (_field2d[ilon][ilat] +
	   _field2d[ilon][ilat+1] +
	   _field2d[ilon+1][ilat] +
	   _field2d[ilon+1][ilat+1]) / 4.0;
      }
    }
    
  }

  return (0);

}

//////////////////////////////////////////////////////////////////
// read in a 2D dot field, leave on dot grid

int MM5DataV2::_read2dField2dot(int field_num_2d,
				const char *field_name, fl32 ***field_p)

{

  if (field_num_2d < 0) {
    return (-1);
  }

  long offset = _offset2d(field_num_2d);
  fseek(_in, offset, SEEK_SET);

  if (ufread(*_field2d, sizeof(fl32), nPtsDotPlane, _in) != nPtsDotPlane) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_read2dField2dot\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot read 2d field %d for %s\n", field_num_2d,
	    field_name);
    perror(_path.c_str());
    return (-1);
  }

  BE_from_array_32(*_field2d, nPtsDotPlane * sizeof(fl32));

  if (*field_p == NULL) {
    *field_p = (fl32 **) umalloc2(nyDot, nxDot, sizeof(fl32));
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      (*field_p)[ilat][ilon] = _field2d[ilon][ilat];
    }
  }

  return (0);

}

/////////////////
// _readwField()
//
// Read in W field - this is on full sigma levels and must be
// interpolated onto half sigma levels
//

int MM5DataV2::_readwField()

{
  
  if (ww == NULL) {
    ww = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  // zero out field
  memset(**ww, 0, nSigma * nLat * nLon * sizeof(fl32));
  
  if (wFieldNum < 0) {
    return (-1);
  }

  long offset = _offset3d(wFieldNum);
  fseek(_in, offset, SEEK_SET);
  
  int npts = nPtsDotPlane * (nSigma + 1);
  if (ufread(**_field3d, sizeof(fl32), npts, _in) != npts) {
    fprintf(stderr, "ERROR - %s:MM5DataV2::_readwField\n", _progName.c_str());
    fprintf(stderr, "Cannot read 3d field for w\n");
    perror(_path.c_str());
    return (-1);
  }
  
  BE_from_array_32(**_field3d, npts * sizeof(fl32));

  int idot = _mif[5][204+wFieldNum] / 10;
    
  if (idot == 0) {

    // cross field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  ww[jsig][ilat][ilon] =
	    (_field3d[isig][ilon][ilat] + _field3d[isig+1][ilon][ilat]) /
	    (pstar[ilat][ilon] * 2.0);
	}
      }
    }

  } else {

    // dot field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  ww[jsig][ilat][ilon] =
	    ((_field3d[isig][ilon][ilat] +
	      _field3d[isig][ilon][ilat+1] +
	      _field3d[isig][ilon+1][ilat] +
	      _field3d[isig][ilon+1][ilat+1] +
	      _field3d[isig+1][ilon][ilat] +
	      _field3d[isig+1][ilon][ilat+1] +
	      _field3d[isig+1][ilon+1][ilat] +
	      _field3d[isig+1][ilon+1][ilat+1]) /
	     (pstar[ilat][ilon] * 8.0));
	}
      }
    }

  }

  return (0);

}

/////////////////////////////////////////////////
// convert pstar to pa
   
void MM5DataV2::_convertPstar()

{
   
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      pstar[ilat][ilon] *= 1000.0;
    }
  }

}
