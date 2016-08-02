/*
 * include for command
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/file.h>

#include <toolsa/umisc.h>

#define MAX_NAME_SIZE  128
#define MAX_LINE_SZ  256

#define TAPE	        10
#define RADAR	        20
#define INITIALIZE      30
#define INITIALIZE_RMT  35
#define RATE_CHANGE     40
#define REWIND	        50
#define PAUSE	        60
#define TAPE_ERR        -1

#define FORWARD	        1
#define BACKWARD        2

struct skip
{
	int	change;
	int	direction;
};


typedef struct skip Skip;

struct flag_pkt
{
	int     caf;
	int     ctf;
	int     cvf;
	int     ccv;
	int     cv;
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
	Flag_pkt        flag;
};
typedef struct cmd_pkt Cmd_pkt;

struct alter_pkt
{
	char	type[20];
	union
	{
		int	value;
		char	string[20];
	} data;

};
typedef struct alter_pkt Alter_pkt;

typedef struct
{
	char    prog_name[MAX_NAME_SIZE];
	int     debug;
	int		remote_tape;
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
	int     input_shmem_key;
	int     output_shmem_key;
	int		snr_threshold;
	int     real_time;
	int     write_shm_output;
	int     write_fmq_output;
	long    output_fmq_size;
	long    output_fmq_nslots;
	int     output_fmq_compress;
	char    output_fmq_path [MAX_NAME_SIZE];
	char    tape_device[MAX_NAME_SIZE];
	int     caf;   
	int     ctf;  
	int     cvf;  
	int     ccv;  
	int     cv;    
	int		true_north;

} Global;

/*
 * prototypes
 */

extern int get_params (Global  *Glob);

extern int send_shm(int sw, int shm_key, unsigned char *ray);





