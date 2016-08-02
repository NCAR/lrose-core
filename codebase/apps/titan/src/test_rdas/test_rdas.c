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
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <toolsa/os_config.h>

#include "../rdas_dr/rdas.h"

typedef struct {
  char radar_site[64];
  char radar_type[64];
  int storage;
  int mds;
  int noise;
  int rvpchi;
  int rvpclo;
  int phi;
  int plo;
  int tx_power;
  float radar_constant; 
  float atmos_atten; 
  int site;
  float srate;
  int bins;
  int rays;
  int skip;
  int samples_per_bin;
  int cf_on_or_off;
  float bin_width;
  float max_range_out;
} rdas_site_t;

typedef struct {
  unsigned int rvpc_base;
  int rvpc_irq;
  int invert_ac_v;
  int el_slew_multiplier;
  float el_there_threshold;
  float az_rpm;
  int el_offset_bias;
  float az_corr;
  float el_corr;
  unsigned int tape_density_code;
  int tape_id;
  int tcp_port_a;
  int swap_bytes_socket_a;
  int tcp_port_b;
  int swap_bytes_socket_b;
  int trigger_alignment;
  int timeout_count;
} rdas_config_t;

#define DEG2COUNT 182.04444

static int read_rdas_code(char *code_path);
static int load_table(int rdas, char *table_path);
static int wait_for_table_xfer(int rdas);
static int read_site(char *site_path);
static int read_config(char *config_path);
static int load_rvpc_cfg(int rdas);
static void uusleep(ui32 usecs);
static void umsleep(ui32 msecs);

static ui32 *rdasCode;
static int nCodeWords;
static rdas_site_t Site;
static rdas_config_t Config;

int main(int argc, char **argv)

{

  int i;
  int iret;
  int rdas;
  ui16 p360c;    /* past 360 count */
  ui16 rayhdr[16];
  ui16 des_el;
  ui16 host_irq_ena;
  ui16 raydata[1024];
  float elevation;

  /*
   * read site file
   */

  if (read_site("../rdas_dr/params/rdas.site")) {
    return (-1);
  }

  /*
   * read config file
   */

  if (read_config("../rdas_dr/params/rdas.config")) {
    return (-1);
  }

  /*
   * open rdas device
   */
  
  if ((rdas = open("/dev/rdas", O_RDWR)) < 0) {
    fprintf(stderr, "Cannot open rdas device\n");
    perror("/dev/rdas");
    return (-1);
  }
  
  /*
   * set base port address
   */

  iret = ioctl(rdas, _RDAS_SET_BASE_ADDR, 0x300);
  if (iret) {
    perror("ioctl _RDAS_SET_BASE_ADDR");
    close(rdas);
    return (-1);
  }

  /*
   * set interrupt number
   */

  iret = ioctl(rdas, _RDAS_SET_IRQ_NUM, Config.rvpc_irq);
  if (iret) {
    perror("ioctl _RDAS_SET_IRQ_NUM");
/*      close(rdas); */
/*      return (-1); */
  }
  
  /*
   * read in code for rdas program
   */

  if (read_rdas_code("../rdas_dr/tables/spc_code")) {
    close(rdas);
    return (-1);
  }

  /*
   * set program size
   */

  iret = ioctl(rdas, _RDAS_SET_PROGRAM_SIZE, nCodeWords);
  if (iret) {
    perror("ioctl _RDAS_SET_PROGRAM_SIZE");
    close(rdas);
    free(rdasCode);
    return (-1);
  }
  
  /*
   * load program
   */

  fprintf(stderr, "-------------->> got this far\n");

  iret = ioctl(rdas, _RDAS_LOAD_PROGRAM, rdasCode);
  if (iret) {
    perror("ioctl _RDAS_LOAD_PROGRAM");
    close(rdas);
    free(rdasCode);
    return (-1);
  }

  fprintf(stderr, "-------------->> and this far\n");

  /*
   * read back the program code and check it
   */

  {
    ui16 checkCode[10000];
    ui16 msw, lsw;
    int code_word;
    int i;
    
    iret = ioctl(rdas, _RDAS_SET_ADDR_PROG, 0);
    if (iret) {
      perror("ioctl _RDAS_SET_ADDR_PROG");
      close(rdas);
      free(rdasCode);
      return (-1);
    }
    
    lseek(rdas, 0, 0);
    
    iret = read(rdas, checkCode, nCodeWords * 4);
    fprintf(stderr, "Read back %d bytes of code\n", iret);
    
    for (i = 0; i < nCodeWords; i++) {
      msw = checkCode[i*2];
      lsw = checkCode[i*2 + 1];
      code_word = msw << 8 | lsw;
      if (code_word != rdasCode[i]) {
	fprintf(stderr, "ERROR - %d: read %x, expected %x\n",
		i, code_word, rdasCode[i]);
      }
    } /* i */
  }

  free(rdasCode);

  /*
   * set to data read/write mode
   */

  iret = ioctl(rdas, _RDAS_SET_ADDR_DATA, 0);
  if (iret) {
    perror("ioctl _RDAS_SET_ADDR_DATA");
    close(rdas);
    return (-1);
  }

  /*
   * load tables
   */

  if (load_table(rdas, "../rdas_dr/tables/cf1.tbl")) { /* ch1 clutter */
    close(rdas);
    return (-1);
  }
  if (load_table(rdas, "../rdas_dr/tables/ch1.tbl")) { /* ch1 displace */
    close(rdas);
    return (-1);
  }
  if (load_table(rdas, "../rdas_dr/tables/cf2.tbl")) { /* ch2 clutter */
    close(rdas);
    return (-1);
  }
  if (load_table(rdas, "../rdas_dr/tables/ch2.tbl")) { /* ch2 displace */
    close(rdas);
    return (-1);
  }

  /*
   * load config
   */

  if (load_rvpc_cfg(rdas)) {
    return (-1);
  }

  /*
   * set the elevation angle to 0.5
   */

  elevation = 0.5;
  des_el = (ui16) (elevation * DEG2COUNT + 0.5);
  lseek(rdas, 17, 0);
  if (write(rdas, &des_el, sizeof(des_el)) != sizeof(des_el)) {
    fprintf(stderr, "ERROR - writing des_el\n");
    return (-1);
  }

  /*
   * enable the DSP to interrupt the PC
   */

  host_irq_ena = 1;
  lseek(rdas, 19, 0);
  if (write(rdas, &host_irq_ena, sizeof(host_irq_ena)) != sizeof(host_irq_ena)) {
    fprintf(stderr, "ERROR - writing host_irq_ena\n");
    return (-1);
  }

  /*
   * read
   */

  for (i = 0; i < 90; i++) {

    int  count = 0;

    /*
     * wait for DAV - this also clear the DAV flag
     */

    while (ioctl(rdas, _RDAS_CHECK_DAV, 0) != 0) {
      count++;
      uusleep(500);
    }
    fprintf(stderr, "slept %d times\n", count);

    /* read */
    
    lseek(rdas, 20, 0);
    iret = read(rdas, rayhdr, 32);
    
    if (iret != 32) {
      
      fprintf(stderr, "--> %d bytes read\n", iret);

    } else {

      p360c = rayhdr[2];

      if (p360c > 1024) {

	/*
	 * slew elevation up
	 */

	elevation += 1.0;
	des_el = (ui16) (elevation * DEG2COUNT + 0.5);

	lseek(rdas, 17, 0);
	if (write(rdas, &des_el, sizeof(des_el)) != sizeof(des_el)) {
	  fprintf(stderr, "ERROR - writing des_el\n");
	  return (-1);
	}

	/*
	 * set p360c to 0
	 */

	p360c = 0;
	lseek(rdas, 22, 0);
	if (write(rdas, &p360c, sizeof(p360c)) != sizeof(p360c)) {
	  fprintf(stderr, "ERROR - writing p360c\n");
	  return (-1);
	}

      } /*  if (p360c > 1024) */
      
    } /*  if (iret != 32) */

    lseek(rdas, 4152, 0);
    iret = read(rdas, raydata, 512);

    fprintf(stderr, "el: %10.3f, az: %10.3f, az_count: %d, %d %d %d\n",
	    (double) rayhdr[4] / DEG2COUNT,
	    (double) rayhdr[1] / DEG2COUNT,
	    rayhdr[2],
	    raydata[100], raydata[160], raydata[200]);

  } /* i */
    
  /*
   * disable the DSP to interrupt the PC
   */

  host_irq_ena = 0;
  lseek(rdas, 19, 0);
  if (write(rdas, &host_irq_ena, sizeof(host_irq_ena)) != sizeof(host_irq_ena)) {
    fprintf(stderr, "ERROR - writing host_irq_ena\n");
    return (-1);
  }

  /*
   * close device
   */

  close(rdas);

  return(0);

}

/******************************************
 * read_rdas_code
 */

static int read_rdas_code(char *code_path)

{

  int i;
  FILE *fp;
  int nlines = 0;
  char line[1024];
  
  if ((fp = fopen(code_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - cannot read in code file.\n");
    perror(code_path);
    return (-1);
  }

  while (fgets(line, 1024, fp) != NULL) {
    nlines++;
  }

  /*
   * ignore header - 3 lines
   */
  
  nCodeWords = nlines - 3;

  /*
   * alloc space for code
   */

  rdasCode = (ui32 *) malloc(nlines * sizeof(ui32));

  /*
   * space over 3 lines of header
   */

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 3; i++) {
    fgets(line, 1024, fp);
  }

  /*
   * load up code
   */

  nCodeWords = 0;
  for (i = 0; i < nlines - 3; i++) {
    fgets(line, 1024, fp);
    if (sscanf(line, "%x", &rdasCode[i]) != 1) {
      break;
    }
    nCodeWords++;
  }

  fclose(fp);

  return (0);

}

/***************************************************************************
 *
 * Load table into DSP
 */

static int load_table(int rdas, char *table_path)

{

  FILE *fp;
  int i, j;
  int table_val;
  int cf_table1_type;
  si16 table[1024];
  ui16 rvpc_command;

  /*
   * Open the table file
   */
  
  if((fp = fopen(table_path, "r")) == NULL) {
    fprintf(stderr, "Couldn't open table file '%s'\n", table_path);
    return (-1);
  }

  /*
   * first get the table type 
   */

  fscanf(fp, "%d", &cf_table1_type); 

  /*
   * read the table data in 8 blocks of 1024
   */
    
  for (i = 0; i < 8; i++) {

    for(j = 0; j < 1024; j++)  {
      fscanf(fp, "%d", &table_val);
      table[j] = (si16) table_val;
    }
    
    /*
     * load the card with table data at temp buffer in DSP
     */

    lseek(rdas, 5176, 0);
    if (write(rdas, table, 2048) != 2048) {
      fprintf(stderr, "ERROR - writing table '%s' to DSP\n",  table_path);
      return (-1);
    }

    /*
     * let the spc know  table data has been loaded
     * by writing a 0 to mailbox 16351
     */
    
    rvpc_command = 0;
    lseek(rdas, 16351, 0);
    if (write(rdas, &rvpc_command, sizeof(ui16)) != sizeof(ui16)) {
      fprintf(stderr, "ERROR - writing load command for table '%s' to DSP\n",
	      table_path);
      return (-1);
    }

    if (wait_for_table_xfer(rdas)) {
      return (-1);
    }

  }

  fclose(fp);

  return (0);

 }

/***************************************************************************/

static int wait_for_table_xfer(int rdas)

{

  int  count = 0;
  ui16 finished = 0;

  /*
   * poll spare9 to see when transfer from internal mem to external
   * CMem is complete
   */
  
  while(!finished) {
    lseek(rdas, 35, 0);
    if (read(rdas, &finished, sizeof(ui16)) != sizeof(ui16)) {
      fprintf(stderr, "ERROR - reading spare9 for table copy to finish.\n");
      return (-1);
    }
    count++;
    if (count > 1000) {
      fprintf(stderr, "ERROR - too many tries waiting for table copy to finish.\n");
      return (-1);
    }
  }

  return (0);

}


/****************************************
 * read site file
 */

static int read_site(char *site_path)

{

  FILE *fp;
  
  /*
   * Open the site file
   */

  if ((fp = fopen (site_path, "r")) == NULL) {
    fprintf (stderr, "Couldn't open site file '%s'\n", site_path);
    return (-1);
  }
  
  fscanf (fp, "%s", Site.radar_site);
  fscanf (fp, "%s", Site.radar_type);
  fscanf (fp, "%d", &Site.storage);
  fscanf (fp, "%d", &Site.mds);
  fscanf (fp, "%d", &Site.noise);
  fscanf (fp, "%d", &Site.rvpchi);    
  fscanf (fp, "%d", &Site.rvpclo);    
  fscanf (fp, "%d", &Site.phi);       
  fscanf (fp, "%d", &Site.plo);       
  fscanf (fp, "%d", &Site.tx_power);
  fscanf (fp, "%f", &Site.radar_constant); 
  fscanf (fp, "%f", &Site.atmos_atten); 
  fscanf (fp, "%d", &Site.site);

  fscanf (fp, "%f", &Site.srate);
  fscanf (fp, "%d", &Site.bins);
  fscanf (fp, "%d", &Site.rays);
  fscanf (fp, "%d", &Site.skip);
  fscanf (fp, "%d", &Site.samples_per_bin);

  if (fscanf (fp, "%d", &Site.cf_on_or_off) != 1) {
    fprintf (stderr, "Couldn't read site file '%s'\n", site_path);
    fclose(fp);
    return (-1);
  }

  Site.bin_width = Site.srate * Site.samples_per_bin;
  Site.max_range_out = (Site.samples_per_bin * 20.0) * Site.srate;

  fclose (fp);

  return (0);

}

/****************************************
 * read config file
 */

static int read_config(char *config_path)

{

  FILE *fp;
  
  /*
   * Open the config file
   */

  if ((fp = fopen (config_path, "r")) == NULL) {
    fprintf (stderr, "Couldn't open config file '%s'\n", config_path);
    return (-1);
  }
  
  fscanf (fp, "%X", &Config.rvpc_base);
  fscanf (fp, "%d", &Config.rvpc_irq);
  fscanf (fp, "%d", &Config.invert_ac_v);
  fscanf (fp, "%d", &Config.el_slew_multiplier);    
  fscanf (fp, "%f", &Config.el_there_threshold);    
  fscanf (fp, "%f", &Config.az_rpm);       
  fscanf (fp, "%d", &Config.el_offset_bias);       
  fscanf (fp, "%f", &Config.az_corr); 
  fscanf (fp, "%f", &Config.el_corr); 
  fscanf (fp, "%x", &Config.tape_density_code);
  fscanf (fp, "%d", &Config.tape_id);
  fscanf (fp, "%d", &Config.tcp_port_a);
  fscanf (fp, "%d", &Config.swap_bytes_socket_b);
  fscanf (fp, "%d", &Config.tcp_port_b);
  fscanf (fp, "%d", &Config.swap_bytes_socket_a);
  fscanf (fp, "%d", &Config.trigger_alignment);

  if (fscanf (fp, "%d", &Config.timeout_count) != 1) {
    fprintf (stderr, "Couldn't read config file '%s'\n", config_path);
    fclose(fp);
    return (-1);
  }

  fclose (fp);

  return (0);

}

/****************************************
 * load the config into the DSP
 */

static int load_rvpc_cfg(int rdas)

{

  ui16 rvpc_config[16];
  ui16 rvpc_command;

  /*
   * 00 srate
   * 01 bins
   * 02 spare
   * 03 skip
   * 04 rays
   * 05 spare
   * 06 ch1 samples per bin
   * 07 ch2 samples per bin
   * 08 ch1 cf on or off
   * 09 ch2 cf on or off
   * 10 ch1  on or off
   * 11 ch2  on or off
   * 12 invert antenna control DAC output
   * 13 el slew roll off multiplier
   * 14 el "THERE" threshold
   * 15 spare
   */

  /*
   * get all variables form site table into rvpc_config array
   * all spare or unused variables must be set to 0
   */
  
  rvpc_config[0]  = (int)(Site.srate *10); /* card needs srate divisor for counter */
  rvpc_config[1]  = Site.bins;
  rvpc_config[2]  = 0;                 /* unused */
  rvpc_config[3]  = Site.skip + Config.trigger_alignment;
  rvpc_config[4]  = Site.rays;
  rvpc_config[5]  = 0;                 /* unused */
  rvpc_config[6]  = Site.samples_per_bin;   /* ch1 */
  rvpc_config[7]  = Site.samples_per_bin;   /* ch2 */
  rvpc_config[8]  = Site.cf_on_or_off;      /* channel 1 cf  */
  rvpc_config[9]  = 0;  /* channel 2 cf  */
  rvpc_config[10] = 1;  /* channel 1 on  */
  rvpc_config[11] = 0;  /* channel 2 off */ 
  rvpc_config[12] = Config.invert_ac_v;        /* invert ac v  */
  rvpc_config[13] = Config.el_slew_multiplier; /* el roll off multiplier */ 
  rvpc_config[14] = (int)(Config.el_there_threshold/0.005); /* el there threshold value */
  rvpc_config[15] = Config.el_offset_bias; /* elevation offset bias */

  /*
   * load up the config array
   */

  lseek(rdas, 0, 0);
  if (write(rdas, rvpc_config, sizeof(rvpc_config)) != sizeof(rvpc_config)) {
    fprintf(stderr, "ERROR - writing config data to DSP\n");
    return (-1);
  }

  /*
   * let the spc know the config data has been loaded
   * by writing a 0 to mailbox 16351
   */
  
  rvpc_command = 0;
  lseek(rdas, 16351, 0);
  if (write(rdas, &rvpc_command, sizeof(ui16)) != sizeof(ui16)) {
    fprintf(stderr, "ERROR - writing load command for config data DSP\n");
    return (-1);
  }

  return (0);

}

/*
 * sleep in micro-seconds
 */

static void uusleep(ui32 usecs)

{

  struct timeval sleep_time;
  fd_set read_value;

  sleep_time.tv_sec = usecs / 1000000;
  sleep_time.tv_usec = usecs % 1000000;

  memset ((void *)  &read_value,
          (int) 0, (size_t)  (sizeof(fd_set)));

  select(30, FD_SET_P &read_value, FD_SET_P 0,
	 FD_SET_P 0, &sleep_time);

}

/*
 * sleep in milli-seconds
 */

static void umsleep(ui32 msecs)

{
  uusleep(msecs * 1000);

}
