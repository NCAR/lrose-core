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
/*
 * include for tdwr2moments
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/file.h>

#include <net/if.h>
#include <netinet/in.h>

#ifdef SUNOS4

# include       <netinet/if_ether.h>
# include       <net/packetfilt.h>
# include       <net/nit_if.h>
# include       <net/nit_pf.h>
# include       <stropts.h>

#else /* ifdef SUNOS4 */

/*
 * Stream buffer structure for send and recv system calls
 */
struct strbuf {
        int     maxlen;                 /* no. of bytes in buffer */
        int     len;                    /* no. of bytes returned */
        char    *buf;                   /* pointer to data */
};

#endif /* ifdef SUNOS4 */

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include "tdwr.h"
#include "moments.h"

#define MAX_NAME_SIZE  128
#define MAX_AZI_PER_TILT  3600
#define UNDEFINED	-1
#define MAX_RDI_GATES  600
#define MAX_BEAM_SZ  2250
#define MAX_AZI  400
#define MAX_RESOLUTION  512
#define MAX_LINE_SZ  256
#define CMD_SIZE		128
#define SPD_OF_LITE		299800000
#define DEFAULT_INPUT_SHMEM_KEY 2750400
#define TRUE_NORTH_FACTOR 300

#define ETHERNET_FRAME_SIZE 756


# define	TAPE		10
# define 	RADAR		20
# define	INITIALIZE	30
# define	INITIALIZE_RMT	35
# define	RATE_CHANGE	40
# define	REWIND		50
# define	PAUSE		60
# define	FLAG_CHANGE	70
# define	TAPE_ERR	-1

# define	WAIT		0
# define	NO_WAIT		1

# define	FORWARD		1
# define	BACKWARD	2

# define		FALSE		0
# define		TRUE		1

# define		MH_RADAR	0
# define		CP_RADAR	1
# define		MH_RAD_PP	2

# define        RECSIZE         66000
# define        NPKTS_MH        3
# define        MAX_PKTS        6
# define        NUM_LL_PARAM    4

struct tdwr_normal_prf_data
{
	unsigned char	dbz;
	unsigned char	snr;
	unsigned short	vel;
	unsigned char	width;
	unsigned char	data_flag;
	unsigned short	dealias_vel;
};

typedef struct tdwr_normal_prf_data     TDWR_normal_prf_data;


struct reformat_data
{
	unsigned char  snr;
	unsigned char  dbz;
	unsigned char  vel;
	unsigned char  width;
};

typedef struct reformat_data    Reformat_data;

struct skip
{
	int	change;
	int	direction;
};


typedef struct skip Skip;

struct flag_pkt
{
	int	caf;
	int	ctf;
	int	cvf;
	int	ccv;
	int	cv;
};

typedef struct flag_pkt Flag_pkt;

struct tape_pkt
{
	char	device[20];
	int	rate;
	int	rewind;
	Skip		position;
};

typedef struct tape_pkt Tape_pkt;

struct cmd_pkt
{
	char	type[30];
	int	dropped_gates;
	Tape_pkt	tape;
	Flag_pkt	flag;
};

typedef struct cmd_pkt Cmd_pkt;

struct alter_pkt
{
	char	type[20];
	union
	{
		int	value;
		char	string[50];
	} data;

};

typedef struct alter_pkt Alter_pkt;

typedef struct
{
	char    prog_name[MAX_NAME_SIZE];
	int     debug;
	int	remote_tape;
	/* radar site specific params */
	char	radar_name[MAX_NAME_SIZE];
	char	site_name[MAX_NAME_SIZE];
	long	latitude;
	long	longitude;
	short	altitude;
	short	polarization;
	short	vel_bias;
	short	vel_scale;
	int		lo_prf_gate_spac;
	int		cmd_shmem_key;
	int		input_shmem_key;
	int		output_shmem_key;
	int		snr_threshold;
	int		real_time;
	int		write_shm_output;
	int		write_fmq_output;
	long	output_fmq_size;
	long	output_fmq_nslots;
	int		output_fmq_compress;
	char    output_fmq_path [MAX_NAME_SIZE];
	char    tape_device[MAX_NAME_SIZE];
	int     caf;
	int     ctf;
	int     cvf;
	int     ccv;
	int     cv;
	int		true_north;
} Global;

typedef struct {
	long id;
	long len;     /* message size in bytes, not including header */
	long seq_no;
} SKU_header_t;


typedef struct {
	unsigned char	ref[MAX_BEAM_SZ];
	unsigned short	azimuth;
} Low_prf_data;

/*
 * prototypes
 */

extern int get_params (Global  *Glob);

extern int get_shm_cmd (char *command);

extern void tdwr_process_logical (TDWR_data_header *data_hdr,
                                 LL_beam_rec *enet_pak);

extern int parse_settings (char *comm_pkt, Global *Glob);

extern int process_exit (int sig);

extern int send_shm (int flag, int shm_info, unsigned char *input_data);

extern int recv_shm (int flag, int shm_key, unsigned char **data);

extern int recv_cmd (int sw, int key, unsigned char **ray);
extern int tape_init (int *fd, char *device);

/* fmq stuff */
extern int open_output_fmq(char *fmq_path, int buf_size, int nslots,
                           int compress, char *prog_name, int debug);
extern int write_output_fmq(ui08 *buffer, int buflen);
extern void close_output_fmq(void);

extern int write_stream(ui08 *write_buffer, int nwrite);


/* tape ops */

extern int rmt_open (char *path, int oflag, int mode);
extern int rmt_close (void);
extern int rmt_read (char *buf, unsigned int nbyte);
extern int rmt_write (char *buf, unsigned int nbyte);
extern long rmt_lseek (long offset, int whence);
extern int rmt_ioctl (int op, char *arg);

extern int get_tape_data (int tape_fd,
                          int data_rate,
                          LL_beam_rec *enet_pak);

/* swapping */

extern void Beam_hdr_to_BE(Beam_hdr *hdr);
extern void be_to_TDWR_data_header(TDWR_data_header *hdr);
extern void be_from_TDWR_data_header(TDWR_data_header *hdr);
extern void be_to_TDWR_packet_header(Packet_hdr *hdr);
extern void be_from_TDWR_packet_header(Packet_hdr *hdr);
