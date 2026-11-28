 /*************************************************************************
 * GET_SPDB_METARS_F.C: Get Metars from an SPDB database and fill in 
 * arrays that get passed in.
 *	F Hage	  NCAR/RAP
 * Note :set tabstop=4
 */

#include <math.h>
#include <toolsa/port.h>

#include <rapmath/math_macros.h>

#include <symprod/spdb_products.h>
#include <symprod/spdb_client.h>

#include <rapformats/station_reports.h>

#define BAD_DATA_VAL -999.90


void get_spdb_metars
        (char *source_string,
	int *t_start,
	int *t_end,
	int *array_size,
	float *u,
	float *v,
	float *t,
	float *dp,
	float *press,
	float *lat,
	float *lon,
	float *bad_value,
	int *num_returned,
	int *status_return
	)
{
	int i,j;
	long n_chunks;

	double uw,vw;

	spdb_chunk_ref_t *chunk_hdr;
	station_report_t *report;

	*bad_value = BAD_DATA_VAL;

	if(SPDB_get_interval(source_string,
			SPDB_STATION_REPORT_ID,
			0, /* All */
			*t_start,
			*t_end,
			(ui32 *) &n_chunks,
			&chunk_hdr,
			(void *) &report))

	{  /* Error condition */ 
		*num_returned = 0;
		*status_return = 1;
		return;
	}
#ifdef DBG
	printf("Got %d reports\n",n_chunks);
#endif

	/* Process array_size maximum reports */
	if(n_chunks > *array_size) n_chunks = *array_size;

	for(i=0; i < n_chunks; i++) {
		/* Convert report to local byte order */
		station_report_from_be(&report[i]);

		t[i] = (report[i].temp != STATION_NAN) ? report[i].temp: BAD_DATA_VAL;
		dp[i] = (report[i].dew_point != STATION_NAN) ? report[i].dew_point: BAD_DATA_VAL;
		press[i] = (report[i].pres != STATION_NAN) ? report[i].pres: BAD_DATA_VAL;
		lat[i] = report[i].lat;
		lon[i] = report[i].lon;

		if(report[i].winddir != STATION_NAN && 
		   report[i].windspd != STATION_NAN) {
		    uw = -sin(report[i].winddir * RAD_PER_DEG) * report[i].windspd;
		    vw = -cos(report[i].winddir * RAD_PER_DEG) * report[i].windspd;

		    u[i] = uw;
		    v[i] = vw;
		} else {
		    u[i] = BAD_DATA_VAL;
		    v[i] = BAD_DATA_VAL;
		}
	}

	*num_returned = n_chunks;
	*status_return = 0;

	return;
}


