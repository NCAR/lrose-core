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
/*******************************************************************

	Preprocess radar data in srdi format;
        SNR thresholding, cliping, selecting data based on frame
	number, PRF and fields and so on, rescaling and 
        rename field names.

	File: data_processing.c

*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <rdi/r_data.h>
#include <rdi/misc.h>

/* configuration file defining the pre-processing */
#define MAX_N_FIELDS 8

struct conf {
    int processing_mode;
    char conf_name[80];		/* the file name */
    int max_prf, min_prf;
    int max_ele, min_ele;
    unsigned char bad_data;
    int frame_skipped[64], n_frame_skipped;
    int n_fields;
    char *field_names[MAX_N_FIELDS], field_names_b[MAX_N_FIELDS * 8];
    char *new_field_names[MAX_N_FIELDS], new_field_names_b[MAX_N_FIELDS * 8];
    int scale[MAX_N_FIELDS], offset[MAX_N_FIELDS];
    int alt_clip;
    float min_snr;
    int scan_type;
    int gate_size;
};
typedef struct conf Conf;

static int statement_rec (char *bp, char *prog_name);
static int read_configuration (char *prog_name);

static Conf cf;
static int tdwr_tmp = 0;
static int cp2_tmp = 0;

/*******************************************************************
preprocess the ray. return 1 if the ray is discarded .
return -1 if an error is found                                    */

int
process_the_ray (char *ray, char **ray_ret,
		 char *prog_name, char *confname,
		 int msg_mode)

{
    Ray_header *rhd, *nrhd;
    Field_header *fhd, *nfhd;
    int i, j, snr_fi, fi;
    int lscale, loffset, b_data;
    int rleng, ng;
    unsigned char *i_pt, *o_pt, *snr_pt, ssnr;
    double f;
    static unsigned char nray[10000];
    int noff;
    static int init = 0;
    static int g_size_cnt = 0, spe_cnt = 0, ele_cnt = 0, frame_cnt = 0,
      scan_type_cnt = 0, prf_cnt = 0, d_cnt = 0;

    if (msg_mode) {
	if ((d_cnt > 0 && d_cnt % 500 == 0) ||
	    d_cnt == 10 || d_cnt == 100) {
	    printf ("Discarded due to: g_size = %d, "
		    "special = %d, elev = %d\n",
		    g_size_cnt, spe_cnt, ele_cnt);
	    printf ("                  frame_# = %d, "
		    "scan_type = %d, prf = %d - %s\n",
		    frame_cnt, scan_type_cnt, prf_cnt, prog_name);
	}
    }

    if (init == 0) {
	init = 1;
	cf.bad_data = 0;
	cf.processing_mode = 0;
	strcpy (cf.conf_name, confname);
	if (confname[0] != '\0') {
	    if (read_configuration (prog_name) == 0)
		cf.processing_mode = 1;
	}
    }

    *ray_ret = ray;

    if (cf.processing_mode == 0)
	return (0);

    /* check prf and elevation */
    rhd = (Ray_header *) ray;
    if (rhd->prf < cf.min_prf || rhd->prf > cf.max_prf) {
	prf_cnt++;
	d_cnt++;
	return (1);
    }
    if (rhd->ele < cf.min_ele || rhd->ele > cf.max_ele) {
	ele_cnt++;
	d_cnt++;
	return (1);
    }
    if (cf.scan_type >= 0 && cf.scan_type != (rhd->r_id >> 4)) {
	scan_type_cnt++;
	d_cnt++;
	return (1);
    }

    if (cp2_tmp == 1 && rhd->ele < 50) {
	spe_cnt++;
	d_cnt++;
	return (1);
    }

    /* check frame index */
    for (i = 0; i < cf.n_frame_skipped; i++) {
	if (rhd->f_cnt == cf.frame_skipped[i]) {
	    d_cnt++;
	    frame_cnt++;
	    return (1);
	}
    }

    /* the ray length */
    f = (double) rhd->ele * 3.141592653589 / 18000.;
    f = f * (1. - f * f * 0.1666667);
    if (f <= .00001)
	f = .00001;
    rleng = (int) ((double) cf.alt_clip * 1.2 / f);	/* 1.2 for protection 
							 */

/* rhd->polar = 0; */

    /* threshold for bad data */
    if (cf.min_snr > -99.) {
	for (snr_fi = 0; snr_fi < (int) rhd->n_fields; snr_fi++) {	/* SNR 
									   field 
									 */
	    fhd = (Field_header *) (ray + rhd->f_pt[snr_fi]);
	    if (strcmp (fhd->f_name, "SNR") == 0)
		break;		/* snr field */
	}
	if (snr_fi >= (int) rhd->n_fields) {
	    printf ("Field SNR not found from the data. - %s\n", prog_name);
	    return (-1);
	}
	else {
	    j = (my_irint ((double) (100. * cf.min_snr)) - fhd->offset) / fhd->scale;
	    if (j < 0)
		j = 0;
	    if (j > 255)
		j = 255;
	    ssnr = j;
	    snr_pt = (unsigned char *) fhd + sizeof (Field_header);
	}
    }
    else
	ssnr = 0;

    /* used for modified the data to avoid bad data */
/*b_data=1;
   if(cf.bad_data==255) b_data= -1; */
    b_data = 0;			/* we do not try to change the original data
				   that is equal bad_data. This causes
				   problem if the data are repeatedly 
				   processed. e.g. The data has been set by
				   ground removal */

    /* copy the main header */
    memcpy (nray, ray, sizeof (Ray_header));
    nrhd = (Ray_header *) nray;
    noff = sizeof (Ray_header);
    nrhd->n_fields = cf.n_fields;

    /* for each output field */
    for (i = 0; i < cf.n_fields; i++) {
	for (fi = 0; fi < (int) rhd->n_fields; fi++) {
	    fhd = (Field_header *) (ray + rhd->f_pt[fi]);
	    if (strcmp (fhd->f_name, cf.field_names[i]) == 0)
		break;		/* a field */
	}
	if (fi >= (int) rhd->n_fields) {
	    printf ("Field %s not found from the data. - %s\n", cf.field_names[i], prog_name);
	    return (-1);
	}
	nrhd->f_pt[i] = noff;
	nfhd = (Field_header *) & nray[noff];
	memcpy (nfhd, fhd, sizeof (Field_header));
	strcpy (nfhd->f_name, cf.new_field_names[i]);
	i_pt = (unsigned char *) fhd + sizeof (Field_header);
	o_pt = (unsigned char *) nfhd + sizeof (Field_header);

	if (cf.gate_size > 0 && fhd->g_size != cf.gate_size) {
	    g_size_cnt++;
	    d_cnt++;
	    return (1);
	}

	ng = (rleng - (int) fhd->range) / ((int) fhd->g_size >> 4);	/* the
									   clipped 
									   #
									   gates 
									 */
	if (ng > (int) fhd->n_gates)
	    ng = fhd->n_gates;
	if (cf.scale[i] != 0 && (cf.scale[i] != fhd->scale || cf.offset[i] != fhd->offset)) {
	    lscale = (fhd->scale << 12) / cf.scale[i];
	    loffset = ((fhd->offset - cf.offset[i]) << 12) / cf.scale[i];
	}
	else
	    lscale = 0;

	/* we dont process snr field */
	if (snr_fi == fi) {
	    for (j = 0; j < ng; j++)
		o_pt[j] = i_pt[j];
	}
	else {
	    if (ssnr == 0) {
		if (lscale == 0) {
		    for (j = 0; j < ng; j++) {
			o_pt[j] = i_pt[j];
			if (o_pt[j] == cf.bad_data)
			    o_pt[j] += b_data;
		    }
		}
		else {
		    for (j = 0; j < ng; j++) {
			o_pt[j] = ((int) i_pt[j] * lscale + loffset) >> 12;
			if (o_pt[j] == cf.bad_data)
			    o_pt[j] += b_data;
		    }
		}
	    }
	    else {
		if (lscale == 0) {
		    if (tdwr_tmp == 0) {	/* regular SNR */
			for (j = 0; j < ng; j++) {
			    o_pt[j] = i_pt[j];
			    if (o_pt[j] == cf.bad_data)
				o_pt[j] += b_data;
			    if (snr_pt[j] < ssnr)
				o_pt[j] = cf.bad_data;
			}
		    }
		    else {	/* TDWR special SNR - 0 is clutter */
			for (j = 0; j < ng; j++) {
			    o_pt[j] = i_pt[j];
			    if (o_pt[j] == cf.bad_data)
				o_pt[j] += b_data;
			    if (snr_pt[j] < ssnr && snr_pt[j] != 0)
				o_pt[j] = cf.bad_data;
			}
		    }
		}
		else {
		    for (j = 0; j < ng; j++) {
			o_pt[j] = ((int) i_pt[j] * lscale + loffset) >> 12;
			if (o_pt[j] == cf.bad_data)
			    o_pt[j] += b_data;
			if (snr_pt[j] < ssnr)
			    o_pt[j] = cf.bad_data;
		    }
		}
	    }
	}

	nfhd->n_gates = ng;
	if (lscale != 0) {
	    nfhd->scale = cf.scale[i];
	    nfhd->offset = cf.offset[i];
	}

	noff = noff + sizeof (Field_header) + ng;
	if ((noff % 4) != 0)
	    noff = noff + 4 - (noff % 4);
    }

    if (rhd->r_h_pt != 0) {
	memcpy (&nray[noff], ray + rhd->r_h_pt, sizeof (Radar_header));
	nrhd->r_h_pt = noff;
	noff = noff + sizeof (Radar_header);
    }

    nrhd->bad_data = cf.bad_data;
    nrhd->length = noff;

    *ray_ret = (char *) nray;

    return (0);

}

/**********************************************************************
reads the configuration                                              */

#define N_CONF_ITEMS 10

static int
read_configuration (char *prog_name)

{
    char buf[81], *pbf, tm[80], tmp[80];
    int i, cnt, line, k, it, ct;
    FILE *fl;
    float f1, f2;

    /* open the file */
    if ((fl = fopen (cf.conf_name, "r")) == NULL) {
	printf ("Can not find the configuration file %s. - %s\n", cf.conf_name, prog_name);
	return (1);
    }
    cf.max_ele = 9000;
    cf.min_ele = 0;
    cf.bad_data = 0;
    cf.min_snr = -100.;
    cf.min_prf = 0;
    cf.max_prf = 100000;
    cf.n_frame_skipped = 0;
    cf.alt_clip = 100000;
    cf.scan_type = -1;
    cf.gate_size = 0;

    cnt = 0;
    line = 0;
    cf.n_fields = 10000;	/* to guarantee all field names to be read */
    while (1) {
	if (fgets (buf, 80, fl) == NULL)
	    break;
	pbf = buf;
	line++;
	if ((i = statement_rec (pbf, prog_name)) == -1)
	    continue;
	if (i == -2)
	    return (1);
	switch (i) {
	case 0:		/* Number of fields */
	    if (sscanf (pbf, "%d", &cf.n_fields) != 1)
		goto err1;
	    if (cf.n_fields < 0 || cf.n_fields > MAX_N_FIELDS)
		goto err2;
	    break;

	case 1:		/* Fields */
	    if (cf.n_fields == 10000) {
		printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
		printf ("You must first specify the number of fields - %s\n", prog_name);
		return (1);
	    }
	    for (k = 0; k < cf.n_fields; k++) {
		int j;
		char bbu[128];

		if (fgets (bbu, 128, fl) == NULL ||
		 (j = sscanf (bbu, "%s %f %f %s", tm, &f1, &f2, tmp)) < 3) {
		    printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
		    printf ("%d items are expected after statement Fields: (%d %d)- %s\n"
			    ,cf.n_fields, k, j, prog_name);
		    return (1);
		}
		if (strlen (tm) > 7)
		    goto err2;
		cf.field_names[k] = &cf.field_names_b[k * 8];
		cf.new_field_names[k] = &cf.new_field_names_b[k * 8];
		strcpy (cf.field_names[k], tm);
		if (j > 3) {
		    tmp[7] = '\0';
		    strcpy (cf.new_field_names[k], tmp);
		}
		else
		    strcpy (cf.new_field_names[k], tm);
		cf.scale[k] = my_irint ((double) (f1 * 100.));
		cf.offset[k] = my_irint ((double) (f2 * 100.));
		if (strcmp (tm, "Vsh") == 0 && strcmp (tmp, "VEL") == 0) {
		    printf ("CP2 special - remove ele<.5\n");
		    cp2_tmp = 1;
		}
	    }
	    break;

	case 2:		/* Bad data */
	    if (sscanf (pbf, "%d", &it) != 1)
		goto err1;
	    if (it < 0 || it > 255)
		goto err2;
	    cf.bad_data = it;
	    break;

	case 3:		/* Minimum SNR */
	    if ((k = sscanf (pbf, "%f %s", &f1, tm)) < 1)
		goto err1;
	    cf.min_snr = f1;
	    if (strcmp (tm, "tdwr") == 0) {
		printf ("Using tdwr temp SNR\n");
		tdwr_tmp = 1;
	    }
	    break;

	case 4:		/* PRF range */
	    if (sscanf (pbf, "%f %f", &f1, &f2) != 2)
		goto err1;
	    if (f1 >= f2)
		goto err2;
	    cf.min_prf = my_irint ((double) f1);
	    cf.max_prf = my_irint ((double) f2);
	    break;

	case 5:		/* Frame skipped */
	    ct = 0;
	    for (k = 0; k < 64; k++) {
		if (fscanf (fl, "%d", &it) != 1)
		    break;
		cf.frame_skipped[ct++] = it;
	    }
	    cf.n_frame_skipped = ct;
	    break;

	case 6:		/* Maxinum altitude */
	    if (sscanf (pbf, "%f", &f1) != 1)
		goto err1;
	    cf.alt_clip = my_irint ((double) f1);
	    break;

	case 7:		/* Elevation range */
	    if (sscanf (pbf, "%f %f", &f1, &f2) != 2)
		goto err1;
	    if (f1 >= f2)
		goto err2;
	    cf.min_ele = my_irint ((double) f1 * 100.);
	    cf.max_ele = my_irint ((double) f2 * 100.);
	    break;

	case 8:		/* Scan type */
	    if (sscanf (pbf, "%d", &it) != 1)
		goto err1;
	    cf.scan_type = it;
	    break;

	case 9:		/* Gate size */
	    if (sscanf (pbf, "%f", &f1) != 1)
		goto err1;
	    cf.gate_size = my_irint ((double) f1 * 16.);
	    break;

	default:
	    break;
	}
	cnt++;
    }

/*
   if(cnt!=N_CONF_ITEMS) {
   printf("Error reading %s:  - %s\n",cf.conf_name,prog_name);
   printf("Missing %d items in the file (%d items found) - %s\n",
   N_CONF_ITEMS-cnt,cnt,prog_name);
   return (1);
   } */

    if (cf.n_fields == 10000) {
	printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
	printf ("Number of field must be specified - %s\n", prog_name);
	return (1);
    }

    printf ("%d items read from the file %s - %s\n",
	    cnt, cf.conf_name, prog_name);

    fclose (fl);

    return (0);

  err1:
    printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
    printf ("An error found in line %d: %s - %s\n", line, buf, prog_name);
    return (1);
  err2:
    printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
    printf ("Data out of range at line %d: %s - %s\n", line, buf, prog_name);
    return (1);

}

/*************************************************************************
function identifying the statements                                     */

static int
statement_rec (char *bp, char *prog_name)

{
    int i, length;
    char *cp;
    static char st[N_CONF_ITEMS][32] =
    {
	"Number of fields:",
	"Fields:",
	"Bad data:",
	"Minimum SNR:",
	"PRF range:",
	"Frame skipped:",
	"Maximum altitude:",
	"Elevation range:",
	"Scan type:",
	"Gate size:"
    };

    cp = bp;
    /* remove leading spaces */
    while (*bp == ' ' || *bp == '\t')
	bp++;

    /* a comment */
    if (*bp == '#' || *bp == '\0' || *bp == '\n')
	return (-1);

    for (i = 0; i < N_CONF_ITEMS; i++) {
	length = strlen (st[i]);
	if (strncmp (st[i], bp, length) == 0 ||
	    (i == 6 && strncmp ("Maxinum altitude:", bp, 17) == 0)) {
	    bp = bp + length;
	    strcpy (cp, bp);
	    st[i][0] = '^';	/* remove the item */
	    return (i);
	}
    }
    printf ("Error reading %s:  - %s\n", cf.conf_name, prog_name);
    printf ("Bad or duplicated statement encountered: %s - %s\n",
	    bp, prog_name);
    return (-2);

}				/* end of the function */


