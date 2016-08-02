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
#include "MdvPlaybackData.hh"
#include "ParmsMdvPlayback.hh"
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <cstdio>
using std::string;

//------------------------------------------------------------------
MdvPlaybackData::
MdvPlaybackData(const time_t &gt, const int index, const std::string &url,
		const bool useDataTime, 
		const std::vector<std::pair<std::string, std::string> > &pth) :
  pGt(gt), pHasLt(false), pLt(0), pWt(0), pUrl(url),
  pHasData(false), pHasBadData(false), pDataIndex(index), pIsNextWrite(false),
  pAlg(NULL)
{
  pFormOutUrl(pth);
  pSetWrittenTime(gt, useDataTime);
}

//------------------------------------------------------------------
MdvPlaybackData::
MdvPlaybackData(const time_t &gt, const int lt, const int index,
		const std::string &url, const bool useDataTime, 
		const std::vector<std::pair<std::string, std::string> > &pth) :
  pGt(gt), pHasLt(true), pLt(lt), pWt(0), pUrl(url),
  pHasData(false), pHasBadData(false), pDataIndex(index), pIsNextWrite(false),
  pAlg(NULL)
{
  pFormOutUrl(pth);
  pSetWrittenTime(gt, lt, useDataTime);
}

//------------------------------------------------------------------
MdvPlaybackData::~MdvPlaybackData()
{
}

//------------------------------------------------------------------
bool MdvPlaybackData::inRange(const ParmsMdvPlayback &p) const
{
  if (p.pMode != ParmsMdvPlayback::PLAYBACK)
  {
    return (pWt < p.pTime0 && pWt != 0);
  }
  else
  {
    return (pWt >= p.pTime0 && pWt <= p.pTime1);
  }
}	  

//------------------------------------------------------------------
void MdvPlaybackData::print(const int index, const bool showRealtime) const
{
  if (pHasLt)
  {
    if (showRealtime)
    {
      time_t now = time(0);
      printf("%5d %s  w:%s   [%s+%d]   %s -> %s\n", index,
	     DateTime::str(now).c_str(),
	     DateTime::str(pWt).c_str(),
	     DateTime::str(pGt).c_str(),
	     pLt, 
	     pUrl.c_str(),
	     pOutUrl.c_str());
    }
    else
    {
      printf("%5d %s   [%s+%d]   %s -> %s\n", index,
	     DateTime::str(pWt).c_str(),
	     DateTime::str(pGt).c_str(),
	     pLt, 
	     pUrl.c_str(),
	     pOutUrl.c_str());
    }
  }
  else
  {
    if (showRealtime)
    {
      time_t now = time(0);
      printf("%5d %s  w:%s   [%s]   %s -> %s\n", index,
	     DateTime::str(now).c_str(),
	     DateTime::str(pWt).c_str(),
	     DateTime::str(pGt).c_str(),
	     pUrl.c_str(),
	     pOutUrl.c_str());
    }
    else
    {
      printf("%5d %s   [%s]   %s -> %s\n", index,
	     DateTime::str(pWt).c_str(),
	     DateTime::str(pGt).c_str(),
	     pUrl.c_str(),
	     pOutUrl.c_str());
    }
  }
}


//------------------------------------------------------------------
void MdvPlaybackData::read(void)
{
  pData = DsMdvx ();
  pHasData = true;
  pHasBadData = false;
  if (pHasLt)
  {
    pData.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, pUrl, 0, pGt, pLt);
  }
  else
  {
    pData.setReadTime(Mdvx::READ_FIRST_BEFORE, pUrl, 0, pGt);
  }
  if (pData.readVolume())
  {
    printf("ERROR reading data from %s\n", pUrl.c_str());
    pHasBadData = true;
  }
}

//------------------------------------------------------------------
bool MdvPlaybackData::write(void) 
{
  if (!pIsNextWrite)
  {
    printf("Bad call to write()\n");

    return false;
  }
  if (!pHasBadData)
  {
    if (pHasLt)
    {
      pData.setWriteAsForecast();
    }
    if (pData.writeToDir(pOutUrl.c_str()))
    {
      printf("ERROR writing data\n");
    }
  }

  // clear out data
  pIsNextWrite = false;
  pHasData = false;
  pHasBadData = false;
  pData = DsMdvx();

  return true;
}

//------------------------------------------------------------------
bool MdvPlaybackData::lessThan(const MdvPlaybackData &t0,
			       const MdvPlaybackData &t1)
{
  if (t0.pWt < t1.pWt)
  {
    return true;
  }
  else if (t0.pWt > t1.pWt)
  {
    return false;
  }
  else
  {
    if (t0.pUrl == t1.pUrl)
    {
      if (t0.pHasLt && t1.pHasLt)
      {
	return t0.pLt < t1.pLt;
      }
      else
      {
	return true;  // order doesn't matter
      }
    }
    else
    {
      return true; // order doesn't matter
    }
  }

}


//------------------------------------------------------------------
vector<time_t> MdvPlaybackData::obsInRange(const string &url, const time_t &t0,
					   const time_t &t1)
{
  DsMdvx D;
  D.setTimeListModeValid(url, t0, t1);
  D.compileTimeList();
  return D.getTimeList();
}

//------------------------------------------------------------------
vector<pair<time_t, int> > MdvPlaybackData::fcstInRange(const string &url,
							const time_t &t0,
							const time_t &t1)
{
  vector<pair<time_t, int> > ret;
  DsMdvx D;
  D.setTimeListModeGen(url, t0, t1);
  D.compileTimeList();
  vector<time_t> gt = D.getTimeList();
  for (int i=0; i<static_cast<int>(gt.size()); ++i)
  {
    D.setTimeListModeForecast(url, gt[i]);
    D.compileTimeList();
    vector<time_t> vt = D.getValidTimes();
    for (int j=0; j<static_cast<int>(vt.size()); ++j)
    {
      int lt = vt[j] - gt[i];
      ret.push_back(pair<time_t,int>(gt[i], lt));
    }
  }
  return ret;
}

//------------------------------------------------------------------
bool MdvPlaybackData::obsExists(const std::string &url, const time_t &t)
{
  DsMdvx D;
  D.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, t);
  return (D.readAllHeaders() == 0);
}

//------------------------------------------------------------------
bool MdvPlaybackData::fcstExists(const std::string &url, const time_t &gt,
				 const int lt)
{
  DsMdvx D;
  D.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, url, 0, gt, lt);
  return (D.readAllHeaders() == 0);
}

//------------------------------------------------------------------
void MdvPlaybackData::
pFormOutUrl(const std::vector<std::pair<std::string, std::string> > &p)
{
  // only support the 'mdvp:://host::stuff'  None of that extra stuff in
  // there
  size_t i = pUrl.find("mdvp:://");
  if (i != 0)
  {
    printf("ERROR in URL format, only support simple MDV protocol\n");
    pOutUrl = "";
    return;
  }
  i = pUrl.find("::", 8);
  if (i == string::npos)
  {
    printf("ERROR in URL format, only support simple MDV protocol\n");
    pOutUrl = "";
    return;
  }
  string endS = pUrl.substr(i+2);
  for (size_t j=0; j<p.size(); ++j)
  {
    if (p[j].first.empty())
    {
      // defer this case, wait for when nothing else matches
      continue;
    }
    if (endS.find(p[j].first) == 0)
    {
      // match to j'th pair, skip past this part
      endS = endS.substr(p[j].first.size());
      if (endS.find("/") == 0)
      {
	// also skip '/' to prevent absolute path 
	endS = endS.substr(1);
      }
      if (p[j].second.empty())
      {
	// output is localhost remaining string
	pOutUrl = "mdvp:://localhost::" + endS;
      }
      else
      {
	// output is localhost remaining string preceded by second
	pOutUrl = "mdvp:://localhost::" + p[j].second + "/" + endS;
      }
      return;
    }
  }

  // nothing matched so try the first=empty case
  for (size_t j=0; j<p.size(); ++j)
  {
    if (p[j].first.empty())
    {
      if (p[j].second.empty())
      {
	// input = output except host, done
	pOutUrl = "mdvp:://localhost::" + endS;
      }
      else
      {
	// input = output with second inserted first
	pOutUrl = "mdvp:://localhost::" + p[j].second + "/" + endS;
      }
      return;
    }
  }
}


//------------------------------------------------------------------
void MdvPlaybackData::pSetWrittenTime(const time_t &t, const int lt,
				      const bool useDataTime)
{
  if (!pHasLt)
  {
    printf("ERROR in getWrittenTime wrong method\n");
    return;
  }
  if (useDataTime)
  {
    pWt = t;
  }
  else
  {

    DsMdvx D;
    D.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, pUrl, 0, t, lt);
    if (D.readAllHeaders() != 0)
    {
      printf("ERROR reading header\n");
      return;
    }
    else
    {
      Mdvx::master_header_t m = D.getMasterHeaderFile();
      pWt = m.time_written;
    }
  }
}

//------------------------------------------------------------------
void MdvPlaybackData::pSetWrittenTime(const time_t &t, const bool useDataTime)
{
  if (pHasLt)
  {
    printf("ERROR in getWrittenTime wrong method\n");
    return;
  }
  if (useDataTime)
  {
    pWt = t;
  }
  else
  {
    DsMdvx D;
    D.setReadTime(Mdvx::READ_FIRST_BEFORE, pUrl, 0, t);
    if (D.readAllHeaders() != 0)
    {
      printf("ERROR reading header\n");
      return;
    }
    else
    {
      Mdvx::master_header_t m = D.getMasterHeaderFile();
      pWt = m.time_written;
    }
  }
}
