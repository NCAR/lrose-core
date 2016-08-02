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
//////////////////////////////////////////////////////////
// Wevent.cc
//
//////////////////////////////////////////////////////////

#include <cstring>
#include "Wevent.hh"
#include "toolsa/udatetime.h"


//////////////////////////////////////////
// buffer-based constructor

Wevent::Wevent(char *buffer)
  
{
    char *start_text;
	char *title_text;
	char *notes_text;
	char *nl_ptr;
	char buf[1024];

	// Must have something for older files
	title = "Untitled";
	notes = "No Notes";

	memset(buf,0,1024);
	if((title_text = strstr(buffer,"Title:")) != NULL) {
		 title_text += 6; // move past "Title:"
		 if((nl_ptr = strchr(title_text,10)) != NULL) { // search for a new line
		     strncpy(buf,title_text,(nl_ptr-title_text));
		 }
		 title = buf;
	}

	memset(buf,0,1024);

	if((notes_text = strstr(buffer,"Notes:")) != NULL) {
		 notes_text += 6; // move past "Notes:"
		 if((nl_ptr = strchr(notes_text,10)) != NULL) { // search for a new line
		     strncpy(buf,notes_text,(nl_ptr-notes_text));
		 }
		 // Replace Hash with Newline.
		 for(int j= 0; j < (int) strlen(buf); j++) {
			 if(buf[j] == '#') buf[j] = '\n';
		 }
		 notes = buf;

	}

	memset(buf,0,1024);
	if((start_text = strstr(buffer,"\nstart ")) != NULL) {
		start_text++; // move past the newline.
		 if((nl_ptr = strchr(start_text,10)) != NULL) { // search for a new line
			 nl_ptr = '\0';
		 }

		date_time_t Start, Finish;
		if ( sscanf(start_text,
				"start %d/%d/%d %d:%d:%d end %d/%d/%d %d:%d:%d",
			&Start.year, &Start.month, &Start.day,
			&Start.hour, &Start.min, &Start.sec,
			&Finish.year, &Finish.month, &Finish.day,
			&Finish.hour, &Finish.min, &Finish.sec)
			== 12) {
				uconvert_to_utime(&Start);
				uconvert_to_utime(&Finish);
		  }

		start_time = Start.unix_time;
		end_time = Finish.unix_time;

	}
}
// Stand alone Constructor.
Wevent::Wevent(void)
  
{
	 time_t now = time(0);
	 char t_buf[1024];

	 strftime(t_buf,1024,"%b %d, %Y", gmtime(&now));

	 title = t_buf;
	 notes = "None";

	 // Round to even 15 minutes
	 start_time = now - (now % 900);
	 end_time = start_time + 86400;
}

//////////////
// destructor
  
Wevent::~Wevent()
{
}

//////////////
// TOASCII : output the event in proper Ascii file form.
  
void Wevent::toAscii(FILE *outfile)
{
    struct tm e_gmt;
    struct tm s_gmt;

	char part1_buf[128];
	char part2_buf[128];
	char n_buf[65536];
	 
    gmtime_r(&start_time,&s_gmt);
    gmtime_r(&end_time,&e_gmt);

    fprintf(outfile,"###############################################################\n");
	fprintf(outfile,"# Title:%s\n",title.c_str());
	strncpy(n_buf,notes.c_str(),65536);
	// replace newlines with hashes.
	for(int j= 0; j < (int) strlen(n_buf); j++) {
	   if(n_buf[j] == '\n') n_buf[j] = '#';
	}
	fprintf(outfile,"# Notes:%s\n",n_buf);
    fprintf(outfile,"#\n");

	strftime(part1_buf,128,"start %Y/%m/%d %T ",&s_gmt);
	strftime(part2_buf,128,"end %Y/%m/%d %T",&e_gmt);
    fprintf(outfile,"%s    %s\n",part1_buf,part2_buf);

}
