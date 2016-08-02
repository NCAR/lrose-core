
# ifndef _NEXRAD_H
# define _NEXRAD_H

/*Class to create and write out NEXRAD data
 */
#define NOTDONE 0
#define DONE 1
#define DATASIZE 2432
#define NUMOMENTS 3
#define SPD_OF_LITE 299800000
#define NEXRAD_FILE 1
#define NEXRAD_SOCKET 2
#define NEXRAD_AVAIL 0
#define NEXRAD_SOCKET_LIMIT 100
#define NEX_DELAY 10  //in millisecs

#include "rdrfilter.h"

//make this sufficiently large to save bothering with dynamic mem alloc
#define DATABUFF_SIZE 3000

  typedef struct message_str
  {
    NEXRAD_frame_hdr ctm_info;
    //NEXRAD_ctm_info ctm_info;
    NEXRAD_msg_hdr msg_hdr;
    NEXRAD_data_hdr data_hdr;
  } message_typ;

  typedef struct message_201str
  {
    NEXRAD_frame_hdr ctm_info;
    NEXRAD_msg_hdr msg_hdr;
    si16 vol_scan_no;
    si16 dummy[21];
  } message_201;

class NexRadOutpDesc
{
public:
  int fd;                //file or socket descriptor
  char address[100];     //IP address of target
  int port;
  int fs_flag;           //0 if error,1 if file, 2 if socket,
  char filepath[256];    //full pathname of output file
  struct sockaddr_in Out_address; 
  bool archive2file;

  NexRadOutpDesc();
  NexRadOutpDesc(char *,int);
  ~NexRadOutpDesc();
  void close_fd();
  void open_socket(int local_port);
  void open_file(char *filename);
  void write_beam(char *data,int datasize);
  int open_output_udp(int loc_port);
  
};

/*
  There is a difference btwn the ctm_info required for UDP packets and files
  UDP packets require  
  ui32 fr_seq;           incremental counter for all packets 
  at the start of the ctm_info while files don't have it
  A kludgy solution is to skip past the 1st 4 bytes of the
  packet when it is written
  It requires the data buffer to be 4 bytes longer to allow the packet
  start to be skipped fwd 4 bytes whilst keeping packet size the same
*/ 


class NexRad
{
  message_typ message;
  
  char data[NEX_PACKET_SIZE+4]; // see above  
  
  int tiltarr[NUMOMENTS];  //indexes of scans in this tilt
  e_data_type tiltTypes[NUMOMENTS];
  bool	 debug;
  bool filter_Vel;
  rdrfilter *velfilter;
  bool filter_Refl;
  rdrfilter *reflfilter;
  bool filter_SpWdth;
  rdrfilter *specwfilter;
  
public:
  
  int scan_tilt_no;        //scan no. in  this tilt, index of above
  int scan_vol_no;	   //scan no. in this volume
  //  int fd;
  rdr_scan *scan;          //the root scan
  int volbegin;	  
  int volend;	 
  int tiltnumber;
  int seq_number;
  ui32 fr_seq;
  int first_scan;
  rdr_angle last_scan_angle;
  int local_port;
  int pattern;
  int packet_size;
  int delay_pac;
  time_t min_time_per_tilt;	// minimum no. of seconds between O/P tilts
  time_t next_tilt_time;	// don't send next tilt until >= this time
  int VolumeNo;
  int refl_rngres_reduce_factor;
  int vel_rngres_reduce_factor;
  
  NexRadOutpDesc **outputdesc;  //array of ptrs to object
  int		  outputcount;
  
 
  NexRad();
  //NexRad(rdr_scan *scan,int fd);
  NexRad(rdr_scan *scan,
	NexRadOutpDesc **outputdesc,
	int outputcount,
	int pattern,
	int packet_size,
	bool debug,
	int VolumeNo,
	int seq_num,
	ui32 fr_seq,
	int delay_pac, 
        bool filter_Vel,
	bool filter_Refl,
	bool filter_SpWdth,
	 int refl_Rngres_reduce_factor, 
	 int vel_Rngres_reduce_factor, 
	int mintimepertilt = 0);
  ~NexRad();
  void send_201(time_t scantime);
  int checkWrite();
  void writeTilt();
  void writeLast();
  int getJulian(time_t time);
  int getMillsecFromMidn(time_t time);
  void print_data_buffer(char *data,message_typ *message);
  void print_201(message_201 *message);
  rdr_scan* goto_scan(int n);
  bool makeSpecWidth;
  
  
};


# endif     /* _NEXRAD_H */


