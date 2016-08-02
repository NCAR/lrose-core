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
 * @file Boundaries.cc
 */
#include <FiltAlg/Boundaries.hh>
#include <FiltAlg/Algorithm.hh>
#include <FiltAlg/Statics.hh>
#include <FiltAlg/GridProj.hh>

#include <toolsa/LogStream.hh>
#include <euclid/LineList.hh>
#include <euclid/PointList.hh>
#include <euclid/MotionVector.hh>
#include <rapformats/Bdry.hh>
#include <rapformats/BdryPolyline.hh>
#include <rapformats/BdryPoint.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>


using std::vector;
using std::pair;

//------------------------------------------------------------------
static bool  _write(const time_t &time, const std::string &bdryUrl,
		    const bool first, Bdry &bdry)
{
  bdry.assemble();

  DsSpdb D;
  if (first)
  {
    D.setPutMode(Spdb::putModeOver);
  }
  else
  {
    D.setPutMode(Spdb::putModeAdd);
  }
  void *buf = bdry.getBufPtr();
  int len = bdry.getBufLen();
  D.addPutChunk(Boundaries::UNCONNECTED_LINES, time, time+100, len, buf);//, 1);
  D.addUrl(bdryUrl);
  if (D.put(SPDB_BDRY_ID, SPDB_BDRY_LABEL) != 0)
  {
    LOG(ERROR) << "writing to spdb";
    printf("%s\n", D.getErrStr().c_str());
    return false;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------
bool Boundaries::write_lines(const time_t &time,
			     const int sequence,
			     const std::string &spdbUrl, 
			     const vector<LineList> &l)
{
  if (l.size() == 0)
  {
    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      UNCONNECTED_LINES, 0, description, 0.0, 0.0, 0.0, 0.0);
    return _write(time, spdbUrl, true, bdry);
  }

  for (size_t i=0; i<l.size(); ++i)
  {
    // pull id, quality and weight out of attributes
    int id;
    double quality, weight;

    if (!l[i].getInt("ID", id))
    {
      LOG(ERROR) << "no i.d. in linelist";
      id = i;
    }
    if (!l[i].getDouble("Quality", quality))
    {
      LOG(ERROR) << "no quality in linelist";
      quality = 1.0;
    }
    if (!l[i].getDouble("Percentile_used", weight))
    {
      LOG(ERROR) << "no Percentile_used in linelist";
      weight = 1.0;
    }
    
    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      UNCONNECTED_LINES, id, description, 0.0, 0.0, quality, quality);


    bdry.addSpareFloat(weight);

    char sbuf[1000];
    sprintf(sbuf, "%d", id);
    string label = sbuf;
    
    for (int j=0; j<l[i].num(); ++j)
    {
      BdryPolyline poly(0, label);
      double x, y;
      double lat, lon;

      Line line = l[i].ithLine(j);
      line.point(0, x, y);
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      
      BdryPoint pt(lat, lon);
      poly.addPoint(pt);

      line.point(1, x, y);
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      BdryPoint pt2(lat, lon);
      poly.addPoint(pt2);

      bdry.addPolyline(poly);
    }

    if (!_write(time, spdbUrl, i == 0, bdry))
    {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Boundaries::write_connected_lines(const time_t &time,
				       const int sequence,
				       const std::string &spdbUrl, 
				       const std::vector<LineList> &l)
{
  if (l.size() == 0)
  {
    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      CONNECTED_LINES, 0, description, 0.0, 0.0, 0.0, 0.0);
    return _write(time, spdbUrl, true, bdry);
  }

  for (size_t i=0; i<l.size(); ++i)
  {
    // pull id, quality out of attributes
    int id;
    double quality;

    if (!l[i].getInt("ID", id))
    {
      LOG(ERROR) << "no i.d. in linelist";
      id = i;
    }
    if (!l[i].getQuality(quality))
    {
      // LOG(ERROR) << "no quality in linelist";
      quality = 1.0;
    }
    
    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      CONNECTED_LINES, id, description, 0.0, 0.0, quality, quality);


    // bdry.addSpareFloat(weight);

    char sbuf[1000];
    sprintf(sbuf, "%d", id);
    string label = sbuf;
    
    for (int j=0; j<l[i].num(); ++j)
    {
      BdryPolyline poly(0, label);
      double x, y;
      double lat, lon;

      Line line = l[i].ithLine(j);
      line.point(0, x, y);
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      
      BdryPoint pt(lat, lon);
      poly.addPoint(pt);

      line.point(1, x, y);
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      BdryPoint pt2(lat, lon);
      poly.addPoint(pt2);

      bdry.addPolyline(poly);
    }

    if (!_write(time, spdbUrl, i == 0, bdry))
    {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Boundaries::write_connected_points(const time_t &time,
					const int sequence,
					const std::string &spdbUrl, 
					const std::vector<PointList> &l)
{

  if (l.size() == 0)
  {
    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      CONNECTED_LINES, 0, description, 0.0, 0.0, 0.0, 0.0);
    return _write(time, spdbUrl, true, bdry);
  }

  for (size_t i=0; i<l.size(); ++i)
  {
    // pull id, quality and motion out of attributes
    int id;
    double quality;
    MotionVector mv;

    if (!l[i].getInt("ID", id))
    {
      LOG(ERROR) << "no i.d. in linelist";
      id = i;
    }
    if (!l[i].getQuality(quality))
    {
      quality = 1.0;
    }
    if (!l[i].getMotionVector(mv))
    {
      mv = MotionVector(0, 0);
    }
    double vx, vy;
    vx = mv.getVx();
    vy = mv.getVy();
    vx = Statics::_gproj.xGrid2km(vx)*1000.0;
    vy = Statics::_gproj.yGrid2km(vy)*1000.0;
    mv = MotionVector(vx, vy);

    double speed, dir;
    _get_speed_dir_for_spdb(mv, speed, dir);


    string description = "mydescription";
    Bdry bdry(time, 0, time+300, "BDC", "GENERIC_LINE", sequence,
	      CONNECTED_LINES, id, description, dir, speed, quality, quality);


    // bdry.addSpareFloat(weight);

    char sbuf[1000];
    sprintf(sbuf, "%d", id);
    string label = sbuf;
    
    BdryPolyline poly(0, label);

    for (int j=0; j<l[i].size(); ++j)
    {
      double lat, lon;

      Point p = l[i].ithPoint(j);
      double x = p.getX();
      double y = p.getY();
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      
      if (p.getMotionVector(mv))
      {
	double vx = mv.getVx();
	double vy = mv.getVy();
	// units = pixels per second, want meters per second
	vx = Statics::_gproj.xGrid2km(vx)*1000.0;
	vy = Statics::_gproj.yGrid2km(vy)*1000.0;
	mv = MotionVector(vx, vy);
      }
      else
      {
	mv = MotionVector(0, 0);
      }
	
      BdryPoint pt(lat, lon, mv.getVx(), mv.getVy());
      poly.addPoint(pt);
    }

    bdry.addPolyline(poly);
    if (!_write(time, spdbUrl, i == 0, bdry))
    {
      return false;
    }

    //#ifdef NOTDEF

    description = "extrap";
    Bdry bdryE(time, 3600, time+300, "BDC", "EXTRAPOLATED_LINE", sequence,
	       CONNECTED_LINES, id, description, dir, speed, quality, quality);


    // bdry.addSpareFloat(weight);

    sprintf(sbuf, "%d", id);
    label = sbuf;
    
    BdryPolyline polyE(3600, label);

    for (int j=0; j<l[i].size(); ++j)
    {
      double lat, lon;

      Point p = l[i].ithPoint(j);
      double x = p.getX();
      double y = p.getY();
      Statics::_gproj.xyIndex2latlon(x, y, lat, lon);
      
      if (p.getMotionVector(mv))
      {
	double vx = mv.getVx();
	double vy = mv.getVy();
	// units = pixels per second, want meters per second
	vx = Statics::_gproj.xGrid2km(vx)*1000.0;
	vy = Statics::_gproj.yGrid2km(vy)*1000.0;
	mv = MotionVector(vx, vy);
      }
      else
      {
	mv = MotionVector(0, 0);
      }
	
      BdryPoint pt(lat, lon, mv.getVx(), mv.getVy());
      polyE.addPoint(pt);
    }

    bdryE.addPolyline(polyE);
    if (!_write(time, spdbUrl, false, bdryE))
    {
      return false;
    }
    //#endif

  }

  return true;
}

//------------------------------------------------------------------
std::vector<LineList> 
Boundaries::read_lines(const time_t &time, const std::string &spdbUrl)
{
  vector<LineList>  ret;

  DsSpdb D;
  if (D.getFirstAfter(spdbUrl, time, 0, 0, 0, false) != 0)
  {
    LOG(DEBUG) << "No SPDB data found in data base " << spdbUrl 
	       << " at time " << DateTime::strn(time);
    return ret;
  }
  int n = D.getNChunks();
  if (n <= 0)
  {
    LOG(DEBUG) << "No SPDB data, " << spdbUrl << ",  at time " 
	       << DateTime::strn(time);
    return ret;
  }
  
  // pull out the chunks and interpret each as a boundary
  Spdb::chunk_ref_t *hdrs = D.getChunkRefs();
  char *data = (char *)D.getChunkData();

  for (int i=0; i<n; ++i)
  {
    // each one of these is a LineList

    void *v = (void *)(data + hdrs[i].offset);
    Bdry bdry;
    bdry.disassemble(v, hdrs[i].len);

    // here try to get the values we want and store as attributes
    // (see read above)
    int id = bdry.getBdryId();
    double q = bdry.getQualityValue();
    fl32 w;
    bool hasW=true;
    if (!bdry.getSpareFloat(0, w))
    {
      w = 0.0;
      hasW = false;
    }

    LineList ll;
    ll.addInt("ID", id);
    ll.setQuality(q);
    ll.addDouble("Percentile_used", w);

    vector<BdryPolyline> poly = bdry.getPolylines();
    // each of these should be a 2 point line, put all of them into this
    // linelist.

    if (!poly.empty() && !hasW)
    {
      LOG(ERROR) << "no spare[0] in Bdry, which should have weight, w=0";
    }

    for (size_t j=0; j<poly.size(); ++j)
    {
      vector<BdryPoint> pts = poly[j].getPoints();
      if (pts.size() != 2)
      {
	LOG(ERROR) << 
	  "unexpected wanted unconnected 2 point lines, got " << pts.size();
      }
      else
      {
	double lat = pts[0].getLat();
	double lon = pts[0].getLon();
	double x, y;
	if (Statics::_gproj.latlon2xyIndex(lat, lon, x, y))
	{
	  LOG(WARNING) << "Point outside grid " << lat << " " << lon;
	}
	else
	{

	  double x2, y2;
	  lat = pts[1].getLat();
	  lon = pts[1].getLon();
	  if (Statics::_gproj.latlon2xyIndex(lat, lon, x2, y2))
	  {
	    LOG(WARNING) << "Point outside grid " << lat << " " << lon;
	  }
	  else
	  {
	    Line l(x, y, x2, y2);
	    l.addDouble("Percentile_used", w);
	    ll.append(l);
	  }
	}
      }
    }
    ret.push_back(ll);
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<LineList> 
Boundaries::read_connected_lines(const time_t &time, const std::string &spdbUrl)
{
  vector<LineList>  ret;

  DsSpdb D;
  if (D.getFirstAfter(spdbUrl, time, 0, 0, 0, false) != 0)
  {
    LOG(DEBUG) << "No SPDB data found in data base " << spdbUrl 
	       << " at time " << DateTime::strn(time);
    return ret;
  }
  int n = D.getNChunks();
  if (n <= 0)
  {
    LOG(DEBUG) << "No SPDB data, " << spdbUrl 
	       << " at time " << DateTime::strn(time);
    return ret;
  }
  
  // pull out the chunks and interpret each as a boundary
  Spdb::chunk_ref_t *hdrs = D.getChunkRefs();
  char *data = (char *)D.getChunkData();

  for (int i=0; i<n; ++i)
  {
    // each one of these is a LineList

    void *v = (void *)(data + hdrs[i].offset);
    Bdry bdry;
    bdry.disassemble(v, hdrs[i].len);

    // here try to get the values we want and store as attributes
    // (see read above)
    int id = bdry.getBdryId();
    double q = bdry.getQualityValue();
    LineList ll;
    ll.addInt("ID", id);
    ll.setQuality(q);

    vector<BdryPolyline> poly = bdry.getPolylines();

    for (size_t j=0; j<poly.size(); ++j)
    {
      vector<BdryPoint> pts = poly[j].getPoints();
      if (pts.size() != 2)
      {
	LOG(ERROR) << "unexpected wanted unconnected 2 point lines, got "
		   << pts.size();
      }
      else
      {
	double lat = pts[0].getLat();
	double lon = pts[0].getLon();
	double x, y;
	if (Statics::_gproj.latlon2xyIndex(lat, lon, x, y))
	{
	  LOG(WARNING) << "Point outside grid " << lat << " " << lon;
	}
	else
	{

	  double x2, y2;
	  lat = pts[1].getLat();
	  lon = pts[1].getLon();
	  if (Statics::_gproj.latlon2xyIndex(lat, lon, x2, y2))
	  {
	    LOG(WARNING) << "Point outside grid " << lat << " " << lon;
	  }
	  else
	  {
	    Line l(x, y, x2, y2);
	    // l.addDouble("Percentile_used", w);
	    ll.append(l);
	  }
	}
      }
    }
    if (!ll.isConnected())
    {
      LOG(ERROR) << "a non-connected line list when connected expected";
    }
    ret.push_back(ll);
  }
  return ret;
}

/*----------------------------------------------------------------*/
void Boundaries::erase(const time_t &time, const std::string &spdbUrl)
{
  DsSpdb D;
  D.erase(spdbUrl, time);
}

/*----------------------------------------------------------------*/
void Boundaries::_get_speed_dir_for_spdb(const MotionVector &mv,
					 double &speed, double &dir)
{
  double vx = mv.getVx();
  double vy = mv.getVy();
  if (vx == 0.0 && vy == 0.0)
  {
    speed = 0.0;
    dir = 0.0;
    return;
  }

  // Calculate the average speed (internal units).
  // then convert to spdb units.
  speed = sqrt(vx*vx + vy*vy);
  speed = Statics::_gproj.xGrid2km(speed)*1000.0;

  double d;
  d = atan2 (-vy , vx)*180.0/3.14159;
  while (d < 0.)
    d += 360.;
  dir = d;
}
