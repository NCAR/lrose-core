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
// Image2Mdv.cc : An Empty template Program with TDRP stuff.
//
using namespace std;

#include <string>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>


#include "MdvDataFactory.hh"  // Our TIFF to MDV converter class.

#include "Image2Mdv.hh"   // The Application Class


const string Image2Mdv::version = "0.1";


//////////////////////////////////////////////////////////////////////
// Application Initializer 
// 
int
Image2Mdv::init(int argc, char **argv )
{
   char *param_path;
   string paramPath;
   tdrp_override_t override;
   int status = 0;

   //
   // Set the program name
   //
   program.setPath(argv[0]);
   msgLog.setApplication( getProgramName() );

   ucopyright( (char*)PROGRAM_NAME );
    
   //
   // Process the command line arguments
   //
   if ( processArgs( argc, argv, &param_path) != 0 )
      POSTMSG( FATAL, "Problems processing arguments");

   //
   // Read the application parameters
   //
   TDRP_init_override(&override);

   if (params.loadFromArgs( argc, argv, override.list, &param_path,false) != 0 )
      POSTMSG( FATAL, "Problems reading parameters");

   TDRP_free_override(&override);

   //
   // Register with procmap now that we have the instance name
   //
   if ( params.instance != NULL ) {
     PMU_auto_init( (char*)PROGRAM_NAME, params.instance,
		    PROCMAP_REGISTER_INTERVAL );
   }

   PMU_auto_register( "Starting up application" );

   if(param_path != NULL) {
       paramPath = param_path;
       //
       // Process the parameters
       //
       if ( processParams( paramPath ) != 0 )
          POSTMSG( FATAL, "Problems processing parameters");
    }


   return status;
}

//////////////////////////////////////////////////////////////////////
// Application Usage Info 
// 
void
Image2Mdv::usage()
{
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               print verbose debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -params params_file ]  set parameter file name\n"
        << "       [ -print_params ]        produce parameter listing\n"
        << "       [ -v -version ]         display version number\n"
        << "       [ -o fname ]         Output Mdv as named file\n"
        << "        -i -image             File name or http URL of Image\n"
        << endl;
   dieGracefully( -1 );
}

//////////////////////////////////////////////////////////////////////
// Process Arguments 
// 
int 
Image2Mdv::processArgs( int argc, char **argv,
                           char  **paramPath)
{
   int          i;
   int          paramArg;

   //
   // Parameter initializations
   //
   paramArg = 0;
   Output_fname = " ";

   for( i=1; i < argc; i++ ) {
      //
      // request for usage information
      //
      if ( !strcmp(argv[i], "--" ) ||
           !strcmp(argv[i], "-h" ) ||
           !strcmp(argv[i], "-help" ) ||
           !strcmp(argv[i], "-man" )) {
         usage();
      }

      //
      // request for debug messages
      //
      if ( !strcmp(argv[i], "debug" )) {
         msgLog.enableMsg( DEBUG, true );
      }  
							   
      if ( !strcmp(argv[i], "-v" ) || !strcmp(argv[i], "-version" )) {
         POSTMSG( "version %s", version.c_str() );
         dieGracefully( 0 );
      }
                    
      if (!strcmp(argv[i], "-i" ) || !strcmp(argv[i], "-image" )) {
           if ( i < argc - 1 ) {
			  Input_url = argv[i+1];
              POSTMSG( "Processing %s", Input_url.c_str() );
			} else {
              POSTMSG( ERROR, "Missing Image file/URL name ." );
              usage();
			}
      }
                    
      if (!strcmp(argv[i], "-o" ) || !strcmp(argv[i], "-output" )) {
           if ( i < argc - 1 ) {
			  Output_fname = argv[i+1];
              POSTMSG( "Will generate: %s", Output_fname.c_str() );
			} else {
              POSTMSG( ERROR, "Missing file name ." );
              usage();
			}
      }

      //
      // parameter file setting
      //
      else if ( !strcmp( argv[i], "-params") ) {
         if ( i < argc - 1 ) {
            paramArg = i+1;
         } else {
            POSTMSG( ERROR, "Missing parameter file name." );
            usage();
         }
      }
   }

   if ( paramArg ) {
      //
      // Check for the files' existence
      //
      if ( Path::exists( argv[paramArg] )) {
         *paramPath = argv[paramArg];
      } else {
         POSTMSG( ERROR, "Cannot find parameter file: %s", argv[paramArg] );
         return( -1 );
      }
   }

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// Process Parameters 
// 
int
Image2Mdv::processParams( const string &paramPath )
{
   if ( !paramPath.empty() ) {
      POSTMSG( DEBUG, "Loading Parameter file: %s", paramPath.c_str() );
   }

   //
   // Initialize message logging
   //
   POSTMSG( DEBUG, "Initializing message log." );
   if ( params.debug == TRUE ) {
      msgLog.enableMsg( DEBUG, true );
   }
   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }
     
   POSTMSG( DEBUG, "Initializing execution parameters." );

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// APPLICATION MAIN ENTRY POINT - DO THE WORK 
// 
void
Image2Mdv::run()
{


	if(params.realtime_mode) {
	  DsInputPath *_input;

	   _input = new DsInputPath("Image2Mdv",
								params.debug,
								params.input_dir,
								params.max_realtime_valid_age,
								PMU_auto_register,
								false, // use_ldata_info
								true); // latest_file_only

		if (strlen(params.file_extension) > 0){
		   _input->setSearchExt(params.file_extension);
		}

		PMU_auto_register("Waiting for data");

		char *FilePath;

		_input->reset();

		while ((FilePath = _input->next()) != NULL) {

		  process_file(FilePath);

		  if(params.erase_files) {
			unlink(FilePath);
		  }
		}

	} else {
	  process_file(Input_url.c_str());
      exit(0);
	}

}

//////////////////////////////////////////////////////////////////////
// PROCESS_FILE: Process one file
// 
void
Image2Mdv::process_file(const char *fname)
{
	Imlib_Image image;

	ui32 height, width;
	DATA32 *data;

	struct stat sbuf;

	if(stat(fname,&sbuf)) {
        fprintf(stderr, "Cannot stat input file\n");
        perror(fname);
	}

	// Open the Fileo
    if ((image =imlib_load_image((char *) fname)) == NULL) {
        fprintf(stderr, "Cannot load input file\n");
        perror(fname);
    }
	imlib_context_set_image(image);

	  // Extract geometry info
	
	  height = imlib_image_get_height();
	  width = imlib_image_get_width();

      POSTMSG( DEBUG, "Image Size: %d X %d\n", width,height);

	  // Extract the RGB data
      if((data = imlib_image_get_data_for_reading_only()) == NULL) {
	     fprintf(stderr, "Problem Processing input file\n");
	     return;
      }

	  // Everything seems happy - Proceed to instantiate A New Mdv file
	  MdvFactory * MdvF;
      if (params.create_image_mdv_file) {
         MdvF = new MdvFactory();
      }
	  else {
         MdvF = new MdvDataFactory();
      }


	  // Set up Headers from Params
	  MdvF->BuildHeaders(params);

	  // Transfer Image data - Using Params for Labels, etc
	  MdvF->PutRGB(data,height,width,fname, params);

	  MdvF->WriteFile(sbuf.st_mtime, Output_fname, params);

	  free(data);

      return;
}
