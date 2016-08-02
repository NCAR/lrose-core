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
////////////////////////////////////////////////////////////////////////////////
//
//


#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rapformats/station_file.h>
#include <toolsa/Except.hh>
#include <toolsa/file_io.h>
using namespace std;

#define FILE_BUF_SIZE  1024	// size of temporary buffer
#define TIME_FIELD_LEN  10	// size of time string
#define MAX_INT 2147483647  
////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS

Station_file::Station_file(const char *fname, const char *mode)
{
	int cur_pos;		// current position in file
	int  cur_index;         // current line count of data
	int  cur_len;		// current length of data in buffer
	char conv_buf[TIME_FIELD_LEN+1];	// space to read time string into
	char buf[FILE_BUF_SIZE]; // data buffer grep 

	#ifdef DBG
	  printf("Station File Constructor\n");
	#endif
	if(ta_stat(fname,&sbuf) < 0) THROW(errno,"Couldn't stat station file\n");
	if((file = fopen(fname,mode)) == NULL) THROW(errno,"Couldn't open station file\n");

	// First pass through file - Count lines, finds max length of data lines
	num_file_lines = 0;
	max_line_len = 0;
	num_allocated_lines = 0;
	while(fgets(buf,FILE_BUF_SIZE,file) != NULL) {
	   if(buf[0] != '!') {	  // Ignore Comment lines
               num_file_lines++;
               cur_len = strlen(buf) + 1;
               if(cur_len > max_line_len) max_line_len = cur_len;
	   }
	}

	max_line_len += ((4 - ( max_line_len & 3 )) & 3);	// create nice strings on word boundies 

	// Allocate memory for index arrays.
	line_index = new long[num_file_lines];
	time_index = new time_t[num_file_lines];
	line = new char*[num_file_lines];
	#ifdef DBG
	  printf("Allocated %d  new index elements - 1 per file line\n", num_file_lines);
	  printf("&line_index =  %x  &time_index = %x &line array:  = %x\n", line_index,time_index,line);
        #endif

	// second pass builds indexs.
	if(fseek(file,0,0) < 0)  THROW(errno,"Couldn't fseek station file\n");

	cur_index = 0;
	cur_pos = 0;	
	 
	while(fgets(buf,FILE_BUF_SIZE,file) != NULL) {
	     if(buf[0] != '!') {	// reject any lines that start with !
                 strncpy(conv_buf,buf,TIME_FIELD_LEN);	// These files have the time field first
	         conv_buf[TIME_FIELD_LEN] = '\0';       // make sure string is terminated

	         time_index[cur_index] = atoi(conv_buf);	// We know that 0 is not a valid time;
                 line_index[cur_index]  = cur_pos;		
	         cur_index++;
	     }
             cur_pos += strlen(buf);	// keep track of where we are in the file 
         }


}

////////////////////////////////////////////////////////////////////////////////
// SET Functions 

////////////////////////////////////////////////////////////////////////////////
// ACCESS Functions 

void Station_file::get_line(time_t time, char*& ptr_ref)
{
    int i;
    int	dist;
    int	file_index = 0;

    dist = MAX_INT;
    for(i=0; i < num_file_lines; i++) {
        if(abs(time - time_index[i]) < dist) {
             dist = abs(time - time_index[i]);
             file_index = line_index[i];
	 }
    }

    // Make sure we have enough lines allocated for the data 
    if(num_allocated_lines <= 0) {
        line[0] = new char[max_line_len];
        num_allocated_lines = 1;
	#ifdef DBG
	  printf("Allocated one line\n");
        #endif
    }

     if(fseek(file,file_index,0) < 0)  THROW(errno,"Couldn't fseek station file\n");
     if(fgets(line[0],max_line_len,file) == NULL) THROW(errno,"Couldn't read station file\n");

     ptr_ref = line[0];
}

void Station_file::get_last_line(char*& ptr_ref)
{

    // Make sure we have enough lines allocated for the data 
    if(num_allocated_lines <= 0) {
        line[0] = new char[max_line_len];
        num_allocated_lines = 1;
	#ifdef DBG
	  printf("Allocated one line\n");
        #endif
    }

     if(fseek(file,line_index[num_file_lines-1],0) < 0)  THROW(errno,"Couldn't fseek station file\n");
     if(fgets(line[0],max_line_len,file) == NULL) THROW(errno,"Couldn't read station file\n");

     ptr_ref = line[0];
}

void Station_file::get_lines(time_t time_start,time_t time_end, char**& ptr_array_ref, int& num_lines)
{
    int i;
    int n_lines;
    int	first_line;
    int	last_line;

    first_line = -1;
    last_line = -1;
    for(i=0; i < num_file_lines; i++) {
        if(first_line < 0  && time_index[i]  >= time_start &&  time_index[i]  <= time_end) {
            first_line = i;
        } 
        if(time_index[i]  >= time_start &&  time_index[i]  <= time_end) {
            last_line = i; 
        } 
    }

    if(last_line < 0)  {	// NO VALID LINES IN FILE
      num_lines = 0;
      ptr_array_ref = (char **)NULL;
    } else {
      n_lines = last_line - first_line + 1;

      // Make sure we have enough lines allocated for the data 
      if(num_allocated_lines < n_lines) {
          for(i=num_allocated_lines; i < n_lines; i++) {
              line[i] = new char[max_line_len];
          }
	#ifdef DBG
	  printf("Allocated %d  new lines\n",(n_lines - num_allocated_lines));
        #endif
          num_allocated_lines = n_lines;
      }
      num_lines = n_lines;

      // read in all the lines;
      for(i= 0; i < n_lines; i++) {
	if(fseek(file,line_index[first_line+i],0) < 0)
	  THROW(errno,"Couldn't fseek station file\n");
         if(fgets(line[i],max_line_len,file) == NULL)
	   THROW(errno,"Couldn't read station file\n");
      }  

      ptr_array_ref = line;
    }
}


void Station_file:: get_last_nsec(int sec, char**& ptr_array_ref, int &num_lines)
{
    int i;
    int n_lines;
    int	first_line;
    time_t  target_time;

    target_time = time_index[num_file_lines -1] - sec;
    first_line = -1;
    for(i=0; i < num_file_lines; i++) {
        if(first_line < 0  && time_index[i]  >= target_time ) {
            first_line = i;
	    break;
        } 
    }

    if(first_line < 0) {	// No lines in file !?
      num_lines = 0;
      ptr_array_ref = (char **)NULL;
      return;
    }

    n_lines = num_file_lines - first_line;

    // Make sure we have enough lines allocated for the data 
    if(num_allocated_lines < n_lines) {
        for(i=num_allocated_lines; i < n_lines; i++) {
            line[i] = new char[max_line_len];
        }
	#ifdef DBG
	  printf("Allocated %d  new lines\n",(n_lines - num_allocated_lines));
        #endif
        num_allocated_lines = n_lines;
    }
    num_lines = n_lines;

    // read in all the lines;
    for(i= 0;  i < n_lines; i++) {
      if(fseek(file,line_index[first_line+i],0) < 0)
	THROW(errno,"Couldn't fseek station file\n");
      if(fgets(line[i],max_line_len,file) == NULL)
	THROW(errno,"Couldn't read station file\n");
    }  

    ptr_array_ref = line;
}


////////////////////////////////////////////////////////////////////////////////
// MODIFY Functions 

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTOR


Station_file::~Station_file()
{
	int i;
	#ifdef DBG
	  printf("Station File Destructor\n");
        #endif

	if((fclose(file)) == EOF) THROW(errno,"Couldn't close station file\n");
	
	#ifdef DBG
	  printf("Deleting %d  lines and  index aryays\n", num_allocated_lines);
	  printf("&line_index =  %x  &time_index = %x &line array: %x\n", line_index,time_index,line);
        #endif

	// Deallocate memory for index arrays.
	delete []  line_index;
	delete [] time_index;

	// delete arrays of line data
	for(i = 0; i < num_allocated_lines; i++) {
	    delete []  line[i];
	}
	delete [] line;

}
