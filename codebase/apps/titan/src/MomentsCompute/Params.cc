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
////////////////////////////////////////////
// Params.cc
//
// Params code for MomentsEngine
//
/////////////////////////////////////////////

#include <vector>
using namespace std;

#include "Params.hh"

////////////////////////////////////////////
// Default constructor
//

Params::Params()

{
	setDefault();
}
////////////////////////////////////////////
// Destructor
//
Params::~Params()
{

}
////////////////////////////////////////////
void
Params::setDefault()
{
	debug = Params::DEBUG_OFF;

	atmos_attenuation    = 0.012; // db/km
	dbz_calib_correction = 0.0;   // dB
	zdr_correction       = 0.0;   // dB
	ldr_correction       = 0.0;   // dB


	// radar params
	radar.xmit_rcv_mode    = DP_ALT_HV_CO_CROSS;
	radar.horiz_beam_width = 0.91;   // deg
	radar.vert_beam_width  = 0.91;   // deg
	radar.pulse_width      = 1.0;    // us
	radar.wavelength_cm    = 10.68;  // cm
	radar.xmit_peak_pwr    = 0.4e6;  // W

	// receiver calibrations
	hc_receiver.noise_dBm        = -77.0310;
	hc_receiver.noise_h_dBm      = hc_receiver.noise_dBm;
	hc_receiver.noise_v_dBm      = hc_receiver.noise_dBm;
	hc_receiver.gain             = 37.2704;
	hc_receiver.radar_constant   = -68.3782;

	hx_receiver.noise_dBm        = -77.1704;
	hx_receiver.noise_h_dBm      = hx_receiver.noise_dBm;
	hx_receiver.noise_v_dBm      = hx_receiver.noise_dBm;
	hx_receiver.gain             = 37.1916;
	hx_receiver.radar_constant   = -68.3782;

	vc_receiver.noise_dBm        = -77.4886;
	vc_receiver.noise_h_dBm      = vc_receiver.noise_dBm;
	vc_receiver.noise_v_dBm      = vc_receiver.noise_dBm;
	vc_receiver.gain             = 35.6743;
	vc_receiver.radar_constant   = -68.3782;

	vx_receiver.noise_dBm        = -77.7405;
	vx_receiver.noise_h_dBm      = vx_receiver.noise_dBm;
	vx_receiver.noise_v_dBm      = vx_receiver.noise_dBm;
	vx_receiver.gain             = 35.4526;
	vx_receiver.radar_constant   = -68.3782;

	// moments manager params

        moments_params.mode                   = DUAL_CP2_SBAND;
	moments_params.n_samples              = 64;
	moments_params.start_range            = 0.0;
	moments_params.gate_spacing           = 0.150;
	moments_params.algorithm              = ALG_PP;
        moments_params.window                 = WINDOW_NONE;
	moments_params.apply_clutter_filter   = false;
	moments_params.index_beams_in_azimuth = true;
	moments_params.azimuth_resolution     = 1.0; // for indexed beams

        correct_for_system_phidp = true;
        system_phidp = 45;

}

