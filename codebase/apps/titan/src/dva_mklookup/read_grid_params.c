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
/***************************************************************************
 * read_grid_params.c
 *
 * Reads grid params from ASCII file produced by dva_mkgrid
 *
 * Mike Dixon
 * Marion Mittermaier
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Aug 1998
 *
 ****************************************************************************/

#include "dva_mklookup.h"

int read_grid_params(dva_grid_t *grid)

{

  FILE *gparams;
  char line[1024];

  if ((gparams = fopen(Glob->params.grid_params_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open grid params output file\n");
    perror(Glob->params.grid_params_file_path);
    return (-1);
  }

  if (fgets(line, 1024, gparams) == NULL) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }

  if (sscanf(line, "%d%d%d", &grid->nx, &grid->ny, &grid->nz) != 3) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }

  if (fgets(line, 1024, gparams) == NULL) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }
  
  if (sscanf(line, "%d%d%lg", &grid->minx, &grid->miny, &grid->minz) != 3) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }

  if (fgets(line, 1024, gparams) == NULL) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }

  if (sscanf(line, "%d%d%lg", &grid->dx, &grid->dy, &grid->dz) != 3) {
    fprintf(stderr, "Cannot read grid params file\n");
    perror (Glob->params.grid_params_file_path);
    fclose(gparams);
    return (-1);
  }

  fclose(gparams);

  return (0);

}
