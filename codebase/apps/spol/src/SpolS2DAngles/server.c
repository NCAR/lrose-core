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
// server.c + s2dspol.c  syncro to digital and time stamped spol angles angel broadcast 
// Last modify dec 7 2010 by paloma@ucar.edu: change to UDP and merge with s2dspol sw 
// Last modify dec 8 2010 by paloma@ucar.edu: added/tested 2nd thread (modify cross compiler)
// Last modify dec 9 2010 by paloma@ucar.edu: fix brodcast not completely happy with speed and usleep
// Last modify dec 9 2010 by paloma@ucar.edu: add mutex 
// Last modify dec 12 2010by paloma@ucar.edu: add ntpwirte info struct and mutex compile 

////////////////////////////
// Read S2D angles for SPOL

// s2dspol & udp broadcast

// basic
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>		
#include <math.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/stat.h>

// networking
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/time.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// threading
#include <pthread.h>

// contains the definitions used by the terminal I/O interfaces
#include <termios.h>

// hardware addresses 
#define PC104    0x21A00000   
//#define PC104  0x11E00000        // 16 bit memeory    2.6 kernel? 0x22000000
#define S2D_BASE 0x00 

// udp broadcast adresses 
#define PORT 12080
#define DEST_ADDR "192.168.4.255"

// size of buffers for strings
#define BUFFER_SIZE 256

// mutex for thread synchronization

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

// location struct

typedef struct { 
  int seq_num; 
  int bin_az; 
  int bin_el; 
  int unix_time;
  int usec;
  int spare;
} ant_pos_t; 

ant_pos_t global_pos;

// angle scaling

const int maxBinAng = 65536;
const double intPerDeg = 65536.0 / 360.0;
const double degPerInt = 360.0 / 65536.0;

// device ids and buffer for display

int fd2, fd3; 
char buff[3];

// pointers into PC104 memory

unsigned short *az_syncro;
unsigned short *el_syncro;
unsigned short *az_offset_dec;
unsigned short *az_offset_num;
unsigned short *el_offset_dec;
unsigned short *el_offset_num;
unsigned short *az_pmac;
unsigned short *el_pmac;
	
// functions

int pc104init(); // initializing pc104 expansion board
int comOpen(const char *device_name); // open COM port
int readAzOffset(); // read azimuth offset
int readElOffset(); // read elevation offset
void lcdinit(); // initialize AZ and EL LCD display
void lcdclear(); // clear LCD
void* lcdwrite (void*); // threaded write to LCD
void* udpwrite (void*); // threaded write to UDP

// main entry

int main(int argc, char **argv)

{
	
  global_pos.seq_num = 0; 

  // initialize PC104

  if (pc104init()) {
    exit(1);
  }

  // open COM2 Port

#ifndef NO_PC104

  fd2 = comOpen("/dev/ttyAM1");
  if (fd2 < 0) {
    exit(1);
  }
  usleep (100);

  // open COM3 Port
  
  fd3 = comOpen("/dev/ttyTS0");
  if (fd3 < 0) {
    exit(1);
  }

  // LCD init

  lcdinit();
  usleep (100); 

#endif

  // create thread for LCD writing

  pthread_t lcdThread;
  int rc = pthread_create(&lcdThread, NULL, lcdwrite, 0);
  if (rc) {
    printf("ERROR - pthread_create() for LCD, retcode: %d\n", rc);
    exit(1);
  }
		
  // create thread for UDP writing

  pthread_t udpThread;
  rc = pthread_create(&udpThread, NULL, udpwrite, 0); 
  if (rc)
  {
    printf("ERROR - pthread_create() for UDP, retcode: %d\n", rc);
    exit(1);
  }
 
  // main loop

  double prev_time =0; 
  int loop_count = 0; 

  while (1) { 

    // get the offsets from the thumbwheels

    int azOffset = readAzOffset();
    int elOffset = readElOffset();
	
    // get the angles from the synchro

    int azSynchro = *az_syncro;
    int elSynchro = *el_syncro;

    // apply the az offset, taking the modulus and correcting for direction
    
    int azCorrected = azSynchro + azOffset;
    azCorrected = (azCorrected + 3 * maxBinAng) % maxBinAng;
    azCorrected = maxBinAng - azCorrected;
    
    // apply the el offset, taking the modulus

    int elCorrected = elSynchro + elOffset;
    elCorrected = (elCorrected + 3 * maxBinAng) % maxBinAng;
    
    // WRITE TO PMAC

    *el_pmac = (short) elCorrected;
    *az_pmac = (short) azCorrected;

    // GET TIME        
 
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // set data in position struct
    
    ant_pos_t pos; 
    pos.bin_az = azCorrected;
    pos.bin_el = elCorrected;
    pos.unix_time = tv.tv_sec;
    pos.usec = tv.tv_usec;
    pos.seq_num = loop_count;
    pos.spare = 0;
    
    // copy to global struct

    pthread_mutex_lock(&mutex);
    memcpy (&global_pos, &pos, sizeof(pos)); 
    pthread_mutex_unlock(&mutex);  

    loop_count++; 

    if (loop_count % 1000 == 0)
    {
      double now = tv.tv_sec + tv.tv_usec /1.0e6;
      double delta_time = now - prev_time;
      double rate = 1000.0/delta_time; 
      fprintf(stderr, "%d %d %d %d %g\n",
              pos.unix_time, pos.usec,
              pos.bin_az, pos.bin_el, rate);
      prev_time = now; 
    }

    // sleep a bit
    usleep (100);

   } //end of while loop  

 } //end of main 

// FUNCTIONS

///////////////////////////////////////////////////
// initialize PC104
// returns 0 on success, -1 on failure

int pc104init()
{
  
#ifdef NO_PC104

  unsigned char *pc104_start = (unsigned char *) malloc(S2D_BASE + 128);

#else

  // PC/104 8 bit initiatate read/write to I/O
  
  int size = getpagesize();
  
  int fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1) // Error checking purposes
  {
    perror("open /dev/mem");
    return -1;
  }
  
  unsigned char *pc104_start;
  
  pc104_start = (unsigned char *)
    mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PC104);
  
  if (MAP_FAILED == pc104_start)    // Error checking purposes
  {
    perror("mmap");
    fprintf(stderr, "Cannot open PC104");
    return -1;
  }

#endif

  az_syncro       = (unsigned short *) (pc104_start + S2D_BASE + 0);
  el_syncro       = (unsigned short *) (pc104_start + S2D_BASE + 2);
  az_offset_num   = (unsigned short *) (pc104_start + S2D_BASE + 4);
  az_offset_dec   = (unsigned short *) (pc104_start + S2D_BASE + 6);
  el_offset_num   = (unsigned short *) (pc104_start + S2D_BASE + 8); 
  el_offset_dec   = (unsigned short *) (pc104_start + S2D_BASE + 10); // x0A
  az_pmac         = (unsigned short *) (pc104_start + S2D_BASE + 12); // x0C
  el_pmac         = (unsigned short *) (pc104_start + S2D_BASE + 14); // x0E

  return 0;

}

///////////////////////////////////////////////////
// open COM port
// returns device id on success, -1 on error

int comOpen(const char *device_name)
{
  
  int fd = open(device_name, O_RDWR|O_NOCTTY|O_NDELAY);
  if (fd < 0) {
    perror(device_name);
    fprintf(stderr, "Cannot open COM port, name: %s\n", device_name);
    return -1;
  }
  
  // terminal IO options
  struct termios options;
  if (tcgetattr(fd, &options) < 0)
  {
    perror("Can't get terminal parameters ");
    return -1;
  }
  
  // control options: 8-N-1
  options.c_cflag &= ~PARENB; // no parity
  options.c_cflag &= ~CSTOPB; // 1 stop bit
  options.c_cflag &= ~CSIZE; // mask character size bits
  // options.c_cflag &= ~(CSIZE|PARENB|CRTSCTS);
  options.c_cflag |= (CS8|CLOCAL|CREAD); /* 8 data bits, local line, 
                                          * enable receiver */
  
  options.c_cflag &= ~CRTSCTS; // disable hardware flow control
  
  // input options
  options.c_iflag = IGNPAR; // ignore parity errors
  
  // output options
  options.c_oflag = 0;
  
  // line/local options
  options.c_lflag = 0;
  
  // control characters
  options.c_cc[VTIME] = 0; // time to wait for data
  options.c_cc[VMIN] = 0; // minimum number of characters to read
  
  // setting baud rate
  cfsetispeed(&options, B19200);
  cfsetospeed(&options, B19200);
  
  // flushes data received but not read
  tcflush(fd, TCIFLUSH);
  
  if (tcflush(fd, TCIFLUSH) < 0)
  {
    perror("TCflush error ");
    return -1;
  }
  
  // write the settings for the serial port
  tcsetattr(fd, TCSANOW, &options);
  
  if (tcsetattr(fd,TCSANOW,&options) <0)
  {
    perror("Can't set terminal parameters ");
    return -1;
  }

  return fd;
  
}	

///////////////////////////////////////////
// read the azimuth offset
// return the result in binary into form

int readAzOffset()

{

  static int _az_off_num = -1;
  static int _az_off_dec = -1;
  static int _az_offset_binary = 0;
  
  // FOR AZIMUTH
  // READ SYNCRO AND OFFSET THUMBWHEEL, COMPUTE POSITION 
    

  int az_off_num = *az_offset_num; 
  int az_off_dec = *az_offset_dec; 

  if (az_off_num == _az_off_num &&
      az_off_dec == _az_off_dec) {
    // no change, return previously stored value
    return _az_offset_binary;
  }

  // save for later testing

  _az_off_num = az_off_num;
  _az_off_dec = az_off_dec;
  
  // invert due to thumbwheels 

  az_off_num = (~az_off_num -0xffff0000); 
  az_off_dec = (~az_off_dec -0xffff0000); 
  
  int numrest = az_off_num >> 12;
  numrest = numrest << 12;  

  // the whole number we want minus the first 'F'
  int num = az_off_num - numrest;
  
  int aa = num >>8; 
  aa = aa <<8;  
  aa = az_off_num - numrest -aa; //34
  int ttnum= (aa>>4)*10; 
  int hhnum =(num >>8)*100;    
  int oonum = num%16; 
  
  num = hhnum+ttnum+oonum;
  
  int decrest = az_off_dec >> 8;
  decrest = decrest << 8;		
  int dec = az_off_dec - decrest; 	// number in hex 
  dec = ((dec >>4)*10)+(dec%8); 	// second decimal digit
  
  double aOffset = num + dec /100.0;
  int iOffset = (int) floor(aOffset * intPerDeg + 0.5);
  if (iOffset > maxBinAng) {
    iOffset = maxBinAng;
  } else if (iOffset < -maxBinAng) {
    iOffset = -maxBinAng;
  }
  
  _az_offset_binary = iOffset;
  return iOffset;

}
  
///////////////////////////////////////////
// read the elevation offset
// return the result in binary into form

int readElOffset()

{

  static int _el_off_num = -1;
  static int _el_off_dec = -1;
  static int _el_offset_binary = 0;
  
  // FOR ELEVATION
  // READ SYNCRO AND OFFSET THUMBWHEEL, COMPUTE POSITION 
    

  int el_off_num = *el_offset_num; 
  int el_off_dec = *el_offset_dec; 

  if (el_off_num == _el_off_num &&
      el_off_dec == _el_off_dec) {
    // no change, return previously stored value
    return _el_offset_binary;
  }

  // save for later testing

  _el_off_num = el_off_num;
  _el_off_dec = el_off_dec;
  
  // invert due to thumbwheels 

  el_off_num = (~el_off_num -0xffff0000); 
  el_off_dec = (~el_off_dec -0xffff0000); 
  
  int numrest = el_off_num >> 12;
  numrest = numrest << 12;  

  // the whole number we want minus the first 'F'
  int num = el_off_num - numrest;
  
  int aa = num >>8; 
  aa = aa <<8;  
  aa = el_off_num - numrest -aa; //34
  int ttnum= (aa>>4)*10; 
  int hhnum =(num >>8)*100;    
  int oonum = num%16; 
  
  num = hhnum+ttnum+oonum;
  
  int decrest = el_off_dec >> 8;
  decrest = decrest << 8;		
  int dec = el_off_dec - decrest; 	// number in hex 
  dec = ((dec >>4)*10)+(dec%8); 	// second decimal digit
  
  double aOffset = num + dec /100.0;
  int iOffset = (int) floor(aOffset * intPerDeg + 0.5);
  if (iOffset > maxBinAng) {
    iOffset = maxBinAng;
  } else if (iOffset < -maxBinAng) {
    iOffset = -maxBinAng;
  }
  
  _el_offset_binary = iOffset;
  return iOffset;

}
  
//////////////////////////////////
// set up LCD screen

void lcdinit()
{
  // Clear Screen
  buff[0] = 254; 
  buff[1] = 88; 
  
  write(fd2, buff, sizeof(buff));
  write(fd3, buff, sizeof(buff));
  
  // Set lcd font to arial 50
  buff[0] = 254; 
  buff[1] = 49; 
  buff[2] = 1;
  
  write(fd2, buff, sizeof(buff));
  write(fd3, buff, sizeof(buff));

  write(fd2, "000.00", 6); 
  write(fd3, "000.00", 6); 

}

//////////////////////////////////
// clear LCD screen

void lcdclear() 
{
  // Clear Screen
  buff[0] = 254; 
  buff[1] = 88; 
  write(fd2, buff, sizeof(buff));
  write(fd3, buff, sizeof(buff));
}

///////////////////////////////////////
// THREADS

// write to UDP

void* udpwrite (void* unused)
{ 

  // udp broadcast setup

  int sockfd;
  int yes=1;
  struct sockaddr_in sendaddr;

  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
    {
      perror("sockfd");
      exit(1);
    }
  
  if((setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST|SO_REUSEADDR,
		 &yes, sizeof(yes))) < 0)
    {
      perror("setsockopt - SO_SOCKET ");
      exit(1);
    }
  
  sendaddr.sin_family = AF_INET;
  sendaddr.sin_port = htons(PORT);
  sendaddr.sin_addr.s_addr = inet_addr(DEST_ADDR);
  memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

  // loop waiting for new data, then send it out
  
  ant_pos_t pos; 
  int prev_count = -1; 
  int err_count = 0;
  char str[1024];
  
  while (1)
  {
    
    // get local copy of position struct
    
    pthread_mutex_lock(&mutex);
    memcpy (&pos, &global_pos, sizeof(ant_pos_t));
    pthread_mutex_unlock(&mutex);

    if (pos.seq_num != prev_count && pos.seq_num > 0)
    {
      sprintf(str,
	      "time, usec, az, el, seq_num: "
	      "%d %d %d %d %d\n",
	      pos.unix_time, pos.usec, pos.bin_az,
	      pos.bin_el, pos.seq_num);
      int nbytes = strlen(str) + 1;
      if (sendto(sockfd, str, nbytes, 0,
                 (struct sockaddr *) &sendaddr,
                 sizeof(sendaddr)) != nbytes) {
        // if (err_count == 1000) {
	perror("sendto");
	fprintf(stderr, "udpwrite: cannot write to UDP");
	err_count = 0;
	// }
        err_count++;
      } else {
	fprintf(stderr, "->%d ", nbytes);
	fprintf(stderr, "%s", str);
      }
    }
    prev_count = pos.seq_num; 
    
    // sleep a bit
    
    usleep (5000);
    
  }
  
  exit(1);
  
  close(sockfd);
  pthread_exit(NULL);

}

/////////////////////////////////////////
// write to LCD panels

void* lcdwrite (void* unused)
{ 

  char el_str[32];
  char al_str[32]; 
  int m,l;
  double ap3 = 0.0; 
  double ep3 = 0.0; 
  
  while (1)
  {

#ifdef NO_PC104
    usleep(100000);
    continue;
#endif

    // get local copy of position struct
    
    ant_pos_t pos; 
    pthread_mutex_lock(&mutex);
    memcpy (&pos, &global_pos, sizeof(ant_pos_t));
    pthread_mutex_unlock(&mutex);
    
    // compute angles
    
    double ep = pos.bin_el * degPerInt;
    double ap = pos.bin_az * degPerInt;
    
    if (ep > 360)
      ep = ep - 360;
	
    if (ap > 360)
      ap = ap -360; 

    // Clear Screen
    buff[0] = 254; 
    buff[1] = 88; 
    double ap2=ap; 
    double ep2=ep; 
	
    ap2 = (floor(ap2*100))/100; 
    ep2 = (floor(ep2*100))/100; 
	
	
    if (ap2!=ap3)
    {
      write(fd2, buff, sizeof(buff)); //clear
      if (ap2 < 10)
      {
        l=sprintf (al_str, "%0.2f", ap2); 
        write(fd2, "00", 2); 
        write(fd2, al_str, 4); 
      }
      else
      {
        if (ap2 < 100) 
        {	
          l=sprintf (al_str, "%0.2f", ap2); 
          write(fd2, "0", 1); 
          write(fd2, al_str, 5); 
        }
        else
        {		
          l=sprintf (al_str, "%0.2f", ap2); 
          write(fd2, al_str, 6); 
        }
      }
    }

    if (ep2!=ep3){
      //make new variable n or k 
      // if mellan 270 - 360 g�r negativt 
      // 360 - value, add negative signe in front. 
			
      write(fd3, buff, sizeof(buff)); //clear
					
      if (ep2 < 10){
        m=sprintf (el_str, "%0.2f", ep2); 
        write(fd3, "00", 2); 
        write(fd3, el_str, 4); 
      }
      else{
        if (ep2 < 100) {	
          m=sprintf (el_str, "%0.2f", ep2); 
          write(fd3, "0", 1); 
          write(fd3, el_str, 5); 
        }
        else{		
          m=sprintf (el_str, "%0.2f", ep2); 
          write(fd3, el_str, 6); 
        }
      }
    }	
    ap3= ap2; 
    ep3= ep2; 

    // do 3 times per second

    usleep (330000);  
	
  }

  pthread_exit(NULL);

}
