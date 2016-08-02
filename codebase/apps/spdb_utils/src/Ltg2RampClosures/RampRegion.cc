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

/**
 * @file RampRegion.cc
 * @brief Source for RampRegion class
 */

#include "RampRegion.hh"
#include <toolsa/DateTime.hh>

using namespace std;

RampRegion::RampRegion(const char* name, const float lat, 
		       const float lon, const float radius,
		       const int closureTime, const string outdir,
                       const Pjg &proj):
  pName(name),
  pLat(lat),
  pLon(lon),
  pRadKm(radius),
  pClosureTime(closureTime),
  pOutdir(outdir),
  pProj(proj)
{

}
RampRegion::~RampRegion()
{
}

void  RampRegion::process( LTG_extended_t &strike)
{
  if (pInRegion(strike.latitude, strike.longitude) )
  {
    pLtgTimeSeries.push_back(strike.time);
  }
}

void  RampRegion::process( LTG_strike_t &strike)
{
  if (pInRegion(strike.latitude, strike.longitude) )
  {
    pLtgTimeSeries.push_back(strike.time);
  }
}
  
void RampRegion::printData()
{
  DateTime dTime;

  cerr << "ramp: " << pName << endl;

  for (int i = 0; i < pLtgTimeSeries.size(); i++)
    cerr << "strike time = " << dTime.strn(pLtgTimeSeries[i]) << endl;
}

void RampRegion::computeClosures()
{

  std::sort(pLtgTimeSeries.begin(), pLtgTimeSeries.end());

  DateTime dTime;

  int i = 0; 

  int closureCount = 0;

  while ( i < pLtgTimeSeries.size())
  {
    time_t closeRampBegin = pLtgTimeSeries[i];
    
    int j = i + 1;
    
    time_t lastStrTime = closeRampBegin;

    int diff =  pLtgTimeSeries[j] -  lastStrTime; 

    while( diff < pClosureTime && j < pLtgTimeSeries.size() -1)
    {
      if ( diff > 0)
      {
	lastStrTime = pLtgTimeSeries[j];
      }
      
      j++;

      diff =  pLtgTimeSeries[j] - lastStrTime; 
    }
    
    time_t closeRampEnd = lastStrTime + pClosureTime;

    RampClosure *rampClosure = new RampClosure(closeRampBegin,closeRampEnd);

    pClosures.push_back(rampClosure);

    i = j;
  }
  pWriteData();
}

bool RampRegion::pInRegion(float &strikeLat, float &strikeLon)
{
  double distKm;

  double theta;

  pProj.latlon2RTheta(pLat, pLon, strikeLat, strikeLon, distKm, theta);
    
  if (distKm < pRadKm)
    return true;
  else
    return false;
}

void RampRegion::pWriteData()
{
  //
  // Create filename
  //
  DateTime beginT(pIntervalStart);

  DateTime endT(pIntervalEnd);

  string beginTStr = beginT.kmltime();

  string endTimeStr =  endT.kmltime();

  string filename("rampClosures");
  filename = filename + "_" + pName + "_" + beginTStr + "_" + endTimeStr;

  cerr << "filename: " << filename << endl;
  //
  // Create output directory and make sure it exists
  //
  string outputDir =  pOutdir + "/" + pName;

  string mkdirCmd = "mkdir -p " + outputDir;

  system(mkdirCmd.c_str());

  string filepath = outputDir + "/" + filename;

  //
  // Open file and write data
  //
  FILE *fptr;
  
  if ( (fptr = fopen( filepath.c_str(), "w")) == NULL)
  {
    cerr << "Could not open " << filename << " for writing" << endl;
  }
  else
  {
    fprintf(fptr, "%s, latitude: %f, longitude: %f, radius: %f km, minimum "
	    "closure time:  %d seconds \n", pName.c_str(), pLat, pLon, pRadKm, 
	    pClosureTime); 
	    
    for (int i = 0; i < pClosures.size() ; i++)
    {
      DateTime dTime;
      fprintf(fptr,"start: %s end: %s duration %d\n", 
	      dTime.strn(pClosures[i]->getStart()).c_str(), 
	      dTime.strn(pClosures[i]->getEnd()).c_str(),
	      pClosures[i]->getDuration());
    }
    
    fclose(fptr);
  }
}

