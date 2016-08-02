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
/*********************************************************************
 * read_params.c: reads the environment
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rctable_generate.h"

#define DEG_TO_RAD 0.01745329251994372
#define TOLERANCE 1.e-8

void read_params(void)

{

  char *resource_str;
  double cart_to_radar_range, cart_to_radar_theta;
  double radar_to_cart_range, radar_to_cart_theta;

  /*
   * file paths
   */

  Glob->rc_table_file_path =
    uGetParamString(Glob->prog_name, 
		    "rc_table_file", "null");

  Glob->slave_table_file_path =
    uGetParamString(Glob->prog_name, 
		    "slave_table_file", "null");

  Glob->azimuth_table_file_path =
    uGetParamString(Glob->prog_name, 
		    "azimuth_table_file", "null");

  /*
   * cartesian grid geometry
   */

  Glob->nx = uGetParamLong(Glob->prog_name, "nx", NX);
  Glob->ny = uGetParamLong(Glob->prog_name, "ny", NY);
  Glob->nz = uGetParamLong(Glob->prog_name, "nz", NZ);

  Glob->minx = uGetParamDouble(Glob->prog_name, "minx", MINX);
  Glob->miny = uGetParamDouble(Glob->prog_name, "miny", MINY);
  Glob->minz = uGetParamDouble(Glob->prog_name, "minz", MINZ);

  Glob->dx = uGetParamDouble(Glob->prog_name, "dx", DX);
  Glob->dy = uGetParamDouble(Glob->prog_name, "dy", DY);
  Glob->dz = uGetParamDouble(Glob->prog_name, "dz", DZ);

  Glob->radar_latitude = uGetParamDouble(Glob->prog_name,
				       "radar_latitude",
				       RADAR_LATITUDE);

  Glob->radar_longitude = uGetParamDouble(Glob->prog_name,
					"radar_longitude",
					RADAR_LONGITUDE);

  Glob->cart_latitude = uGetParamDouble(Glob->prog_name,
				       "cart_latitude",
				       CART_LATITUDE);

  Glob->cart_longitude = uGetParamDouble(Glob->prog_name,
					"cart_longitude",
					CART_LONGITUDE);

  Glob->cart_rotation = uGetParamDouble(Glob->prog_name,
					"cart_rotation",
					CART_ROTATION);

  Glob->radarz = uGetParamDouble(Glob->prog_name, "radarz", RADARZ);

  /*
   * compute the range and theta from the cart grid origin
   * to the radar, and vice-versa
   */

  uLatLon2RTheta(Glob->cart_latitude, Glob->cart_longitude,
		 Glob->radar_latitude, Glob->radar_longitude,
		 &cart_to_radar_range, &cart_to_radar_theta);

  uLatLon2RTheta(Glob->radar_latitude, Glob->radar_longitude,
		 Glob->cart_latitude, Glob->cart_longitude,
		 &radar_to_cart_range, &radar_to_cart_theta);


  /*
   * compute the radar (x, y) on the cart grid
   */

  if (fabs(radar_to_cart_range) < TOLERANCE) {

    Glob->radarx = 0.0;
    Glob->radary = 0.0;
    Glob->rotation_at_radar = Glob->cart_rotation;;

  } else {

    Glob->radarx = cart_to_radar_range *
      sin((cart_to_radar_theta - Glob->cart_rotation) * DEG_TO_RAD);

    Glob->radary = cart_to_radar_range *
      cos((cart_to_radar_theta - Glob->cart_rotation) * DEG_TO_RAD);

    Glob->rotation_at_radar =
      (Glob->cart_rotation + radar_to_cart_theta -
       cart_to_radar_theta - 180.0);

  } /* if (fabs(radar_to_cart_range) < TOLERANCE) */

  if (Glob->rotation_at_radar < -180.0)
    Glob->rotation_at_radar += 360.0;

  printf("radarx = %g\n", Glob->radarx);
  printf("radary = %g\n", Glob->radary);
  printf("rotation_at_radar = %g\n", Glob->rotation_at_radar);

  /*
   * create_slave_table?
   */

  resource_str = uGetParamString(Glob->prog_name,
                                 "create_slave_table", CREATE_SLAVE_TABLE);
  
  if (uset_true_false_param(Glob->prog_name,
                            "setup_scan",
                            Glob->params_path_name,
                            resource_str,
                            &Glob->create_slave_table,
                            "create_slave_table"))
    exit(-1);
  
  ufree(resource_str);

  /*
   * set up mode option
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "mode", "cart");
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->mode,
			"cart", CartMode,
			"ppi", PpiMode,
			"polar", PolarMode,
			"mode"))
    exit(-1);
  
  ufree(resource_str);
  
}
