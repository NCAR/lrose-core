/**
 * @file MultiThreshBiasMapping.cc
 */

//------------------------------------------------------------------
#include <Spdb/MultiThreshBiasMapping.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
MultiThreshBiasMapping::MultiThreshBiasMapping(void) :
_chunkValidTime(0), _chunkTimeWritten(0), _latlonsOptional(true)

{
}

//------------------------------------------------------------------
MultiThreshBiasMapping::MultiThreshBiasMapping(const std::string &spdb) :
  MultiThresholdsBiasMapping(),
  _url(spdb),
  _chunkValidTime(0),
  _chunkTimeWritten(0),
  _latlonsOptional(true)
{

}

//------------------------------------------------------------------
MultiThreshBiasMapping::
MultiThreshBiasMapping(const std::string &path,
		       const std::vector<double> &ltHours,
		       const std::vector<std::string> &fields,
		       const TileInfo &tiling, bool latlonsOptional) :
  MultiThresholdsBiasMapping(ltHours, fields, tiling),
  _url(path),
  _chunkValidTime(0),
  _chunkTimeWritten(0),
  _latlonsOptional(latlonsOptional)
{
}

//------------------------------------------------------------------
MultiThreshBiasMapping::~MultiThreshBiasMapping()
{
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::
readFirstBefore(const time_t &t, int maxSecondsBack,
		int maxSecondsBeforeColdstart,
		const std::vector<FieldThresh2> &coldstartThresh)
{
  DsSpdb s;
  if (s.getFirstBefore(_url, t-1, maxSecondsBack) != 0)
  {
    LOG(DEBUG) << "No SPDB data found in data base "
	       << _url << " before " << DateTime::strn(t) 
	       << " , within " << maxSecondsBack << " seconds";
    return false;
  }
  else
  {
    if (!_load(s))
    {
      return false;
    }
    else
    {
      return
	MultiThresholdsBiasMapping::checkColdstart(t,
						   maxSecondsBeforeColdstart, 
						   coldstartThresh);
    }
  }
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::readNewestInRange(const time_t &t0,
					       const time_t &t1)

{
  std::vector<time_t>  times = timesInRange(t0, t1);
  if (times.empty())
  {
    LOG(DEBUG) << "No SPDB data found in data base "
	       << _url << " between " << DateTime::strn(t0)
	       << " and " << DateTime::strn(t1);
    return false;
  }
  time_t tbest = *(times.rbegin());

  return read(tbest);
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::readClosestToTargetInRange(const time_t &target,
							const time_t &t0,
							const time_t &t1)
{
  std::vector<time_t>  times = timesInRange(t0, t1);
  if (times.empty())
  {
    LOG(DEBUG) << "No SPDB data found in data base "
	       << _url << " between " << DateTime::strn(t0)
	       << " and " << DateTime::strn(t1);
    return false;
  }
  time_t bestT = times[0];
  int bestDelta = times[0] - target;
  if (bestDelta < 0)
  {
    bestDelta = -bestDelta;
  }

  for (size_t i=1; i<times.size(); ++i)
  {
    int delta = times[i] - target;
    if (delta < 0)
    {
      delta = -delta;
    }
    if (delta < bestDelta)
    {
      bestDelta = delta;
      bestT = times[i];
    }
  }
  return read(bestT);
}


//------------------------------------------------------------------
bool MultiThreshBiasMapping::write(const time_t &t)
{
  string xml = MultiThresholdsBiasMapping::toXml();

  DsSpdb s;
  s.setPutMode(Spdb::putModeOver);
  
  s.clearPutChunks();
  s.clearUrls();
  s.addUrl(_url);

  MemBuf mem;
  mem.free();
  
  mem.add(xml.c_str(), xml.size() + 1);
  if (s.put(SPDB_XML_ID, SPDB_XML_LABEL, 1, t, t, mem.getLen(),
	    (void *)mem.getPtr()))
  {
    LOG(ERROR) << "problems writing out SPDB";
    return false;
  }
  return true;
}

//------------------------------------------------------------------
std::vector<time_t> 
MultiThreshBiasMapping::timesInRange(const time_t &t0, const time_t &t1) const
{
  DsSpdb D;
  std::vector<time_t> ret;
  if (D.compileTimeList(_url, t0, t1))
  {
    LOG(ERROR) << "Compiling time list";
    return ret;
  }
  ret = D.getTimeList();
  return ret;
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::read(const time_t &time)
{
  MultiThresholdsBiasMapping::clearMapping();

  DsSpdb D;
  
  bool stat = true;
  if (D.getExact(_url, time) != 0)
  {
    stat = false;
  }
  else
  {
    stat = _load(D, false);
  }
  if (!stat)
  {
    LOG(WARNING) << "No SPDB data found in data base "
		 << _url << " at " << DateTime::strn(time);
  }
  return stat;
  
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::readExactPreviousOrCurrent(const time_t &gt,
							int maxDaysBack)
{
  DsSpdb D;
  bool stat = true;
  if (D.getExact(_url, gt) != 0)
  {
    stat = false;
  }
  else
  {
    stat = _load(D, false);
  }
  if (!stat)
  {
    stat = readExactPrevious(gt, maxDaysBack);
  }
  return stat;
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::readExactPrevious(const time_t &gt,
					       int maxDaysBack)
{
  for (int i=1; i<=maxDaysBack; ++i)
  {
    time_t ti = gt - i*3600*24;
    DsSpdb D;
    bool stat = true;
    if (D.getExact(_url, ti) != 0)
    {
      stat = false;
    }
    else
    {
      stat = _load(D, false);
    }
    if (stat)
    {
      return true;
    }
  }
  LOG(WARNING) << "No SPDB data up to maximum days back " << _url;
  return false;
}

//------------------------------------------------------------------
bool MultiThreshBiasMapping::_load(DsSpdb &s, bool fieldsLeadsTilesSet)
{
  const vector<Spdb::chunk_t> &chunks = s.getChunks();
  int nChunks = static_cast<int>(chunks.size());
  if (nChunks <= 0)
  {
    //LOG(WARNING) << "No chunks";
    return false;
  }
  if (nChunks > 1)
  {
    LOG(WARNING) << "Too many chunks " << nChunks << " expected 1";
    return false;
  }
  vector<Spdb::chunk_t> lchunks;
  for (int i=0; i<nChunks; ++i)
  {
    lchunks.push_back(chunks[i]);
  }
  // Process out the results.
  for (int jj = 0; jj < nChunks; jj++)
  {
    const Spdb::chunk_t &chunk = lchunks[jj];
    
    LOG(DEBUG) << "Thresholds SPDB time[" << jj << "]=" 
	       << DateTime::strn(chunk.valid_time);
    LOG(DEBUG_VERBOSE) << "Url=" << _url;
    // _print_chunk_hdr(chunk);
    
    // int data_len = chunk.len;
    void *chunk_data = chunk.data;
    if (s.getProdId() == SPDB_XML_ID)
    {
      string xml((char *)chunk_data);
      _chunkValidTime = chunk.valid_time;
      _chunkTimeWritten = chunk.write_time;
      return MultiThresholdsBiasMapping::fromXml(xml, fieldsLeadsTilesSet,
						 _latlonsOptional);
    }
    else
    { 
      LOG(ERROR) << "spdb data is not XML data, want " << SPDB_XML_ID 
		 << " got " << s.getProdId();
      return false;
    }
  }
  return true;
}
