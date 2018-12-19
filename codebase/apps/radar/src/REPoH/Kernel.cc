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
 * @file Kernel.cc
 */
#include "Kernel.hh"
#include "KernelTemplate.hh"
#include "CloudGap.hh"
#include "KernelGrids.hh"
#include <Mdv/MdvxProj.hh>
#include <Spdb/DsSpdb.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/LogStream.hh>
#include <Mdv/GenPolyGrid.hh>
#include <dataport/port_types.h>
#include <cmath>

static bool _debug = false;

/*----------------------------------------------------------------*/
Kernel::Kernel(double vlevel, bool isFar, const Grid2d &mask,
	       const Grid2d &omask, const CloudGap &g, 
	       const RepohParams &params, const MdvxProj &gp)
{
  // get centerpoint from gaps object
  _center = Point(g.getX(isFar), g.getY());

  // debugging
  _debug = _initializeDebugging(gp, vlevel, params);

  // Mdvx::coord_t coord = gp.getCoord();

  bool atRadar = g.isClosest() && !isFar;

  _kpts = KernelPoints(mask, omask, gp, _center, atRadar, isFar, params);
  _totalKpts = _kpts;
  
  // vlevel passed in
  _vlevel = vlevel;

  // initialize to a dummy color
  _color = 1;

}

/*----------------------------------------------------------------*/
Kernel::~Kernel()
{
}

/*----------------------------------------------------------------*/
void Kernel::centerpointToGrid(Grid2d &g) const
{
  _center.toGrid(g, _color);
}

/*----------------------------------------------------------------*/
void Kernel::printStatus(void) const
{
  printf("Kernel:v:%6.3lf id:%02d %s", _vlevel, _color,
	 _data.sprintStatus().c_str());
  if (_data.passedTests())
  {
    printf("A:%8.2lf H:%8.2lf\n", _Ag, _q);
  }
}

/*----------------------------------------------------------------*/
void Kernel::print(void) const
{
  printf("kernel%s pts:\n", _center.sprint().c_str());
  _kpts.print();
}

/*----------------------------------------------------------------*/
void Kernel::printDebug(const string dir, int id, double vlevel,
			const KernelGrids &grids, double km_per_gate)
{
  char buf[1000];
  sprintf(buf, "%s/Kernel_%.2lf_%02d", dir.c_str(), vlevel, id);
  FILE *fp = fopen(buf, "w");
  fprintf(fp, "%s\n", _kpts.sprintDebug(km_per_gate).c_str());
  _kpts.printData(fp, *grids._sdbz);
  _kpts.printData(fp, *grids._kdbzAdjusted);
  _kpts.printData(fp, *grids._szdr);
  _kpts.printData(fp, *grids._snoise);
  _kpts.printData(fp, *grids._knoise);
  _kpts.printData(fp, *grids._srhohv);
  fprintf(fp, "%d %10.5lf %10.5lf %10.5lf %s\n", id, vlevel, _Ag, _q,
	  _data.sprintDebug().c_str());
  fclose(fp);
}

/*----------------------------------------------------------------*/
void Kernel::finishProcessing(const time_t &time, int id, double vlevel,
			      const KernelGrids &grids, const RepohParams &P,
			      double kmPerGate)
{
  // set color
  _color = id;

  // do all the tests that require data
  if (atRadar())
  {
    _data.evaluate();
  }
  else
  {
    _kpts.removeOutliers(*grids._dbz_diff, P.dbz_diff_threshold,
			 P.min_kernel_size);
    _data.evaluate(grids, P, _kpts, _kpts.enoughPoints(P.min_kernel_size),
		   _totalKpts.npt() - _kpts.npt());
  }
  _Ag = _gaseousAttenuation(kmPerGate);
  _q = humidityFromAttenuation(_Ag);

  // print out information to stdout and to an output file in outdir
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
    printStatus();
}

/*----------------------------------------------------------------*/
bool Kernel::writeGenpoly(const time_t &t, bool outside, const MdvxProj &proj,
			  DsSpdb &D) const
{
  if (atRadar())
    return true;

  Mdvx::coord_t coord = proj.getCoord();
  int nx = coord.nx;
  int ny = coord.ny;

  GenPolyGrid poly;
  GridAlgs gr("tmp", nx, ny, -1);
  _kpts.toGrid(gr, 1.0, outside);
  if (!poly.setBoxes(t, t, _color, gr, proj))
  {
    return false;
  }
  poly.clearVals();
  poly.setNLevels(1);
  poly.clearFieldInfo();

  poly.addFieldInfo("i", "index");
  poly.addVal((double)_color);

  poly.addFieldInfo("D", "mm");
  poly.addVal(_data.getD0());

  poly.addFieldInfo("c", "%");
  poly.addVal(_data.getCorr());

  poly.addFieldInfo("q", "gm-3");
  poly.addVal(_q);
  poly.assemble();
  if (D.put(SPDB_GENERIC_POLYLINE_ID,
	    SPDB_GENERIC_POLYLINE_LABEL,
	    poly.getId(), t, t,
	    poly.getBufLen(),
	    (void *)poly.getBufPtr() ))
  {
    LOG(ERROR) << "problems writing to SPDB";
    return false;
  }
  return true;
}

/*----------------------------------------------------------------*/
void Kernel::getAttenuation(double &x_ave, double &y,
			     double &attenuation) const
{
  x_ave = _kpts.xAverage();
  y = _center.getY();
  attenuation = _data.attenuation();//_sdbz_ave -_kdbz_ave;
}
  
/*----------------------------------------------------------------*/
double Kernel::humidityFromAttenuation(const double a)
{
  return 201.40*a*a*a -209.60*a*a + 120.55*a - 2.25;
}

/*----------------------------------------------------------------*/
double Kernel::_gaseousAttenuation(const double km_per_gate) const
{
  return (_data.attenuation())/(2.0*_kpts.xAverage()*km_per_gate);
}

/*----------------------------------------------------------------*/
bool Kernel::_initializeDebugging(const MdvxProj &gp, double vlevel, 
				  const RepohParams &params) const
{
  Mdvx::coord_t coord = gp.getCoord();
  double range = coord.minx + _center.getX()*coord.dx;
  double az = coord.miny + _center.getY()*coord.dy;
  while (az > 360.0)
    az -= 360.0;
  while (az < 0)
    az += 360.0;
  for (int i=0; i<params.kernel_debug_n; ++i)
  {
    if (_isDebug(params._kernel_debug[i], range, az, vlevel))
    {
      return true;
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
bool Kernel::_isDebug(const RepohParams::mask_t &mask, double range,
		      double az, double vlevel) const
{
  if (range < mask.range0 || range > mask.range1)
    return false;
  if (vlevel < mask.elevation0 || vlevel > mask.elevation1)
    return false;
  return (az >= mask.azimuth0 && az <= mask.azimuth1);
}




