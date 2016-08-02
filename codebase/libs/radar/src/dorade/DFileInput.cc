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
//
// Implementation of the data file input class.
//
# include <unistd.h>
# include <radar/dorade/DataInput.hh>

DFileInput::DFileInput (const char *file, int zorch)
//
// Create a data file input instance, starting with the given sweep file.
//
{
	SFile = new dd_sweepfile_access;
	OK = SFile->access_sweepfile ((char *) file, Mapper);
	FirstBeam = 1;
	if ((Zorch = zorch))
		unlink (file);
}




DFileInput::DFileInput (int zorch)
//
// Create a DFileInput thingy, but without opening an initial file.
//
{
	SFile = new dd_sweepfile_access;
	OK = 0;
	Zorch = zorch;
}





DFileInput::~DFileInput ()
//
// Get rid of this thing.
//
{
	delete SFile;
}




int
DFileInput::NewFile (const char *file)
//
// Open up a new file
//
{
	OK = SFile->access_sweepfile ((char *) file, Mapper);
	if (Zorch && OK)
		unlink (file);
	return (OK);
}




dd_mapper *
DFileInput::get_beam ()
//
// Return a mapper holding the next beam in the sweep file, or NULL once
// we hit the end.  This guy should really return a const pointer, but
// the methods of dd_mapper aren't set up for that.
//
{
//
// The first beam gets read when we open the sweepfile, so don't read
// another one here.
//
	if (FirstBeam)
	{
		FirstBeam = 0;
		return (Mapper);
	}
//
// The rest of the time we read something.
//
	int ret = SFile->next_ray ();
	if (ret == LAST_RAY || ret == READ_FAILURE)
	{
		OK = 0;
		return (0);
	}
	return (Mapper);
}
