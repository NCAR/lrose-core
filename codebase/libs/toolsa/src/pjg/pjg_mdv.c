/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/**************************************************************
 *  pjg_mdv.c
 *
 * convert pjg types to mdv types, and vice versa
 */

#include <toolsa/pjg.h>

/* MDV Projection types */

#define     MDV_PROJ_NATIVE           -1
#define     MDV_PROJ_LATLON           0   /* x,y in degrees. 
                                             z defined by vert proj type */
#define     MDV_PROJ_ARTCC            1   /* x,y in km */
#define     MDV_PROJ_STEREOGRAPHIC    2   /* x,y in km */
#define     MDV_PROJ_LAMBERT_CONF     3   /* x,y in km */
#define     MDV_PROJ_MERCATOR         4   /* x,y in km */
#define     MDV_PROJ_POLAR_STEREO     5   /* x,y in km */
#define     MDV_PROJ_POLAR_ST_ELLIP   6   /* x,y in km */
#define     MDV_PROJ_CYL_EQUIDIST     7   /* x,y in km */
#define     MDV_PROJ_FLAT             8   /* Cartesian, x,y in km. 
                                             z defined by vert proj type*/
#define     MDV_PROJ_POLAR_RADAR      9   /* Radial range, Azimuth angle,
                                           * x is gate spacing in km .
                                           * y is azimuth in degrees. from
					   * true north + is clockwise
					   * z is elevation angle in degrees. 
                                           */
#define     MDV_PROJ_RADIAL          10   /* x = Radius, Meters,
					   * y = azimuth in degrees
					   * z = Defined by MDV_VERT_TYPE...
					   */
#define     MDV_PROJ_UNKNOWN         99


/*******************************************
 * pjg2mdv_type()
 *
 * Converts pjg projection type to mdv type
 */

int pjg2mdv_type(int pjg_type)

{

  switch(pjg_type) {

  case PJG_ARTCC:
    return (MDV_PROJ_ARTCC);
    break;

  case PJG_OBLIQUE_STEREOGRAPHIC:
    return (MDV_PROJ_STEREOGRAPHIC);
    break;

  case PJG_LAMBERT_CONFORMAL2:
    return (MDV_PROJ_LAMBERT_CONF);
    break;

  case PJG_TRANSVERSE_MERCATOR:
    return (MDV_PROJ_MERCATOR);
    break;

  case PJG_FLAT:
    return (MDV_PROJ_FLAT);
    break;

  case PJG_STEREOGRAPHIC_ELLIPSOID:
    return (MDV_PROJ_POLAR_ST_ELLIP);
    break;

  case PJG_DENVER_ARTCC:
    return (MDV_PROJ_ARTCC);
    break;

  case PJG_CYL_EQUIDISTANT:
    return (MDV_PROJ_CYL_EQUIDIST);
    break;

  case PJG_SPHERICAL:
    return (MDV_PROJ_POLAR_RADAR);
    break;

  default:
    return (MDV_PROJ_UNKNOWN);

  }

}

/*******************************************
 * mdv2pjg_type()
 *
 * Converts mdv projection type to pjg type
 */

int mdv2pjg_type(int mdv_type)

{

  switch(mdv_type) {

  case MDV_PROJ_NATIVE:
    return (PJG_UNKNOWN);
    break;

  case MDV_PROJ_LATLON:
    return (PJG_LATLON);
    break;

  case MDV_PROJ_ARTCC:
    return (PJG_DENVER_ARTCC);
    break;

  case MDV_PROJ_STEREOGRAPHIC:
    return (PJG_OBLIQUE_STEREOGRAPHIC);
    break;

  case MDV_PROJ_LAMBERT_CONF:
    return (PJG_LAMBERT_CONFORMAL2);
    break;

  case MDV_PROJ_MERCATOR:
    return (PJG_TRANSVERSE_MERCATOR);
    break;

  case MDV_PROJ_POLAR_STEREO:
    return (PJG_UNKNOWN);
    break;

  case MDV_PROJ_POLAR_ST_ELLIP:
    return (PJG_STEREOGRAPHIC_ELLIPSOID);
    break;

  case MDV_PROJ_CYL_EQUIDIST:
    return (PJG_CYL_EQUIDISTANT);
    break;

  case MDV_PROJ_FLAT:
    return (PJG_FLAT);
    break;

  case MDV_PROJ_POLAR_RADAR:
    return (PJG_SPHERICAL);
    break;

  }

  return(PJG_UNKNOWN);
}


