/**
 * @file ShapePolygons.cc
 */
#include <FiltAlgVirtVol/ShapePolygons.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/GenPolyGrid.hh>
#include <Spdb/DsSpdb.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <cmath>

//------------------------------------------------------------------
ShapePolygons::ShapePolygons(void) : MathUserData(), _ok(false)
{
}

//------------------------------------------------------------------
ShapePolygons::ShapePolygons(const time_t &t, int expireSeconds,
			     const MdvxProj &proj,
			     const Grid2d &g, bool isDiamondShape) :
  MathUserData(), _ok(true)
{
  if (isDiamondShape)
  {
    _createDiamonds(t, expireSeconds, proj, g, false, -1);
  }
  else
  {
    _createWrappedShapes(t, expireSeconds, proj, g);
  }
}

//------------------------------------------------------------------
ShapePolygons::ShapePolygons(const time_t &t, int expireSeconds,
			     const MdvxProj &proj, double shapeSizeKm,
			     const Grid2d &g) :
  MathUserData(), _ok(true)
{
  _createDiamonds(t, expireSeconds, proj, g, true, shapeSizeKm);
}

/*----------------------------------------------------------------*/
bool ShapePolygons::getFloat(double &v) const
{
  return 0.0;
}

/*----------------------------------------------------------------*/
void ShapePolygons::output(const time_t &t, int expireSeconds,
			   const std::string &url)
{
  DsSpdb D;
  D.clearPutChunks();
  D.setPutMode(Spdb::putModeAdd);
  D.addUrl(url);
  for (size_t i=0; i<_shapes.size(); ++i)
  {
    _shapes[i].assemble();
    if (D.put(SPDB_GENERIC_POLYLINE_ID,
	      SPDB_GENERIC_POLYLINE_LABEL,
	      i+1, t, t+expireSeconds,
	      _shapes[i].getBufLen(),
	      (void *)_shapes[i].getBufPtr() ))
    {
      LOG(ERROR) << "problems writing to SPDB";
    }
    else
    {
      LOG(DEBUG) << "Wrote SPDB";
    }
  }
}

//------------------------------------------------------------------
void ShapePolygons::_createDiamonds(const time_t &t, int expireSeconds,
				    const MdvxProj &proj, const Grid2d &g,
				    bool isFixed, double fixedSizeKm) 
{
  Grid2dClump c(g);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  LOG(DEBUG) << "Creating shape polygons for "
	     << clumps.size() << " clumps";
  for (size_t i=0; i<clumps.size(); ++i)
  {
    clump::Region_citer_t c;
    int xmin, ymin, xmax, ymax;
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      if (c == clumps[i].begin())
      {
	xmin = xmax = c->first;
	ymin = ymax = c->second;
      }
      else
      {
	if (c->first < xmin) xmin = c->first;
	if (c->first > xmax) xmax = c->first;
	if (c->second < ymin) ymin = c->second;
	if (c->second > ymax) ymax = c->second;
      }
    }

    // now have extent in x and y. lets assum the average is the center.
    int xmid = (xmin + xmax)/2;
    int ymid = (ymin + ymax)/2;

    // get an area total based on distance from the radar (roughly)
    Mdvx::coord_t coord = proj.getCoord();
    double R = coord.dx*(double)xmid + coord.minx;  // km assumed
    double daz = coord.dy;  // degrees assumed 
    double darclen = daz*3.14159/180.0*R;  // r*theta

    double radius;
    if (isFixed)
    {
      radius = fixedSizeKm/sqrt(2); // len/sqrt(2.0);
    }
    else
    {
      double area = coord.dx*(double)(xmax-xmin+1) * darclen*(double)(ymax-ymin+1);

      // the area of the diamond should be this, meaning sqrt per side
      double len = sqrt(area);
	
      // a bit of geometry  (radius^2 + radius^2) = len^2  for dist from center x or y
      radius = len/sqrt(2.0);
    }
    

    // rotate later for now  figure out npt x and npt y from center to end

    int i0 = (int)((R - radius - coord.minx)/coord.dx);
    int i1 = (int)((R + radius - coord.minx)/coord.dx);
    int ic = xmid;

    // arc length per y index is darclen above
    int dj = (int)(radius/darclen);
    int j0 =  ymid - dj;
    int j1 = ymid + dj;
    int jc = ymid;
    

    // figure out approximate change in longitudes and latitudes
    double minlat, maxlat, minlon, maxlon;
    double dlat, dlon;
    proj.xyIndex2latlon(i0, jc, dlat, dlon);
    minlat = maxlat = dlat;
    minlon = maxlon = dlon;
    proj.xyIndex2latlon(ic, j1, dlat, dlon);
    if (dlat < minlat) minlat = dlat;
    if (dlat > maxlat) maxlat = dlat;
    if (dlon < minlon) minlon = dlon;
    if (dlon > maxlon) maxlon = dlon;
    proj.xyIndex2latlon(i1, jc, dlat, dlon);
    if (dlat < minlat) minlat = dlat;
    if (dlat > maxlat) maxlat = dlat;
    if (dlon < minlon) minlon = dlon;
    if (dlon > maxlon) maxlon = dlon;
    proj.xyIndex2latlon(i0, jc, dlat, dlon);
    if (dlat < minlat) minlat = dlat;
    if (dlat > maxlat) maxlat = dlat;
    if (dlon < minlon) minlon = dlon;
    if (dlon > maxlon) maxlon = dlon;
    proj.xyIndex2latlon(ic, j0, dlat, dlon);
    if (dlat < minlat) minlat = dlat;
    if (dlat > maxlat) maxlat = dlat;
    if (dlon < minlon) minlon = dlon;
    if (dlon > maxlon) maxlon = dlon;

    // use these extremes as our diamond point, with center remaining as is (kind of a rotation) 
    double clat, clon;
    proj.xyIndex2latlon(ic, jc, clat, clon);
    

    LOG(DEBUG) << "creating a poly from " << clumps[i].size() << " clump points";
    GenPoly gp;
    gp.clear();
    gp.setName("Poly");
    gp.setId(i+1);
    gp.setTime(t);
    gp.setExpireTime(t + expireSeconds);
    gp.setNLevels(1);
    gp.setClosedFlag(true);
    gp.clearVertices();
    gp.clearVals();
    gp.clearFieldInfo();

    GenPoly::vertex_t vertex;
    vertex.lat = clat;
    vertex.lon = minlon;
    gp.addVertex(vertex);
    vertex.lat = maxlat;
    vertex.lon = clon;
    gp.addVertex(vertex);
    vertex.lat = clat;
    vertex.lon = maxlon;
    gp.addVertex(vertex);
    vertex.lat = clat;
    vertex.lon = minlon;
    gp.addVertex(vertex);
    vertex.lat = minlat;
    vertex.lon = clon;
    gp.addVertex(vertex);
    vertex.lat = maxlat;
    vertex.lon = clon;
    gp.addVertex(vertex);
    vertex.lat = clat;
    vertex.lon = maxlon;
    gp.addVertex(vertex);
    vertex.lat = minlat;
    vertex.lon = clon;
    gp.addVertex(vertex);
    _shapes.push_back(gp);
  }
}

//------------------------------------------------------------------
void ShapePolygons::_createWrappedShapes(const time_t &t, int expireSeconds,
					 const MdvxProj &proj, const Grid2d &g) 
{
  GridAlgs ga(g);
  Grid2dClump c(g);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  LOG(DEBUG) << "Creating shape polygons for "
	     << clumps.size() << " clumps";
  for (size_t i=0; i<clumps.size(); ++i)
  {
    int num=0;
    ga.setAllMissing();
    clump::Region_citer_t c;
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      num++;
      ga.setValue(c->first, c->second, 1.0);
    }
    LOG(DEBUG) << "creating a poly from " << num << " clump points";
    GenPolyGrid gpg;
    gpg.set(t, t+expireSeconds, i+1, ga, proj);
    gpg.setAndSmooth(t, t+expireSeconds, i+1, ga, proj, 10, 0.5);
    _shapes.push_back(gpg);
  }
}

