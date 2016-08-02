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
// mdv2netCDF.cc
//
// mdv2netCDF object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// mdv2netCDF converts MDV files to netCDF, using the
// COARDS convention.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <Mdv/DsMdvx.hh>
#include <dsserver/DsLdataInfo.hh>
#include "mdv2netCDF.hh"
#include "RapNetCDF.hh"
using namespace std;

// Constructor

mdv2netCDF::mdv2netCDF(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "mdv2netCDF";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }
  
  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  if (_args.outputFilePath.size() > 0) {
    if (_args.inputFileList.size() != 1) {
      cerr << "ERROR: -of specified, for one output file." << endl;
      cerr << "  Therefore, specify one input file with -if." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.InMdvURL, _params.RealtimeMaxAge,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.InMdvURL,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  if (!isOK) {
    return;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.Instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

mdv2netCDF::~mdv2netCDF()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int mdv2netCDF::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.clearRead();
  mdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);

  // Set the input field names, if requested

  if (!_params.DoAllFields)
  {
    for (int i = 0; i < _params.FieldNames_n; ++i)
      mdvx.addReadField(_params._FieldNames[i]);
  }
  
  //
  // Set up remappings as requested by the user.
  //
  // Vlevel limits.
  //
  if (_params.applyVleveLimits){
    mdvx.setReadVlevelLimits(_params.minVlevel, _params.maxVlevel);
    if (_params.debug){
      cerr << "Setting vlevel limits from " << _params.minVlevel;
      cerr << " to " <<  _params.maxVlevel << endl;
    }
  }
  //
  // Compositing.
  //
  if (_params.composite){
    mdvx.setReadComposite();
    if (_params.debug)
      cerr << "Compositing switched on." << endl;
  }
  //
  // Remapping.
  //
  if (_params.remap_xy){

    switch (_params.remap_projection){


    case Params::PROJ_LATLON :
      if (_params.debug)
	cerr << "Remapping to lat/lon." << endl;
      mdvx.setReadRemapLatlon(_params.remap_grid.nx,
			      _params.remap_grid.ny,
			      _params.remap_grid.minx,
			      _params.remap_grid.miny,
			      _params.remap_grid.dx,
			      _params.remap_grid.dy);
      break;

    case Params::PROJ_FLAT :
      if (_params.debug)
	cerr << "Remapping to flat." << endl;
      mdvx.setReadRemapFlat(_params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
			    _params.remap_rotation);
      break;
 
    case Params::PROJ_LAMBERT_CONF :
      if (_params.debug)
	cerr << "Remapping to lambert." << endl;
      mdvx.setReadRemapLc2(_params.remap_grid.nx,
			   _params.remap_grid.ny,
			   _params.remap_grid.minx,
			   _params.remap_grid.miny,
			   _params.remap_grid.dx,
			   _params.remap_grid.dy,
			   _params.remap_origin_lat,
			   _params.remap_origin_lon,
			   _params.remap_lat1,
			   _params.remap_lat2);
      break;

    default :
      cerr << "Unrecognised projection type : ";
      cerr << _params.remap_projection << endl;
      exit(-1);
      break;
    }
  }

  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // set for uncompressed floats.

    mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
    if (_input.readVolumeNext(mdvx)) {
      cerr << "ERROR - mdv2netCDF::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Reading MDV file: " << mdvx.getPathInUse() << endl;
    }
    
    // write out

    PMU_auto_register("Before write");

    if (_convert(mdvx)) {
      cerr << "ERROR - mdv2netCDF::Run" << endl;
      cerr << "  Cannot convert file: " << mdvx.getPathInUse() << endl;
      return -1;
    }

    if (_params.Once) {
      return 0;
    }

  } // while

  return 0;

}

///////////////////////////////
// write the output text header

int mdv2netCDF::_convert(const DsMdvx &mdvx)

{

  date_time_t dataTime;
  dataTime.unix_time = mdvx.getMasterHeader().time_centroid;
  uconvert_from_utime( &dataTime );
  
  // set up output file path

  char outPath[MAX_PATH_LEN];

  if (_args.outputFilePath.size() > 0) {
    
    STRncopy(outPath, _args.outputFilePath.c_str(), MAX_PATH_LEN);
    
  } else if (_params.OutputPathFromTime) {

    sprintf(outPath, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	    _params.OutDir,
	    PATH_DELIM,
	    dataTime.year, dataTime.month, dataTime.day,
	    PATH_DELIM,
	    dataTime.hour, dataTime.min, dataTime.sec,
	    _params.OutFileExt);

  } else {

    // See if we need to append '.nc' to the filename.
    
    char *last3 =
      _params.NetCDF_FileName + strlen(_params.NetCDF_FileName) - 3;
    char fileName[MAX_PATH_LEN];
    
    if ((strlen(_params.NetCDF_FileName) <4) || (strncmp(last3,".nc",3))){
      sprintf(fileName,"%s.nc",_params.NetCDF_FileName);
    } else {
      sprintf(fileName,"%s",_params.NetCDF_FileName);
    }
    
    sprintf(outPath, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_%s",
	    _params.OutDir,
	    PATH_DELIM,
	    dataTime.year, dataTime.month, dataTime.day,
	    dataTime.hour, dataTime.min, dataTime.sec,
	    fileName);

  }

  if (_params.debug) {
    cerr << "Writing netCDF file: " << outPath << endl;
  }

  // make sure output dir exists

  Path outP(outPath);
  if (outP.makeDirRecurse()) {
    cerr << "ERROR - mdv2netCDF::_convert" << endl;
    cerr << "  Cannot create dir for path: " << outPath << endl;
    return -1;
  }

  // create ndcf converter object

  RapNetCDF ncdf(_params, dataTime, outPath);
  RapNetCDF::RapNCD_t Q;
  
  // Write the first field.

  MdvxField *field = mdvx.getFieldByNum(0);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  
  int firstNx = fhdr.nx;
  int firstNy = fhdr.ny;
  int firstNz = fhdr.nz;
  
  ncdf.Mdv2NCD(field, dataTime, &Q);

  char VarName[256];

  if (_params.debug) {
    sprintf(VarName,"%s",fhdr.field_name);
    cerr << "Adding var name: " << VarName << endl;
    }


  ncdf.NCDWriteFirstVar(Q); 

  //
  // write subsequent fields
  //
  char Units[256];
  char LongName[256];

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader(); 

  for (int i=1; i < mhdr.n_fields; i++){
    PMU_auto_register("Converting field...");
    field = mdvx.getFieldByNum(i); 

    const Mdvx::field_header_t &fhdr = field->getFieldHeader();

    if (firstNx != fhdr.nx ||
	firstNy != fhdr.ny ||
	firstNz != fhdr.nz)
    {
      cerr << "Field dimensions differ - I cannot cope!" << endl;
      exit(-1);
    }
    
    //
    // See if we are doing substitutions.
    //
    int subsIndex = -1;
    for (int j=0; j < _params.substitutions_n; j++){
      if (0==strcmp(_params._substitutions[j].mdvFieldname,
		    fhdr.field_name)){
	subsIndex = j;
	break;
      }
    }
    //
    // OK - at this point, if subsIndex == -1 then we
    // are NOT doing substitutions for name, units etc. Otherwise
    // we are.
    //
    if (subsIndex == -1){

      sprintf(Units,"%s",fhdr.units);
      if (_params.AddNumToFieldName) {
	sprintf(VarName,"%s_%02d",fhdr.field_name,i);
      } else {
	sprintf(VarName,"%s",fhdr.field_name);
      }

      for(size_t k=0; k < strlen(VarName); k++){
	if (!(isalnum(VarName[k]))) VarName[k] = '_';
      }

      sprintf(LongName, "%s", VarName);

      
    } else {
      //
      // We are doing substitutions.
      //
      sprintf(VarName, "%s", _params._substitutions[subsIndex].netCDFfieldname);
      sprintf(Units, "%s", _params._substitutions[subsIndex].units);
      sprintf(LongName, "%s", _params._substitutions[subsIndex].longName );
    }
      
    float *data = (float *) field->getVol();
    for (int l=0; l< fhdr.nx * fhdr.ny * fhdr.nz; l++){
      if (data[l] == fhdr.bad_data_value)
	data[l] = fhdr.missing_data_value;
    }

    if (_params.debug) {
      cerr << "Adding var name: " << VarName << endl;
    }

    PMU_auto_register("Adding another field.");
    ncdf.NCDAddAnother(VarName,
		       Units,
		       LongName,
		       fhdr.missing_data_value,
		       fhdr.missing_data_value,
		       data);      

  }

  //
  // See if we want to add a height variable.
  //
  if (_params.addHeight.addHeightVar){
    
    MdvxField *heightField = 
      mdvx.getFieldByName(_params.addHeight.mdvFieldName);
    
    if (heightField == NULL){
      cerr << "ERROR - height field " << _params.addHeight.mdvFieldName;
      cerr << " not found." << endl;
      exit(-1);
    }
    
    const Mdvx::field_header_t &hFhdr = field->getFieldHeader();
    const Mdvx::vlevel_header_t &hVhdr=  field->getVlevelHeader();
    
    //
    // Put together the variable.
    //
    fl32 *heightData = (fl32 *) malloc(sizeof(fl32)*hFhdr.nz*hFhdr.ny*hFhdr.nx);
    if (heightData == NULL){
      cerr << "Height malloc failed!" <<  endl; // unlikely.
      exit(-1);
    }
    
    for (int iz=0; iz < hFhdr.nz; iz++){
      for (int iy=0; iy < hFhdr.ny; iy++){
	for (int ix=0; ix < hFhdr.nx; ix++){
	  //
	  // Take either plane number or height, depending on
	  // the user's preference.
	  //
	  if (_params.addHeight.usePlaneNumber){
	    heightData[iz*hFhdr.ny*hFhdr.nx + iy*hFhdr.nx + ix] = iz;
	  } else {
	    heightData[iz*hFhdr.ny*hFhdr.nx + iy*hFhdr.nx + ix] = hVhdr.level[iz];
	  }
	}
      }
    }
    //
    // OK, now add the variable to the output netCDF file.
    //
    ncdf.NCDAddAnother(_params.addHeight.netcdfName,
		       _params.addHeight.units,
		       _params.addHeight.netcdfName,
		       -999.0,
		       -999.0,
		       heightData);      

    free(heightData);
  }

  // file is closed in destructor for ncdf, which executes at the
  // end of this routine as it goes out of scope

  PMU_auto_register("Closing file.");
  ncdf.RapNCDFree(Q);

  // write latest data info file if required

  if (_params.WriteLdataInfo) {
    DsLdataInfo ldata(_params.OutDir);
    ldata.setDataFileExt(_params.OutFileExt);
    ldata.setWriter(_progName.c_str());
    ldata.setRelDataPath(outP.getFile().c_str());
    ldata.setUserInfo1(outP.getBase().c_str());
    ldata.setUserInfo2(outP.getFile().c_str());
    if (ldata.write(dataTime.unix_time)) {
      cerr << "ERROR - mdv2netCDF::_convert" << endl;
      cerr << "  Cannot write _latest_data_info file" << endl;
      cerr << "  OutDir: " << _params.OutDir << endl;
      return -1;
    }
  }
    
  return 0;
  
}

