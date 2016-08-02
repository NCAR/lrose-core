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
 * read_rdas_cal.c
 *
 * Reads rdas cal data from ASCII file produced by dva_ingest
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

int read_rdas_cal(dva_rdas_cal_t *cal)

{

  FILE *calfile;
  char line[1024];

  if ((calfile = fopen(Glob->params.rdas_cal_data_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open cal data file\n");
    perror(Glob->params.rdas_cal_data_path);
    return (-1);
  }

  if (fgets(line, 1024, calfile) == NULL) {
    fprintf(stderr, "Cannot read cal data file\n");
    perror (Glob->params.rdas_cal_data_path);
    fclose(calfile);
    return (-1);
  }

  if (sscanf(line, "%lg%lg%lg%lg%d%lg%lg%d",
	     &cal->viphi, &cal->dbzhi, &cal->viplo, &cal->dbzlo,
	     &cal->mus, &cal->gate_spacing,
	     &cal->start_range, &cal->ngates) != 8) {
    fprintf(stderr, "Cannot decode cal file\n");
    perror (Glob->params.rdas_cal_data_path);
    fclose(calfile);
    return (-1);
  }

  fclose(calfile);

  return (0);

}
