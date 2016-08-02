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
//
// Program to produce an XML file describing header information
// from a command line specified MDV file. If a second command line
// argument is specified, it is taken to be a subdirectory to
// create and write the XML file to.
//
// Niles Oien October 2006
//
//
//

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>  
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <toolsa/file_io.h>

using namespace std;

int main(int argc, char *argv[])
{


  char FileName[1024];

  // See if we got a filename on the input line.
  // if not, ask for one.
 
  if (argc > 1){
    sprintf(FileName,"%s", argv[1]);
  } else {
    fprintf(stdout,"Input file name --->");
    fscanf(stdin,"%s",FileName);
  }

  bool usingSubdir = false;
  char subDir[1024];
  if (argc > 2){
    usingSubdir = true;
    sprintf(subDir,"%s", argv[2]);
  }

  DsMdvx In;

  In.setReadPath(FileName);
         
  if (In.readVolume()){
    //
    // There is a probable cause for this that is worth going
    // into in some detail -
    //
    fprintf(stderr,"Failed to open MDV file %s\n\n",FileName);
    fprintf(stderr,"Filenames must either be specified as\n"
	    "relative to $RAP_DATA_DIR, or start with a dot,\n"
	    "or be absolute. For example, if RAP_DATA_DIR is\n"
	    "set to /d2/data and we want to generate an XML\n"
	    "file for the MDV file /d2/data/mdv/radar/20050401193702.mdv\n"
	    "and the current direcory is /d2, then we\n"
	    "can do one of the following :\n\n"
	    "MdvHeaders2Xml /d2/data/mdv/radar/20050401193702.mdv [Specify whole path]\n"
	    "MdvHeaders2Xml ./data/mdv/radar/20050401193702.mdv [Specify path relative to cwd]\n"
	    "MdvHeaders2Xml mdv/radar/20050401193702.mdv [Specify path relative to RAP_DATA_DIR]\n\n");
    exit(-1);
  }


  //
  // Make the output directory, if one was specified.
  //
  if (usingSubdir){
    if ( ta_makedir_recurse(subDir) ){
      fprintf(stderr,"Failed to create specified output subdirectory %s\n",
	      subDir);
      exit(-1);
    }
  }

  //
  // Open the output file.
  //
  char outFilename[1024];
  if (usingSubdir){
    sprintf(outFilename,"%s/_fields.xml", subDir);
  } else {
    sprintf(outFilename,"_fields.xml");
  }

  FILE *ofp = fopen(outFilename,"w");
  if (ofp == NULL){
    fprintf(stderr, "Failed to open %s for writing.\n", outFilename);
    exit(-1);
  }

  fprintf(ofp,"<ModelFieldList>\n");

  // Get the master header so we can loop through
  // the fields.

  Mdvx::master_header_t InMhdr = In.getMasterHeader();     

  for (int i=0; i < InMhdr.n_fields; i++){

    MdvxField *InField;
    InField = In.getFieldByNum( i );

    if (InField == NULL){
      fprintf(stderr,"Field number %d not there!!\n",i); // Unlikely.
      fclose(ofp);
      exit(-1);
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();   
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();   

    fprintf(ofp," <Field name=\"%s\" >\n", InFhdr.field_name);

    fprintf(ofp,"  <FieldConfigMap key=\"fieldName\" value=\"%s\" />\n", InFhdr.field_name);
    fprintf(ofp,"  <FieldConfigMap key=\"vlevelType\" value=\"%s\" />\n", 
	    Mdvx::vertType2Str(InVhdr.type[0]));
    fprintf(ofp,"  <FieldConfigMap key=\"numVlevels\" value=\"%d\" />\n", InFhdr.nz);
    fprintf(ofp,"  <FieldConfigMap key=\"units\" value=\"%s\" />\n", InFhdr.units);
    fprintf(ofp,"  <FieldConfigMap key=\"desc\" value=\"%s\" />\n", InFhdr.field_name_long);

    fprintf(ofp," </Field>\n");

  }

  fprintf(ofp,"</ModelFieldList>\n");

  fclose(ofp);

  fprintf(stdout,"XML written to %s\n", outFilename);

  return 0;

}






