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
/************************************************************************/

/*
 * nex.c
 *
 *      These routines convert the format of radar beams from tdwr to LL.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1995
 *
 * Gary Blackburn
 *
 */

/************************************************************************/

# include   "tdwr2moments.h"
# include <dataport/bigend.h>
# include   <memory.h>
# include   <time.h>
/*# include	"defs.h"*/

/************************************************************************/

# define	FIRST_TIME 999

/************************************************************************/

extern      Global  Glob;

/************************************************************************/

 
 /*                      
  * file scope prototypes
  */
	
static void init_headers (LL_beam_rec *enet_pak);

static void reset_tilt_spec_hdr (LL_beam_rec *enet_pak,
									  TDWR_data_header *radar_beam); 

static void reset_beam_spec_hdr (LL_beam_rec *enet_pak,
											TDWR_data_header *radar_beam,
											short radial_num);
static int store_data (TDWR_data_header *input_data, 
											unsigned char *output_data, 
											unsigned short low_prf);

static void print_data (Beam_hdr *header);

/************************************************************************/


void tdwr_process_logical (TDWR_data_header *data_hdr, LL_beam_rec *enet_pak)

{

static int	initial_proc_call = TRUE;
static int	beam_count = 0;
static int	status = 0;
/*static int	last_elev_num = 0;*/
int		elev_num = 0;
int		data_len = 0;
int		hour = 0;
int		min = 0;
int		seconds = 0;
int		low_prf = FALSE;
struct tm   *gmt_time;


	if (data_hdr->message_id == LLWASII_DATA ||
			data_hdr->message_id == LLWASIII_DATA ||
			data_hdr->message_id == LLWASII_MAPPING)
		return;

/*	Determine if this is the start of a new 360 degree tilt */
	if ((data_hdr->scan_info_flag & START_OF_NEW_ELEVATION) ||
	    (data_hdr->volume_flag & BEGINNING_OF_VOL_SCAN)
		|| initial_proc_call) 
	{
		if (0)
			(void) printf ("\n, elev number = %d\n", elev_num);

		/*	A new sweep will begin to be processed; if a previous packet
 	 	 *	has been defined mark this beam as the last beam in the sweep
 		 *	and send the packets to the network
 	 	 */
		if (!initial_proc_call)
		{
			enet_pak->header.end_tilt_flag = 1;

			data_len = enet_pak->header.gates_per_beam * NUM_LL_PARAM 
								+ sizeof (Beam_hdr);
			if (Glob.debug)
				print_data (&enet_pak->header);
			else 
			{	
				write_stream ((unsigned char *) enet_pak, data_len);
			}

			enet_pak->header.end_tilt_flag = 0;
		}
		else
		{
			init_headers (enet_pak);
			initial_proc_call = FALSE;
		}

		/*	Begin processing a new sweep first by initializing
		 *	aspects of the header specific to all beams common
		 *	to this sweep
		 */

		(void) printf ("%d beams - \n", beam_count);
			
		fflush (stdout);

		elev_num = data_hdr->scan_info_flag >> 24;

		if ((data_hdr->volume_flag & BEGINNING_OF_VOL_SCAN) 
								== BEGINNING_OF_VOL_SCAN && elev_num == 1)
		{
			gmt_time = gmtime ((time_t *) &data_hdr->timestamp);

			seconds = gmt_time->tm_sec;
			hour = gmt_time->tm_hour;
			min = gmt_time->tm_min;

			(void) printf ("\n*********************************************\n");
			(void) printf ("New Volume %2d :%2d :%2d", hour, min, seconds);		
			(void) printf (" - TDWR radar\n\n");
				
		}

		reset_tilt_spec_hdr (enet_pak, data_hdr);

		if (!Glob.debug)
		(void) printf ("          - fixed ang %5.2f - prf %d gates %d gate size %d\r",
			       (float) (enet_pak->header.target_elev) 
				/ 100., enet_pak->header.prf, enet_pak->header.gates_per_beam, 
														enet_pak->header.gate_spacing);

		fflush (stdout);

		beam_count = 0;

	}
	else 
	{
	/*  Send the moments formatted beam created by the previous   
	 *	iteration of this procedure so new beam processing can proceed
	 */
		data_len = enet_pak->header.gates_per_beam * NUM_LL_PARAM 
								+ sizeof (Beam_hdr);
		if (Glob.debug)
			print_data (&enet_pak->header);
		else 
		{	
			write_stream ((unsigned char *) enet_pak, data_len);
		}
	}

	/*...	get new beam specif info */
	reset_beam_spec_hdr (enet_pak, data_hdr, beam_count);
	/*last_elev_num = elev_num;*/

	if (data_hdr->message_id == LOW_PRF_BASE_DATA)
		low_prf = TRUE;

	/*...   move the radar data into the MOM buffer */
	status = store_data (data_hdr, enet_pak->data, low_prf);

	beam_count++;
}

/************************************************************************/

void reset_tilt_spec_hdr (LL_beam_rec *enet_pak, TDWR_data_header *radar_beam)

/* update a MOM buffer with the MHR tilt specific data */

{
	(void) strcpy (enet_pak->header.radar_name, Glob.radar_name);

	(void) strcpy (enet_pak->header.site_name, Glob.site_name);

	enet_pak->header.latitude = Glob.latitude; 		/* deg X 100000 */	

	enet_pak->header.longitude = Glob.longitude;		/* deg X 100000 */

	enet_pak->header.altitude  = Glob.altitude;

	/* modification to LL format - notion of volume control pattern */
	enet_pak->header.vcp = radar_beam->volume_flag & SCAN_STRATEGY;


	/* moments format assigns 1 to PPI (surveillance), 2 for RHI, and 3 for
	 * sector */
	if ((radar_beam->scan_info_flag & SECTOR_SCAN) == SECTOR_SCAN)
		enet_pak->header.scan_mode = 3;
	else 
	{
		if (enet_pak->header.vcp == PPI || 
						enet_pak->header.vcp == MONITOR ||
						enet_pak->header.vcp == CLEAR_AIR ||
						enet_pak->header.vcp == CLUTTER_COL ||
						enet_pak->header.vcp == CLUTTER_EDIT ||
						enet_pak->header.vcp == HAZARDOUS) 
			enet_pak->header.scan_mode = 1;
		else
		{			
			if (enet_pak->header.vcp == RHI)
				enet_pak->header.scan_mode = 2;
			else
			{
				(void) printf ("error in scan mode - received %d\n", 
									radar_beam->scan_info_flag & SCAN_STRATEGY);
				/* to keep the software from crashing I am adding this hack!*/
				/* we frequently receive a 0 */
				enet_pak->header.scan_mode = 2;

			}
		}
	}

	enet_pak->header.tilt_num = radar_beam->scan_info_flag >> 24;

	enet_pak->header.vol_count = radar_beam->volume_count;

	enet_pak->header.target_elev = 
						(short ) (radar_beam->current_elevation * 100);


	/* ? leaving the same as MHR */
	enet_pak->header.num_spacing_segs = 1; 

	/* the pri is a micro-sec interval - this has to be converted into
	 * frequency - cycles / sec
	 */
	enet_pak->header.prf = (short ) ((float ) 1000000. / radar_beam->pri + .5);

	enet_pak->header.scale[0] = SNR_SCALE;
	enet_pak->header.bias[0] = SNR_BIAS;

	enet_pak->header.scale[1] = DBZ_SCALE;
	enet_pak->header.bias[1] = DBZ_BIAS;

	enet_pak->header.bias[2] = VEL_BIAS;
	enet_pak->header.scale[2] = VEL_SCALE;
		
	enet_pak->header.scale[3] = SW_SCALE;
	enet_pak->header.bias[3] = SW_BIAS;

	enet_pak->header.power_trans = radar_beam->power_trans;


	if (radar_beam->scan_info_flag & LOW_PRF)
	{
		enet_pak->header.gate_spacing  = LOW_PRF_RESOLUTION;
		/*enet_pak->header.gates_per_beam  = LOW_PRF_GATES;*/
		enet_pak->header.gates_per_beam = radar_beam->rng_samples_per_dwell;
	
		if (enet_pak->header.gates_per_beam > MAX_RDI_GATES)
			enet_pak->header.gates_per_beam = MAX_RDI_GATES;

	}
	else
	{
		enet_pak->header.gate_spacing  = NORMAL_PRF_RESOLUTION;
		enet_pak->header.gates_per_beam = radar_beam->rng_samples_per_dwell;
		
		if (enet_pak->header.gates_per_beam > MAX_RDI_GATES)
			enet_pak->header.gates_per_beam = MAX_RDI_GATES;
	}


	/* values of 1 = clockwise, -1 counter-clockwise, and 0 standing still
	 */
	if (radar_beam->angular_scan_rate > 0)
		enet_pak->header.scan_dir = 1;
	else 
		if (radar_beam->angular_scan_rate < 0)
			enet_pak->header.scan_dir = -1;
		else 
			enet_pak->header.scan_dir = 0;


	enet_pak->header.polarization = Glob.polarization;

}

/*----------------------------------------------------------------------------*/

void reset_beam_spec_hdr (LL_beam_rec *enet_pak, TDWR_data_header *radar_beam, 
								short radial_num)

/* update a Lincoln Lab buffer with the MHR radial specific data */

{
	struct tm	*gmt_time;
	int	true_north_value = 0;

	enet_pak->header.rad_seq_num = radial_num;

	gmt_time = gmtime ((time_t *) &radar_beam->timestamp);

	enet_pak->header.month = (short) gmt_time->tm_mon + 1;
	enet_pak->header.day = (short) gmt_time->tm_mday;
	enet_pak->header.year = (short) gmt_time->tm_year;

	enet_pak->header.hour = (short) gmt_time->tm_hour;
	enet_pak->header.min = (short) gmt_time->tm_min;
	enet_pak->header.sec = (short) gmt_time->tm_sec;

	/* the TDWR radar does not have a record entry for actual elev
	 * using the target elevation value
	 */
	enet_pak->header.elevation =
								(short ) (radar_beam->current_elevation * 100);


	if (Glob.true_north)
	{
		true_north_value = 
					((int ) (radar_beam->azimuth * 100)) + TRUE_NORTH_FACTOR;

		enet_pak->header.azimuth = (short ) (true_north_value % 36000);
	}
	else
		enet_pak->header.azimuth =
								(short ) (radar_beam->azimuth * 100);
}

/*----------------------------------------------------------------------------*/

void init_headers (LL_beam_rec *enet_pak)
/* set the static sections of the Lincoln headers */

{

char    *var1 = "SN      ";
char    *var2 = "DZ      ";
char    *var3 = "V       ";
char    *var4 = "SW      ";

        enet_pak->header.num_spacing_segs = 1;

        (void) strncpy (enet_pak->header.prod_name[0], var1, 8);
        (void) strncpy (enet_pak->header.prod_name[1], var2, 8);
 		(void) strncpy (enet_pak->header.prod_name[2], var3, 8);
		(void) strncpy (enet_pak->header.prod_name[3], var4, 8);

        enet_pak->header.min_behind_uct = 420;

/*      assume 1 byte vel data until data indicates differently */
        enet_pak->header.vel_field_size = 0;

        enet_pak->header.start_gate_first_pkt = 1;

		enet_pak->header.range_to_first_gate = RANGE_TO_FIRST_GATE;

		enet_pak->header.ant_beamwidth = BEAM_WIDTH;

		enet_pak->header.pulse_width  = PULSE_WIDTH;

		enet_pak->header.freq = FREQUENCY;

}

/*----------------------------------------------------------------------------*/

int
store_data (TDWR_data_header *input_data, unsigned char *output_data, 
											unsigned short low_prf)

/* copy the fields from a set of radar ethernet packets to a
 * Lincoln Lab buffer
 */

{
Reformat_data    *moments_data;
TDWR_normal_prf_data    *normal_prf;
unsigned char  *flag_ptr;
unsigned short  flag;
unsigned char	*dbz_ptr, *snr_ptr;
unsigned short	max_8bit_vel;
int     i;
unsigned short	num_gates = 0;
static unsigned short	min_compressed_value = 0;
static unsigned short	max_compressed_value = 0;
static short			first_time = TRUE;

	normal_prf = (TDWR_normal_prf_data *) ((char *) input_data 
			+ sizeof (TDWR_data_header));

	moments_data = (Reformat_data *) output_data;

	if (first_time)
	{
		first_time = FALSE;
  	   /* to allow dynamic modification of the velocity bin size
	    * calculate min and max values (using the 16 bit TDWR scale and 
	    * bias) used to determine a range of accepted values defined by 
		* the Glob.vel_bias and Glob.vel_scale specified by the user
	    */
		min_compressed_value =
			(Glob.vel_bias - TDWR_VEL_BIAS) / TDWR_VEL_SCALE;

		/* because it is possible to have a range that includes values from
		 * VEL_BIAS to -(vel_bias - 1) a max 8 bit velocity is calculated based
		 * on the user defined scale and bias
		 */
		max_8bit_vel = 255 * Glob.vel_scale + Glob.vel_bias; 

		max_compressed_value =
			(max_8bit_vel - TDWR_VEL_BIAS) / TDWR_VEL_SCALE;
	}

	if (!low_prf)
	{
		if (input_data->rng_samples_per_dwell > MAX_RDI_GATES)
			num_gates = MAX_RDI_GATES;
		else
			num_gates = input_data->rng_samples_per_dwell;
			
		for (i = 0; i < num_gates; i++)
		{
	
			if (i >= input_data->final_range_sample)
			{
				moments_data->vel = 0;
				moments_data->dbz = 0;
				moments_data->width = 0;
				moments_data->snr = 0;
			}
			else
			{
				moments_data->snr = normal_prf->snr;
				moments_data->dbz = normal_prf->dbz;
				moments_data->width = normal_prf->width;

				/* input data is 16 bit data - use the scale and bias to
				 * map into 8 bits
				 */
	
				normal_prf->vel = BE_to_ui16(normal_prf->vel);
				normal_prf->dealias_vel = BE_to_ui16(normal_prf->dealias_vel);

				if (normal_prf->vel > max_compressed_value)
					moments_data->vel = 255;
				else if (normal_prf->vel < min_compressed_value)
					moments_data->vel = 0;
				else
					moments_data->vel 
						= (unsigned char) (((float )(normal_prf->dealias_vel
						* TDWR_VEL_SCALE + TDWR_VEL_BIAS - Glob.vel_bias) 
									/ (float )(Glob.vel_scale)) + .5);

				if (0)
				if (i == 99)
					(void) printf ("%g %g %d %g\n ", input_data->azimuth, (float ) normal_prf->dealias_vel * (TDWR_VEL_SCALE /100.)  + (TDWR_VEL_BIAS/100.), normal_prf->vel, (float )moments_data->vel * Glob.vel_scale / 100. + Glob.vel_bias / 100. );
     	       if (Glob.caf)
				{
					if ((normal_prf->data_flag & CAF) == CAF)
					/* compressed delalias algorithm failure flag*/
					{
						/* this should be activated only on conditioned vel*/
						moments_data->vel = 0; 
					}
				}
				if (Glob.ctf)
				{
					if ((normal_prf->data_flag & CTF) == CTF)
					/* point target flag */
					{
						moments_data->width = 0;
						moments_data->vel = 0;
						moments_data->dbz = 0;
					}
				}

				if (Glob.ccv)
				{
					if ((normal_prf->data_flag & CCV) == 0)
					/* compressed conditioned valid flag removes clutter
					 * and implements thresholding on velocity and SN */
					{
						moments_data->width = 0;
						moments_data->vel = 0;
					}
				}

				if (Glob.cvf)
				{
					if ((normal_prf->data_flag & CVF) == 0)
					/* compressed valid flag removes clutter */
					{
						moments_data->width = 0;
						moments_data->vel = 0;
						moments_data->dbz = 0;
					}
				}
			}

			/* increment pointer */
			normal_prf++;
			moments_data++;
		}
	}
	else
	{
		num_gates = input_data->rng_samples_per_dwell;
		dbz_ptr = (unsigned char *) input_data + sizeof (TDWR_data_header);
		snr_ptr = (unsigned char *) dbz_ptr + num_gates;
		flag_ptr = (unsigned char *) snr_ptr + num_gates;


		if (input_data->rng_samples_per_dwell > MAX_RDI_GATES)
		{
			num_gates = MAX_RDI_GATES;
		}

		for (i = 0; i < num_gates; i++)
		{
		/*
			if (*snr_ptr < Glob.snr_threshold)
			{
				moments_data->dbz = 0;
				moments_data->snr = *snr_ptr;
			}
			else
			{
				moments_data->snr = *snr_ptr;
				moments_data->dbz = *dbz_ptr;
			}
		 */
			moments_data->snr = *snr_ptr;
			moments_data->dbz = *dbz_ptr;
			moments_data->vel = 0;
			moments_data->width = 0;

			if (i < 1760)
				flag = *flag_ptr;

			if (Glob.ctf)
			{
				if ((flag & CTF) == CTF)
				/* point target flag */
				{
					moments_data->dbz = 0;
				}
			}
			if (Glob.cv)
			{
				if ((flag & CV) == 0)
				{
					moments_data->snr = 0;
					moments_data->dbz = 0;
				}
			}

			/* increment pointers */
			dbz_ptr++;
			snr_ptr++;
			flag_ptr++;
			moments_data++;
		}
	}

	return (0);
}

/*---------------------------------------------------------------------------*/

void print_data (Beam_hdr *header)

{
	static	vol_count = 0;
	static  count = 0;


	if (header->vol_count != vol_count)
	{
		vol_count = header->vol_count;
		(void) printf ("\n\nvolume  radar     site   latitude  longitude  altitude  beamwidth  polarization  power trans   freq  pulse width");
		(void) printf ("\n  %d    %s   %s  %.2f N   %.2f W    %d       %.2f         %d           %.2f      %d      %d\n",
						header->vol_count, 
						header->radar_name, header->site_name, 
						(float) (header->latitude) / 100000., 
						(float) (header->longitude) /100000., header->altitude, 
						header->ant_beamwidth / 100., 
						header->polarization, 
						(float) (header->power_trans) / 256., 
						header->freq, header->pulse_width);
	}
	/*if (count % 20 == 0)*/
	if (count % 1 == 0)
	{
		if (count % 200 == 0)
		{
	(void) printf ("\n\nvolume  radar   site  latitude longitude  altitude  beamwidth  polarization  power trans   freq  pulse width");
			(void) printf ("\n  %d    %s   %s  %.2f N   %.2f W    %d       %.2f         %d           %.2f      %d      %d\n",
						header->vol_count,
						header->radar_name, header->site_name,
						(float) (header->latitude) / 100000.,
						(float) (header->longitude) /100000., header->altitude,
						header->ant_beamwidth / 100.,
						header->polarization,
						(float) (header->power_trans) / 256.,
						header->freq, header->pulse_width);
	 	(void) printf ("\ntilt num  elevation    gpb    gate spacing   prf     date       time     azimuth     VCP  scan dir scan mode");
		}



(void) printf ("\n   %2d       %5.2f      %3d        %3d       %d  %2d/%02d/%02d   %2d:%02d:%02d    %6.2f      %d       %d      %d", 
						header->tilt_num, (float ) header->target_elev / 100., 
						header->gates_per_beam, header->gate_spacing, 
						header->prf, header->month, header->day, header->year, 
						header->hour, header->min, header->sec, 
						(float) (header->azimuth) / 100., header->vcp, header->scan_dir, 
						header->scan_mode);
	}
	count++;

}


