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
/******************************************************************************
 *  TEST_SERVER.C Test program for the data servers 
 */

#include <stdio.h>
#include <signal.h>
#include <cidd/cdata_util.h>
#include <toolsa/utim.h>

#define WRITE_BUF_SIZE 1024
#define	READ_BUF_SIZE 262144
#define	DIVISOR 8192
 
/************** GLOBAL DATA *****************************/
char	service_name[64];
char	host_name[64];
int	port,server_id;
int	protofd,sockfd;
int	verbose_mode;

cd_command_t	com;
cd_reply_t		reply;
cd_grid_info_t	info;
 
/******************************************************************************
 * MAIN :	Open file and send the output to STDOUT
 *
 */
int main(argc,argv)
	int	argc;
	char	*argv[];
{
	int i,j;
	int	field_num;
	int want_most_recent;
	double lo_height, hi_height;
	unsigned char	*buf,*ptr;	
	char	string[128];
	UTIMstruct req_time;
	void	signal_trap();
	void	sig_io_trap();

	process_args(argc,argv);	 /* process command line arguments */
 
	signal(SIGINT,signal_trap);
	signal(SIGTERM,signal_trap);

	while(1) {

		fprintf(stderr,"Enter Field #, Low Height, High Height (km): ");
		fgets(string,128,stdin);
		sscanf(string,"%d %lf %lf",&field_num,&lo_height,&hi_height);

		fprintf(stderr,"Enter year month day hour min sec, RET for most recent: ");
		fgets(string,128,stdin);
		if (sscanf(string,
			   "%ld%ld%ld%ld%ld%ld",
			   &req_time.year,
			   &req_time.month,
			   &req_time.day,
			   &req_time.hour,
			   &req_time.min,
			   &req_time.sec) != 6) {
		  want_most_recent = 1;
		} else {
		  want_most_recent = 0;
		  req_time.unix_time = UTIMdate_to_unix(&req_time);
		  com.time_cent = req_time.unix_time;
		  com.time_min = com.time_cent - 86400;
		  com.time_max = com.time_cent + 86400;
		}

		if (want_most_recent)
		  fprintf(stderr,"Want most recent\n");
		else
		  fprintf(stderr,"time : %ld %ld %ld %ld %ld %ld %ld\n",
			 req_time.year,
			 req_time.month,
			 req_time.day,
			 req_time.hour,
			 req_time.min,
			 req_time.sec,
			 req_time.unix_time);

		/* Set up the command request */
		if (want_most_recent) {
		  com.primary_com = GET_INFO | GET_MOST_RECENT;
		} else {
		  com.primary_com = GET_INFO;
		}

		com.second_com = GET_XY_PLANE;

		com.lat_origin = 39.880980;	/* MHR 1992 */
		com.lon_origin = -104.761730;

		com.min_x = -10000.0;
		com.max_x = 10000.0;
		com.min_y = -10000.0;
		com.max_y = 10000.0;

		com.min_z = lo_height;
		com.max_z = hi_height;
	
		com.data_field = field_num;
		com.data_type = CDATA_CHAR;
	
		/* Pick up info on the origin */
		buf = cdata_get(&com,&reply,&info,host_name,port);

	    com.primary_com |=  GET_DATA;
		com.lat_origin = info.lat_origin;
		com.lon_origin = info.lon_origin;

		/* Now try for the actual data */
		buf = cdata_get(&com,&reply,&info,host_name,port);

		print_reply(&reply,stdout);
	
		print_info(&info,stdout);
	
		if(buf != NULL ) {
			if(verbose_mode == 1) {
	
				ptr = buf;
				for(i=0; i < reply.nx; i++ ) {
					printf("\n");
					for(j=0;j < reply.ny; j++) {
						printf("%3u ",(unsigned int) *ptr++);
					}
				}
				printf("\n");
			}
		} else {
			printf("\n*** No data Returned! ***\n");
		}
	}
}

/*****************************************************************
 * PRINT_INFO: Produce diagnostic output on the info 
 */

int print_info(info,file)
	cd_grid_info_t	*info;
	FILE	*file;
{
	 
	fprintf(file,"\n***************************** GRID INFO **********************\n");
	fprintf(file,"data_field: %d\t",info->data_field);
	fprintf(file,"projection: %d\n",info->projection);
	fprintf(file,"source_x: %g\t",info->source_x);
	fprintf(file,"source_z: %g\t",info->source_y);
	fprintf(file,"source_y: %g\n",info->source_z);
	fprintf(file,"nx: %d\t",info->nx);
	fprintf(file,"ny: %d\t",info->ny);
	fprintf(file,"nz: %d\n",info->nz);
	fprintf(file,"num_gates: %d\t",info->num_gates);
	fprintf(file,"max_gates: %d\t",info->max_gates);
	fprintf(file,"min_gates: %d\n",info->min_gates);
	fprintf(file,"num_tilts: %d\t",info->num_tilts);
	fprintf(file,"data_length: %d\n",info->data_length);

	fprintf(file,"\nlat_origin: %g\t",(info->lat_origin));
	fprintf(file,"lon_origin: %g\t",(info->lon_origin));
	fprintf(file,"ht_origin: %g\n",(info->ht_origin));
	fprintf(file,"dx: %g\t",(info->dx));
	fprintf(file,"min_x: %g\t",(info->min_x));
	fprintf(file,"max_x: %g\n",(info->max_x));
	fprintf(file,"dy: %g\t",(info->dy));
	fprintf(file,"min_y: %g\t",(info->min_y));
	fprintf(file,"max_y: %g\n",(info->max_y));
	fprintf(file,"dz: %g\t",(info->dz));
	fprintf(file,"min_z: %g\t",(info->min_z));
	fprintf(file,"max_z: %g\n",(info->max_z));
	fprintf(file,"north_angle: %g\t",(info->north_angle));
	fprintf(file,"gate_spacing: %g\t",(info->gate_spacing));
	fprintf(file,"wavelength: %g\t",(info->wavelength));
	fprintf(file,"frequency: %g\n",(info->frequency));
	fprintf(file,"min_range: %g\t",(info->min_range));
	fprintf(file,"max_range: %g\n",(info->max_range));
	fprintf(file,"min_elev: %g\t",(info->min_elev));
	fprintf(file,"max_elev: %g\n",(info->max_elev));
	fprintf(file,"radar_const: %g\t",(info->radar_const));
	fprintf(file,"delta_azmith: %g\t",(info->delta_azmith));
	fprintf(file,"start_azmith: %g\t",(info->start_azmith));
	fprintf(file,"beam_width: %g\t",(info->beam_width));
	fprintf(file,"pulse_width: %g\n",(info->pulse_width));
	fprintf(file,"noise_thresh: %g\t",(info->noise_thresh));

	fprintf(file,"\nunits_label_x: %s\n",info->units_label_x);
	fprintf(file,"units_label_y: %s\n",info->units_label_y);
	fprintf(file,"units_label_z: %s\n",info->units_label_z);
	fprintf(file,"field_units: %s\n",info->field_units);
	fprintf(file,"field_name: %s\n",info->field_name);
	fprintf(file,"source_name: %s\n",info->source_name);

 return 0;
}

/*****************************************************************
 * PRINT_REPLY: Produce diagnostic output on the reply 
 */

int print_reply(rep,file)
	cd_reply_t	*rep;
	FILE	*file;
{
	UTIMstruct dt;

	fprintf(file,"\n***************************** REPLY **********************\n");
	fprintf(file,"status: %d\t",rep->status);
	fprintf(file,"orient: %d\n",rep->orient);
	fprintf(file,"nx: %d\t",rep->nx);
	fprintf(file,"ny: %d\t",rep->ny);
	fprintf(file,"nz: %d\n",rep->nz);
	fprintf(file,"dx: %g\t",(rep->dx));
	fprintf(file,"dy: %g\t",(rep->dy));
	fprintf(file,"dz: %g\n",(rep->dz));
	fprintf(file,"x1: %d\t",rep->x1);
	fprintf(file,"x2: %d\t",rep->x2);
	fprintf(file,"y1: %d\t",rep->y1);
	fprintf(file,"y2: %d\t",rep->y2);
	fprintf(file,"z1: %d\t",rep->z1);
	fprintf(file,"z2: %d\n",rep->z2);
	fprintf(file,"scale: %g\t",(rep->scale));
	fprintf(file,"bias: %g\n",(rep->bias));

	fprintf(file,"time_begin: %d\t",rep->time_begin);
	UTIMunix_to_date(rep->time_begin,&dt);
	fprintf(file," %ld/%ld/%ld %ld:%02ld:%02ld\n", dt.month, dt.day, dt.year,
			dt.hour, dt.min, dt.sec);

	fprintf(file,"time_end: %d\t",rep->time_end);
	UTIMunix_to_date(rep->time_end,&dt);
	fprintf(file," %ld/%ld/%ld %ld:%02ld:%02ld\n", dt.month, dt.day, dt.year,
			dt.hour, dt.min, dt.sec);

	fprintf(file,"time_cent: %d\t",rep->time_cent);
	UTIMunix_to_date(rep->time_cent,&dt);
	fprintf(file," %ld/%ld/%ld %ld:%02ld:%02ld\n", dt.month, dt.day, dt.year,
			dt.hour, dt.min, dt.sec);

	fprintf(file,"bad_data_val: %d\t",rep->bad_data_val);
	fprintf(file,"data_type: %d\t",rep->data_type);
	fprintf(file,"data_field: %d\t",rep->data_field);
	fprintf(file,"n_points: %d\t",rep->n_points);
	fprintf(file,"data_length: %d\n",rep->data_length);

 return 0;
}

#define ARG_OPTION_STRING	"p:s:h:v"
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *	And print usage info if necessary
 */

int process_args(argc,argv)
	 int argc;
	 char *argv[];
{
	 int err_flag =0;
	 int  c;
	 extern  int optind; /* index to remaining arguments */
	 extern  char *optarg;	/* option argument string */

	if(argc < 3) err_flag++;
 
	 while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
		  switch(c) {
				case 'v' :  /* verbose mode */
					 verbose_mode = 1;
				break;
 
				case 'p' :  /* alternate port */
					 port = atoi(optarg);
				break;
 
				case 's' :  /* alternate service name */
					 strncpy(service_name,optarg,32);
				break;
 
				case 'h' :  /* alternate host name */
					 strncpy(host_name,optarg,32);
				break;
 
				case '?':	/* error in options */
				default:
					 err_flag++;
				break;
		  }
 
	 };
 
	 if(err_flag) {
		  fprintf(stderr,"Usage:test_server -h host_name -p port [-v]\n");
		  fprintf(stderr,"-v: Verbose mode - Prints data plane\n");
		  exit(-1);
	 }

	 return 0;
 
}
 
/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */
void
sig_io_trap()
{

	fprintf(stderr,"\n############## SIG_IO ##################\n");
	fflush(stderr);

}
 
/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */
void
signal_trap()
{

	exit(0);
}


