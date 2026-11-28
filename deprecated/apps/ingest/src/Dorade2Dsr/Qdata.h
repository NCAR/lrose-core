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
/* 	$Id: Qdata.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

# ifndef INCQdatah
# define INCQdatah

struct qparamdata_d {		/* new for 1995 */

    char pdata_desc[4];		/* parameter data descriptor identifier: ASCII
				 * characters "QDAT" for a block that contains
				 * the data plus some supplemental and
				 * identifying information */
    
    si32  pdata_length;		/* parameter data descriptor length in bytes.
				 * this represents the size of this header
				 * information plus the data
				 *
				 * for this data block the start of the data
				 * is determined by using "offset_to_data"
				 * in the corresponding parameter descriptor
				 * "struct parameter_d"
				 * the offset is from the beginning of
				 * this descriptor/block
				 */
    
    char  pdata_name[8];	/* name of parameter */

    si32  extension_num;
    si32  config_num;		/* facilitates indexing into an array
				 * of radar descriptors where the radar
				 * characteristics of each ray and each
				 * parameter might be unique such as phased
				 * array antennas
				 */
    si16 first_cell[4];
    si16 num_cells[4];
				/* first cell and num cells demark
				 * some feature in the data and it's
				 * relation to the cell vector
				 * first_cell[n] = 0 implies the first datum
				 * present corresponds to "dist_cells[0]
				 * in "struct cell_d"
				 * for TRMM data this would be the
				 * nominal sample where the cell vector is
				 * at 125 meter resolution instead of 250 meters
				 * and identified segments might be the
				 * rain echo oversample "RAIN_ECH" and the
				 * surface oversample "SURFACE"
				 */
    fl32 criteria_value[4];	/* criteria value associated
				 * with a criteria name
				 * in "struct parameter_d"
				 */
}; /* End of Structure */



typedef struct qparamdata_d qparamdata_d;
typedef struct qparamdata_d QPARAMDATA;

# endif /* INCQdatah */






