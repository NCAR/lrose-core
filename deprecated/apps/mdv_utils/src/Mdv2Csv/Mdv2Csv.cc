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
///////////////////////////////////////////////////////////
// Mdv2Csv.cc
//
// Mdv2Csv object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2015
//

#include "Mdv2Csv.hh"
#include <toolsa/Path.hh>
#include <cstring>
using namespace std;

//
// Constructor
//
Mdv2Csv::Mdv2Csv(int argc, char **argv)
{

  // initialize

  isOK = true;

  _trigger = NULL;

  //
  // set programe name
  //
  _progName = "Mdv2Csv";

  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  //
  // get TDRP params
  //
  char path[8] = "unknown";

  _paramsPath = path;

  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  //
  // If an additional TDRP file was specified, load that
  // over the existing params.
  //
  if (NULL != _args.additional_tdrp_file){

    if (_params.debug){
      cerr << "Attempting to load additional param file " << _args.additional_tdrp_file << endl;
    }

    if (_params.load(_args.additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters in file: " << _args.additional_tdrp_file << endl;
      isOK = false;
      return;
    }
  }

  //
  // set up trigger
  //
  if (_setUpTrigger()) {
    isOK = FALSE;
    return;
  }

  //
  // Init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

}

//////////////////////////////////////////////////////
//
// destructor
//
Mdv2Csv::~Mdv2Csv()

{

  //
  // unregister process
  //
  PMU_auto_unregister();

  if( _trigger) {
    delete _trigger;
  }
}


//////////////////////////////////////////////////
//
// Run
//
int Mdv2Csv::Run ()
{

  int iret = 0;

  //
  // register with procmap
  //
  PMU_auto_register("Run");
  
  //
  // process data: 
  //
  
  time_t inputTime;

  while (!_trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      inputTime = _trigger->next(triggerInfo);
      if (_processData(triggerInfo.getIssueTime(), 
		       triggerInfo.getFilePath()))
	{
	  cerr << "Mdv2Csv::Run" <<"  Errors in processing time: "
	       <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  
	  iret = 1;
	}
    } // while
  
  return iret;
}


/////////////////////////////////////////
// Set up triggering mechanisme
//
// Returns 0 on success, 1 on failure

int Mdv2Csv::_setUpTrigger()

{ 
 
  if (_params.mode == Params::ARCHIVE)
  {
    DsTimeListTrigger  *interval_trigger = new DsTimeListTrigger();
  
    if (interval_trigger->init(_params._urlFieldNames[0].url,
			       _params.startTime,
			       _params.endTime))
    {
      cerr << interval_trigger->getErrStr();
      
      _trigger = 0;
      
      return 1;
    } 
    else
    {
      if (_params.debug)
      {
	cerr << "Mdv2Csv::Run(): Creating time list  archive trigger" << endl;
      }
      
      _trigger = interval_trigger;
    }  
  } 
  else 
  {

    if (_params.debug)
    {
      cerr << "Mdv2Csv::Run(): Creating trigger, Note: in realtime csv file will keep growing. Currently not designed for realtime use" << endl;
    }
    //
    // realtime mode
    //
    DsLdataTrigger *realtime_trigger = new DsLdataTrigger();
    
    if (realtime_trigger->init(_params._urlFieldNames[0].url,
			       _params.max_valid_realtime_age,
			       PMU_auto_register))
    {
      cerr << realtime_trigger->getErrStr();
      
      _trigger = 0;
      
      return 1;
    }
    else
    {
      _trigger = realtime_trigger;
    }
  }

  return 0;
}

///////////////////////////////////
//
//  process data at trigger time
//
int Mdv2Csv::_processData(const time_t inputTime,  const string filepath)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    {
      cerr << "Mdv2Csv::_processData: Processing time: issue: " << inputTime 
	   << filepath << endl;
    }
  
  if (_readMdv(inputTime,  filepath))
    {
      cerr << "ERROR - Mdv2Csv::_processData()" << endl;
      return 1;
    }
  
  return 0;
}

//
// Read mdv file.
//
int Mdv2Csv::_readMdv(const time_t requestTime, const string filepath)
{

  vector < fl32* > fieldDataVec;

  vector <fl32> missingValsVec;

  bool firstField = true;

  int numPts;

  time_t dataTime;

  int nx,ny,nz;

  DsMdvx mdvx;

  for (int i = 0 ; i < _params.urlFieldNames_n; i++)
  {
    //
    // Set up for reading mdv data, reinitialize DsMdvx objecturlFieldNames_
    //
    if ( (i == 0) || 
	 ( (i > 0) &&  (strcmp(_params._urlFieldNames[i].url,  _params._urlFieldNames[i-1].url) != 0)))
    {
      mdvx.clearRead();
      
      mdvx.setReadTime(Mdvx::READ_CLOSEST, _params._urlFieldNames[i].url, 300, requestTime);
    
      //
      // perform the read
      //
      if (mdvx.readVolume()) 
      {
	cerr << "Mdv2Csv::readMdv():"
	     << "  " << mdvx.getErrStr() << endl;
	//
	// clean up before return, we dont have this volume for all datasets
	//
	for ( int i = 0; i < (int) fieldDataVec.size(); i++)
	{
	  delete [] fieldDataVec[i];
	}
      
	return 0;
      }

      if (_params.debug == Params::DEBUG_VERBOSE) 
      {
	cerr << "Mdv2Csv::_readMdv() : Reading data for URL: "
	     << _params._urlFieldNames[i].url << endl;
	
	mdvx.printReadRequest(cerr);
      }
    }
       
    //
    // Get master header
    //
    Mdvx::master_header_t masterHdr = mdvx.getMasterHeader();
    
    //
    // Get Field data 
    //
    MdvxField *field = mdvx.getField( _params._urlFieldNames[i].fieldname);

    field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    
    Mdvx::field_header_t fHdr  =  field->getFieldHeader();
    
    fl32 *fieldData = (fl32*)field->getVol();
    
    fl32 *fdata = new fl32[ fHdr.nx * fHdr.ny * fHdr.nz];

    for (int j = 0; j < fHdr.nx * fHdr.ny * fHdr.nz; j++)
      fdata[j] = fieldData[j];

    fieldDataVec.push_back(fdata);

    missingValsVec.push_back(fHdr.missing_data_value);
    
    if (firstField)
    {
      nx = fHdr.nx;
      
      ny = fHdr.ny;
      
      nz = fHdr.nz;
   
      dataTime = masterHdr.time_centroid;

      numPts =  fHdr.nx * fHdr.ny * fHdr.nz;

      firstField = false;
    }
    else
    {
      if ( nx != fHdr.nx || ny != fHdr.ny ||  nz != fHdr.nz)
      {
	cerr << "Mdv2Csv::_readMdv(): ERROR: Fields have different dimensions. Exiting" << endl; 
	exit(-1);
      }
    } 
  }

  _writeCsv(fieldDataVec, missingValsVec, numPts, dataTime);  

  // clean up
  for ( int i = 0; i < (int) fieldDataVec.size(); i++)
  {
    delete [] fieldDataVec[i];
  }
  

  return 0;
}


int Mdv2Csv::_writeCsv( vector <fl32*> &fieldDataVec,  vector <fl32> &missingValsVec, int numPts, time_t dataTime)
{
  FILE *strikesFilePtr;

  FILE *noStrikesFilePtr;

  DateTime dateTime(dataTime);

  string outputStrikesFile =  string(_params.csvDir) + "/" + "strikes/" + dateTime.getDateStrPlain() + "/" + dateTime.getTimeStrPlain() + string(".csv");
  
  string outputNoStrikesFile =  string(_params.csvDir) + "/" + "noStrikes/" + dateTime.getDateStrPlain() + "/" + dateTime.getTimeStrPlain() + string(".csv");

  if ( (strikesFilePtr = fopen(outputStrikesFile,"w")) == NULL)
  {
    cerr << "Mdv2Csv::_writeCsv: Cannot open file," <<  outputStrikesFile.c_str()<< " for writing/appending\n";
    exit(-1);
  }

  if ( ( noStrikesFilePtr = fopen(outputNoStrikesFile,"w")) == NULL)
  {
    cerr << "Mdv2Csv::_writeCsv: Cannot open file," << outputNoStrikesFile.c_str() << " for writing/appending\n";
    exit(-1);
  }
  

  for (int j = 0; j < numPts; j++)
  {
    bool skipRecord = false;
    
    for ( int i = 0; i < (int) fieldDataVec.size(); i++)
    {
      if ( fabs( fieldDataVec[i][j] - missingValsVec[i]) <= .00001)
      {
	skipRecord = true;
      }
    }
    if (!skipRecord)
    {
      if ( fabs( fieldDataVec[i][2] - 0) <= .00001 )
      {
	fprintf(noStrikesFilePtr,"%ld,%d,",dataTime,j);

	for ( int i = 0; i < (int) fieldDataVec.size() - 1; i++)
	{
	  fprintf(noStrikesFilePtr,"%f,", fieldDataVec[i][j]);	
	}
	fprintf(noStrikesFilePtr,"%f\n", fieldDataVec[(int) fieldDataVec.size() - 1][j]);
      }
      else
      {
	fprintf(strikesFilePtr,"%ld,%d,",dataTime,j);

	for ( int i = 0; i < (int) fieldDataVec.size() - 1; i++)
	{
	  fprintf(strikesFilePtr,"%f,", fieldDataVec[i][j]);	
	}
	fprintf(strikesFilePtr,"%f\n", fieldDataVec[(int) fieldDataVec.size() - 1][j]);
      }
    }
  }
  fclose(strikesFilePtr);
  
  fclose(noStrikesFilePtr);
  
  return 0;
}
