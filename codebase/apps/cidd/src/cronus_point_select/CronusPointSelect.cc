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
// CronusPointSelect.cc
//
// CronusPointSelect object
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <rapmath/math_macros.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

#include "CronusPointSelect.hh"
#include "CronusParams.hh"

#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>

#include <cstdio>

using namespace std;

static double lastPrintLat = 0;
static double lastPrintLon = 0;

void printData(time_t data_time,
	       double lat,
	       double lon,
	       Params *_params);


// Constructor

CronusPointSelect::CronusPointSelect(int argc, char **argv)
  
{
  
  isOK = true;
  
  // set programe name

  _progName = "cronusPointSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // Attach to Cidd's shared memory segment
  
  if ((_coordShmem =
       (coord_export_t *) ushm_create(_params.coord_shmem_key,
				      sizeof(coord_export_t),
				      0666)) == NULL) {
    cerr << "ERROR: " << _progName << endl;
    cerr <<
      "  Cannot create/attach Cidd's coord export shmem segment." << endl;
    isOK = false;
  }
  

  return;

}

// destructor

CronusPointSelect::~CronusPointSelect()
  
{

  // detach from shared memory
  
  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int CronusPointSelect::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  int last_no = _coordShmem->pointer_seq_num - 1;
  time_t last_display_time = 0;
  
  // Now, operate forever
  
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    time_t curr_time = time(NULL);
    
    if (_coordShmem->pointer_seq_num != last_no ||
	(_params.auto_click_interval > 0 &&
	 abs(curr_time - last_display_time) > _params.auto_click_interval)) {
      
      if (_coordShmem->pointer_seq_num != last_no) {
	
	if (_params.mouse_button == 0 ||
	    _coordShmem->button == _params.mouse_button) {
	  
	  if (_params.debug) {
	    fprintf(stderr,
		    "Click - lat = %g, lon = %g, mouse button = %d\n",
		    _coordShmem->pointer_lat,
		    _coordShmem->pointer_lon,
		    (int) _coordShmem->button);
	  }
	  
	  time_t data_time;

	  if(_params.use_cidd_time) {
	    data_time = _coordShmem->time_max;
	  } else {
	    data_time = curr_time;
	  }
	 
	  printData(data_time,
		    _coordShmem->pointer_lat,
		    _coordShmem->pointer_lon,
		    &_params);
 
	  last_display_time = curr_time;
	  
	} else {

	  if (_params.debug) {
	    fprintf(stderr, "   Not processing clicks for mouse button %ld\n",
		    _coordShmem->button);
	  }

	}
      
	last_no = _coordShmem->pointer_seq_num;
	
      } else {

	time_t data_time;

	if(_params.use_cidd_time) {
	  //
	  // Use the time at the end of the current frame.
	  //
	  data_time = (time_t) _coordShmem->time_max;
	} else {
	  data_time = curr_time;
	}

	printData(data_time,
		  _params.startup_location.lat,
		  _params.startup_location.lon,
		  &_params);


	last_display_time = curr_time;
	
      }
      
    } // if (_coordShmem->pointer_seq_num != last_no ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(forever)
  
  return (0);

}


void printData(time_t data_time,
	       double lat,
	       double lon,
	       Params *_params) {

  //
  // Just bail if nothing has changed.
  //
  if (
      (lat == lastPrintLat) &&
      (lon == lastPrintLon)
      ){
    return;
  }
  
  lastPrintLat = lat; lastPrintLon = lon;
  
  //
  // Load the param file. Do this anew every time, since
  // it may be a link that we will have to re-load.
  //
  char *fakeParams[3];
  fakeParams[0] = (char *)"cronus";
  fakeParams[1] = (char *)"-params";
  fakeParams[2] = _params->cronusParamFile;
  
  CronusParams CrP;
  int iret = CrP.loadFromArgs(3, fakeParams, NULL, NULL);
  if (iret){
    cerr << "Could not get cronus params using file " << _params->cronusParamFile << endl ;
    cerr << "Error code was " << iret << endl;
    cerr << "   Giving up, dying." << endl;
    exit(-1);
  }      
  
  //
  // Loop through forecasts.
  //

  for (int k=0; k < 16; k++){
    cerr << endl;
  }

  cerr << "Location : " << lat << ", " << lon << endl;
  cerr << "Using cronus parameters from " << _params->cronusParamFile << endl ;

  for (int ifore = 0; ifore < CrP.forecasts_n; ifore++ ){
    
    if (_params->debug){
      cerr << "Forecast " << CrP._forecasts[ifore].name;
      cerr << " has URL " << CrP._forecasts[ifore].url << endl;
    }
    //
    // Read data for this forecast.
    //
    DsMdvx New;
    
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, 
		    CrP._forecasts[ifore].url, 3600, 
		    data_time);
    New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
    if (New.readVolume()){
      cerr << "  Read failed at " << utimstr(data_time) << " from ";
      cerr <<  CrP._forecasts[ifore].url << endl;
      continue;
    }   
    
    Mdvx::master_header_t Mhdr = New.getMasterHeader();
    
    //
    // Loop through interest fields, pick out those pertaining to
    // this forecast.
    //
    double totalWeight = 0.0;
    double totalCronusValue = 0.0;

    int printHeader = 1;
    for (int i=0; i < CrP.interest_fields_n; i++ ){
      if (!(strcmp(CrP._interest_fields[i].forecast, CrP._forecasts[ifore].name))){
	if (_params->debug){
	  cerr << "   For field " << CrP._interest_fields[i].output_field;
	  cerr << " weight = " << CrP._interest_fields[i].weight << endl;
	}
	
	//
	// First, get the cronus value.
	//
	MdvxField *Field = New.getFieldByName( CrP._interest_fields[i].output_field );
	if ( Field == NULL){
	  cerr << "FAILED TO FIND FIELD " << CrP._interest_fields[i].output_field << endl;
	  cerr << " Has the regieme changed?" << endl;
	  continue;
	}
	
	Mdvx::field_header_t Fhdr = Field->getFieldHeader();
	
	if (Fhdr.nz != 1){
	  cerr << "WARNING : Cronus data are not two dimensional." << endl;
	}
	
	MdvxProj Proj(Mhdr, Fhdr);
	fl32 *crData = (fl32 *) Field->getVol();
	
	int ixcr,iycr;
	if ( Proj.latlon2xyIndex(lat, lon, ixcr, iycr)){
	  cerr << "Point " << lat << ", " << lon << " outside of cronus grid." << endl;
	  continue;
	}
	
	int index =  iycr * Fhdr.nx + ixcr;
	
	
	double cronusValue = 0.0;
	
	if (
	    (crData[index] != Fhdr.bad_data_value) &&
	    (crData[index] != Fhdr.missing_data_value)
	    ){
	  cronusValue = crData[index];
	}

	//
	// For this interest field, find the input dataset, and then read the actual data.
	//
	int datasetIndex = -1;
	for (int j=0; j < CrP.input_datasets_n; j++){
	  if (!(strcmp( CrP._input_datasets[j].name, CrP._interest_fields[i].input_dataset  ))){
	    datasetIndex = j;
	    break;
	  }   
	}

	if (datasetIndex == -1){
	  cerr << "Failed to find input dataset " << CrP._interest_fields[i].input_dataset << endl;
	  exit(-1); // Fairly gnarly error - params are inconsistent.
	}

	if (_params->debug){
	  cerr << "     URL for dataset " <<  CrP._input_datasets[datasetIndex].name;
	  cerr << " is " << CrP._input_datasets[datasetIndex].url;
	  cerr << " field name " << CrP._interest_fields[i].input_field;
	  cerr << " dataset number " << datasetIndex;
	  cerr << endl;
	}


	//
	// Now, read the data.
	//
  
	time_t cronusTime = Mhdr.time_centroid;
	
	if (printHeader){
	  cerr << " Forecast " << CrP._forecasts[ifore].name;
	  cerr << " at " << utimstr(cronusTime) << " :" << endl;
	  printHeader = 0;
	}
	int lookBack = 60 * CrP._input_datasets[datasetIndex].look_back;
	DsMdvx NewData; NewData.clearRead();
    
	NewData.setReadTime(Mdvx::READ_FIRST_BEFORE, 
			    CrP._input_datasets[datasetIndex].url, lookBack, 
			    cronusTime);
	NewData.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
	NewData.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
	NewData.addReadField( CrP._interest_fields[i].input_field ); // Only one field to read.

	//
	// If these data need compositing, do it.
	//
	for (int ik=0; ik < CrP.composite_list_n; ik++){
	  if (datasetIndex == CrP._composite_list[ik].num){
	    NewData.setReadVlevelLimits(CrP._composite_list[ik].compositeMin, 
				    CrP._composite_list[ik].compositeMax);
	    NewData.setReadComposite();
	    break;
	  }
	}



	if (NewData.readVolume()){
	  cerr << "  No data at " << utimstr(cronusTime) << " from ";
	  cerr << CrP._input_datasets[datasetIndex].url << " lookback ";
	  cerr << CrP._input_datasets[datasetIndex].look_back << " mins" << endl;
	  continue;
	}   
    
	Mdvx::master_header_t DataMhdr = NewData.getMasterHeader();
    
	MdvxField *DataField = NewData.getFieldByName( CrP._interest_fields[i].input_field );
	if ( DataField == NULL){
	  cerr << "FAILED TO FIND FIELD " << CrP._interest_fields[i].input_field << endl;
	  cerr << " Has the input changed?" << endl;
	  continue;
	}
	
	Mdvx::field_header_t DataFhdr = DataField->getFieldHeader();
	
	if (DataFhdr.nz != 1){
	  cerr << "WARNING : " << CrP._interest_fields[i].input_field << " data are not two dimensional ";
	  cerr << DataFhdr.nz << " levels found." << endl;
	}
	
	MdvxProj DataProj(DataMhdr, DataFhdr);
	fl32 *Data = (fl32 *) DataField->getVol();
	
	int ixd,iyd;
	if ( DataProj.latlon2xyIndex(lat, lon, ixd, iyd)){
	  cerr << "Point " << lat << ", " << lon << " outside of data grid." << endl;
	  continue;
	}

	int dataIndex =  iyd * DataFhdr.nx + ixd;
	
	double dataValue = 0.0;	
	if (
	    (Data[dataIndex] != DataFhdr.bad_data_value) &&
	    (Data[dataIndex] != DataFhdr.missing_data_value)
	    ){
	  dataValue = Data[dataIndex];
	}
	
	//
	// OK - now, have cronusValue, dataValue, weight. print it.
	//
	cerr << "   Weight =";

	char handyStr[32];
	sprintf(handyStr,"%7.2f", CrP._interest_fields[i].weight);
	cerr << handyStr;

	cerr << "   data =";

	sprintf(handyStr,"%7.2f", dataValue);
	cerr << handyStr;

	cerr << "   cronus =";

	sprintf(handyStr,"%7.2f", cronusValue);
	cerr << handyStr;

        totalCronusValue += cronusValue * CrP._interest_fields[i].weight;

	cerr << " weighted cronus = ";
	sprintf(handyStr,"%7.2f", cronusValue * CrP._interest_fields[i].weight);
	cerr << handyStr;

	cerr << " (field " << CrP._interest_fields[i].output_field << ")";

	cerr << endl;

	totalWeight += CrP._interest_fields[i].weight;
      } // End of if this is an interest field in our forecast.
    } // End of loop through interest fields.
    //    cerr << " Sum of weights for ";
    //    cerr << CrP._forecasts[ifore].name << " forecast : " << totalWeight << endl << endl;
    cerr << " Sum of weighted cronus values for ";
    cerr << CrP._forecasts[ifore].name << " forecast : " << totalCronusValue << endl << endl;
    

  } // End of loop through forecasts.
  return;
}
