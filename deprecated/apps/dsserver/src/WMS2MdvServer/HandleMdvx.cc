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
///////////////////////////////////////////////////////////////
// HandleMdvx.cc
//
// Handle Mdvx requests for WMS2MdvServer Object
//
//
// F. hage June 2003
// Based on code from Mike Dixon, Oct 1999
///////////////////////////////////////////////////////////////


#include <X11/Xlib.h>
#include <Imlib2.h>
#include "Params.hh"
#include "WMS2MdvServer.hh"

#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <dsserver/DmapAccess.hh>

#include "MdvFactory.hh"
#include <toolsa/pjg.h>

using namespace std;

// Handle Mdvx data requests

int WMS2MdvServer::handleMdvxCommand(Socket *socket,
                                   const void *data, ssize_t dataSize,
				   Params *paramsInUse)
{

  if (_isVerbose) { cerr << "Handling Mdvx command in WMS2MdvServer." << endl; }

  // disassemble message
  
  DsMdvx mdvx;
  DsMdvxMsg msg;
  if (_isVerbose) {
    msg.setDebug();
  }
  
  if (msg.disassemble(data, dataSize, mdvx)) {
    string errMsg  = "Error in WMS2MdvServer::handleMdvxCommand(): ";
    errMsg += "Could not disassemble message.";
    errMsg += msg.getErrStr();
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM -WMS2MdvServer - " << statusString << endl;
    }
    return 0;
  }

  // check security
  
  if (msg.getSubType() == DsMdvxMsg::MDVP_WRITE_TO_DIR ||
      msg.getSubType() == DsMdvxMsg::MDVP_WRITE_TO_PATH) {
    
    if (_isSecure) {
      string urlStr = mdvx.getWriteUrl();
      string securityErr;
      if (!DsServerMsg::urlIsSecure(urlStr, securityErr)) {
	cerr << "ERROR - DsFCopyServer::handleDataCommand" << endl;
	cerr << "  Running in secure mode." << endl;
	cerr << securityErr;
	cerr << "  URL: " << urlStr << endl;
	return -1;
      }
    }

  }

  int iret;

  switch (msg.getSubType()) {
    
  case DsMdvxMsg::MDVP_READ_ALL_HDRS: {
    if (_isDebug) { cerr << "MDVP_READ_ALL_HDRS: NOT IMPLEMENTED:" << endl; }
    msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_ALL_HDRS,
			      "MDVP_READ_ALL_HDRS: NOT IMPLEMENTED");
    break;
  }
    
  case DsMdvxMsg::MDVP_READ_VOLUME: {
    if (_isDebug) {
      cerr << "Client host: " << msg.getClientHost() << endl;
      cerr << "Client ipaddr: " << msg.getClientIpaddr() << endl;
      cerr << "Client user: " << msg.getClientUser() << endl;
      mdvx.printReadRequest(cerr);
    }

	//  Get 
    iret = _readWMServer(&mdvx, paramsInUse);

    if (_isVerbose) { mdvx.printAllFileHeaders(cerr); }
    if (iret) {
      msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_VOLUME,
			      "WMS - Map Not Available" ,
			      "No Image Returned" );
    } else {
      msg.assembleReadVolumeReturn(mdvx);
    }
    break;
  }

  case DsMdvxMsg::MDVP_READ_VSECTION: {
    if (_isDebug) { cerr << "ILLEGAL Request - Cannot Get VSection from WMS" << endl; }
    msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_VSECTION,
			      "WMS Have no sense of volume", "No vertical info available");
    break;
  }
    
  case DsMdvxMsg::MDVP_WRITE_TO_DIR: {
    if (_isDebug) { cerr << "ILLEGAL Request - Cannot Write to WMS" << endl; }
    msg.assembleErrorReturn(DsMdvxMsg::MDVP_WRITE_TO_DIR,
			      "ILLEGAL Request - Cannot Write to WMS");

    break;
  }
    
  case DsMdvxMsg::MDVP_WRITE_TO_PATH: {
    if (_isDebug) { cerr << "ILLEGAL Request - Cannot Write to WMS"
						 << mdvx.getWritePath() << endl; }
    msg.assembleErrorReturn(DsMdvxMsg::MDVP_WRITE_TO_PATH,
						  "ILLEGAL Request - Cannot Write to WMS");
    break;
  }
    
  case DsMdvxMsg::MDVP_COMPILE_TIME_LIST: {
    if (_isDebug) { cerr << "WMS Have no sense of Time - Cannot comply";}
      msg.assembleErrorReturn(DsMdvxMsg::MDVP_COMPILE_TIME_LIST,
			      "WMS Have no sense of Time");
    break;
  }
    
  } // Switch
  
  // send reply

  void *msgToSend = msg.assembledMsg();
  int msgLen = msg.lengthAssembled();
  if (socket->writeMessage(0, msgToSend, msgLen)) {
    cerr << "ERROR - COMM -HandleMdvxCommand." << endl;
    cerr << "  Sending message to server." << endl;
    cerr << socket->getErrStr() << endl;
  }

  return 0;

}

void WMS2MdvServer::_buildBBOX(DsMdvx *mdvx, Params *paramsInUse, double &x1, double &y1,
double &x2, double &y2, double &delta_x, double &delta_y)
{

    static char *buf[1024];

	bzero(buf,1024);

	x1 = -180; // Sensible defaults ?
	x1 = 180;
	y1 = -90;
	y2 = 90;

    switch(paramsInUse->in_projection) {
	  case Params::PROJ_LATLON:
        x1 = mdvx->_readMinLon;
		y1 = mdvx->_readMinLat;
        x2 = mdvx->_readMaxLon;
		y2 = mdvx->_readMaxLat;
	  break;


	  case Params::PROJ_STEREOGRAPHIC:
		  PJGps_init(paramsInUse->proj.origin_lat,paramsInUse->proj.origin_lon,
					 paramsInUse->proj.param1,paramsInUse->proj.param2,
					 paramsInUse->proj.param3);

		  PJGps_latlon2xy(mdvx->_readMinLat,mdvx->_readMinLon,&x1,&y1);
		  PJGps_latlon2xy(mdvx->_readMaxLat,mdvx->_readMaxLon,&x2,&y2);
	  break;

	  case Params::PROJ_POLAR_ST_ELLIP:
		  PJGpse_init(paramsInUse->proj.origin_lat,paramsInUse->proj.origin_lon,
					 paramsInUse->proj.param1,paramsInUse->proj.param2,
					 paramsInUse->proj.param3,paramsInUse->proj.param4);

		  PJGpse_latlon2xy(mdvx->_readMinLat,mdvx->_readMinLon,&x1,&y1);
		  PJGpse_latlon2xy(mdvx->_readMaxLat,mdvx->_readMaxLon,&x2,&y2);
	  break;

	  case Params::PROJ_LAMBERT_CONF:
		  PJGlc2_init(paramsInUse->proj.origin_lat,paramsInUse->proj.origin_lon,
					 paramsInUse->proj.param1,paramsInUse->proj.param2);

		  PJGlc2_latlon2xy(mdvx->_readMinLat,mdvx->_readMinLon,&x1,&y1);
		  PJGlc2_latlon2xy(mdvx->_readMaxLat,mdvx->_readMaxLon,&x2,&y2);
	  break;

	  case Params::PROJ_MERCATOR:
		  PJGtm_init(paramsInUse->proj.origin_lat,paramsInUse->proj.origin_lon,
					 paramsInUse->proj.param1);

		  PJGtm_latlon2xy(mdvx->_readMinLat,mdvx->_readMinLon,&x1,&y1);
		  PJGtm_latlon2xy(mdvx->_readMaxLat,mdvx->_readMaxLon,&x2,&y2);
	  break;

	  case Params::PROJ_CYL_EQUIDIST:
	  break;

	  case Params::PROJ_FLAT:
		  PJGstruct *ps;
		  ps = PJGs_flat_init(paramsInUse->proj.origin_lat,paramsInUse->proj.origin_lon,
					 paramsInUse->proj.param1);

		  PJGs_flat_latlon2xy(ps,mdvx->_readMinLat,mdvx->_readMinLon,&x1,&y1);
		  PJGs_flat_latlon2xy(ps,mdvx->_readMaxLat,mdvx->_readMaxLon,&x2,&y2);
		  x1 *= 1000;  // convert from KM to meters
		  x2 *= 1000;  // convert from KM to meters
		  y1 *= 1000;  // convert from KM to meters
		  y2 *= 1000;  // convert from KM to meters
	  break;

	  default: 
	  break;

    }

	delta_x = x2 - x1;
	delta_y = y2 - y1;

	return;
}

int WMS2MdvServer::_readWMServer(DsMdvx *mdvx, Params *paramsInUse)
{
    char buf[1024];
    char cmd_buf[4096];
    char wms_url[4096];
    char img_fname[MAX_PATH_LEN];

	unsigned char *data;

	double delta_x, delta_y;

	double x1,x2,y1,y2;

    Imlib_Image image;
    uint32 height, width;

    int iret = -1;
	// Build the WMS URL

	// BASE
    strcpy(wms_url,paramsInUse->WMS_BASE);

	// WMTVER
    strcat(wms_url,"?");  // The First Arg gets a ?
    strcat(wms_url,paramsInUse->WMS_WMTVER);

	// REQUEST
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_REQUEST);

	// LAYERS
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_LAYERS);
    for(size_t i = 0; i < mdvx->_readFieldNames.size(); i++) {
      sprintf(buf,",%s",mdvx->_readFieldNames[i].c_str());
      strcat(wms_url,buf);
    }

	// STYLES
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_STYLES);

	// SRS
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_SRS);

	// BBOX
    strcat(wms_url,"&");
    _buildBBOX(mdvx,paramsInUse,x1,y1,x2,y2,delta_x,delta_y);
    sprintf(buf,"BBOX=%.6f,%.6f,%.6f,%.6f",x1,y1,x2,y2);
	strcat(wms_url,buf);

	// WIDTH - Force Square pixels in the request.
	int big_edge = (int) (sqrt((double)mdvx->_readDecimateMaxNxy) + 0.5);
	int request_nx;
	int request_ny;
	if (mdvx->_readDecimate) {
		if(delta_x > delta_y) {
		   request_nx = big_edge;
		   request_ny = (int) ((request_nx * delta_y/delta_x) + 0.5); 
		} else {
		   request_ny = big_edge;
		   request_nx = (int) ((request_ny * delta_x/delta_y) + 0.5); 
		}
	} else {
		if(delta_x > delta_y) {
		   request_nx = paramsInUse->request_nx_pixels;
		   request_ny = (int) ((request_nx * delta_y/delta_x) + 0.5); 
		} else {
		   request_ny = paramsInUse->request_ny_pixels;
		   request_nx = (int) ((request_ny * delta_x/delta_y) + 0.5); 
		}
	}

    request_nx = paramsInUse->request_nx_pixels;
    request_ny = paramsInUse->request_ny_pixels;
    strcat(wms_url,"&");
	sprintf(buf,"WIDTH=%d",request_nx);
    strcat(wms_url,buf);

	// HEIGHT
    strcat(wms_url,"&");
	sprintf(buf,"HEIGHT=%d",request_ny);
    strcat(wms_url,buf);

	// WMS_FORMAT
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_FORMAT);

	// TRANSPARENT
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_TRANSPARENT);

	// BGCOLOR
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_BGCOLOR);

	// EXCEPTIONS
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_EXCEPTIONS);

	// VENDOR
    strcat(wms_url,"&");
    strcat(wms_url,paramsInUse->WMS_VENDOR);

    // Build the temp image filename
    sprintf(img_fname,"/tmp/WMS2MdvServer_%d.img",getpid());

    // Build the wget command
	sprintf(cmd_buf,"wget -O %s -q \"%s\"",img_fname,wms_url);

    if (_isDebug) { cerr << "WMS_URL CMD: " << cmd_buf  << endl; }
    // Run the wget command to retrieve the image locally
	system(cmd_buf);

    // OK,  now Suck in the resultant image
    if (( image= imlib_load_image(img_fname)) == NULL) {
        fprintf(stderr, "Cannot load input file\n");
        perror(img_fname);
		return iret;
    }

	imlib_context_set_image(image);


    // Extract geometry info
    height = imlib_image_get_height();
    width = imlib_image_get_width();

	if(_isDebug) { cerr << "Height,Width: " << height << "," << width  << endl; }
	

	DATA32 *d32;

    // Extract the RGBA data
    if((d32 = imlib_image_get_data_for_reading_only()) == NULL) {
       fprintf(stderr, "Problem Processing input file\n");
       return iret;
    
	 }


    // Everything seems happy - Proceed to instantiate A New Mdv file
    MdvFactory MdvF(mdvx);

    // Set up Headers from Params & request domain
    MdvF.BuildHeaders(paramsInUse,mdvx->_readFieldNames[0].c_str(),x1,y1,(delta_x/width),(delta_y/height));

    // Transfer Image data - Using Params for Lables, etc
    MdvF.PutRGB(d32,height,width,mdvx->_readFieldNames[0].c_str(), paramsInUse);

	imlib_free_image();
	iret = 0;
    
	if(_isDebug) {
		cerr << "Results left in " << img_fname << endl;
	} else {
		unlink(img_fname);
	}
		

	// For Now PUNT on the Image Valid time
    time_t searchTime = mdvx->_readSearchTime;
    if (iret == 0) {
      mdvx->_mhdr.time_begin = searchTime;
      mdvx->_mhdr.time_end = searchTime + 84600;
      mdvx->_mhdr.time_centroid = searchTime;
    }

    return iret;

}
