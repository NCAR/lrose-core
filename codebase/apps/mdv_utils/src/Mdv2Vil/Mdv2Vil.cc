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
// Mdv2Vil.cc
//
// Mdv2Vil object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Terri Betancourt
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Mdv2Vil.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
#include <didss/DsInputPath.hh>
using namespace std;

// Constructor

const fl32 Mdv2Vil::_missingFloat = -9999;

Mdv2Vil::Mdv2Vil(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Mdv2Vil";
  ucopyright(_progName.c_str());

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

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  if (!isOK) {
    return;
  }

  // input file object

  if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In FILELIST mode you must specify the files using -f arg." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }

  } else if (_params.mode == Params::ARCHIVE) {
      
    if (_args.startTime != 0 && _args.endTime != 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir,
			       _args.startTime,
			       _args.endTime);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
      
  } else {

    // realtime mode
    
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
                             _params.latest_data_info_avail);
    
  } // if (_params.mode == Params::FILELIST)

  _sssWeakDbzMax = _params.sss_weak_dbz_max;
  _sssSevereDbzMax = _params.sss_severe_dbz_max;
  _sssStdDeviationLimit = _params.sss_std_deviation_limit;
  _sssBaseCenterMassMax = _params.sss_base_center_mass_max;
  _sssTopCenterMassMin = _params.sss_top_center_mass_min;
  _sssBaseHeightMax = _params.sss_base_height_max;
  _sssTopHeightMin = _params.sss_top_height_min;

  _nfieldsOut = 0;
  _calcTotalVil = _calcDiffVil = _calcSSSIndex = false;
  if (_params.output_totalVil) {
    _nfieldsOut++;
    _calcTotalVil = true;
  }
  if (_params.output_dVil) {
    _nfieldsOut++;
    _calcDiffVil = true;
  }
  if (_params.output_SSS_index) {
    _nfieldsOut++;
    _calcSSSIndex = true;
  }
  if (_nfieldsOut == 0) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  No output fields have been specified." << endl;
    isOK = false;
    return;
  }

  // auto register with procmap
  
  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

Mdv2Vil::~Mdv2Vil()

{

  // unregister process

  PMU_auto_unregister();

  // free up
  
  delete _input;

}

//////////////////////////////////////////////////
// Run

int Mdv2Vil::Run ()
{

  PMU_auto_register("Mdv2Vil::Run");

  int iret = 0;
  const char *filePath;
  while ((filePath = _input->next()) != NULL) {
    if (_processFile(filePath)) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////
// _performIntegration()
//
// Load up grid and perform vertical integration
//
// Returns 0 on success, -1 on failure

int Mdv2Vil::_processFile(const char *filePath)

{

  if (_params.debug) {
    fprintf(stderr, "Processing file: %s\n", filePath);
  }

  PMU_auto_register("Processing file");

  // set up MDV object for reading
  
  DsMdvx mdvxIn;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvxIn.setDebug(true);
  }
  _setupRead(mdvxIn);
  mdvxIn.setReadPath(filePath);

  if (mdvxIn.readVolume()) {
    cerr << "ERROR - Mdv2Vil::_processFile" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvxIn.getErrStr() << endl;
    return -1;
  }
  
  // create output object
  
  DsMdvx mdvxOut;

  // set master header

  Mdvx::master_header_t mhdr = mdvxIn.getMasterHeader();
  mhdr.data_dimension = 2;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.time_gen = time(NULL);
  mhdr.n_fields = _nfieldsOut;
  mhdr.vlevel_included = true;
  mhdr.field_grids_differ = false;
  mdvxOut.setMasterHeader(mhdr);
  mdvxOut.setDataSetName("VIL");
  mdvxOut.setDataSetSource(filePath);
  string info = mhdr.data_set_info;
  info += "\nCreated by Mdv2Vil\n";
  mdvxOut.setDataSetInfo(info.c_str());

  // integrate to get VIL

  _computeVil(mdvxIn, mdvxOut);
  
  // write out

  if (_params.debug) {
    cerr << "Writing out results ..." << endl;
  }

  if(mdvxOut.writeToDir(_params.output_dir)) {
    cerr << "ERROR - Mdv2Vil::_processFile" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << mdvxOut.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << mdvxOut.getPathInUse() << endl;
  }

  return 0;

}

////////////////////////////////////////////
// set up the read

void Mdv2Vil::_setupRead(DsMdvx &mdvx)

{
  
  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // get file headers, to save encoding and compression info
  
  mdvx.setReadFieldFileHeaders();
  
  if (strlen(_params.dbz_field_name) > 0) {
    // use field name
    mdvx.addReadField(_params.dbz_field_name);
  } else {
    // use field number
    mdvx.addReadField(_params.dbz_field);
  }

  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}

///////////////////////////////////////////
// compute VIL

void Mdv2Vil::_computeVil(DsMdvx &inMdvx, DsMdvx &outMdvx)

{

  int iz, ipoint;
  fl32 *totalPlane, *diffPlane;
  ui08 *sssPlane;

  double *totalVilArray, *totalVilPtr;
  double *lowerVilArray, *lowerVilPtr;
  double *diffVilArray,  *diffVilPtr;
  int *sssIndexArray, *sssIndexPtr;
  
  double totalVilSum;
  double lowerVilSum;
  double upperVilVal;

  double planeHeight;

  // Used for SSS index calculations

  double centerMass, maxDbzHeight, maxDbz;
  double stdDeviation, mass = 0.0, *verticalMass;

  // set grid params

  const MdvxField *field = inMdvx.getField(0);
  const Mdvx::field_header_t &inFhdr = field->getFieldHeader();

  int nx = inFhdr.nx;
  int ny = inFhdr.ny;
  int nz = inFhdr.nz;
  int npoints = nx * ny;
  
  fl32 inMissing = (fl32) inFhdr.missing_data_value;

  // allocate data arrays

  totalVilArray = (double *) ucalloc (npoints, sizeof(double));
  lowerVilArray = (double *) ucalloc (npoints, sizeof(double));
  diffVilArray = (double *) ucalloc (npoints, sizeof(double));
  sssIndexArray = (int *) ucalloc (npoints, sizeof(int));

  totalPlane   = (fl32 *) ucalloc (npoints, sizeof(fl32));
  diffPlane    = (fl32 *) ucalloc (npoints, sizeof(fl32));
  sssPlane     = (ui08 *) ucalloc (npoints, sizeof(ui08));

  for (int ii = 0; ii < npoints; ii++) {
    totalPlane[ii] = _missingFloat;
    diffPlane[ii] = _missingFloat;
    sssPlane[ii] = 0;
  }
  
  // load up ht and delta ht arrays
  
  double *ht = (double *) umalloc(nz * sizeof(double));
  double *dht = (double *) umalloc(nz * sizeof(double));
  for (iz = 0; iz < nz; iz++) {
    ht[iz] = _getHeight(iz, *field);
    dht[iz] = _getDeltaHt(iz, *field);
  }

  // Initialize mass for SSS index calculations

  verticalMass = (double *) ucalloc (nz, sizeof(double));
  
  // loop through the points

  for (ipoint = 0; ipoint < npoints; ipoint++) {

    totalVilPtr = totalVilArray + ipoint;
    lowerVilPtr = lowerVilArray + ipoint;
    diffVilPtr  = diffVilArray + ipoint;
    sssIndexPtr = sssIndexArray + ipoint;

    VIL_INIT(totalVilSum);
    VIL_INIT(lowerVilSum);

    maxDbz = 0.0;
    maxDbzHeight = 0.0;
    
    // loop through planes
    
    for (iz = 0; iz < nz; iz++) {
      
      // get plane and height data
      // skip over this plane if it is below the specified min.
      
      planeHeight = ht[iz];
      if (planeHeight < _params.min_height) {
        continue;
      }
      double deltaHeight = dht[iz];
      fl32 *dbzPlane = ((fl32 *) field->getVol()) + npoints * iz;
      fl32 dbz = dbzPlane[ipoint];
      if (dbz == inMissing) {
        continue;
      }
      
      // Get reflectivity and max reflectivity at this point

      if (dbz > maxDbz) {
        maxDbz = dbz;
        maxDbzHeight = planeHeight;
      }

      // Accumulate the total vil
      
      VIL_ADD(totalVilSum, dbz, deltaHeight);
      
      // Accumulate the lower vil
      // ONLY IF we are below the specified difference_height
      
      if (planeHeight <= _params.difference_height) { 
        VIL_ADD(lowerVilSum, dbz, deltaHeight);
      }
      
      // Get mass for SSS index calculation
      
      MASS_Z(mass, dbz);
      verticalMass[iz] = mass;
      
    } /* iz */
    
    // Compute the total vil for this point
    
    VIL_COMPUTE(totalVilSum);
    *totalVilPtr = totalVilSum;

    // Compute the lower vil for this point 
    // Don't need the min/max since we're not writing this out to mdv

    VIL_COMPUTE(lowerVilSum);
    *lowerVilPtr = lowerVilSum;

    // Compute the vil difference for this point

    upperVilVal = *totalVilPtr - *lowerVilPtr;
    *diffVilPtr = upperVilVal - *lowerVilPtr;

    // Compute the sss index for this point

    if (_calcSSSIndex) {
      centerMass = stdDeviation = 0.0;
      if (totalVilSum > 0.1 && maxDbz > 30.0) {
        for (iz = 0; iz < nz; iz++) {
          centerMass += ht[iz] * verticalMass[iz] / totalVilSum;
        }
        for (iz = 0; iz < nz; iz++) {
          stdDeviation += pow(ht[iz] - centerMass, 2) 
            * verticalMass[iz] / totalVilSum;
        }
      }
      if (maxDbz >= 3.0) {
        *sssIndexPtr = _computeSSS(stdDeviation, centerMass, 
                                   maxDbzHeight, maxDbz);
      }
    }
    
  } // ipoint

  // free up ht arrays

  ufree(ht);
  ufree(dht);
  ufree(verticalMass);

  // Copy the results into the mdv planes

  for (ipoint = 0; ipoint < npoints; ipoint++) {

    if (totalVilArray[ipoint] > 0.01) {
      totalPlane[ipoint] = totalVilArray[ipoint];
    }

    if (fabs(diffVilArray[ipoint] > 0.01)) {
      diffPlane[ipoint]  = diffVilArray[ipoint];
    }

    sssPlane[ipoint] = sssIndexArray[ipoint];
    
  }

  // set up field header templates

  Mdvx::field_header_t tmpFhdr(inFhdr);
  tmpFhdr.nz = 1;
  tmpFhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  tmpFhdr.dz_constant = true;
  tmpFhdr.data_dimension = 2;
  tmpFhdr.grid_minz = 0;
  tmpFhdr.grid_dz = 1;

  Mdvx::vlevel_header_t tmpVhdr;
  MEM_zero(tmpVhdr);
  tmpVhdr.level[0] = 0;
  tmpVhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // Set the total vil field

  if (_calcTotalVil) {
    
    Mdvx::field_header_t fhdr(tmpFhdr);
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = npoints * sizeof(fl32);
    fhdr.missing_data_value = _missingFloat;
    fhdr.bad_data_value = _missingFloat;
    MdvxField *fld = new MdvxField(fhdr, tmpVhdr, totalPlane);
    fld->setFieldName("vil");
    fld->setFieldNameLong("Vertically-integrated liquid");
    fld->setUnits("kg/m2");
    fld->setTransform("none");
    fld->computeMinAndMax();
    fld->convertType(Mdvx::ENCODING_INT16,
                     Mdvx::COMPRESSION_GZIP);
    outMdvx.addField(fld);
    
  }

  // Set the difference vil field

  if (_calcDiffVil) {
    
    Mdvx::field_header_t fhdr(tmpFhdr);
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = npoints * sizeof(fl32);
    fhdr.missing_data_value = _missingFloat;
    fhdr.bad_data_value = _missingFloat;
    MdvxField *fld = new MdvxField(fhdr, tmpVhdr, diffPlane);
    fld->setFieldName("dVil");
    fld->setFieldNameLong("difference vil");
    fld->setUnits("kg/m2");
    fld->setTransform("none");
    fld->computeMinAndMax();
    fld->convertType(Mdvx::ENCODING_INT16,
                     Mdvx::COMPRESSION_GZIP);
    outMdvx.addField(fld);
    
  }

  // Set the SSS index field

  if (_calcSSSIndex) {
    
    Mdvx::field_header_t fhdr(tmpFhdr);
    fhdr.encoding_type = Mdvx::ENCODING_INT8;
    fhdr.data_element_nbytes = sizeof(ui08);
    fhdr.volume_size = npoints * sizeof(ui08);
    fhdr.missing_data_value = 0;
    fhdr.bad_data_value = 0;
    fhdr.scale = 1;
    fhdr.bias = 0;
    MdvxField *fld = new MdvxField(fhdr, tmpVhdr, sssPlane);
    fld->setFieldName("SSS");
    fld->setFieldNameLong("Storm Severity Index");
    fld->setUnits("index");
    fld->setTransform("none");
    fld->computeMinAndMax();
    fld->convertType(Mdvx::ENCODING_INT8,
                     Mdvx::COMPRESSION_GZIP);
    outMdvx.addField(fld);
    
  }

  // free resources

  ufree(totalVilArray);
  ufree(lowerVilArray);
  ufree(diffVilArray);
  ufree(sssIndexArray);
  ufree(totalPlane);
  ufree(diffPlane);
  ufree(sssPlane);

}

/////////////////////////////
// get height of level
//

double Mdv2Vil::_getHeight(int z, const MdvxField &field)                   
{
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  return vhdr.level[z];
}

////////////////////////////////////////////
// compute delta height from vertical levels

double Mdv2Vil::_getDeltaHt(int z, const MdvxField &field)
{
  
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  int nz = fhdr.nz;

  double base_ht = 0, top_ht = 0;

  if (nz == 1) {
    base_ht = vhdr.level[0] - fhdr.grid_dz / 2.0;
    top_ht  = vhdr.level[0] + fhdr.grid_dz / 2.0;
  } else {
    if (z == 0) {
      base_ht =  vhdr.level[0] - (vhdr.level[1] - vhdr.level[0]) / 2.0;
    } else {
      base_ht = (vhdr.level[z] + vhdr.level[z - 1]) / 2.0;
    }
    if (z == nz - 1) {
      top_ht  =  vhdr.level[nz - 1] + (vhdr.level[nz - 1] - vhdr.level[nz - 2]) / 2.0;
    } else {
      top_ht  = (vhdr.level[z] + vhdr.level[z + 1]) / 2.0;
    }
  }

  return(top_ht - base_ht);

}

///////////////////////////////////////////
// compute SSS

int Mdv2Vil::_computeSSS(double stdDeviation, double centerMass,
                         double maxDbzHeight, double maxDbz)
  
{

  int sss = 0;
  
  if(maxDbz < _sssWeakDbzMax) {
    if(centerMass < _sssBaseCenterMassMax) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = WB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssBaseHeightMax)
        sss = WV;
      if(stdDeviation < _sssStdDeviationLimit)
        sss = WB;
    } else if(centerMass < _sssTopCenterMassMin) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = WB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight == _sssBaseHeightMax)
        sss = WV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = WT;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = WV;
    } else {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssTopHeightMin)
        sss = WV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = WT;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = WT;
    }
  }
  
  if(maxDbz >= _sssWeakDbzMax  && maxDbz < _sssSevereDbzMax) {
    if(centerMass < _sssBaseCenterMassMax) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = SB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssBaseHeightMax)
        sss = SV;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = SB;
    } else if(centerMass < _sssTopCenterMassMin) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = SB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight == _sssBaseHeightMax)
        sss = SV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = ST;
      if(stdDeviation < _sssStdDeviationLimit)
        sss = SV;
    } else {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssTopHeightMin)
        sss = SV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = ST;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = ST;
    }
  }
  
  if(maxDbz > _sssSevereDbzMax) {
    if(centerMass < _sssBaseCenterMassMax) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = VSB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssBaseHeightMax)
        sss = VSV;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = VSB;
    } else if(centerMass < _sssTopCenterMassMin) {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssBaseHeightMax)
        sss = VSB;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight == _sssBaseHeightMax)
        sss = VSV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = VST;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = VSV;
    } else {
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight < _sssTopHeightMin)
        sss = VSV;
      if(stdDeviation >= _sssStdDeviationLimit &&
         maxDbzHeight >= _sssTopHeightMin)
        sss = VST;
      if(stdDeviation < _sssStdDeviationLimit) 
        sss = VST;
    }
  }

  return sss;

}
