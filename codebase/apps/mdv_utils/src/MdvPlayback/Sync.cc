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
 * @file Sync.cc
 */

#include "Sync.hh"
#include "MdvPlaybackData.hh"
#include <toolsa/DateTime.hh>
#include <cstdio>

//----------------------------------------------------------------
Sync::Sync(const bool obs, const ParmsMdvPlayback &parm) :
  pParms(parm),
  pSyncGenTime(-1),
  pSyncLeadSeconds(-1),
  pSyncObs(obs)
{
}

//----------------------------------------------------------------
Sync::~Sync()
{
}

//----------------------------------------------------------------
void Sync::init(const MdvPlaybackData &data)
{
  pSyncGenTime = data.pGt;

  // no data yet actually
  pSyncLeadSeconds = -1;  

  if (pParms.pDebugSync)
  {
    printf("initial sync data_time=%s\n", DateTime::strn(pSyncGenTime).c_str());
  }
  
  initDerived(data);
}

//----------------------------------------------------------------
void Sync::synchronize(const std::vector<std::string> &outUrls)
{
  // here is where we wait for the output to complete
  int secondsWait = 0;
  printf("Synchronizing\n");
  for (size_t j=0; j<outUrls.size(); ++j)
  {
    if (pParms.pDebugSync)
    {	
      printf("begin waiting for synchronizing output data for %s at %s+%d\n",
	     outUrls[j].c_str(),
	     DateTime::strn(pSyncGenTime).c_str(), pSyncLeadSeconds);
    }
    while (!pSyncDataExists(outUrls[j]))
    {
      secondsWait += 5;
      if (secondsWait > pParms.pMaxWaitSeconds)
      {
	printf("ERROR never got synchronizing data from %s, give up\n", 
	       outUrls[j].c_str());
	break;
      }
      sleep(5);
    }
    if (pParms.pDebugSync)
    {
      printf("Got sync data for %s\n", outUrls[j].c_str());
    }
  }
}

//----------------------------------------------------------------
void Sync::incrementSyncTime(const MdvPlaybackData &data)
{
  pSyncGenTime = data.pGt;
  pSyncLeadSeconds = -1;
  if (pParms.pDebugSync)
  {
    printf("next sync data_time=%s\n", DateTime::strn(pSyncGenTime).c_str());
  }

  incrementSyncDerived();
}  

//----------------------------------------------------------------
void Sync::updateInput(const MdvPlaybackData &data)
{
  if (pSyncObs)
  {
    pSyncGenTime = data.pGt;

    if (pParms.pDebugSync)
    {
      printf("Adjusted sync time using input to %s\n", 
	     DateTime::strn(pSyncGenTime).c_str());
    }
    pSyncLeadSeconds = -1;
  }
  else
  {
    pSyncGenTime = data.pGt;
    pSyncLeadSeconds = data.pLt;
    if (pParms.pDebugSync)
    {
      printf("Adjusted sync time using input to %s+%d\n", 
	     DateTime::strn(pSyncGenTime).c_str(), pSyncLeadSeconds);
    }
  }
}


//------------------------------------------------------------------
bool Sync::pSyncDataExists(const std::string &url) const
{
  if (pSyncObs)
  {
    return MdvPlaybackData::obsExists(url, pSyncGenTime);
  }
  else
  {
    if (pSyncLeadSeconds == -1)
    {
      printf("ERROR checking sync, no data from input sync url for gentime=%s",
	     DateTime::strn(pSyncGenTime).c_str());
      printf("Proceed without waiting\n");
      return true;
    }
    else
    {
      return MdvPlaybackData::fcstExists(url, pSyncGenTime, pSyncLeadSeconds);
    }
  }
}
