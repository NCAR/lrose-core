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
 * @file SyncTimeWritten.cc
 */

#include "SyncTimeWritten.hh"
#include "MdvPlaybackData.hh"
#include <toolsa/DateTime.hh>
#include <cstdio>

//----------------------------------------------------------------
SyncTimeWritten::SyncTimeWritten(const bool obs, const ParmsMdvPlayback &parm)
  : Sync(obs, parm)
{
  pSyncTwritten = -1;
  pSyncNextTwritten = -1;
  pResSeconds = static_cast<int>(pParms.pResolutionMinutes*60.0);
}

//----------------------------------------------------------------
SyncTimeWritten::~SyncTimeWritten()
{
}

//----------------------------------------------------------------
void SyncTimeWritten::initDerived(const MdvPlaybackData &data)
{
  DateTime dt(data.pWt);
  int y = dt.getYear();
  int m = dt.getMonth();
  int d = dt.getDay();
  int h = dt.getHour();
  int min = dt.getMin();

  DateTime dt2(y, m, d, h, 0, 0);
  DateTime dt3(y, m, d, h, min, 0);

  time_t t = dt3.utime();
  pSyncTwritten = dt2.utime();
  while (pSyncTwritten < t)
  {
    if (pSyncTwritten + pResSeconds >= t)
    {
      break;
    }
    pSyncTwritten += pResSeconds;
  }
  pSyncNextTwritten = pSyncTwritten + pResSeconds;

  if (pParms.pDebugSync)
  {
    printf("initial sync time=%s\n", DateTime::strn(pSyncTwritten).c_str());
    printf("next sync time=%s\n",
	   DateTime::strn(pSyncNextTwritten).c_str());
  }
}

//----------------------------------------------------------------
bool SyncTimeWritten::timeToSync(const MdvPlaybackData &data) const
{
  return (data.pWt >= pSyncNextTwritten);
}


//----------------------------------------------------------------
void SyncTimeWritten::incrementSyncDerived(void)
{
  pSyncTwritten = pSyncNextTwritten;
  pSyncNextTwritten = pSyncTwritten + pResSeconds;
  if (pParms.pDebugSync)
  {
    printf("next sync time=%s\n", DateTime::strn(pSyncNextTwritten).c_str());
  }
}

