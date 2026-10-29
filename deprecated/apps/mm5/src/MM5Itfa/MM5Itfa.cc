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
// MM5Itfa.cc
//
// MM5Itfa object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "MM5Itfa.hh"
#include <mm5/MM5DataV2.hh>
#include <mm5/MM5DataV3.hh>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <physics/IcaoStdAtmos.hh>
using namespace std;

// Constructor

MM5Itfa::MM5Itfa(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = STRdup("MM5Itfa");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // initialize the ldata handle

  LDATA_init_handle(&_ldata, _progName, _params->debug);

  // init process mapper registration

  if (_params->mode == Params::REALTIME) {
    PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In MM5Itfa constructor");
  }

  return;

}

// destructor

MM5Itfa::~MM5Itfa()

{

  // free up LDATA handle

  LDATA_free_handle(&_ldata);

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_params);
  delete(_args);
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int MM5Itfa::Run ()
{

  PMU_auto_register("MM5Itfa::Run");

  // check the parameters

  if (_checkParams()) {
    return -1;
  }

  // create input path object

  InputPath *inputPath;
  if (_params->mode == Params::REALTIME) {
    inputPath = new InputPath(_progName, *_params,
			      PMU_auto_register);
  } else {
    inputPath = new InputPath(_progName, *_params,
			      _args->nFiles, _args->filePaths);
  }
  
  // run
  
  int iret = _run(*inputPath);

  // clean up

  delete(inputPath);

  if (iret) {
    return (-1);
  } else {
    return (0);
  }

}

//////////////////////////////////////////////////
// check the parameters
//
// returns 0 on success, -1 on failure
  
int MM5Itfa::_checkParams()
  
{

  int iret = 0;
  
  for (int i = 0; i < _params->itfa_derived_indices_n; i++) {

    int min_fl = _params->_itfa_derived_indices[i].min_flight_level;
    int max_fl = _params->_itfa_derived_indices[i].max_flight_level;

    if (min_fl > max_fl) {
      cerr << "TDRP ERROR" << endl;
      cerr << "  Min flight level must no exceed max flight level" << endl;
      cerr << "  Param: itfa_derived_indices[]" << endl;
      cerr << "    i: " << i << endl;
      iret = -1;
    }

    
  } // i

  for (int i = 0; i < _params->itfa_model_indices_n; i++) {

    int min_fl = _params->_itfa_model_indices[i].min_flight_level;
    int max_fl = _params->_itfa_model_indices[i].max_flight_level;

    if (min_fl > max_fl) {
      cerr << "TDRP ERROR" << endl;
      cerr << "  Min flight level must no exceed max flight level" << endl;
      cerr << "  Param: itfa_model_indices[]" << endl;
      cerr << "    i: " << i << endl;
      iret = -1;
    }

  } // i

  return iret;

}

//////////////////////////////////////////////////
// _run

int MM5Itfa::_run(InputPath &input_path)

{
  
  PMU_auto_register("MM5Itfa::_run");
  
  // loop through the input files
  
  string filePath;
  while ((filePath = input_path.next()) != "") {
    
    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Processing file %s\n", filePath.c_str());
    }
    
    int version;
    if (MM5Data::getVersion(filePath, version)) {
      fprintf(stderr, "Getting version number from file %s\n",
	      filePath.c_str());
      return -1;
    }

    MM5Data *inData;
    if (version == 2) {
      inData = new MM5DataV2(_progName,
			     filePath,
			     _params->debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register);
    } else {
      inData = new MM5DataV3(_progName,
			     filePath,
			     _params->debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register);
    }
    
    if (!inData->OK) {
      delete (inData);
      return -1;
    }

    while (inData->more()) {
      if (inData->read() == 0) {
	time_t gen_time, forecast_time;
	int lead_time;
	gen_time = inData->modelTime;
	lead_time = inData->forecastLeadTime;
	// round the lead_time to nearest 15 mins, and compute forecast time
	lead_time = ((int) (lead_time / 900.0 + 0.5)) * 900;
	forecast_time = gen_time + lead_time;
	if (_params->debug) {
	  cerr << "gen_time: " << utimstr(gen_time) << endl;
	  cerr << "forecast_time: " << utimstr(forecast_time) << endl;
	  cerr << "lead_time: " << lead_time << endl;
	}
	_processForecast(*inData, gen_time, lead_time,
			 forecast_time);
      }
    } // while (inData->more()
    
    delete (inData);
    
  }

  return (0);

}

/////////////////////////////////////////
// _processForecast()
//
// Process data for a given forecast time

int MM5Itfa::_processForecast(MM5Data &inData,
			   time_t gen_time,
			   int lead_time,
			   time_t forecast_time)

{

  PMU_auto_register("In MM5Itfa::_processForecast");
  
  // compute the derived fields

  inData.computeDerivedFields();
  
  // create ItfaIndices object:
  
  ItfaIndices indices(inData, PMU_auto_register);
  if (_params->debug) {
    indices.setDebug(true);
  }
 // Initialize all indices to "nan"  8/29/2002
 
  indices.init_all_fields_to_nan();

  // calculate the indices
  
  indices.calculate_indices();

  // compute weighted combined fields
  
  _computeItfa(inData, indices);
  
  //indices.test_indices();
  
  // if (_params->debug >= Params::DEBUG_VERBOSE) {
  if (_params->debug) {
    indices.printInternals(cout);
    // indices.printFields(cout);
  }
  
  int dim_array[3] = { inData.get_nLon(),
		       inData.get_nLat(),
		       inData.get_nSigma() };

  //indices.write_field(inData.pres, "1", dim_array);
  //indices.write_field(inData.zz, "7", dim_array);

  fl32 *** pres = inData.pres;
  fl32 *** zz = inData.zz;
  write_index_field(inData, pres, "1", dim_array);
  write_index_field(inData, zz, "7", dim_array);
  write_index_field(inData, indices.getBrown1(), "400", dim_array);
  write_index_field(inData, indices.getBrown2(), "401", dim_array);
  write_index_field(inData, indices.getColson_panofsky(), "402", dim_array);
  write_index_field(inData, indices.getEllrod1(), "403", dim_array);
  write_index_field(inData, indices.getEllrod2(), "404", dim_array);
  write_index_field(inData, indices.getRichardson(), "405", dim_array);
  write_index_field(inData, indices.getCcat(), "406", dim_array);
  write_index_field(inData, indices.getDutton(), "412", dim_array);
  write_index_field(inData, indices.getEndlich(), "413", dim_array);
  write_index_field(inData, indices.getLaz(), "414", dim_array);
  write_index_field(inData, indices.getNgm1(), "415", dim_array);
  write_index_field(inData, indices.getNgm2(), "416", dim_array);
  write_index_field(inData, indices.getVwshear(), "418", dim_array);
  write_index_field(inData, indices.getRit(), "419", dim_array);
  write_index_field(inData, indices.getHshear(), "420", dim_array);
  write_index_field(inData, indices.getStability(), "421", dim_array);
  write_index_field(inData, indices.getDef_sqr(), "422", dim_array);
  write_index_field(inData, indices.getVort_sqr(), "423", dim_array);
  write_index_field(inData, indices.getPvort(), "424", dim_array);
  write_index_field(inData, indices.getSat_ri(), "429", dim_array);
  write_index_field(inData, indices.getPvort_gradient(), "430", dim_array);
  //
  //write_index_field(inData, indices.getTke_gwb(), "408", dim_array);
  //write_index_field(inData, indices.getTke_kh3(), "409", dim_array);
  //write_index_field(inData, indices.getTke_kh4(), "431", dim_array);
  //write_index_field(inData, indices.getTke_kh5(), "432", dim_array);

  write_index_field(inData, indices.getcalc_divergence(), "434", dim_array);
  write_index_field(inData, indices.getcalc_tgrad(), "438", dim_array);
// Added the following new indices: (March 2002 Celia Chen)
//       447 - Stone inertial instability index
//       448 - Vorticity advection
//       449 - The NASA/NCSU turbulence index
  write_index_field(inData, indices.getcalc_siii(), "447", dim_array);
  write_index_field(inData, indices.getcalc_nva(), "448", dim_array);
  write_index_field(inData, indices.getcalc_ncsui(), "449", dim_array);
  write_index_field(inData, indices.getCombined(), "505", dim_array);
  
  // ulturb(425) absia(426) agi(427) divtm(428) 
  // random_number_index(433) divergence  (434) RTW (435) temp_gradient(438)
  //
  //write_index_field(inData, indices.get(), "425", dim_array);
  //write_index_field(inData, indices.get(), "426", dim_array);
  //write_index_field(inData, indices.get(), "427", dim_array);
  //write_index_field(inData, indices.get(), "428", dim_array);
  //write_index_field(inData, indices.get(), "433", dim_array);
  //write_index_field(inData, indices.get(), "435", dim_array);
  return (0);

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: write_index_field
// 
// Description: prints data from InputFile field array into *.F format
//              so it can be read and rendered by existing code for itfa.
//              
// Returns: 
//
// Notes: 
//
//

void MM5Itfa::write_index_field(const MM5Data &inData,
			     fl32*** field,
			     char* filename_base,
			     int *dim_array)

{

  char dirname[MAX_PATH_LEN];
  char filename[MAX_PATH_LEN];
  FILE *fld_file;
  size_t num_dim;
  int fld_size;
  int n;

  // create filename:

  date_time_t runTime;
  runTime.unix_time = inData.modelTime;
  uconvert_from_utime(&runTime);

  int leadHr = inData.forecastLeadTime / 3600;
  
  sprintf(dirname,
	  "%s/%.4d%.2d%.2d/indices.%.4d%.2d%.2d.i%.2d.f%.2d",
	  _params->output_dir,
	  runTime.year, runTime.month, runTime.day,
	  runTime.year, runTime.month, runTime.day,
	  runTime.hour,
	  leadHr);

  sprintf(filename,
	  "%s/%s.F",
	  dirname, filename_base);
  
  ta_makedir_recurse(dirname);

  //open outfile:
  if( (fld_file = fopen(filename, "w")) == NULL ) {
    cerr << "\nTrouble opening " << filename << endl;
    exit(1);
  }

  // get array dimensions and fld_size:
  if( dim_array[2] == 1 ){
    num_dim = 2;
    fld_size = dim_array[0]*dim_array[1];
  }
  else{
    num_dim = 3;
    fld_size = dim_array[0]*dim_array[1]*dim_array[2];
  }
  
  /* writes number of dimensions */
  if( fwrite((char *) &num_dim, sizeof(int), 1, fld_file) != 1 ) {
    printf("Trouble writing num_dim\n\n");
    exit(1);
  }

  /* writes dimensions in order xyz */
  if( fwrite((char *) dim_array, sizeof(int),num_dim , fld_file) != num_dim ) {
    printf("Trouble writing dim_array\n\n");
    exit(1);
  }
 
  
  /* write field array */
  for(int k = 0; k < dim_array[2]; k++)
    for (int j = 0; j< dim_array[1]; j++)
      for (int i = 0; i < dim_array[0]; i++)
	{
	  n = fwrite((char *) &field[k][j][i], sizeof(fl32), 1, fld_file); 
	}
  
  fclose( fld_file );
 
}   


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: write_index_field
// 
// Description: prints data from InputFile field array into *.F format
//              so it can be read and renedered by existing itfa code.
//              
// Returns: 
//
// Notes: 
//
//
void MM5Itfa::write_index_field(const MM5Data &inData,
			     fl32** field,
			     char* filename_base,
			     int *dim_array)

{

  char dirname[MAX_PATH_LEN];
  char filename[MAX_PATH_LEN];
  FILE *fld_file;
  size_t num_dim;
  int fld_size;
  int n;

  // create filename:

  date_time_t runTime;
  runTime.unix_time = inData.modelTime;
  uconvert_from_utime(&runTime);

  int leadHr = inData.forecastLeadTime / 3600;
  
  sprintf(dirname,
	  "%s/%.4d%.2d%.2d/indices.%.4d%.2d%.2d.i%.2d.f%.2d",
	  _params->output_dir,
	  runTime.year, runTime.month, runTime.day,
	  runTime.year, runTime.month, runTime.day,
	  runTime.hour,
	  leadHr);

  sprintf(filename,
	  "%s/%s.F",
	  dirname, filename_base);
  
  ta_makedir_recurse(dirname);

  //open outfile:
  if( (fld_file = fopen(filename, "w")) == NULL ) {
    cerr << "\nTrouble opening " << filename << endl;
    exit(1);
  }

  // get array dimensions and fld_size:
  if( dim_array[2] == 1 ){
    num_dim = 2;
    fld_size = dim_array[0]*dim_array[1];
  }
  else{
    num_dim = 3;
    fld_size = dim_array[0]*dim_array[1]*dim_array[2];
  }

  /* writes number of dimensions */
  if( fwrite((char *) &num_dim, sizeof(int), 1, fld_file) != 1 ) {
    printf("Trouble writing num_dim\n\n");
    exit(1);
  }

  /* writes dimensions in order xyz */
  if( fwrite((char *) dim_array, sizeof(int), num_dim, fld_file) != num_dim ) {
    printf("Trouble writing dim_array\n\n");
    exit(1);
  }
 
  /* write field array */
  for (int j = 0; j<dim_array[1]; j++)
    for (int i = 0; i < dim_array[0] ; i++)
      {
	n = fwrite((char *) &field[j][i], sizeof(fl32), 1, fld_file);
	if(n !=1)
	  cerr << filename << ": write failed " << i << " " << j << endl;
      }

  fclose( fld_file );
}   

//////////////////////////////////////////////////////////
// computeItfa()
//
// Compute ITFA using the weights and flight levels in the 
// params file

void MM5Itfa::_computeItfa(MM5Data &inData,
			       ItfaIndices &itfa)
  
{

  // clear the combined field

  itfa.clearCombined();
  IcaoStdAtmos isa;

  // add the itfa derived fields to the combined fields, by weight

  for (int index = 0; index < _params->itfa_derived_indices_n; index++) {
    
    ItfaIndices::test_sense_t sense = ItfaIndices::LESS_THAN;
    switch (_params->_itfa_derived_indices[index].sense) {
    case Params::LESS_THAN:
      sense = ItfaIndices::LESS_THAN;
      break;
    case Params::GREATER_THAN:
      sense = ItfaIndices::GREATER_THAN;
      break;
    case Params::INSIDE_INTERVAL:
      sense = ItfaIndices::INSIDE_INTERVAL;
      break;
    case Params::OUTSIDE_INTERVAL:
      sense = ItfaIndices::OUTSIDE_INTERVAL;
      break;
    } // switch

    double threshold_1 = _params->_itfa_derived_indices[index].threshold_1;
    double threshold_2 = _params->_itfa_derived_indices[index].threshold_2;
    double weight = _params->_itfa_derived_indices[index].weight;

    double min_level = _params->_itfa_derived_indices[index].min_flight_level;
    double max_level = _params->_itfa_derived_indices[index].max_flight_level;
    double min_pressure = isa.flevel2pres(min_level);
    double max_pressure = isa.flevel2pres(max_level);
      
    switch (_params->_itfa_derived_indices[index].name) {

    case Params::BROWN1:
      itfa.addToCombined("Brown1", itfa.getBrown1(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::BROWN2:
      itfa.addToCombined("Brown2", itfa.getBrown2(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::CCAT:
      itfa.addToCombined("Ccat", itfa.getCcat(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::COLSON_PANOFSKY:
      itfa.addToCombined("ColsonPanofsky", itfa.getColson_panofsky(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::DEF_SQR:
      itfa.addToCombined("DefSqr", itfa.getDef_sqr(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::ELLROD1:
      itfa.addToCombined("Ellrod1", itfa.getEllrod1(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::ELLROD2:
      itfa.addToCombined("Ellrod2", itfa.getEllrod2(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::DUTTON:
      itfa.addToCombined("dutton", itfa.getDutton(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::ENDLICH:
      itfa.addToCombined("endlich", itfa.getEndlich(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::HSHEAR:
      itfa.addToCombined("hshear", itfa.getHshear(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::LAZ:
      itfa.addToCombined("laz", itfa.getLaz(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::PVORT:
      itfa.addToCombined("pvort", itfa.getPvort(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::PVORT_GRADIENT:
      itfa.addToCombined("pvort_gradient", itfa.getPvort_gradient(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::NGM1:
      itfa.addToCombined("Ngm1", itfa.getNgm1(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::NGM2:
      itfa.addToCombined("Ngm2", itfa.getNgm2(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::RICHARDSON:
      itfa.addToCombined("richardson", itfa.getRichardson(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::RIT:
      itfa.addToCombined("rit", itfa.getRit(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::SAT_RI:
      itfa.addToCombined("sat_ri", itfa.getSat_ri(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::STABILITY:
      itfa.addToCombined("stability", itfa.getStability(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::VORT_SQR:
      itfa.addToCombined("vort_sqr", itfa.getVort_sqr(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::VWSHEAR:
      itfa.addToCombined("vwshear", itfa.getVwshear(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::TKE_GWB:
      itfa.addToCombined("tke_gwb", itfa.getTke_gwb(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::TKE_KH3:
      itfa.addToCombined("tke_kh3", itfa.getTke_kh3(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::TKE_KH4:
      itfa.addToCombined("tke_kh4", itfa.getTke_kh4(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::TKE_KH5:
      itfa.addToCombined("tke_kh5", itfa.getTke_kh5(),
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    } // switch

  } // index

  // add the itfa model fields to the combined fields, by weight

  for (int index = 0; index < _params->itfa_model_indices_n; index++) {
    
    ItfaIndices::test_sense_t sense = ItfaIndices::LESS_THAN;
    switch (_params->_itfa_model_indices[index].sense) {
    case Params::LESS_THAN:
      sense = ItfaIndices::LESS_THAN;
      break;
    case Params::GREATER_THAN:
      sense = ItfaIndices::GREATER_THAN;
      break;
    case Params::INSIDE_INTERVAL:
      sense = ItfaIndices::INSIDE_INTERVAL;
      break;
    case Params::OUTSIDE_INTERVAL:
      sense = ItfaIndices::OUTSIDE_INTERVAL;
      break;
    } // switch

    double threshold_1 = _params->_itfa_model_indices[index].threshold_1;
    double threshold_2 = _params->_itfa_model_indices[index].threshold_2;
    double weight = _params->_itfa_model_indices[index].weight;

    double min_level = _params->_itfa_model_indices[index].min_flight_level;
    double max_level = _params->_itfa_model_indices[index].max_flight_level;
    double min_pressure = isa.flevel2pres(min_level);
    double max_pressure = isa.flevel2pres(max_level);
      
    switch (_params->_itfa_model_indices[index].name) {

    case Params::W_ITFA:
      itfa.addToCombined("w", inData.ww,
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    case Params::WSPD_ITFA:
      itfa.addToCombined("wind speed", inData.wspd,
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;
      
    case Params::DIVERGENCE_ITFA:
      itfa.addToCombined("divergence", inData.divergence,
			 weight, sense,
			 threshold_1, threshold_2,
			 min_pressure, max_pressure);
      break;

    } // switch

  } // index

  // normalize the field

  itfa.normalizeCombined();

  // itfa.printCombined(cerr);

}

//////////////////////////////////////////////////
// trim unwanted values from the turbulence field
  
void MM5Itfa::_trimTurbField(ItfaIndices &itfa)

{
  
  if (_params->min_turb_severity_threshold <= 0.0) {
    return;
  }

  fl32 ***combined = itfa.getCombined();
  
  for (int k = 0; k < itfa.getnK(); k++) {
    
    for (int j = 0; j < itfa.getnJ(); j++) {

      for (int i = 0; i < itfa.getnI(); i++) {

	if (combined[k][j][i] < _params->min_turb_severity_threshold) {
	  combined[k][j][i] = 0;
	}

      } //i

    } // j

  } // k
	  
}
      
