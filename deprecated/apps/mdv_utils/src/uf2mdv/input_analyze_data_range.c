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
#include <ctetwws/smem.h>
#include <input.h>
#include <input_data.h>
#include <input_analyze_data_range.h>
#include <uf_data_format.h>
#include <uf_util.h>

typedef struct
{
  char *name;
  int npt;
  double min_value;
  double max_value;
} data_range_t;

static int nfields = 0;
static data_range_t *fields = NULL;

/*----------------------------------------------------------------*/
static void compute_scale_bias(double min, double max, double *scale,
			       double *bias, double *scale2, 
			       double *bias2)
{
    double v;

    *bias = min;
    *scale = (max - min)/255.0;

    if (min < 0.0 && max > 0.0)
    {
        v = -min;
	if (v < max)
	    v = max;
	*bias2 = -v;
	*scale2 = 2.0*v/255.0;
    }
    else
    {
        *bias2 = *bias;
	*scale2 = *scale;
    }
}

/*----------------------------------------------------------------*/
static void add_to_field(data_range_t *d, double value)
{
    if (d->npt <= 0)
    {
	d->npt = 1;
	d->min_value = d->max_value = value;
    }
    else
    {
	++d->npt;
	if (value < d->min_value)
	    d->min_value = value;
	if (value > d->max_value)
	    d->max_value = value;
    }
}

/*----------------------------------------------------------------*/
static data_range_t *add_field(char *name)
{
    data_range_t *f;

    if (nfields == 0)
        fields = MEM_ALLOC(data_range_t);
    else
        fields = MEM_GROW(fields, nfields+1, data_range_t);
    f = &fields[nfields++];
    f->name = MEM_STRDUP(name);
    f->npt = 0;
    f->min_value = 0.0;
    f->max_value = 0.0;
    return f;
}

/*----------------------------------------------------------------*/
static data_range_t *find_field(char *name)
{
    int i;

    for (i=0; i<nfields; ++i)
        if (strcmp(fields[i].name, name) == 0)
	   return &fields[i];
    return NULL;
}

/*----------------------------------------------------------------*/
static void add_to_fields(char *uf,
			  Mandatory_header_block *m,
			  Field_header *f, char *name)
{
    data_range_t *d;
    signed short *spt;        /* pointer to short */
    int i; /*, j, j0, j1;*/
    double value;

    d = find_field(name);
    if (d == NULL)
        d = add_field(name);

    spt = (signed short *)(POSITION(uf, f->position));
    for (i=0; i<f->num_samples; ++i, ++spt)
    {
	if (*spt == m->missing)
	    continue;
	value = (double)(*spt)/(double)(f->scale_factor);
	add_to_field(d, value);
    }
    return;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
void input_analyze_data_range_init(void)
{
    nfields = 0;
    fields = NULL;
}

/*----------------------------------------------------------------*/
void input_analyze_data_range_load_next(input_data_t *uf)
{
    Data_header *d;
    char *c, field_name[3];
    int i; /*, rvalue, index; */
    signed short *spt;
    Field_header *f;
    Mandatory_header_block *m;

    m = (Mandatory_header_block *)uf->data;
    d = (Data_header *)(POSITION(uf->data, m->data_header));
    c = (char *)d + sizeof(Data_header);
    for (i=0; i<d->num_flds_rec; ++i, c+=4)
    {
	memcpy(field_name, c, 2);
	field_name[2] = 0;
	spt = (signed short *)(c + 2);
	f = (Field_header *)(POSITION(uf->data, *spt));
	add_to_fields(uf->data, m, f, field_name);
    }
}

/*----------------------------------------------------------------*/
void input_analyze_data_range_summarize(void)
{
    int i;
    double scale, bias, scale2, bias2;

    for (i=0; i<nfields; ++i)
    {
         if (fields[i].npt > 0)
	     compute_scale_bias(fields[i].min_value,
				fields[i].max_value,
				&scale, &bias,
				&scale2, &bias2);
	 else
	     scale = bias = 0.0;

         printf("field:\t'%s' npt:%d    min:%f   max:%f \n"
		"\ttight     scale:%f   bias:%f\n"
		"\tsymmetric scale:%f   bias:%f\n",
		fields[i].name, fields[i].npt, fields[i].min_value, 
		fields[i].max_value, scale, bias, scale2, bias2);
	 MEM_FREE(fields[i].name);
    }
    if (nfields > 0)
        MEM_FREE(fields);

    input_analyze_data_range_init();
}

