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
////////////////////////////////////////////////////////////////////////////////
// STATION_FILE.H:  Class to provide support for ASCII line oriented mseonet station
// files
//
//

#include <stdio.h>
#include <math.h>
#include <string.h>

class Station_file {
protected:
        int  num_file_lines;	// number of data lines in each file
        int  max_line_len;	// Size of the larges line in the file
        int  num_allocated_lines;	// memory buffer size for the lines
        long *line_index;       // File positions for  each line
        time_t *time_index;     // Time of data measurement for each line
        char   **line;           // Array of pointers to data lines from the file
	FILE *file;	
	struct stat sbuf;

public:
 // constructors
   Station_file(const char *fname, const char *mode = "rw");

 // Set functions

 // Access functions

    // The pointers the following functions provide are only valid until any of these get routines
    // are called again. Users should copy the data if necessary.

    // return a pointer to the data line in the file closest to time
    void   get_line(time_t time, char*& ptr_ref);

    // return a pointer to the last data line in the file 
    void   get_last_line(char*& ptr_ref);

    // return a pointer to an array of data lines in the file  between times
    void  get_lines(time_t time_start,time_t time_end, char**& ptr_array_ref, int& num_lines);

    // Returns the latest N seconds of data from the file
    void  get_last_nsec(int sec, char**& ptr_array_ref, int& num_lines);

 // Modifiers

 // destructors
    ~Station_file();
 	 
};
