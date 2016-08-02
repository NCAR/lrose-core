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
///////////////////////////////////////////////////////////////////
// TstormGrid class
//   Contains information about the grid used for thunderstorms
//
// $Id: TstormGrid.cc,v 1.8 2016/03/03 18:06:33 dixon Exp $
///////////////////////////////////////////////////////////////////

#include <string>

#include <dsdata/TstormGrid.hh>
#include <toolsa/str.h>
using namespace std;


TstormGrid::TstormGrid() :
  Pjg()
{
   clear();
}

int
TstormGrid::set( titan_grid_t* tgrid ) 
{
   originLat  = tgrid->proj_origin_lat;
   originLon  = tgrid->proj_origin_lon;

   dzConstant = tgrid->dz_constant == 1 ? true : false;

   sensorX    = tgrid->sensor_x;
   sensorY    = tgrid->sensor_y;
   sensorZ    = tgrid->sensor_z;
   sensorLat  = tgrid->sensor_lat;
   sensorLon  = tgrid->sensor_lon;

   xUnits     = tgrid->unitsx;
   yUnits     = tgrid->unitsy;
   zUnits     = tgrid->unitsz;

   if( setProjection( tgrid ) < 0 )
      return( -1 );

   return( 0 );
}

int
TstormGrid::setProjection(titan_grid_t* tgrid ) 
{
   clearProjInfo();
   
   switch( tgrid->proj_type ) {
       case TITAN_PROJ_LATLON:
	  projection = PjgTypes::PROJ_LATLON;
	  initLatlon(tgrid->nx, tgrid->ny, tgrid->nz, 
		     tgrid->dx, tgrid->dy, tgrid->dz,
		     tgrid->minx, tgrid->miny, tgrid->minz);
	  break;
	  
       case TITAN_PROJ_STEREOGRAPHIC:
	  projection = PjgTypes::PROJ_STEREOGRAPHIC;
	  break;
	  
       case TITAN_PROJ_LAMBERT_CONF:
	  projection = PjgTypes::PROJ_LC2;

          lat1    = tgrid->proj_params.lc2.lat1;
	  lat2    = tgrid->proj_params.lc2.lat2;
	  SwLat   = tgrid->proj_params.lc2.SW_lat;
	  SwLon   = tgrid->proj_params.lc2.SW_lon;
	  originX = tgrid->proj_params.lc2.origin_x;
	  originY = tgrid->proj_params.lc2.origin_y;
	  
	  initLc2(tgrid->proj_params.lc2.SW_lat, tgrid->proj_params.lc2.SW_lon,
		  tgrid->proj_params.lc2.lat1, tgrid->proj_params.lc2.lat2);
	  break;
	  
       case TITAN_PROJ_MERCATOR:
	  projection = PjgTypes::PROJ_MERCATOR;
	  break;

       case TITAN_PROJ_POLAR_STEREO:
	  projection = PjgTypes::PROJ_POLAR_STEREO;
	  break;
	  
       case TITAN_PROJ_POLAR_ST_ELLIP:
	  projection = PjgTypes::PROJ_POLAR_ST_ELLIP;
	  break;
	  
       case TITAN_PROJ_CYL_EQUIDIST:
	  projection = PjgTypes::PROJ_CYL_EQUIDIST;
	  break;
	  
       case TITAN_PROJ_FLAT:
	  projection = PjgTypes::PROJ_FLAT;
	  
	  rotation = tgrid->proj_params.flat.rotation;
	  
	  initFlat(tgrid->proj_origin_lat, tgrid->proj_origin_lon,
		   tgrid->proj_params.flat.rotation);
	  
	  break;
	  
       case TITAN_PROJ_POLAR_RADAR:
	  projection = PjgTypes::PROJ_POLAR_RADAR;

	  initPolarRadar(tgrid->proj_origin_lat, tgrid->proj_origin_lon);
	  
	  break;
	  
       case TITAN_PROJ_RADIAL:
	  projection = PjgTypes::PROJ_RADIAL;
	  break;
	  
       case TITAN_PROJ_UNKNOWN:
	  projection = PjgTypes::PROJ_UNKNOWN;
	  break;
	  
       default:
	  projection = PjgTypes::PROJ_INVALID;
	  break;
   }

   //   setGridDims(tgrid->nx, tgrid->ny, tgrid->nz);
   //   setGridDeltas(tgrid->dx, tgrid->dy, tgrid->dz);
   //   setGridMins(tgrid->minx, tgrid->miny, tgrid->minz);
   
   if( projection == PjgTypes::PROJ_INVALID ) 
      return( -1 );
   
   return( 0 );
   
}

void
TstormGrid::clearProjInfo() 
{
   rotation   = 0.0;
   lat1       = 0.0;
   lat2       = 0.0;
   SwLat      = 0.0;
   SwLon      = 0.0;
   originX    = 0.0;
   originY    = 0.0;
}

void
TstormGrid::clear() 
{
   originLat  = 0.0;
   originLon  = 0.0;
   dzConstant = false;
   sensorX    = 0.0;
   sensorY    = 0.0;
   sensorLat  = 0.0;
   sensorLon  = 0.0;
   projection = PjgTypes::PROJ_INVALID;
   xUnits     = "";
   yUnits     = "";
   zUnits     = "";

   clearProjInfo();
}


void TstormGrid::setGridValues(titan_grid_t &titan_grid) const
{
  memset(&titan_grid, 0, sizeof(titan_grid_t));
  
  titan_grid.proj_origin_lat = originLat;
  titan_grid.proj_origin_lon = originLon;
  
  titan_grid.minx = getMinx();
  titan_grid.miny = getMiny();
  titan_grid.minz = getMinz();
  
  titan_grid.dx = getDx();
  titan_grid.dy = getDy();
  titan_grid.dz = getDz();
  
  titan_grid.sensor_x = sensorX;
  titan_grid.sensor_y = sensorY;
  titan_grid.sensor_z = sensorZ;
  titan_grid.sensor_lat = sensorLat;
  titan_grid.sensor_lon = sensorLon;
  
  if (dzConstant)
    titan_grid.dz_constant = 1;
  
  titan_grid.nx = getNx();
  titan_grid.ny = getNy();
  titan_grid.nz = getNz();
  
  titan_grid.nbytes_char = 3 * TITAN_GRID_UNITS_LEN;
  
  STRcopy(titan_grid.unitsx, xUnits.c_str(), TITAN_GRID_UNITS_LEN);
  STRcopy(titan_grid.unitsy, yUnits.c_str(), TITAN_GRID_UNITS_LEN);
  STRcopy(titan_grid.unitsz, zUnits.c_str(), TITAN_GRID_UNITS_LEN);
  
  switch (projection)
  {
  case PjgTypes::PROJ_LATLON :
    titan_grid.proj_type = TITAN_PROJ_LATLON;
    break;
    
  case PjgTypes::PROJ_STEREOGRAPHIC :
    titan_grid.proj_type = TITAN_PROJ_STEREOGRAPHIC;
    break;
    
  case PjgTypes::PROJ_LC2 :
    titan_grid.proj_type = TITAN_PROJ_LAMBERT_CONF;
    titan_grid.proj_params.lc2.lat1 = lat1;
    titan_grid.proj_params.lc2.lat2 = lat2;
    titan_grid.proj_params.lc2.SW_lat = SwLat;
    titan_grid.proj_params.lc2.SW_lon = SwLon;
    titan_grid.proj_params.lc2.origin_x = originX;
    titan_grid.proj_params.lc2.origin_y = originY;
    break;
    
  case PjgTypes::PROJ_MERCATOR :
    titan_grid.proj_type = TITAN_PROJ_MERCATOR;
    break;
    
  case PjgTypes::PROJ_POLAR_STEREO :
    titan_grid.proj_type = TITAN_PROJ_POLAR_STEREO;
    break;
    
  case PjgTypes::PROJ_POLAR_ST_ELLIP :
    titan_grid.proj_type = TITAN_PROJ_POLAR_ST_ELLIP;
    break;
    
  case PjgTypes::PROJ_CYL_EQUIDIST :
    titan_grid.proj_type = TITAN_PROJ_CYL_EQUIDIST;
    break;
    
  case PjgTypes::PROJ_FLAT :
    titan_grid.proj_type = TITAN_PROJ_FLAT;
    titan_grid.proj_params.flat.rotation = rotation;
    break;
    
  case PjgTypes::PROJ_POLAR_RADAR :
    titan_grid.proj_type = TITAN_PROJ_POLAR_RADAR;
    break;
    
  case PjgTypes::PROJ_RADIAL :
    titan_grid.proj_type = TITAN_PROJ_RADIAL;
    break;
    
  case PjgTypes::PROJ_INVALID :
  default:
    titan_grid.proj_type = TITAN_PROJ_UNKNOWN;
    break;
  } /* endswitch - projection */
  
}


void
TstormGrid::print(ostream &out, const string &leader) const
{
  out << leader << "TstormGrid object" << endl;
  out << leader << "=================" << endl;
  out << leader << "proj type: " << projType2String(projection) << endl;
  out << leader << "origin lat: " << originLat << endl;
  out << leader << "origin lon: " << originLon << endl;
  out << leader << "dz constant?: " << dzConstant << endl;
  out << leader << "nx: " << getNx() << endl;
  out << leader << "ny: " << getNy() << endl;
  out << leader << "nz: " << getNz() << endl;
  out << leader << "minx: " << getMinx() << endl;
  out << leader << "miny: " << getMiny() << endl;
  out << leader << "minz: " << getMinz() << endl;
  out << leader << "dx: " << getDx() << endl;
  out << leader << "dy: " << getDy() << endl;
  out << leader << "dz: " << getDz() << endl;
  out << leader << "sensor x: " << sensorX << endl;
  out << leader << "sensor y: " << sensorY << endl;
  out << leader << "sensor z: " << sensorZ << endl;
  out << leader << "sensor lat: " << sensorLat << endl;
  out << leader << "sensor lon: " << sensorLon << endl;
  out << leader << "x units: " << xUnits << endl;
  out << leader << "y units: " << yUnits << endl;
  out << leader << "z units: " << zUnits << endl;
}


void
TstormGrid::print(FILE *out, const string &leader) const
{
  fprintf(out, "%sTstormGrid object\n", leader.c_str());
  fprintf(out, "%s=================\n", leader.c_str());
  fprintf(out, "%sproj type: %s\n",
	  leader.c_str(), projType2String(projection).c_str());
  fprintf(out, "%sorigin lat: %f\n", leader.c_str(), originLat);
  fprintf(out, "%sorigin lon: %f\n", leader.c_str(), originLon);
  fprintf(out, "%sdz constant?: %d\n", leader.c_str(), dzConstant);
  fprintf(out, "%snx: %d\n", leader.c_str(), getNx());
  fprintf(out, "%sny: %d\n", leader.c_str(), getNy());
  fprintf(out, "%snz: %d\n", leader.c_str(), getNz());
  fprintf(out, "%sminx: %f\n", leader.c_str(), getMinx());
  fprintf(out, "%sminy: %f\n", leader.c_str(), getMiny());
  fprintf(out, "%sminz: %f\n", leader.c_str(), getMinz());
  fprintf(out, "%sdx: %f\n", leader.c_str(), getDx());
  fprintf(out, "%sdy: %f\n", leader.c_str(), getDy());
  fprintf(out, "%sdz: %f\n", leader.c_str(), getDz());
  fprintf(out, "%ssensor x: %f\n", leader.c_str(), sensorX);
  fprintf(out, "%ssensor y: %f\n", leader.c_str(), sensorY);
  fprintf(out, "%ssensor z: %f\n", leader.c_str(), sensorZ);
  fprintf(out, "%ssensor lat: %f\n", leader.c_str(), sensorLat);
  fprintf(out, "%ssensor lon: %f\n", leader.c_str(), sensorLon);
  fprintf(out, "%sx units: %s\n", leader.c_str(), xUnits.c_str());
  fprintf(out, "%sy units: %s\n", leader.c_str(), yUnits.c_str());
  fprintf(out, "%sz units: %s\n", leader.c_str(), zUnits.c_str());
}


const string
TstormGrid::projType2String(const PjgTypes::proj_type_t proj_type)
{
  switch (proj_type)
  {
  case PjgTypes::PROJ_INVALID :
    return "INVALID";
    
  case PjgTypes::PROJ_LATLON :
    return "LATLON";
    
  case PjgTypes::PROJ_STEREOGRAPHIC :
    return "STEREOGRAPHIC";
    
  case PjgTypes::PROJ_LC1 :
    return "LAMBERT_CONFORMAL1";
    
  case PjgTypes::PROJ_LC2 :
    return "LAMBERT_CONFORMAL2";
    
  case PjgTypes::PROJ_MERCATOR :
    return "MERCATOR";
    
  case PjgTypes::PROJ_POLAR_STEREO :
    return "POLAR_STEREO";
    
  case PjgTypes::PROJ_POLAR_ST_ELLIP :
    return "POLAR_ST_ELLIP";
    
  case PjgTypes::PROJ_CYL_EQUIDIST :
    return "CYL_EQUIDIST";
    
  case PjgTypes::PROJ_FLAT :
    return "FLAT";
    
  case PjgTypes::PROJ_POLAR_RADAR :
    return "POLAR_RADAR";
    
  case PjgTypes::PROJ_RADIAL :
    return "RADIAL";
    
  case PjgTypes::PROJ_OBLIQUE_STEREO :
    return "OBLIQUE_STEREO";
    
  case PjgTypes::PROJ_TRANS_MERCATOR :
    return "TRANS_MERCATOR";
    
  case PjgTypes::PROJ_AZIM_EQUIDIST :
    return "AZIM_EQUIDIST";
    
  case PjgTypes::PROJ_ALBERS :
    return "ALBERS";
    
  case PjgTypes::PROJ_LAMBERT_CONF :
    return "LAMBERT_CONF";
    
  case PjgTypes::PROJ_LAMBERT_AZIM :
    return "LAMBERT_AZIM";
    
  case PjgTypes::PROJ_VERT_PERSP :
    return "VERT_PERSP";
    
  case PjgTypes::PROJ_UNKNOWN :
    return "UNKNOWN";
  }
  
  return "INVALID PROJ TYPE";
}

