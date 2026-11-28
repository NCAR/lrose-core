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
#include <input.h>
#include <uf_data.h>
#include <uf_data_format.h>
#include <ctetwws/smem.h>

/*----------------------------------------------------------------*/
static void mandatory_header_byteswap(Mandatory_header_block *m)
{
    byte_swap((unsigned char *)&(m->Rec_len), sizeof(m->Rec_len));
    byte_swap((unsigned char *)&(m->non_mandatory), sizeof(m->non_mandatory));
    byte_swap((unsigned char *)&(m->local_use), sizeof(m->local_use));
    byte_swap((unsigned char *)&(m->data_header), sizeof(m->data_header));
    byte_swap((unsigned char *)&(m->physical_rec), sizeof(m->physical_rec));
    byte_swap((unsigned char *)&(m->volume_scan), sizeof(m->volume_scan));
    byte_swap((unsigned char *)&(m->ray_number), sizeof(m->ray_number));
    byte_swap((unsigned char *)&(m->ray_record), sizeof(m->ray_record));
    byte_swap((unsigned char *)&(m->sweep_number), sizeof(m->sweep_number));
    byte_swap((unsigned char *)&(m->degrees_lat), sizeof(m->degrees_lat));
    byte_swap((unsigned char *)&(m->minutes_lat), sizeof(m->minutes_lat));
    byte_swap((unsigned char *)&(m->seconds_lat), sizeof(m->seconds_lat));
    byte_swap((unsigned char *)&(m->degrees_lon), sizeof(m->degrees_lon));
    byte_swap((unsigned char *)&(m->minutes_lon), sizeof(m->minutes_lon));
    byte_swap((unsigned char *)&(m->seconds_lon), sizeof(m->seconds_lon));
    byte_swap((unsigned char *)&(m->height_antenna), sizeof(m->height_antenna));
    byte_swap((unsigned char *)&(m->year), sizeof(m->year));
    byte_swap((unsigned char *)&(m->month), sizeof(m->month));
    byte_swap((unsigned char *)&(m->day), sizeof(m->day));
    byte_swap((unsigned char *)&(m->hour), sizeof(m->hour));
    byte_swap((unsigned char *)&(m->minute), sizeof(m->minute));
    byte_swap((unsigned char *)&(m->second), sizeof(m->second));
    byte_swap((unsigned char *)&(m->azimuth), sizeof(m->azimuth));
    byte_swap((unsigned char *)&(m->elevation), sizeof(m->elevation));
    byte_swap((unsigned char *)&(m->sweep_mode), sizeof(m->sweep_mode));
    byte_swap((unsigned char *)&(m->fixed_angle), sizeof(m->fixed_angle));
    byte_swap((unsigned char *)&(m->sweep_rate), sizeof(m->sweep_rate));
    byte_swap((unsigned char *)&(m->gen_year), sizeof(m->gen_year));
    byte_swap((unsigned char *)&(m->gen_month), sizeof(m->gen_month));
    byte_swap((unsigned char *)&(m->gen_day), sizeof(m->gen_day));
    byte_swap((unsigned char *)&(m->missing), sizeof(m->missing));
}

/*----------------------------------------------------------------*/
static void data_header_byteswap(Data_header *d)
{
    byte_swap((unsigned char *)&(d->num_flds_ray), sizeof(d->num_flds_ray));
    byte_swap((unsigned char *)&(d->num_rec_ray), sizeof(d->num_rec_ray));
    byte_swap((unsigned char *)&(d->num_flds_rec), sizeof(d->num_flds_rec));
}

/*----------------------------------------------------------------*/
static void field_header_byteswap(Field_header *f, int n_not_swap)
{
    if (n_not_swap <= 0)
        byte_swap((unsigned char *)&(f->position), sizeof(f->position));
    if (n_not_swap <= 1)
        byte_swap((unsigned char *)&(f->scale_factor),
		  sizeof(f->scale_factor));
    if (n_not_swap <= 2)
        byte_swap((unsigned char *)&(f->rnge_frst_gate),
		  sizeof(f->rnge_frst_gate));
    if (n_not_swap <= 3)
        byte_swap((unsigned char *)&(f->adjustment), sizeof(f->adjustment));
    if (n_not_swap <= 4)
        byte_swap((unsigned char *)&(f->vol_space), sizeof(f->vol_space));
    if (n_not_swap <= 5)
        byte_swap((unsigned char *)&(f->num_samples), sizeof(f->num_samples));
    if (n_not_swap <= 6)
        byte_swap((unsigned char *)&(f->vol_depth), sizeof(f->vol_depth));
    if (n_not_swap <= 7)
        byte_swap((unsigned char *)&(f->hor_bm_wdth), sizeof(f->hor_bm_wdth));
    if (n_not_swap <= 8)
        byte_swap((unsigned char *)&(f->ver_bm_wdth), sizeof(f->ver_bm_wdth));
    if (n_not_swap <= 9)
        byte_swap((unsigned char *)&(f->receiver_wdth),
		  sizeof(f->receiver_wdth));
    if (n_not_swap <= 10)
        byte_swap((unsigned char *)&(f->polarization),
		  sizeof(f->polarization));
    if (n_not_swap > 10)
    {
        printf("Not swapping more than 10 words is very suspicious\n");
        printf("Aborting\n");
	exit(0);
    }
    byte_swap((unsigned char *)&(f->wavelength), sizeof(f->wavelength));
    byte_swap((unsigned char *)&(f->num_sam_fld), sizeof(f->num_sam_fld));
    byte_swap((unsigned char *)&(f->thresh_fld), sizeof(f->thresh_fld));
    byte_swap((unsigned char *)&(f->thresh_val), sizeof(f->thresh_val));
    byte_swap((unsigned char *)&(f->scale), sizeof(f->scale));
    byte_swap((unsigned char *)&(f->edit_code), sizeof(f->edit_code));
    byte_swap((unsigned char *)&(f->pulse_rep), sizeof(f->pulse_rep));
    byte_swap((unsigned char *)&(f->bits_volume), sizeof(f->bits_volume));
    byte_swap((unsigned char *)&(f->Nyquist), sizeof(f->Nyquist));
    byte_swap((unsigned char *)&(f->FL), sizeof(f->FL));
    byte_swap((unsigned char *)&(f->Radar_const), sizeof(f->Radar_const));
    byte_swap((unsigned char *)&(f->noise_pwr), sizeof(f->noise_pwr));
    byte_swap((unsigned char *)&(f->receiver_gn), sizeof(f->receiver_gn));
    byte_swap((unsigned char *)&(f->peak_pwr), sizeof(f->peak_pwr));
    byte_swap((unsigned char *)&(f->ant_gain), sizeof(f->ant_gain));
    byte_swap((unsigned char *)&(f->pulse_dur), sizeof(f->pulse_dur));
}

/*----------------------------------------------------------------*/
static void byteswap_all_data(char *uf,
			      Mandatory_header_block *m,
			      Field_header *f)
{
    signed short *spt;        /* pointer to short */
    int i;

    spt = (signed short *)(POSITION(uf, f->position));
    for (i=0; i<f->num_samples; ++i, ++spt)
	byte_swap((unsigned char *)spt, sizeof(signed short));
    return;
}

/*----------------------------------------------------------------*/
void uf_byteswap(char *data, int nbytes)
{
#ifdef __linux
    Mandatory_header_block *m;
    Data_header *d;
    Field_header *f;
    int i, n_not_swap, *sptlist;
    char *c;
    signed short *spt;

    if (!params.byte_swap)
        return;
    m = (Mandatory_header_block *)data;
    mandatory_header_byteswap(m);
    d = (Data_header *)(POSITION(data, m->data_header));
    data_header_byteswap(d);
    c = (char *)d + sizeof(Data_header);
    if (d->num_flds_rec > 0)
        sptlist = MEM_CALLOC(d->num_flds_rec, int);
    else
        sptlist = NULL;
    for (i=0; i<d->num_flds_rec; ++i, c +=4)
    {
	spt = (signed short *)(c+2);  /* skip over field name */
	byte_swap((unsigned char *)spt, sizeof(signed short));
	sptlist[i] = *spt;
	if (i > 0)
	{
	    int diff;
	    diff = sptlist[i] - sptlist[i-1];
	    if (diff < sizeof(Field_header)/2)
	    {
		n_not_swap = sizeof(Field_header)/2 - diff;
	        /*printf("field header byteswap problem..try not swapping %d\n",
		       n_not_swap);*/
	    }
	    else
	        n_not_swap = 0;
	}
	else
	    n_not_swap = 0;
	f = (Field_header *)(POSITION(data, sptlist[i]));
	field_header_byteswap(f, n_not_swap);
	byteswap_all_data(data, m, f);
    }
    MEM_FREE(sptlist);
#else
    return;
#endif
}

/*----------------------------------------------------------------*/
void byte_swap(unsigned char *buf, int len)
{
#ifdef __linux
    unsigned char *temp;
    int i, j;

    if (!params.byte_swap)
        return;

    i = 0;
    j = len-1;
    temp = MEM_CALLOC(len, unsigned char);
    while (i<=j)
    {
	temp[i] = buf[j];
	if (i != j)
	    temp[j] = buf[i];
	++i;
	--j;
    }
    for (i=0; i<len; ++i)
	buf[i] = temp[i];
    MEM_FREE(temp);
#else
    return;
#endif
}

/*----------------------------------------------------------------*/
int byteswap_needed(void)
{
#ifdef __linux
  return 1;
#else
  return 0;
#endif
}

  
