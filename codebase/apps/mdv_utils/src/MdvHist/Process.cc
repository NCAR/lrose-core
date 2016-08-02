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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/umisc.h>
using namespace std;

//
// Constructor
//
Process::Process(Params *params){

  _initCounts(params, _totalCounts);
  
  return;
}

////////////////////////////////////////////
//
// get ith total count.
//

int Process::getCount(int index){
  return _totalCounts[index].count;
}


////////////////////////////////////////////
//
// output total counts to file.
//

void Process::outputTotalCounts(Params *params,
				DateTime startTime,
				DateTime endTime){

  char outFilename[MAX_PATH_LEN];
  sprintf(outFilename,
	  "%s/MdvHistTotals_%04d%02d%02d_%02d%02d%02d_%04d%02d%02d_%02d%02d%02d.hist",
	  params->outDir,
	  startTime.getYear(), startTime.getMonth(), startTime.getDay(),
	  startTime.getHour(), startTime.getMin(), startTime.getSec(),
	  endTime.getYear(), endTime.getMonth(), endTime.getDay(),
	  endTime.getHour(), endTime.getMin(), endTime.getSec());
  
  FILE *fp = fopen(outFilename, "w");
  if (fp == NULL){
    cerr << "Could not create " << outFilename << endl;
    return;
  }


  int grandTotal = 0;
  for (int ib=0; ib < params->numBins; ib++){
    grandTotal += _totalCounts[ib].count;
  }

  int runningTotal = 0;
  for (int ib=0; ib < params->numBins; ib++){
    runningTotal += _totalCounts[ib].count;

    fprintf(fp,"%g\t%g\t", params->binStart + ib * params->binWidth,  params->binStart + (ib+1) * params->binWidth);

    fprintf(fp,"%4.3f\t%4.3f\t", 100.0*double(_totalCounts[ib].count)/double(grandTotal),
	    100.0*double(runningTotal)/double(grandTotal));

    fprintf(fp,"%d\t%d", _totalCounts[ib].count, runningTotal);
    
    fprintf(fp,"\n");
  }

  fclose(fp);


  return;
}


////////////////////////////////////////////
//
// output individual counts to file.
//

bool Process::outputIndividualCountsPlain(const string &output_path,
					  const DateTime &start_time,
					  const int nGood,
					  const int numOutsideBins,
					  vector< Bin > &counts){

  // cerr << outFilename << endl;

  FILE *fp;
  if ((fp = fopen(output_path.c_str(), "w")) == 0)
  {
    cerr << "Failed to create " << output_path << endl;
    perror(output_path.c_str());
    
    return false;
  }

  fprintf(fp, "%04d %02d %02d %02d %02d %02d ",
	  start_time.getYear(), start_time.getMonth(), start_time.getDay(),
	  start_time.getHour(), start_time.getMin(), start_time.getSec());
  fprintf(fp, "%d %d ", nGood, numOutsideBins );
  
  for (size_t ib=0; ib < counts.size(); ib++)
  {
    if (ib == counts.size() - 1)
    {
      fprintf(fp, "%d\n", counts[ib].count);
    }
    else
    {
      fprintf(fp, "%d\t", counts[ib].count);
    }	     
  }

  fclose(fp);

  return true;
}



bool Process::outputIndividualCountsFormatted(const string &output_path,
					      const DateTime &start_time,
					      const int nGood,
					      const int numOutsideBins,
					      vector< Bin > &counts){

  // cerr << outFilename << endl;

  FILE *fp;
  if ((fp = fopen(output_path.c_str(), "w")) == 0)
  {
    cerr << "Failed to create " << output_path << endl;
    perror(output_path.c_str());
    
    return false;
  }

  fprintf(fp, "Date: %04d %02d %02d\n",
	  start_time.getYear(), start_time.getMonth(), start_time.getDay());
  fprintf(fp, "Time: %02d %02d %02d\n",
	  start_time.getHour(), start_time.getMin(), start_time.getSec());
  fprintf(fp, "Num of non-missing points in the domain: %d\n", nGood);
  fprintf(fp, "Num of non-missing points, not in the histogram bins: %d\n",
	  numOutsideBins );
  
  fprintf(fp, "\n");
  fprintf(fp, "   Min       Max    Counts\n");
  
  for (size_t ib=0; ib < counts.size(); ib++)
    fprintf(fp, "%7.2f   %7.2f   %d\n",
	    counts[ib].start, counts[ib].end, counts[ib].count);

  fclose(fp);

  return true;
}



////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *params, time_t trigger_time, int leadTime){


  if (params->Debug){
    cerr << "Data at " << utimstr(trigger_time) << endl;
  }

  //
  // Read in the data field
  //
  MdvxField *InField;
  time_t gen_time;
  
  if ((InField = readField(params, trigger_time, leadTime, gen_time)) == 0)
    return -1;
  
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

  fl32 *InData = (fl32 *) InField->getVol();

  // Initialize the counts vector

  _initCounts(params, _individualCounts);
  
  int nGood = 0;
  int numOutsideBins = 0;

  for (int index = 0; index < InFhdr.nx * InFhdr.ny * InFhdr.nz; ++index){
    //
    // If this data point is bad, leave it.
    //
    if (
      (InData[index] == InFhdr.missing_data_value) ||
      (InData[index] == InFhdr.bad_data_value)
      ) {
      continue;
    }
    nGood++;
    //
    // Determine which bin this value lies in and increment
    // the count for that bin.
    //
    bool inBin = false;
    
    for (int ib=0; ib < params->numBins; ib++){

      //
      // See if we are in that interval.
      //
      if (_individualCounts[ib].isInBin(InData[index]))
      {
	++_individualCounts[ib].count;
//	if (params->Debug)
//	  cerr << "Value: " << InData[index] << " is in bin " << ib << endl;
	inBin = true;
      }
    }
    if (!inBin)
    {
      numOutsideBins++;
//      if (params->Debug)
//	cerr << "Value: " << InData[index] << " is outside of the bins" << endl;
    }
    
  }

  date_time_t start_time;
  start_time.unix_time = trigger_time + leadTime;
  uconvert_from_utime( &start_time );

  if (params->fileOut)
  {
    // If we are not outputing individual files, then we will output a
    // total file.  The total file is output at the end of processing.

    string output_path = _constructIndividualPath(params,
						  gen_time,
						  leadTime,
						  start_time);
  
    if (params->outputIndividualFiles)
    {
      switch (params->individualOutputType)
      {
      case Params::OUTPUT_PLAIN :
	outputIndividualCountsPlain(output_path, start_time.unix_time,
				    nGood, numOutsideBins, _individualCounts);
	break;

      case Params::OUTPUT_FORMATTED :
	outputIndividualCountsFormatted(output_path, start_time.unix_time,
					nGood, numOutsideBins,
					_individualCounts);
	break;
      } /* endswitch - params->individualOutputType */
    }
    
  }
  else
  {
    // Write the information to the screen

    cout << start_time.year << "\t";
    cout << start_time.month << "\t";
    cout << start_time.day << "\t";
    cout << start_time.hour << "\t";
    cout << start_time.min << "\t";
    cout << start_time.sec << "\t";
    
    cout << nGood << "\t";
    cout << numOutsideBins << "\t";
    
    for (int ib=0; ib < params->numBins; ib++)
      cout << _individualCounts[ib].count << "\t";
    
    cout << endl;
  }
  
  //
  // Add these counts to the cumulative total.
  //
  
  for (int ib=0; ib < params->numBins; ib++)
    _totalCounts[ib].count += _individualCounts[ib].count;

  if (params->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  delete InField;
  
  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
}



////////////////////////////////////////////////////
//
// readField()
//
MdvxField *Process::readField(const Params *params,
			      const time_t trigger_time,
			      const int leadTime,
			      time_t &gen_time)
{
  //
  // Set up for the new data.
  //
  DsMdvx mdvx;

  if ((params->forecastMode) && (params->Mode == Params::ARCHIVE))
  {
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
		     params->TriggerUrl, 86400, trigger_time, leadTime);
  }
  else 
  {
    mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
		     params->TriggerUrl, 0, trigger_time);
  }

  mdvx.addReadField(params->FieldName);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  mdvx.setReadHorizLimits(params->latMin,
			  params->lonMin,
			  params->latMax,
			  params->lonMax);

  if (params->applyVerticalLimits)
    mdvx.setReadPlaneNumLimits(params->minVerticalLevel,
			       params->maxVerticalLevel);

  if (mdvx.readVolume())
  {
    cerr << "Read failed at " << utimstr(trigger_time) << " from ";
    cerr << params->TriggerUrl  << endl;
    return 0;
  }     

  Mdvx::master_header_t InMhdr = mdvx.getMasterHeader();
  gen_time = mdvx.getMasterHeader().time_gen;
  
  //
  // Get the desired field.
  //
  MdvxField *InField;
   
  InField = mdvx.getField(0);

  if (InField == 0)
  {
    cerr << "Field " << params->FieldName << " not found." << endl;
    return 0;
  }

  return new MdvxField(*InField);
}

////////////////////////////////////////////////////
//
// _initCounts()
//
void Process::_initCounts(const Params *params,
			  vector< Bin > &counts)
{
  // Allocate space for the bins.  If there are already the right number
  // of bins, then the bin start and end values have already been initialized
  // and we only have to reset the counts to 0.

  if ((int)counts.size() == params->numBins)
  {
    for (int i = 0; i < params->numBins; ++i)
      counts[i].count = 0;
  }
  else
  {
    counts.clear();
    counts.reserve(params->numBins);

    for (int i = 0; i < params->numBins; ++i)
    {
      Bin bin;
      
      bin.start = params->binStart + ((double)i * params->binWidth);
      bin.end = bin.start + params->binWidth;
      bin.count = 0;

      counts.push_back(bin);
    }
    
  }
}



////////////////////////////////////////////////////
//
// _constructIndividualPath()
//
string Process::_constructIndividualPath(const Params *params,
					 const time_t genTimeUnix,
					 const int leadTime,
					 const date_time_t startTime)
{
  char outFileDir[1024];
  char outFilename[1024];

  if (params->forecastMode)
  {
    // Construct the path for forecast mode

    date_time_t genTime;
    genTime.unix_time = genTimeUnix;
    uconvert_from_utime( &genTime );

    if (params->outputDateSub)
    {
      sprintf(outFileDir,"%s/%04d%02d%02d",
	      params->outDir, genTime.year, genTime.month, genTime.day);
	
      if (ta_makedir_recurse( outFileDir))
	cerr << "Failed to make directory " << outFileDir << endl;

      sprintf(outFilename,"%s/MdvHist_%04d%02d%02d_%02d%02d%02d_%08d.hist",
	      outFileDir, genTime.year, genTime.month, genTime.day,
	      genTime.hour, genTime.min, genTime.sec,leadTime);
    }
    else
    {
      sprintf(outFilename,"%s/MdvHist_%04d%02d%02d_%02d%02d%02d_%08d.hist",
	      params->outDir, genTime.year, genTime.month, genTime.day,
	      genTime.hour, genTime.min, genTime.sec,leadTime);
    }
	
  } else {
    // Construct the path for non-forecast mode

    if (params->outputDateSub)
    {
      sprintf(outFileDir,"%s/%04d%02d%02d",
	      params->outDir, startTime.year, startTime.month, startTime.day);
	
      if (ta_makedir_recurse( outFileDir))
      {
	cerr << "Failed to make directory " << outFileDir << endl;
      }

      sprintf(outFilename,"%s/MdvHist_%04d%02d%02d_%02d%02d%02d.hist",
	      outFileDir, startTime.year, startTime.month, startTime.day,
	      startTime.hour, startTime.min, startTime.sec);
    }
    else
    {
      sprintf(outFilename,"%s/MdvHist_%04d%02d%02d_%02d%02d%02d.hist",
	      params->outDir, startTime.year, startTime.month, startTime.day,
	      startTime.hour, startTime.min, startTime.sec);
    }
  }

  return outFilename;
  
}
