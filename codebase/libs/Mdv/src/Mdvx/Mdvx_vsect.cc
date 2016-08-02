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
//////////////////////////////////////////////////////////
// Mdvx_vsect.cc
//
// Vertical section routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h>
using namespace std;

///////////////////////////////////////
// access to vertical section geometry

const Mdvx::vsect_waypt_t &Mdvx::getVsectWayPt(const int i) const
{
  return (_vsectWayPts[i]);
}

const Mdvx::vsect_samplept_t &Mdvx::getVsectSamplePt(const int i) const
{
  return (_vsectSamplePts[i]);
}

const Mdvx::vsect_segment_t &Mdvx::getVsectSegment(const int i) const
{
  return (_vsectSegments[i]);
}

const vector<Mdvx::vsect_waypt_t> &Mdvx::getVsectWayPts() const
{
  return (_vsectWayPts);
}

const vector<Mdvx::vsect_samplept_t> &Mdvx::getVsectSamplePts() const
{
  return (_vsectSamplePts);
}

const vector<Mdvx::vsect_segment_t> &Mdvx::getVsectSegments() const
{
  return (_vsectSegments);
}

//////////////////////////////////////////////////
// assemble/disassable vert section chunk data

//////////////////////////////////////////////////
// assemble vsection waypoint buffer
// handle byte-swapping into BE order

void Mdvx::assembleVsectWayPtsBuf(const vector<Mdvx::vsect_waypt_t> &wayPts,
                                  MemBuf &buf)
  
{
  
  buf.free();
  
  int npts = wayPts.size();

  chunkVsectWayPtHdr_t hdr;
  MEM_zero(hdr);
  hdr.npts = npts;

  buf.add(&hdr, sizeof(hdr));
  for (int i = 0; i < npts; i++) {
    chunkVsectWayPt_t waypt;
    waypt.lat = wayPts[i].lat;
    waypt.lon = wayPts[i].lon;
    buf.add(&waypt, sizeof(waypt));
  }

  BE_from_array_32(buf.getPtr(), buf.getLen());

}

//////////////////////////////////////////////////
// assemble vsection sample pt buffer
// handle byte-swapping into BE order

void Mdvx::assembleVsectSamplePtsBuf(const vector<Mdvx::vsect_samplept_t> &samplePts,
                                     double dxKm,
                                     MemBuf &buf)
  
{

  buf.free();
  
  int npts = samplePts.size();

  chunkVsectSamplePtHdr_t hdr;
  MEM_zero(hdr);
  hdr.npts = npts;
  hdr.dx_km = dxKm;

  buf.add(&hdr, sizeof(hdr));
  for (int i = 0; i < npts; i++) {
    chunkVsectSamplePt_t sample;
    sample.lat = samplePts[i].lat;
    sample.lon = samplePts[i].lon;
    sample.segNum = samplePts[i].segNum;
    sample.spare = 0;
    buf.add(&sample, sizeof(sample));
  }

  BE_from_array_32(buf.getPtr(), buf.getLen());

}

//////////////////////////////////////////////////
// assemble vsection segments buffer
// handle byte-swapping into BE order

void Mdvx::assembleVsectSegmentsBuf(const vector<Mdvx::vsect_segment_t> &segments,
                                    double totalLength,
                                    MemBuf &buf)

{

  buf.free();
  
  int nsegments = segments.size();
  chunkVsectSegmentHdr_t hdr;
  MEM_zero(hdr);
  hdr.nsegments = nsegments;
  hdr.total_length = totalLength;

  buf.add(&hdr, sizeof(hdr));
  for (int i = 0; i < nsegments; i++) {
    chunkVsectSegment_t seg;
    seg.length = segments[i].length;
    seg.azimuth = segments[i].azimuth;
    buf.add(&seg, sizeof(seg));
  }

  BE_from_array_32(buf.getPtr(), buf.getLen());

}

//////////////////////////////////////////////////
// disassemble vsection waypoints buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string

int Mdvx::disassembleVsectWayPtsBuf(const MemBuf &buf,
                                    vector<Mdvx::vsect_waypt_t> &wayPts,
                                    string &errStr)

{

  wayPts.clear();
  
  MemBuf copy(buf);
  size_t bufLen = copy.getLen();
  char *bptr = (char *) copy.getPtr();

  if (bufLen < sizeof(chunkVsectWayPtHdr_t)) {
    errStr += "ERROR - Mdvx::_disassembleVsectWayPtsBuf.\n";
    errStr += "  Waypt buffer is too small.\n";
    TaStr::AddInt(errStr, "  Size expected at least: ",
		  sizeof(chunkVsectWayPtHdr_t));
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }
  
  BE_to_array_32(bptr, bufLen);
  
  chunkVsectWayPtHdr_t whdr;
  memcpy(&whdr, bptr, sizeof(chunkVsectWayPtHdr_t));
  bptr += sizeof(chunkVsectWayPtHdr_t);
  size_t sizeExpected =
    sizeof(chunkVsectWayPtHdr_t) + whdr.npts * sizeof(chunkVsectWayPt_t);
  if (bufLen < sizeExpected) {
    errStr += "ERROR - Mdvx::_disassembleVsectWayPtsBuf.\n";
    errStr += "  Waypt buffer is too small.\n";
    TaStr::AddInt(errStr, "  Npts found: ", whdr.npts);
    TaStr::AddInt(errStr, "  Size expected at least: ", sizeExpected);
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }

  for (int i = 0; i < whdr.npts; i++) {
    chunkVsectWayPt_t waypt;
    memcpy(&waypt, bptr, sizeof(waypt));
    bptr += sizeof(waypt);
    vsect_waypt_t pt;
    pt.lat = waypt.lat;
    pt.lon = waypt.lon;
    wayPts.push_back(pt);
  }

  return 0;

}

//////////////////////////////////////////////////
// disassemble vsection samplepts buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string

int Mdvx::disassembleVsectSamplePtsBuf(const MemBuf &buf,
                                       vector<Mdvx::vsect_samplept_t> &samplePts,
                                       double &dxKm,
                                       string &errStr)
  
{

  samplePts.clear();
  dxKm = 0.0;

  MemBuf copy(buf);
  size_t bufLen = copy.getLen();
  char *bptr = (char *) copy.getPtr();

  if (bufLen < sizeof(chunkVsectSamplePtHdr_t)) {
    errStr += "ERROR - Mdvx::_disassembleVsectSamplePtsBuf.\n";
    errStr += "  Samplept buffer is too small.\n";
    TaStr::AddInt(errStr, "  Size expected at least: ",
		  sizeof(chunkVsectSamplePtHdr_t));
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }
  
  BE_to_array_32(bptr, bufLen);
  
  chunkVsectSamplePtHdr_t shdr;
  memcpy(&shdr, bptr, sizeof(chunkVsectSamplePtHdr_t));
  bptr += sizeof(chunkVsectSamplePtHdr_t);

  dxKm = shdr.dx_km;
  
  size_t sizeExpected =
    sizeof(chunkVsectSamplePtHdr_t) + shdr.npts * sizeof(chunkVsectSamplePt_t);
  if (bufLen < sizeExpected) {
    errStr += "ERROR - Mdvx::_disassembleVsectSamplePtsBuf.\n";
    errStr += "  Samplept buffer is too small.\n";
    TaStr::AddInt(errStr, "  Size expected: ", sizeExpected);
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }

  for (int i = 0; i < shdr.npts; i++) {

    chunkVsectSamplePt_t samplept;
    memcpy(&samplept, bptr, sizeof(samplept));
    bptr += sizeof(samplept);

    Mdvx::vsect_samplept_t pt;
    pt.lat = samplept.lat;
    pt.lon = samplept.lon;
    pt.segNum = samplept.segNum;

    samplePts.push_back(pt);

  }

  return 0;

}
  
//////////////////////////////////////////////////
// disassemble vsection segments buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string

int Mdvx::disassembleVsectSegmentsBuf(const MemBuf &buf,
                                      vector<Mdvx::vsect_segment_t> &segments,
                                      double &totalLength,
                                      string &errStr)
  
{

  segments.clear();
  totalLength = 0.0;

  MemBuf copy(buf);
  size_t bufLen = copy.getLen();
  char *bptr = (char *) copy.getPtr();

  if (bufLen < sizeof(chunkVsectSegmentHdr_t)) {
    errStr += "ERROR - Mdvx::_disassembleVsectSegmentsBuf.\n";
    errStr += "  Segments buffer is too small.\n";
    TaStr::AddInt(errStr, "  Size expected at least: ",
		  sizeof(chunkVsectSegmentHdr_t));
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }

  BE_to_array_32(bptr, bufLen);

  chunkVsectSegmentHdr_t shdr;
  memcpy(&shdr, bptr, sizeof(chunkVsectSegmentHdr_t));
  bptr += sizeof(chunkVsectSegmentHdr_t);
  totalLength = shdr.total_length;

  size_t sizeExpected =
    sizeof(chunkVsectSegmentHdr_t) + shdr.nsegments * sizeof(chunkVsectSegment_t);

  if (bufLen < sizeExpected) {
    errStr += "ERROR - Mdvx::_disassembleVsectSegmentsBuf.\n";
    errStr += "  Segments buffer is too small.\n";
    TaStr::AddInt(errStr, "  Size expected: ", sizeExpected);
    TaStr::AddInt(errStr, "  Size found in message: ", bufLen);
    return -1;
  }

  for (int i = 0; i < shdr.nsegments; i++) {
    chunkVsectSegment_t segment;
    memcpy(&segment, bptr, sizeof(segment));
    bptr += sizeof(segment);
    Mdvx::vsect_segment_t seg;
    seg.length = segment.length;
    seg.azimuth = segment.azimuth;
    segments.push_back(seg);
  }

  return 0;
}

//////////////////////////////////////////////////
// add chunks specific to the vertical section info
// this is performed after a readVsection request

void Mdvx::_addVsectChunks()
  
{

  MdvxChunk *wayptChunk = new MdvxChunk;
  wayptChunk->setId(CHUNK_VSECT_WAY_PTS);
  wayptChunk->setInfo("Vertical section waypoints");
  MemBuf wayPtBuf;
  assembleVsectWayPtsBuf(_vsectWayPts, wayPtBuf);
  wayptChunk->setData(wayPtBuf.getPtr(), wayPtBuf.getLen());
  addChunk(wayptChunk);

  MdvxChunk *sampleptChunk = new MdvxChunk;
  sampleptChunk->setId(CHUNK_VSECT_SAMPLE_PTS);
  sampleptChunk->setInfo("Vertical section sample points");
  MemBuf samplePtBuf;
  assembleVsectSamplePtsBuf(_vsectSamplePts, _vsectDxKm, samplePtBuf);
  sampleptChunk->setData(samplePtBuf.getPtr(), samplePtBuf.getLen());
  addChunk(sampleptChunk);

  MdvxChunk *segmentChunk = new MdvxChunk;
  segmentChunk->setId(CHUNK_VSECT_SEGMENTS);
  segmentChunk->setInfo("Vertical section segments");
  MemBuf segmentBuf;
  assembleVsectSegmentsBuf(_vsectSegments, _vsectTotalLength, segmentBuf);
  segmentChunk->setData(segmentBuf.getPtr(), segmentBuf.getLen());
  addChunk(segmentChunk);

}

//////////////////////////////////////////////////
// load vertical setion info from chunks
// This is performed if a readVolume request is made to a file
// which already contains a vertical section

int Mdvx::_loadVsectInfoFromChunks()
  
{

  int iret = 0;

  for (int ii = 0; ii < (int) _chunks.size(); ii++) {

    const MdvxChunk &chunk = *_chunks[ii];

    if (chunk.getId() == CHUNK_VSECT_WAY_PTS) {

      MemBuf buf;
      buf.add(chunk.getData(), chunk.getSize());
      if (disassembleVsectWayPtsBuf(buf, _vsectWayPts, _errStr)) {
        _errStr += "ERROR - _loadVsectInfoFromChunks()\n";
        iret = -1;
      }
      
    } else if (chunk.getId() == CHUNK_VSECT_SAMPLE_PTS) {

      MemBuf buf;
      buf.add(chunk.getData(), chunk.getSize());
      if (disassembleVsectSamplePtsBuf(buf, _vsectSamplePts, _vsectDxKm, _errStr)) {
        _errStr += "ERROR - _loadVsectInfoFromChunks()\n";
        iret = -1;
      }
      
    } else if (chunk.getId() == CHUNK_VSECT_SEGMENTS) {

      MemBuf buf;
      buf.add(chunk.getData(), chunk.getSize());
      if (disassembleVsectSegmentsBuf(buf, _vsectSegments, _vsectTotalLength, _errStr)) {
        _errStr += "ERROR - _loadVsectInfoFromChunks()\n";
        iret = -1;
      }
      
    }

  } // ii

  return iret;

}

////////////////////////////////////////////////
// print way points, from a packed memory buffer

void Mdvx::printVsectWayPtsBuf(const MemBuf &buf,
                                ostream &out)
  
{

  vector<Mdvx::vsect_waypt_t> pts;
  string errStr;
  if (disassembleVsectWayPtsBuf(buf, pts, errStr)) {
    cerr << "ERROR - DsMdvxMsg::_print_way_points" << endl;
    cerr << "  Bad way point buffer" << endl;
    cerr << errStr << endl;
    return;
  }
  
  out << "----------way points ------------" << endl;
  out << "  npts: " << pts.size() << endl;
  for (int ii = 0; ii < (int) pts.size(); ii++) {
    out << "  pt i, lat, lon: " << ii << ", "
        << pts[ii].lat << ", " << pts[ii].lon << endl;
  }

}

///////////////////////////////////////////////////
// print sample points, from a packed memory buffer

void Mdvx::printVsectSamplePtsBuf(const MemBuf &buf,
                                   ostream &out)
  
{

  vector<Mdvx::vsect_samplept_t> pts;
  double dxKm;
  string errStr;
  if (Mdvx::disassembleVsectSamplePtsBuf(buf, pts, dxKm, errStr)) {
    cerr << "ERROR - DsMdvxMsg::_print_sample_points" << endl;
    cerr << "  Bad sample point buffer" << endl;
    cerr << errStr << endl;
    return;
  }

  out << "----------sample points ------------" << endl;
  out << "  npts: " << pts.size() << endl;
  out << "  dx_km: " << dxKm << endl;
  for (int ii = 0; ii < (int) pts.size(); ii++) {
    out << "  pt i, lat, lon, segNum: " << ii << ", "
        << pts[ii].lat << ", " << pts[ii].lon << ", " << pts[ii].segNum << endl;
  }

}

///////////////////////////////////////////////
// print segments, from a packed memory buffer

void Mdvx::printVsectSegmentsBuf(const MemBuf &buf,
                                  ostream &out)
  
{

  vector<Mdvx::vsect_segment_t> segs;
  double totalLength;
  string errStr;
  if (Mdvx::disassembleVsectSegmentsBuf(buf, segs, totalLength, errStr)) {
    cerr << "ERROR - DsMdvxMsg::_print_segments" << endl;
    cerr << "  Bad segment buffer" << endl;
    cerr << errStr << endl;
    return;
  }
  
  out << "---------- segments ------------" << endl;
  out << "  nsegments: " << segs.size() << endl;
  out << "  total_length: " << totalLength << endl;
  for (int ii = 0; ii < (int) segs.size(); ii++) {
    out << "  seg i, length, azimuth: " << ii << ", "
        << segs[ii].length << ", " << segs[ii].azimuth << endl;
  }
}

