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
 * @file MdvPlayback.cc
 */

#include "MdvPlayback.hh"
#include "SyncTimeWritten.hh"
#include "SyncDataTime.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadSimple.hh>
#include <cstdio>
#include <algorithm>
#include <unistd.h>

using std::vector;
using std::pair;

//----------------------------------------------------------------
TaThread *MdvPlayback::ThreadAlg::clone(const int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(MdvPlayback::compute);
  t->setThreadContext(this);
  return dynamic_cast<TaThread *>(t);
}

//----------------------------------------------------------------
MdvPlayback::MdvPlayback(const ParmsMdvPlayback &parms,
			 void tidyAndExit(int)) :  
  pParms(parms), pSync(NULL), pRealTimeWritten(0)
{
  // initialize each data input
  pInit();

  // initialize synchronization
  if (!pInitSync())
  {
    tidyAndExit(1);
  }

  pThread.init(parms.pNumThreads, parms.pThreadDebug);
}

//----------------------------------------------------------------
MdvPlayback::~MdvPlayback()
{
  if (pSync != NULL)
  {
    delete pSync;
  }
}

//----------------------------------------------------------------
void MdvPlayback::run(void)
{
  printf("Begin playback\n");
  for (size_t i=0; i<pData.size(); ++i)
  {
    pProcess(i);
  }
  pThread.waitForThreads();
  if (pParms.pSecondsDelayBeforeExit > 0)
  {
    printf("Final sleep = %d\n", pParms.pSecondsDelayBeforeExit);
    sleep(pParms.pSecondsDelayBeforeExit);
  }
}

//------------------------------------------------------------------
void MdvPlayback::pProcess(const int i)
{
  pThread.thread(i, &pData[i]);//, this, compute);
}

//------------------------------------------------------------------
void MdvPlayback::compute(void *i)
{
  MdvPlaybackData *data = static_cast<MdvPlaybackData *>(i);
  MdvPlayback *alg = data->pAlg;

  // now play back
  alg->pPlaybackRealtime(data);
}

//------------------------------------------------------------------
void MdvPlayback::pInit(void)
{
  for (size_t i=0; i< pParms.pInput.size(); ++i)
  {
    printf("Setting up playback state for %s\n",
	   pParms.pInput[i].pUrl.c_str());
    pSetState(pParms.pInput[i]);
  }  
  printf("%ld total playback items from all input urls\n",
	 pData.size());
  printf("Sorting the data for playback\n");
  sort(pData.begin(), pData.end(), MdvPlaybackData::lessThan);

  for (size_t i=0; i<pData.size(); ++i)
  {
    pData[i].setIndex(i);
    pData[i].setAlg(this);
    if (i == 0)
    {
      pData[i].setNextWritten();
    }
    if (pParms.pDebug)
    {
      pData[i].print(pParms.pShowRealtime);
    }
  }
}

//------------------------------------------------------------------
bool MdvPlayback::pInitSync(void)
{
  if (pParms.pInputSyncUrl.empty())
  {
    return true;
  }

  for (size_t i=0; i<pData.size(); ++i)
  {
    string s = pData[i].pOutUrl;
    if (s == pParms.pInputSyncUrl)
    {
      if (pParms.pSyncToTimeWritten)
      {
	pSync = dynamic_cast<Sync *>(new SyncTimeWritten(!pData[i].pHasLt,
							 pParms));
      }
      else
      {
	pSync = dynamic_cast<Sync *>(new SyncDataTime(!pData[i].pHasLt,
						      pParms));
      }
      pSync->init(pData[0]);
      return true;
    }
  }
  printf("ERROR sync URL not found in list of output urls %s\n",
	 pParms.pInputSyncUrl.c_str());
  return false;
}

//------------------------------------------------------------------
void MdvPlayback::pSetState(const ParmData &P)
{
  time_t lt0, lt1;
  if (!pSetRange(P, lt0, lt1))
  {
    return;
  }

  if (P.pIsObs)
  {
    pSetObsState(P, lt0, lt1);
  }
  else
  {
    pSetFcstState(P, lt0, lt1);
  }
}

//------------------------------------------------------------------
void MdvPlayback::pSetObsState(const ParmData &P,
			       const time_t &lt0,
			       const time_t &lt1)
{
  vector<time_t> t = MdvPlaybackData::obsInRange(P.pUrl, lt0, lt1);

  if (pParms.pMode == ParmsMdvPlayback::PRE_PLAYBACK)
  {
    for (size_t i=t.size()-1; i>= 0; --i)
    {
      MdvPlaybackData data(t[i], i, P.pUrl, P.pUseDataTime,
			   pParms.pInputOutputPath);
      if (data.inRange(pParms))
      {
	pData.push_back(data);
	break;
      }
    }
  }
  else
  {
    for (size_t i=0; i<t.size(); ++i)
    {
      MdvPlaybackData data(t[i], i, P.pUrl, P.pUseDataTime,
			   pParms.pInputOutputPath);
      if (data.inRange(pParms))
      {
	pData.push_back(data);
      }
    }
  }
}

//------------------------------------------------------------------
void MdvPlayback::pSetFcstState(const ParmData &P,
				const time_t &lt0,
				const time_t &lt1)
{
  vector<pair<time_t, int> > gtlt = MdvPlaybackData::fcstInRange(P.pUrl,
								 lt0, lt1);
  if (pParms.pMode == ParmsMdvPlayback::PRE_PLAYBACK)
  {
    for (int i=static_cast<int>(gtlt.size())-1; i>= 0; --i)
    {
      MdvPlaybackData data(gtlt[i].first, gtlt[i].second, i, P.pUrl,
			   P.pUseDataTime, pParms.pInputOutputPath);
      if (data.inRange(pParms))
      {
	pData.push_back(data);
	break;
      }
    }
  }
  else
  {
    for (size_t i=0; i<gtlt.size(); ++i)
    {
      MdvPlaybackData data(gtlt[i].first, gtlt[i].second, i, P.pUrl,
			   P.pUseDataTime,
			   pParms.pInputOutputPath);
      if (data.inRange(pParms))
      {
	pData.push_back(data);
      }
    }
  }
}

//------------------------------------------------------------------
bool MdvPlayback::pSetRange(const ParmData &P, time_t &lt0, time_t &lt1)
{
  // max lookback seconds
  lt0 = pParms.pTime0 - P.pLatencySecondsMax;

  // latest allowed gen time or obs time
  if (pParms.pMode == ParmsMdvPlayback::PRE_PLAYBACK)
  {
    // only look up to starting time
    lt1 = pParms.pTime0;
  }
  else
  {
    lt1 = pParms.pTime1;
  }
  return true;
}

//------------------------------------------------------------------
void MdvPlayback::pPlaybackSync(const MdvPlaybackData *data)
{
  if (pSync->timeToSync(*data))
  {
    pThread.lockForIO();
    pSync->synchronize(pParms.pOutputSyncUrl);
    pSync->incrementSyncTime(*data);
    pThread.unlockAfterIO();
  }
  if (data->pOutUrl == pParms.pInputSyncUrl)
  {
    pThread.lockForIO();
    pSync->updateInput(*data);
    pThread.unlockAfterIO();
  }
}

//------------------------------------------------------------------
void MdvPlayback::pPlaybackRealtime(MdvPlaybackData *data)
{
  int index = data->getIndex();

  // read the data
  data->read();

  bool done = false;
  while (!done)
  {
    pThread.lockForIO();
    done = data->isNextWrite();
    pThread.unlockAfterIO();
    if (!done)
    {
      sleep(1);
      continue;
    }

    // if synchronizing, need to not write out anything too new
    // until all synched up.
    if (pSync != NULL)
    {
      // deal with synchronization
      pPlaybackSync(data);
    }

    bool notFirst = index > 0;
    bool notLast = (index < static_cast<int>(pData.size()) - 1);

    // just before write, see if sleep called for
    if (notFirst && pParms.pMode == ParmsMdvPlayback::PLAYBACK)
    {
      int deltat = pData[index].pWt - pData[index-1].pWt;
      time_t t1 = time(0);
      int real_deltat = t1 - pRealTimeWritten;
      if (deltat > real_deltat)
      {
	deltat =
	  static_cast<int>(static_cast<double>(deltat-real_deltat)/
			   pParms.pSpeedup);
	if (deltat > 0)
	{
	  printf("%5d Sleeping %d\n", index, deltat);
	  sleep(deltat);
	}
      }
    }
    pThread.lockForIO();
    data->write();
    pRealTimeWritten = time(0);
    data->print(index, pParms.pShowRealtime);
    if (notLast)
    {
      pData[index+1].setNextWritten();
    }
    pThread.unlockAfterIO();
  }
}

