// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////////////////
// KAV6H2MDV.C  A Simple Application to convert KAVOURAS 6 bit "High
//  Resolution Image format" data to MDV format data
//
// Frank Hage    Feb 1995 NCAR, Research Applications Program
///

#define KAV6H2MDV_MAIN

#include "kav6h2mdv.h"
#include <toolsa/ldata_info.h>
using namespace std;

void process_args( int argc, char *argv[]);
void convert_file(char * file_name);
int copy_headers(dcmp6h_header_t *k_head);
char *create_mdv_file_name(dcmp6h_header_t *k_head);
int write_mdv_file(dcmp6h_header_t *k_head);

void signal_trap( int sig);

//////////////////////////////////////////////////////////////////////
// MAIN: Process arguments, initialize and begin application
///
 
int main(int argc, char **argv)
{
    int i;
    int ret;
    time_t now = 0;       // The current time
    time_t last_tm = 0;   // The last time we converted files
    struct dirent *dp;
    DIR *dirp1;
    struct stat sbuf;
    struct stat dir_sbuf;
    char file_path[1024];
    char status_str[256];

    ZERO_STRUCT(&gd);    // Initialize the global data to all zeros
     
    process_args(argc,argv);    // process command line arguments

	PORTsignal(SIGINT,signal_trap);
	PORTsignal(SIGTERM,signal_trap);
	PORTsignal(SIGQUIT,signal_trap);
	PORTsignal(SIGPIPE,signal_trap);

	/* Initialize tha Process Mapper Functions */
	PMU_auto_init(gd.app_name,gd.app_instance,PROCMAP_REGISTER_INTERVAL);


    if(gd.cont_mode) {
      while(1) {
		 sprintf(status_str,"Checking %s",gd.input_dir);
		 PMU_auto_register(status_str);
         if(stat(gd.input_dir,&dir_sbuf) < 0) {
             if(errno == ENOENT) perror("Kav6h2mdv: Can't find input_directory");
             if(errno != ENOENT) perror("Kav6h2mdv: Can't stat input_directory");
           exit(-1);
        }
        if(dir_sbuf.st_mtime > last_tm) { // Examine directory for new entries
            now = dir_sbuf.st_mtime; // Since Kavouras often runs in the future pick
	                             //   The dir mod time as the "latest" time
            if((dirp1 = opendir(gd.input_dir)) == NULL) {
                perror("Kav6h2mdv: Can't open input_directory");
                exit(-1);
            }
            for(dp = readdir(dirp1); dp != NULL; dp = readdir(dirp1)) { // loop through each entry
               if(strstr(dp->d_name,gd.match_string) != NULL) {       // see if the name contains a match
                   sprintf(file_path,"%s/%s",gd.input_dir,dp->d_name);
                    
                   if((ret = stat(file_path,&sbuf)) < 0) {
		        fprintf(stderr,"Stat returned: %d on file %s\n",ret,file_path);
		        continue; // skip this file on problem stat
		   }

                   // If the file is newer than the last time we looked AND it is at least a few seconds old
                   // Convert the file
                   if((sbuf.st_mtime + QUESCENT_SECS) >=  last_tm && 
                        (sbuf.st_mtime + QUESCENT_SECS) < now) {
							sprintf(status_str,"Converting %s",file_path);
							PMU_force_register(status_str);
                            convert_file(file_path);
                    }
               }
            }
            closedir(dirp1);
            last_tm = now;
        }
        sleep(CHECK_SECS);
      } // End of infinite loop */
    } else {
        for(i=0; i < gd.nfiles; i++)  convert_file(gd.f_name[i]);
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////
// SIGNAL_TRAP : Traps Signals so as to die gracefully
//
void
 signal_trap( int sig)
 {
	 fprintf(stderr,"Caught Signal %d\n",sig);
	 PMU_auto_unregister();
	 exit(0);
 }


#define ARG_OPTION_STRING   "bc:d:D:i:mM:o:s:"
//////////////////////////////////////////////////////////////////////
// PROCESS_ARGS: Progess command line arguments. Set option flags
//       And print usage info if necessary
///

void process_args( int argc, char *argv[])
{
    int err_flag =0;
    int     c,fname_start;
    extern  char *optarg;   // option argument string
     
    gd.nfiles = argc -1;
    gd.field = -1;
    gd.plane = -1;
    gd.compress = 0;
    gd.cont_mode = 0;
    gd.x1 = -1;
    gd.y1 = -1;
    gd.x2 = -1;
    gd.y2 = -1;
    gd.f_name = &argv[1];
    gd.output_dir = ".";
    gd.input_dir = ".";
    gd.match_string = ".";
    gd.suffix = "mdv";
	gd.app_name = argv[0];
	gd.app_instance = "Generic";
    fname_start = 1;
     
    while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
        switch(c) {
            case 'b':   // turn on debug mode
              gd.debug = 1;
              gd.nfiles -= 1;
              fname_start++;
            break;

            case 'c':   // Specify the clipping region
              sscanf(optarg,"%d,%d,%d,%d",&gd.x1,&gd.y1,&gd.x2,&gd.y2);
              fname_start += 2;
              gd.nfiles -= 2;
            break;

            case 'd':   // Specify a directory to leave data in
              gd.output_dir = optarg;
              fname_start += 2;
              gd.nfiles -= 2;
            break;
         
            case 'D':   // Specify a directory to monitor
              gd.input_dir = optarg;
              gd.cont_mode = 1;
            break;
         
         
            case 'i':   // Specify a instance name 
              gd.app_instance = optarg;
			  fname_start += 2;
			  gd.nfiles -= 2;
            break;
         
            case 'M':   // Data files must contin this string in continious mode
              gd.match_string = optarg;
              gd.cont_mode = 1;
            break;
         
            case 'm':   // turn on compress(minimize) mode
              gd.compress = 1;
              gd.nfiles -= 1;
              fname_start++;
            break;

            case 'o':   // origin information
              if(sscanf(optarg,"%lf,%lf,%lf",
                 &gd.origin_lat,&gd.origin_lon,&gd.origin_alt) != 3) {
                  err_flag++;
              }
              fname_start += 2;
              gd.nfiles -= 2;
            break;

            case 's':   // Specify an alternate data file suffix 
              gd.suffix = optarg;
              fname_start += 2;
              gd.nfiles -= 2;
            break;

            case '?':   // error in options
            default:
                err_flag++;
            break;
        }
 
    };

    // sanity check on files to process
    if(gd.cont_mode) {
    if(strlen(gd.match_string) < 1) err_flag++;
    } else {
        gd.f_name = &argv[fname_start];
        if(fname_start >= argc) err_flag++;
    }

    // sanity check on clipping coordinates
    if(gd.x1 < 0 ||  gd.y1 < 0 || gd.x2 < 0 || gd.y2 < 0 ) err_flag++; // must be set
    if(gd.x1 >= COLOR_BAR_POS) gd.x1 = COLOR_BAR_POS -1;  // No sense getting blank data
    if(gd.x2 >= COLOR_BAR_POS) gd.x2 = COLOR_BAR_POS -1;
    if(gd.y1 >= NUM_6H_ROWS) gd.y1 = NUM_6H_ROWS -1;
    if(gd.y2 >= NUM_6H_ROWS) gd.y2 = NUM_6H_ROWS -1;
    if(gd.x2 <= gd.x1 || gd.y2 <= gd.y1 )  err_flag++;    // Make user put into proper order

    if(err_flag) {
        fprintf(stderr,"Usage:kav6h2mdv -c x1,y1,x2,y2 [-o lat,lon,alt] [-b] [-m]\n");
        fprintf(stderr,"\t[-s output_suffix] [-d output_data_dir] [kavouras_file_name kavouras_file_name ...] OR \n");
        fprintf(stderr,"\t[-D input_dir -M match_string ]\n");
        fprintf(stderr,"\n\t-b: Turn On deBugging info \n");
        fprintf(stderr,"\t-c: Clip to these grid coords, inclusive \n");
        fprintf(stderr,"\t-d: Directory to leave new files in; Default = '.' \n");
        fprintf(stderr,"\t-D: Directory to Monitor; Default = '.' \n");
        fprintf(stderr,"\t-m: Turn On compression (minimize data) \n");
        fprintf(stderr,"\t-M: Turns on monitor directory mode looking for new files that contiin this string\n");
        fprintf(stderr,"\t-o: Overrides the origin in the kavouras files (alt in km) \n");
        fprintf(stderr,"\t-s: File suffix to use on output; Default = '.mdv' \n");
        fprintf(stderr,"\n-Note: this program runs in either in a continious monitoring mode,\n");
        fprintf(stderr,"checking the input data dir every %d seconds for new files,\n",CHECK_SECS);
        fprintf(stderr,"when using the -M and the optional -D arguments OR avoid these arguments\n");
        fprintf(stderr,"and use an explicit list of files as the last arguments on the command line\n");
        exit(-1);
    }
}  
 
//////////////////////////////////////////////////////////////////////*
// CONVERT_FILE: 
///

void convert_file(char * file_name)
{
    int i;
    FILE *in_file;
    u_char *kav_plane;
    u_char *glob_ptr;
    u_char *local_ptr;

    if(gd.debug) fprintf(stderr,"\nConverting file: %s\n",file_name);
     
    // READ Kavouras Image File into internal buffers
    if((in_file = fopen(file_name,"r")) == NULL) {
        fprintf(stderr,"\nProblems opening file: %s\n",file_name);
        perror("KAV6H2MDV");
        return;
    } 

    if((gd.k_head = dcmp6h_get_header(in_file)) == NULL) {
        fprintf(stderr,"\nProblems reading header in file: %s\n",file_name);
        perror("KAV6H2MDV");
        return;
    } 

//    gd.k_head->scan_time = (gd.k_head->scan_time >> 16) | (( gd.k_head->scan_time & 0x0000ffff) << 16); 
     
    if((kav_plane = dcmp6h_get_image(in_file)) == NULL) {
        fprintf(stderr,"\nProblems reading image in file: %s\n",file_name);
        perror("KAV6H2MDV");
        return;
    } 

    if(fclose(in_file) != 0) {
        fprintf(stderr,"\nProblems closing file: %s\n",file_name);
        perror("KAV6H2MDV");
        return;
    }

    // reorder the data 
    local_ptr = kav_plane + ((NUM_6H_ROWS -1) * NUM_6H_COLS); // set pointer to last row of Kav image
    glob_ptr = gd.k_image;  // Set pointer to beginning of programs image area.
    for(i=0; i < NUM_6H_ROWS; i++) {
    memcpy(glob_ptr,local_ptr,NUM_6H_COLS);
    local_ptr -= NUM_6H_COLS;
    glob_ptr += NUM_6H_COLS;
    } 

    if(copy_headers(gd.k_head) < 0) {
       fprintf(stderr,"Warning: Copy Header elements error:\n");
       return;
    }

    if(write_mdv_file(gd.k_head) < 0) {
       fprintf(stderr,"Warning: write_mdv_file error!\n");
    }
}

//////////////////////////////////////////////////////////////////////
// COPY_HEADERS: Copy elements of a Dcmp6H format header to the proper Gint
//  header elements 
///

int copy_headers(dcmp6h_header_t *k_head)
{
    if(fill_mdv_master_header(k_head, &gd.mh) < 0) return -1;
    if(fill_mdv_field_header(k_head, &gd.mh, &gd.fh) < 0) return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////
// CREATE_MDV_FILE: Create a file name for mdv data in the appropriate sub
// directory . Make sure the daily subdirectory exists.
///

char *create_mdv_file_name(dcmp6h_header_t *k_head)
{
     
    long ftime,cur_time;              // file time,current time
    struct stat dir_sbuf;
    char format_str[1024];
    char dir_name[1024];
    struct tm *tm;

    static char fname[1024];

    cur_time = time(0);
    ftime = (k_head->scan_date -1) * 86400 + (k_head->scan_time_hi * 65536) + k_head->scan_time_lo;

    // Sanity check on date in the file
    if((ftime - cur_time) > 3600) return NULL;
     
    tm = gmtime(&ftime);  // Break out the time into year, mon, day , etc

    // build the directory name - output_dir/YYYYMMDD/ 
    strncpy(format_str,gd.output_dir,1024);
    strncat(format_str,"/%Y%m%d",1024);
    strftime(dir_name,1024,format_str,tm);


   // Check for the existance of the directory
   errno = 0;
   if(stat(dir_name,&dir_sbuf) < 0) {
       if(errno == ENOENT) { // No directory found
           // Create one
          if(mkdir(dir_name,0775) < 0) perror("kav6h2mdv: Can not create output dir");
       } else {
           perror("kav6h2mdv:");
           exit(-1);
     }

   }

   // Build output filename - HHMMSS.suffix
   strncpy(format_str,gd.output_dir,1024);
   strncat(format_str,"/%Y%m%d/%H%M%S.",1024);
   strncat(format_str,gd.suffix,1024);
   strftime(fname,1024,format_str,tm);

   return fname;
}

#define M_SEC_KTS 0.51479027  // Meters per second per kt/hr
//////////////////////////////////////////////////////////////////////
// WRITE_MDV_FILE: Open the File, Write out the headers,
// Then go through each data plane, put in order and write it out.
// Close the file.
///

int write_mdv_file(dcmp6h_header_t *k_head)
{
    int i,k;
    int nx,ny;
    int x_pos;
    int num_bad;
    int color_offset;
    int color_index;
    int plane_size;
    int field_data_offset;
    u_char *out_ptr;
    u_char *plane_ptr;    // Pointer to start of dobson plane
    u_char *buf_ptr;    // Pointer to memory buffer
    double scale,bias;
    double dval;
    double interval;
    double factor;
    u_char byte_val[16];
    char *fname;
    char tmp_fname[128];
    char cmd_string[2048];
    FILE * mdv_file;
     
    // Straighten out the data thresholds in header
    for(i=1; i < 16; i++) {
        factor = 1.0;
        if(k_head->data_level[i] & (1<< 12)) factor *= 10.0;
        if(k_head->data_level[i] & (1<< 8)) factor *= -1.0;
        k_head->data_level[i] =
	  (short) ((k_head->data_level[i] & 0xff) *  factor);
    }
     
    // compute the interval between bins.
    interval = ((k_head->data_level[15] - k_head->data_level[1]) / 14.0);

    // compute a scale, bias based on the dynamic range of the data
    scale =   interval / 10.0;  // set bins 10 bytes apart to avoid rounding problems and Art's wrath.
    bias =  k_head->data_level[1] - interval;

    // precompute each u_char value for each data level
    for(i=1; i <= 15; i++) {
        dval = ((double) k_head->data_level[i]);
        byte_val[i-1] = (int) ((dval - bias) / scale + 0.5);
    }

    //  Make sure the daily dir exists and get the real file name
    if((fname = create_mdv_file_name(k_head)) ==  NULL) {
        fprintf(stderr,"Problem creating %s - Aborting conversion\n",fname);
        return -1;
    }
    strncpy((char *) gd.mh.data_set_name,fname,80);
    if(gd.debug) fprintf(stderr,"New File name: %s\n",fname);

    // Build a unique temporary file name
    sprintf(tmp_fname,"/tmp/kav6h2mdv%d", (int) getpid());

   // Open Output file
   errno = 0;
   if((mdv_file = fopen(tmp_fname,"w")) == NULL) {
       perror("kav6h2mdv: Couldn't open output file");
   }

   nx = gd.fh.nx;
   ny = gd.fh.ny;
   plane_size = nx * ny;

    // Allocate space for the sub plane
    if((buf_ptr = (unsigned char *) calloc(1,plane_size)) == NULL) {
       fprintf(stderr,"Memory Error- Aborting conversion\n");
       fclose(mdv_file);
    return -3;
    }

    // Take care of differences based on field type
    switch(k_head->product_code) {
    case 16:   // Base Reflectivity
    case 17: 
    case 18: 
    case 19: 
    case 20: 
    case 21: 
    case 35:  // Composite Reflectivity
    case 36: 
    case 37: 
    case 38: 
    case 63:  // Layer composite reflectivity
    case 64: 
    case 65: 
    case 66: 
    case 89: 
    case 90: 
    color_offset = 49;
    break;
     
    case 22:  // Base Velocity
    case 23: 
    case 24: 
    case 25: 
    case 26: 
    case 27: 
    scale *= M_SEC_KTS;   // Convert to m/sec
    bias *= M_SEC_KTS;
    color_offset = 33;
    break;
     
    default:  // For now - The NEXRAD base Reflectivity
    color_offset = 49;
    break;
    }
    
    // Extract the sub plane in the correct ordering, as UINT8's
    plane_ptr = gd.k_image;
    out_ptr = buf_ptr;
    num_bad = 0;
    x_pos = 1;
   
    plane_ptr += gd.x1 + (gd.y1 * NUM_6H_COLS) ;  // Index into original grid
    for(k=plane_size; k--; ) {    // Loop through Each grid cell in clipped region

        color_index = *plane_ptr - color_offset;
        if(color_index < 0 || color_index > 15 ) { // 
           num_bad++;
           *out_ptr++ = (unsigned int) gd.fh.bad_data_value;
        } else {
            *out_ptr++ = byte_val[color_index];
        }
        // Key off X position in grid for clipping
        if(x_pos >= nx) { // At the right edge of our clip region
            x_pos = 1;
            plane_ptr += (NUM_6H_COLS - nx) +1;  // move to start of next line
        } else {
            x_pos++;
            plane_ptr++;
        }
    }

    // set some values before writing out data
    gd.fh.volume_size = plane_size;
    gd.fh.scale = scale;
    gd.fh.bias = bias;


    // Output master header
    if(MDV_write_master_header(mdv_file,&gd.mh) != MDV_SUCCESS) {
        fprintf(stderr,"Write Master Header Error- Aborting conversion\n");
        fclose(mdv_file);
        return -4;

    }
     
    // Output field data 

    field_data_offset = sizeof(MDV_master_header_t) +
                        sizeof(MDV_field_header_t) +
                        sizeof(gd.mh.record_len1); 


    if(gd.compress) {  // Compress data plane if requested

       if ((MDV_write_field(mdv_file, &gd.fh, (void *)buf_ptr, 0, 
               field_data_offset,MDV_PLANE_RLE8)) <= 0 ) {

          fprintf(stderr,"Write Plane Error- Aborting conversion\n");
          fclose(mdv_file);
          return -4;

        }
     }
     else {  // No compression

       if (MDV_write_field(mdv_file,&gd.fh, (void *)buf_ptr, 0, 
              field_data_offset,MDV_INT8) != plane_size ) {

         fprintf(stderr,"Write Plane Error- Aborting conversion\n");
         fclose(mdv_file);
         return -4;
        }
     }

    free(buf_ptr);
    fclose(mdv_file);


    // relink file to the correct name
    sprintf(cmd_string,"mv %s %s\n",tmp_fname,fname);
    system(cmd_string);

    { // for local varaiables 

    // initialize the latest data info handle
    static int ldata_init = FALSE;
    static LDATA_handle_t ldata;
    if (!ldata_init) {
      LDATA_init_handle(&ldata, gd.app_name, gd.debug);
      ldata_init = TRUE;
    }

    // Update the index file
    long ftime; // file time
    ftime = (k_head->scan_date -1) * 86400 +
      (k_head->scan_time_hi * 65536) + k_head->scan_time_lo;

    LDATA_info_write(&ldata, gd.output_dir, ftime, gd.suffix,
		     "MDV format NIDS Data", "From KAVOURAS data",
		     0, NULL);

    }
    return 0;
}


