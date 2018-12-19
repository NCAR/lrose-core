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
/* 	$Id: dd_sweepfiles.cc,v 1.9 2018/10/13 22:42:50 dixon Exp $	 */


# include <cerrno>
# include <cstdio>
# include <cstdlib>
# include <unistd.h>
# include <cstring>
# include <sys/types.h>
# include <time.h>
# include <sys/time.h>
# include <signal.h>

# include <radar/dorade/dd_sweepfiles.hh>
const int dd_sweepfile::angle_ndx_size = 360;
const int dd_mem_sweepfile::angle_ndx_size = 360;

// static const int MAX_FILE_SIZE = 10*1024*1024;



// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

int
parameter_info( const char * name, PARAMETER * parm )
{
    int id;

    strncpy(parm->parameter_name, "          ", 8);
    int nn = (strlen(name) < 8) ? strlen(name) : 8;
    strncpy( parm->parameter_name, name, nn );

    parm->binary_format = DD_16_BITS;
    strncpy(parm->threshold_field, "         ", 8);
    parm->threshold_value = EMPTY_FLAG;      
    parm->parameter_scale = 100.;      
    parm->parameter_bias = 0.;       
    parm->bad_data  = EMPTY_FLAG;             

	// unique to each field


    if(!strcmp(name, "VR") || !strcmp(name, "V_HH_K") || !strcmp(name, "V_VH_K")) {
	strcpy(parm->param_description
	       , "Raw doppler velocities");
	strcpy(parm->param_units, "m/s");
	id = VR_NDX;
    }
    else if(!strcmp(name, "VT")) {
	strcpy(parm->param_description
	       , "Doppler velocity thresholded on NCP");
	strcpy(parm->param_units, "m/s");
	id = VT_NDX;
    }
    else if(!strcmp(name, "DM") || !strcmp(name, "P_HH_K") || !strcmp(name, "P_VH_K")) {
	strcpy(parm->param_description
	       , "Reflected power in dBm");
	strcpy(parm->param_units, "dBm");
	id = DBM_NDX;
    }
    else if(!strcmp(name, "DBZ") || !strcmp(name, "DZ") ||
	    !strcmp(name, "DBZ_K") || !strcmp(name, "Z_HH_K")) {
	strcpy(parm->param_description
	       , "Reflectivity factor");
	strcpy(parm->param_units, "dBz");
	id = DBZ_NDX;
    }
    else if(!strcmp(name, "DY") || !strcmp(name, "DY_MM")) {
	strcpy(parm->param_description
	       , "Vertical reflectivity factor");
	strcpy(parm->param_units, "dBz");
	id = DBZ_NDX;
    }
    else if(!strcmp(name, "CDBZ") || !strcmp(name, "CDBZ_MM")) {
	strcpy(parm->param_description
	       , "Coherent Reflectivity factor");
	strcpy(parm->param_units, "dBz");
	id = CDBZ_NDX;
    }
    else if(!strcmp(name, "NCP") || !strcmp(name, "NCP_HH_K") || !strcmp(name, "NCP_VH_K")) {
	strcpy(parm->param_description
	       , "Normalized Coherent Power");
	strcpy(parm->param_units, " ");
	id = NCP_NDX;
    }
    else if(!strcmp(name, "SW") || !strcmp(name, "SW_HH_K")) {
	strcpy(parm->param_description
	       , "Spectral width");
	strcpy(parm->param_units, "m/s");
	id = SW_NDX;
    }
    else if(!strcmp(name, "ZDR") || !strcmp(name, "ZDR_K")) {
	strcpy(parm->param_description
	       , "Polarization_diversity");
	strcpy(parm->param_units, "dBm");
	id = ZDR_NDX;
    }
    else if(!strcmp(name, "PHIDP") || !strcmp(name, "PHIDP_K")) {
	strcpy(parm->param_description
	       , "Differential Phase");
	strcpy(parm->param_units, "deg.");
	id = PHIDP_NDX;
	parm->parameter_scale = 10.;      
	parm->parameter_bias = 0.;       
    }
    else if(!strcmp(name, "CPHI")) {
	strcpy(parm->param_description
	       , "CSU Filtered Differential Phase");
	strcpy(parm->param_units, "deg.");
	id = CPHI_NDX;
	parm->parameter_scale = 10.;      
	parm->parameter_bias = 0.;       
    }
    else if(!strcmp(name, "NPHI")) {
	strcpy(parm->param_description
	       , "NSSL Filtered Differential Phase");
	strcpy(parm->param_units, "deg.");
	id = NPHI_NDX;
	parm->parameter_scale = 10.;      
	parm->parameter_bias = 0.;       
    }
    else if(!strcmp(name, "NSDEV")) {
	strcpy(parm->param_description
	       , "NSSL Phi Std. Dev. field");
	strcpy(parm->param_units, "deg.");
	id = NSDEV_NDX;
	parm->parameter_scale = 10.;      
	parm->parameter_bias = 0.;       
    }
    else if(!strcmp(name, "RHOHV") || !strcmp(name, "RHO_HV_K")) {
	strcpy(parm->param_description
	       , "Copolar Cross Correlation");
	strcpy(parm->param_units, " ");
	parm->parameter_scale = 1000.;      
	parm->parameter_bias = 0.;       
	id = RHOHV_NDX;
    }
    else if(!strcmp(name, "RSDEV")) {
	strcpy(parm->param_description
	       , "Standard Deviation of RhoHV");
	strcpy(parm->param_units, " ");
	parm->parameter_scale = 1000.;      
	parm->parameter_bias = 0.;       
	id = RSDEV_NDX;
    }
    else if(!strcmp(name, "KDP")) {
	strcpy(parm->param_description
	       , "Derivative of Differential Phase");
	strcpy(parm->param_units, "deg/km.");
	id = KDP_NDX;
    }
    else if(!strcmp(name, "CKDP")) {
	strcpy(parm->param_description
	       , "Derivative of Differential Phase (CSU)");
	strcpy(parm->param_units, "deg/km.");
	id = CKDP_NDX;
    }
    else if(!strcmp(name, "KRATE")) {
	strcpy(parm->param_description
	       , "Rainfall Rate from Kdp");
	strcpy(parm->param_units, "mm/hr");
	parm->parameter_scale = 1.;      
	parm->parameter_bias = 0.;       
	id = KRATE_NDX;
    }
    else if(!strcmp(name, "CRATE")) {
	strcpy(parm->param_description
	       , "Rainfall Rate from CSU Kdp");
	strcpy(parm->param_units, "mm/hr");
	parm->parameter_scale = 1.;      
	parm->parameter_bias = 0.;       
	id = CRATE_NDX;
    }
    else if(!strcmp(name, "NRATE")) {
	strcpy(parm->param_description
	       , "Rainfall Rate from Kdp (NSSL)");
	strcpy(parm->param_units, "mm/hr");
	parm->parameter_scale = 10.;      
	parm->parameter_bias = 0.;       
	id = NRATE_NDX;
    }
    else if(!strcmp(name, "ZRATE")) {
	strcpy(parm->param_description
	       , "ZR Rainfall Rate");
	strcpy(parm->param_units, "mm/hr");
	parm->parameter_scale = 1.;      
	parm->parameter_bias = 0.;       
	id = ZRATE_NDX;
    }
    else if(!strcmp(name, "DRATE")) {
	strcpy(parm->param_description
	       , "Rainfall Rate from Zdr");
	strcpy(parm->param_units, "mm/hr");
	parm->parameter_scale = 1.;      
	parm->parameter_bias = 0.;       
	id = DRATE_NDX;
    }
    else if(!strcmp(name, "KAC")) {
	strcpy(parm->param_description
	       , "Rainfall Accumulation from Kdp");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = KAC_NDX;
    }
    else if(!strcmp(name, "ZAC")) {
	strcpy(parm->param_description
	       , "ZR Rainfall Accumulation");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = ZAC_NDX;
    }
    else if(!strcmp(name, "DAC")) {
	strcpy(parm->param_description
	       , "Rainfall Accumulation from Zdr");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = DAC_NDX;
    }
    else if(!strcmp(name, "HAC")) {
	strcpy(parm->param_description
	       , "Hybrid Rainfall Accumulation");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = HAC_NDX;
    }
    else if(!strcmp(name, "CAC")) {
	strcpy(parm->param_description
	       , "CSU Rainfall Accumulation");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = HAC_NDX;
    }
    else if(!strcmp(name, "GAC")) {
	strcpy(parm->param_description
	       , "DSD based Rainfall Accumulation");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = HAC_NDX;
    }
    else if(!strcmp(name, "NAC")) {
	strcpy(parm->param_description
	       , "NSSL Rainfall Accumulation");
	strcpy(parm->param_units, "mm");
	parm->parameter_scale = 10;      
	parm->parameter_bias = 0.;       
	parm->bad_data = 0;
	id = HAC_NDX;
    }
    else if(!strcmp(name, "HDR")) {
	strcpy(parm->param_description
	       , "Hail Detection from Zdr and dBz");
	strcpy(parm->param_units, "dB");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = HDR_NDX;
    }
    else if(!strcmp(name, "SDZDR")) {
	strcpy(parm->param_description
	       , "Standard deviation of Zdr");
	strcpy(parm->param_units, "dB");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "SPHI")) {
	strcpy(parm->param_description
	       , "Standard deviation of Phidp");
	strcpy(parm->param_units, "dB");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "SVR")) {
	strcpy(parm->param_description
	       , "Standard deviation of raw velocity");
	strcpy(parm->param_units, "dB");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "UDBZ")) {
	strcpy(parm->param_description
	       , "Unattenuated Reflectivity Factor");
	strcpy(parm->param_units, "dBz");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "UZDR")) {
	strcpy(parm->param_description
	       , "Unattenuated Differential Reflectivity");
	strcpy(parm->param_units, "dB ");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "ULDR")) {
	strcpy(parm->param_description
	       , "Unattenuated Linear Depolarization Ratio");
	strcpy(parm->param_units, "dB ");
	parm->parameter_scale = 100.;      
	parm->parameter_bias = 0.;       
	id = 99;		// id now only a bad flag
    }
    else if(!strcmp(name, "NKDP")) {
	strcpy(parm->param_description
	       , "NSSL Kdp 17 gates averaged");
	strcpy(parm->param_units, "deg/km.");
	id = NKDP_NDX;
    }
    else if(!strcmp(name, "MKDP")) {
	strcpy(parm->param_description
	       , "NSSL Kdp 49 gates averaged");
	strcpy(parm->param_units, "deg/km.");
	id = MKDP_NDX;
    }
    else if(!strcmp(name, "LDR") || !strcmp(name, "LDR_VH_K")) {
	strcpy(parm->param_description
	       , "Linear Depolarization Ratio");
	strcpy(parm->param_units, "dBm");
	id = LDR_NDX;
    }
    else if(!strcmp(name, "DL") || !strcmp(name, "DL_MM")) {
	strcpy(parm->param_description
	       , "Vertical Reflected power in dBm");
	strcpy(parm->param_units, "dBm");
	id = DBMV_NDX;
    }
    else if(!strcmp(name, "DX")) {
	strcpy(parm->param_description
	       , "Crosspole Reflected power in dBm");
	strcpy(parm->param_units, "dBm");
	id = DX_NDX;
    }
    else if(!strcmp(name, "PD")) {
	strcpy(parm->param_description
	       , "Particle Type");
	strcpy(parm->param_units, "???");
	parm->bad_data = 0;
	id = PD_NDX;
    }
    else if(!strcmp(name, "WPD")) {
	strcpy(parm->param_description
	       , "Particle Type");
	strcpy(parm->param_units, "???");
	parm->bad_data = 0;
	id = PD_NDX;
    }
    else if(!strcmp(name, "CH")) {
	strcpy(parm->param_description
	       , "Horizontal Rho value");
	strcpy(parm->param_units, "???");
	id = RH_NDX;
    }
    else if(!strcmp(name, "AH")) {
	strcpy(parm->param_description
	       , "Horizontal Rho angle");
	strcpy(parm->param_units, "deg.");
	id = AH_NDX;
    }
    else if(!strcmp(name, "CV")) {
	strcpy(parm->param_description
	       , "Vertical Rho value");
	strcpy(parm->param_units, "???");
	id = RV_NDX;
    }
    else if(!strcmp(name, "AV")) {
	strcpy(parm->param_description
	       , "Vertical Rho angle");
	strcpy(parm->param_units, "deg.");
	id = AV_NDX;
    }
    else if (!strcmp(name, "AIQ")) {
        strcpy(parm->param_description
           , "Angle avg I,Q");
	parm->parameter_scale = 1.0; // 65536.0/(2.0*M_PI);      
	parm->parameter_bias = 0.0;       
	strcpy(parm->param_units, "degrees");
	id  = IQ_ANG_NDX;
    }

    else if (!strcmp(name, "NIQ")) {
        strcpy(parm->param_description
           , "log (avgI^2+Q^2)");
	strcpy(parm->param_units, "db(1/2)");
        parm->parameter_scale = 1.0;     
	parm->parameter_bias = 0.;       
	id  = IQ_NORM_NDX;
    } else if (!strcmp(name, "N")) {
        strcpy(parm->param_description, "Index of Refraction");
        parm->parameter_scale = 10.0;     
        parm->parameter_bias = 0.;       
        strcpy(parm->param_units, "none");
        id  = N_NDX;
    } else if (!strcmp(name, "DELTA_N")) {
        strcpy(parm->param_description, "Delta Index of Refraction");
        parm->parameter_scale = 10.0;     
        parm->parameter_bias = 0.;       
        strcpy(parm->param_units, "none");
        id  = DELTA_N_NDX;
    } else if (!strcmp(name, "SIGMA_N")) {
        strcpy(parm->param_description, "SIGMA Index of Refraction");
        parm->parameter_scale = 10.0;     
        parm->parameter_bias = 0.;       
        strcpy(parm->param_units, "none");
        id  = SIGMA_N_NDX;
    } else if (!strcmp(name, "SIGMA_DN")) {
        strcpy(parm->param_description, "SIGMA of Delta Index of Refraction");
        parm->parameter_scale = 10.0;     
        parm->parameter_bias = 0.;       
        strcpy(parm->param_units, "none");
        id  = SIGMA_DN_NDX;
    } else if(!strcmp(name, "LVDR")) {
	strcpy(parm->param_description
	       , "Vertical Linear Depolarization Ratio");
	strcpy(parm->param_units, "");
	id = LDRV_NDX;
    }
    else if(!strcmp(name, "N0_DSD")) {
	strcpy(parm->param_description
	       , "LOG10 of DSD offset parameter N0");
	strcpy(parm->param_units, "???");
	id = N0_NDX;
    }
    else if(!strcmp(name, "MU_DSD")) {
	strcpy(parm->param_description
	       , "Shape DSD Parameter MU");
	strcpy(parm->param_units, "???");
	id = MU_NDX;
    }
    else if(!strcmp(name, "LAM_DSD")) {
	strcpy(parm->param_description
	       , "Slope DSD Parameter Lambda");
	strcpy(parm->param_units, "1/mm");
	id = LAM_NDX;
    }
    else if(!strcmp(name, "D0_DSD")) {
	strcpy(parm->param_description
	       , "Median Vol. Dia. from DSD");
	strcpy(parm->param_units, "mm");
	id = D0_NDX;
    }
    else if(!strcmp(name, "RR_DSD")) {
	strcpy(parm->param_description
	       , "LOG10 of rain rate from DSD");
	strcpy(parm->param_units, "mm/hr");
	id = RR_NDX;
    }
    else if(!strcmp(name, "RNX")) {
	strcpy(parm->param_description
	       , "LOG10 of rain rate from Z");
	strcpy(parm->param_units, "mm/hr");
	id = RNX_NDX;
    }
    else if(!strcmp(name, "RZD")) {
	strcpy(parm->param_description
	       , "LOG10 of rain rate from ZDR");
	strcpy(parm->param_units, "mm/hr");
	id = RZD_NDX;
    }
    else if(!strcmp(name, "RKD")) {
	strcpy(parm->param_description
	       , "LOG10 of rain rate from KDP");
	strcpy(parm->param_units, "mm/hr");
	id = RKD_NDX;
    }
    else if(!strcmp(name, "DKD_DSD")) {
	strcpy(parm->param_description
	       , "Kdp from DSD");
	strcpy(parm->param_units, "deg/km");
	id = DKD_NDX;
    }
    else if(!strcmp(name, "RES_DSD") || !strcmp(name, "DSIZE")) {
	strcpy(parm->param_description
	       , "Radar est. size from DSD");
	strcpy(parm->param_units, "mm");
	id = RES_NDX;
    }
    else if(!strcmp(name, "LWC_DSD") || !strcmp(name, "LWC")) {
	strcpy(parm->param_description
	       , "Liquid Water Content from DSD");
	strcpy(parm->param_units, "g/m^3");
        parm->parameter_scale = 1000.0;     
	id = LWC_NDX;
    }
    else if(!strcmp(name, "NT_DSD")) {
	strcpy(parm->param_description
	       , "LOG10 Total No. Concentration");
	strcpy(parm->param_units, "N/m^3");
        parm->parameter_scale = 1000.0;     
	id = NT_NDX;
    }
    else {
	// generic unknown (don't complain)
	id = -1;
    }
    return id;

# ifdef obsolete
    else if(!strcmp(name, "")) {
	strcpy(parm->param_description
	       , "");
	strcpy(parm->param_units, "");
	id = _NDX;
    }
# endif
}

// c---------------------------------------------------------------------------

dd_sweepfile::
dd_sweepfile( int min_rays) {
    min_rays_per_sweep = min_rays;
    sfile = 0;

    int ii;
    convert_to_8_bit = NO;
    for( ii = 0; ii < 64; ii++ ) { // used in 8 bit conversion
	scale8[ii] = 1.;
	bias8[ii] = 0;
    }
    produce_CSFD = NO;

    ddt = new DD_Time();

    int nb = ( 4 +  MAX_PARMS*4 ) * MAXCVGATES;
    local_buf = new char[nb];
    memset(local_buf, 0, nb);
    free_at = local_buf;

    sswb = new super_SWIB;
    strncpy((char *)sswb, "SSWB", 4);
    sswb->sizeof_struct = sizeof(*sswb);
    sswb->sizeof_file = 0;
    sswb->version_num = 1;
    sswb->num_key_tables = 1;
    sswb->key_table[0].type = KEYED_BY_ROT_ANG;
    sswb->key_table[0].offset = 0;
    sswb->key_table[0].size = 0;

    vold = new VOLUME;
    radd = new RADARDESC;
    for(ii = 0; ii < MAX_RADARDESC; ii++) {
        radds[ ii ] = new RADARDESC;
    }
    num_radardesc = 1;

    swib = new SWEEPINFO;

    tmp_parm = new PARAMETER;

    csfd = new CELLSPACINGFP;
    strncpy( csfd->cell_spacing_des, "CSFD", 4 );
    csfd->cell_spacing_des_len = sizeof(*csfd);


    null_des = new generic_descriptor;
    strncpy((char *)null_des->name_struct, "NULL", 4);
    null_des->sizeof_struct = sizeof(*null_des);

    compressed_data = free_at;
    the_typeof_compression = NO_COMPRESSION;

    ddm = NULL;
    max_rays = 0;
    rotang_buf = NULL;
    enlarge_rotang_table();
    swp_count_out = 0;
    suppressing_filename_print = 1;
}
dd_sweepfile::
~dd_sweepfile()
{
    delete [] local_buf;
    delete ddt;
	delete [] rotang_buf;

    delete null_des ;
    delete sswb;
    delete swib;
    delete vold ;
    delete radd;
    delete csfd;
    delete tmp_parm;
    for(int ii = 0; ii < MAX_RADARDESC; ii++) {
        delete radds[ ii ];
    }
}


// c---------------------------------------------------------------------------

int
dd_sweepfile::
begin_sweepfile(dd_mapper * ddmpr, const char * directory,  const char * qualifier
		, int version_num )
{
    // Sets up to write a new sweep file.
    // send in a mapper object with the first ray mapped
    // this information is used to construct the sweepfile name
    // also send in a directory, a qualifier/comment and a version number
    // It is up to the user to write the first ray with the
    // "add_to_sweepfile()" method
    //

    swp_ray_num = 0;

    if(this->sswb->sizeof_file > (int)sizeof(*sswb)) { // finalize last sweepfile
	this->end_sweepfile(0);
    }

    ddm = ddmpr;
    swp_count_out++;
    char full_path_name[256];
    int ii, nn;

    int vm = (version_num >= 0) ? version_num * 1000 + ddm->millisecond()
	:  ddm->millisecond();

    str_terminate(this->sswb->radar_name, ddm->radd->radar_name, 8);

    // create the file name for the new sweepfile

    sprintf( filename
	     , "%s%s%02d%02d%02d%02d%02d%02d%s%s%s%d"
	     , "swp"
	     , DD_NAME_DELIMITER
	     , ddm->year() -1900
	     , ddm->month()
	     , ddm->day()
	     , ddm->hour()
	     , ddm->minute()
	     , ddm->second()
	     , DD_NAME_DELIMITER
	     , this->sswb->radar_name
	     , DD_NAME_DELIMITER
	     , vm
	);
    strcpy(tmp_filename, filename);
    strcat(tmp_filename, ".tmp");

    memcpy(swib, ddm->swib, sizeof(*swib));
    swib->sweep_num = ddm->sweep_count();
    swib->sweep_num = swp_count_out;

    sprintf( ascii_fixed_angle, ".%.1f_", swib->fixed_angle );

    if( qualifier && strlen(qualifier) )
	{ strcpy( the_qualifier, qualifier ); }
    else
	{ strcpy( the_qualifier, "" ); }

    strcpy(dir_name, directory);
    nn = strlen(dir_name);
    if(nn < 1) {
	strcpy(dir_name, "./");
    }
    ddm->slashPath( dir_name );

    if(swp_count_out == 1) {
	if( !suppressing_filename_print )
	    { printf("     %s\n", dir_name); }
    }
    
    if( !suppressing_filename_print ) {
	printf("%2d) New: %s%s%s\n", swp_count_out, filename
	       , ascii_fixed_angle, the_qualifier );
    }
    // open the tmp file and write out the headers up to the RYIB

    strcpy(full_path_name, dir_name);
    strcat(full_path_name, tmp_filename);

    if (sfile)
    {
	delete sfile;
	sfile = 0;
    }
    //    sfile = new std::ofstream(full_path_name, std::ios::in | std::ios::out);
    sfile = new std::ofstream (full_path_name);
    if(sfile->fail()) {
	char message[256];
	sprintf(message, "Unable to open sweepfile name\n%s:\t%s\n"
		, full_path_name, strerror(errno));
	dd_Testify(message);
	delete sfile;
	sfile = 0;
	return 1;
    }
    memcpy(vold, ddm->vold, sizeof(*vold));
    vold->year = ddm->ddt->year();
    vold->month = ddm->ddt->month();
    vold->day = ddm->ddt->day();
    vold->data_set_hour = ddm->ddt->hour();
    vold->data_set_minute = ddm->ddt->minute();
    vold->data_set_second = ddm->ddt->second();

    // set todays date
    ddt->accept_time_stamp( (DTime)time_now() );
    ddt->unstamp_the_time();
    vold->gen_year = ddt->year();
    vold->gen_month = ddt->month();
    vold->gen_day = ddt->day();
 
    sfile->write((const char *)sswb, sizeof(*sswb));
    sfile->write((const char *)vold, sizeof(*vold));

    memcpy( radd, ddm->radd, sizeof( *radd ));
    radd->data_compress = 0;	// no compression for now
    radd->data_compress = the_typeof_compression;


    sfile->write((const char *)radd, sizeof(*ddm->radd));
    nn = sizeof(**ddm->parms);

    for(int pn = 0; pn < ddm->radd->num_parameter_des; pn++) {

	if( convert_to_8_bit ) {
	    memcpy( tmp_parm, ddm->parms[pn], nn );
	    tmp_parm->binary_format = DD_8_BITS;
	    tmp_parm->parameter_scale = scale8[ pn ];

	    if (strncmp (tmp_parm->parameter_name, "WPD", 3) != 0 &&
		strncmp (tmp_parm->parameter_name, "PD", 2) != 0 )
	      { tmp_parm->bad_data = -128; }

	    if (strncmp (tmp_parm->parameter_name, "ZDR", 3) == 0)
	      { tmp_parm->parameter_scale = 4; }
	    tmp_parm->parameter_bias = bias8[ pn ];
	    sfile->write((const char *)tmp_parm, nn );
	}
	else
	    { sfile->write((const char *)ddm->parms[pn], sizeof(**ddm->parms)); }
    }
    float gs;

    if( produce_CSFD ) {
	csfd->num_segments = 1;
	gs = ddm->celv->dist_cells[1] - ddm->celv->dist_cells[0];
	csfd->distToFirst = ddm->celv->dist_cells[0];
	csfd->spacing[0] = csfd_gate_skip * gs;
	csfd->num_cells[0] = (csfd_num_gates) ? csfd_num_gates :
	  ddm->celv->number_cells;
	sfile->write((const char *)csfd, csfd->cell_spacing_des_len);
    }
    else {
	sfile->write((const char *)ddm->celv, ddm->celv->cell_des_len);
    }

    if (ddm->frib) {
      sfile->write((const char *)ddm->frib, ddm->frib->field_radar_info_len);
    }

    offset_to_cfac = sfile->tellp();

    sfile->write((const char *)ddm->cfac, sizeof(*ddm->cfac));

    if(( nn = ddm->return_comment_count() ) > 0 ) {
	for( ii = 0; ii < nn; ii++ ) {
	    sfile->write((const char *)ddm->comms[ii], sizeof(*ddm->comm));
	}
    }

    offset_to_swib = sfile->tellp();

    this->swib->num_rays = 0;
    this->swib->start_angle = ddm->rotation_angle();
    sfile->write((const char *)this->swib, sizeof(*swib));

    this->sswb->sizeof_file = sfile->tellp();
    this->sswb->d_start_time = ddm->dtime;

    reset_rotang_table();

    // leave it to the calling method to add the first beam

    return 0;
}

// c---------------------------------------------------------------------------

const char *
dd_sweepfile::
end_sweepfile( int kill_it )	// may be passed a kill flag
{
    // finishes a sweepfile by
    // by tacking on a NULL descripter and the rotation angle table
    // and updating the SSWB and SWIB descripters
    //
    char shell_command[512];
    std::streampos spos;

    if(this->sswb->sizeof_file <= 0)
	{ return NULL; }

    else if(this->sswb->sizeof_file > 0 &&
	    (kill_it || swib->num_rays < min_rays_per_sweep)) {
	delete sfile;
	sfile = 0;
	strcpy(shell_command, dir_name);
	strcat(shell_command, tmp_filename);
/*	int ii = unlink( shell_command ); */
	unlink( shell_command );
	this->sswb->sizeof_file = 0;
	char message[256];
	sprintf( message, "killed_sweepfile: %s < %d rays %ld\n"
		 , tmp_filename
		 , min_rays_per_sweep
		 , swib->num_rays
	    );
	dd_Testify(message);
	return NULL;
    }


    sfile->seekp( (std::streampos)sswb->sizeof_file );

    // add the null descriptor
    sfile->write((const char *)null_des, sizeof(*null_des));

    // write out the rotation angle table
    this->sswb->key_table[0].offset = sfile->tellp();
    this->sswb->key_table[0].size = rktb->sizeof_struct;
    sfile->write((const char *)rktb, rktb->sizeof_struct);
    this->sswb->sizeof_file = sfile->tellp();

    this->end_sweepfile_size = sfile->tellp();

    // postion to write the super_SWIB again
    sfile->seekp(0);
    sfile->write((const char *)this->sswb, this->sswb->sizeof_struct);

    // write out the correction factors again
    sfile->seekp(offset_to_cfac);
    sfile->write((const char *)ddm->cfac, ddm->cfac->correction_des_length);

    // write out the sweep info block again
    sfile->seekp(offset_to_swib);
    sfile->write((const char *)this->swib, this->swib->sweep_des_length);


    delete sfile;
    sfile = 0;

    this->sswb->sizeof_file = 0;

    // move the file to its permanent name
    
    strcpy(permanent_name, dir_name);
    strcat(permanent_name, filename);
    strcat(permanent_name, ascii_fixed_angle);
    strcat(permanent_name, the_qualifier);

# ifdef obsolete
    strcpy(shell_command, "mv ");
    strcat(shell_command, dir_name);
    strcat(shell_command, tmp_filename);
    strcat(shell_command, " ");
    strcat(shell_command, permanent_name);
    system( shell_command );
# else
    strcpy(shell_command, dir_name);
    strcat(shell_command, tmp_filename);
/*    int ii = rename( shell_command, permanent_name ); */
    rename( shell_command, permanent_name );
# endif

    return permanent_name;
}

// c---------------------------------------------------------------------------

int
dd_sweepfile::
add_to_sweepfile()
{
    // writes the next ray to the sweepfile
    //
    int fn, nb = 0, sos, bad;

    swp_ray_num = ++this->swib->num_rays;
    ddm->ryib->sweep_num = swp_count_out;

    if (!sfile) {
	dd_Testify("add_to_sweepfile:  sfile is NULL!\n");
	return 0;
    }
    this->sswb->sizeof_file = sfile->tellp();
    update_rotang_table();

    sfile->write((const char *)ddm->ryib, sizeof(*ddm->ryib));
    nb += sizeof(*ddm->ryib);

    if(ddm->new_mpb()) {	// platform block
	sfile->write((const char *)ddm->asib, sizeof(*ddm->asib));
	nb += sizeof(*ddm->asib);
    }
    if (ddm->xstf && ddm->xstf->sizeof_struct > 0) { 
      // deal with the XTRA_STUFF catch all descripter
      
	sfile->write((const char *)ddm->xstf, ddm->xstf->sizeof_struct);
	nb += ddm->xstf->sizeof_struct;
    }

    int num_short;
    int sizeof_data;
    void * vdp;
    int ii, nn, skip = 1;
    double rcp_scale, bias, dval, s8, b8;
    short *ss;
    char * aa;
    
    for( fn = 0; fn < ddm->radd->num_parameter_des; fn++ ) {
	nn = num_short = ddm->celv->number_cells;
	
	sos = (ddm->qdats[fn]->pdata_desc[0] == 'R')
	    ?  sizeof(**ddm->rdats) :  sizeof(**ddm->qdats);
	
	if( convert_to_8_bit ) {
	    
	    if( produce_CSFD ) {
		skip = csfd_gate_skip;
		nn = (csfd_num_gates) ? csfd_num_gates :
		  ddm->celv->number_cells;
	    }

           s8 = scale8[fn];
	   if (strncmp (ddm->parms[fn]->parameter_name, "ZDR", 3) == 0)
	     { s8 = 4; }
	   b8 = bias8[fn];
	   rcp_scale = 1./ddm->parms[fn]->parameter_scale; 
	   bias = ddm->parms[fn]->parameter_bias;
	   bad = ddm->parms[fn]->bad_data;
	   aa = compressed_data;
	   ss = (short *)ddm->raw_data[fn];
	   for( ii = 0; ii < nn; ii++, aa++, ss += skip ) {
	     if (*ss == bad) {
	       *aa = (char)bad;
	     }
	     else {
	       dval = DD_UNSCALE( *ss, rcp_scale, bias );
	       *aa = (char)DD_SCALE( dval, s8, b8 );
	     }
	   }
	   vdp = (void *)compressed_data;
	   sizeof_data = LONGS_TO_BYTES( BYTES_TO_LONGS( nn ));
	}
	else if( the_typeof_compression == HRD_COMPRESSION ) {
	    num_short = dd_compress
		( (unsigned short *)ddm->raw_data[fn]
		  , (unsigned short *)compressed_data
		  , (unsigned short)ddm->parms[fn]->bad_data, num_short );
	    vdp = (void *)compressed_data;
	    sizeof_data = LONGS_TO_BYTES( SHORTS_TO_LONGS( num_short ));
	}
	else {
	    sizeof_data = LONGS_TO_BYTES( SHORTS_TO_LONGS( num_short ));
	    vdp = (void *)ddm->raw_data[fn];
	}

	ddm->qdats[fn]->pdata_length = sos + sizeof_data;

	sfile->write((const char *)ddm->qdats[fn], sos);
	nb += sizeof(**ddm->rdats);

	sfile->write((const char *)vdp, sizeof_data);
	nb += sizeof_data;
    }
    rte->size = nb;		// finish the rotation angle table entry

    this->sswb->d_stop_time = ddm->dtime;
    this->sswb->sizeof_file = sfile->tellp();
    this->swib->stop_angle = ddm->rotation_angle();

    return 1;
}

// c---------------------------------------------------------------------------

void
dd_sweepfile::
enlarge_rotang_table()
{
    max_rays += 1000;
    int mm = sizeof(*rktb);
    mm = ((mm -1)/8 +1) * 8; // start tables on an 8 byte boundary 
    int nn = mm + angle_ndx_size * sizeof(long); // angle index table

    char * aa = new char [nn + max_rays * sizeof(*first_rte)];

    if(rotang_buf) {
	memcpy(aa, rotang_buf, rktb->sizeof_struct);
	delete [] rotang_buf;
	rktb = (rot_ang_table *)aa;
    }
    else {			// initializing
	rktb = (rot_ang_table *)aa;
	strncpy(rktb->name_struct, "RKTB", 4);
	rktb->ndx_que_size = angle_ndx_size;
	rktb->angle2ndx = (float)angle_ndx_size/360.;
	rktb->angle_table_offset = mm;
	rktb->first_key_offset = nn;
	rktb->sizeof_struct = rktb->first_key_offset;
    }
    rotang_buf = aa;
    angle_zero = (long *)(aa + rktb->angle_table_offset);
    first_rte = (rot_table_entry *)(aa + rktb->first_key_offset);
}

// c---------------------------------------------------------------------------

void
dd_sweepfile::
update_rotang_table()
{
    if(rktb->num_rays +1 > max_rays)
	{ enlarge_rotang_table(); }
    
    int rnum = rktb->num_rays++;
    rte = first_rte + rnum;
    
    float rot = ddm->rotation_angle();
    int ii = (int)(rot * rktb->angle2ndx);
    *(angle_zero + ii) = rnum;
    rte->rotation_angle = rot;
    rte->offset = sswb->sizeof_file;
    rktb->sizeof_struct += sizeof(*rte);
}

// c---------------------------------------------------------------------------

int
dd_sweepfile::
seek_ray( int ray_num)
{
    if(ray_num < 0 || ray_num >= rktb->num_rays)
	{ return -1; }
	
    rte = first_rte + ray_num;
    swp_ray_num = ray_num;
	
    sfile->seekp((std::streampos)rte->offset);
    if(sfile->fail()) {
	printf( " out seek failed: %x\n", sfile->rdstate());
	return READ_FAILURE;
    }

    return 1;
}

// c---------------------------------------------------------------------------

int
dd_sweepfile::
reread_ray( int ray_num, char * data_ptr )
{
# ifdef notyet
    // read in the next ray
    
    this->seek_ray( ray_num );
    
    sfile->read(data_ptr, (int)rte->size);
    
    if(sfile->fail()) {
	printf( " out read failed: %x\n", sfile->rdstate());
	return READ_FAILURE;
    }
    int bytes_used = 0;
    
    ddm->map_ptrs(data_ptr, (int)rte->size, &bytes_used);
    
    swp_ray_num++;
    rte++;
    return bytes_used;
# endif
    return 0;
}

// c---------------------------------------------------------------------------

void
dd_sweepfile::
rewrite_ray( int ray_num )
{

    this->seek_ray( ray_num );

    int fn, nb = 0, sos;
    int sizeof_data = LONGS_TO_BYTES
	(BYTES_TO_LONGS(ddm->celv->number_cells * sizeof(short)));

    sfile->write((const char *)ddm->ryib, sizeof(*ddm->ryib));
    nb += sizeof(*ddm->ryib);

    if(ddm->new_mpb()) {
	sfile->write((const char *)ddm->asib, sizeof(*ddm->asib));
	nb += sizeof(*ddm->asib);
    }
    sos = (ddm->qdats[0]->pdata_desc[0] == 'R')
	?  sizeof(**ddm->rdats) :  sizeof(**ddm->qdats);

    for( fn = 0; fn < ddm->radd->num_parameter_des; fn++ ) {
	ddm->qdats[fn]->pdata_length = sos + sizeof_data;
	sfile->write((const char *)ddm->qdats[fn], sos);
	nb += sizeof(**ddm->rdats);
	sfile->write((const char *)ddm->raw_data[fn], sizeof_data);
	nb += sizeof_data;
    }
}

// c---------------------------------------------------------------------------



// c---------------------------------------------------------------------------

int
dd_sweepfile_access::
access_sweepfile( const char * full_path_name, dd_mapper * ddmpr )
{
    // open it and read in the first ray

    ddm = ddmpr;

    if (sfile)
    {
	delete sfile;
	sfile = 0;
    }
    sfile = new std::ifstream(full_path_name, std::ios::in);

    if(sfile->fail()) {
	char message[256];
	perror("fstream");
	sprintf(message, "dd_sweepfile_access::access_sweepfile : fstream failure - Unable to access sweepfile name\n\t%s\n"
		, full_path_name);
	dd_Testify(message);
    delete sfile;
    sfile = NULL;
	return 0;
    }
#ifdef NOTDEF
    int len = 2 * sizeof( SUPERSWIB );
#else
    int len_gd =  sizeof(struct generic_descriptor);
    GD_PTR gPtr;
#endif

    // read in the super_SWIB

    sfile->read((char *)data_ptr, len_gd);

    if(sfile->gcount() < len_gd) {
	char message[256];
	sprintf(message, "Unable to read from sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }

    if( strncmp( data_ptr, "SSWB", 4 ) ) {
	char message[256];
	sprintf(message, "sac:No SSWB in sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }
    gPtr= (GD_PTR) data_ptr;
    // determine size of struct, byte-swapping if necessary.
    // since the size of the struct differs between x86 and SPARC
    // processors, and the struct could have been written on a different
    // processor, it is safest to use read the size ,and then read
    // the remaining bytes of the SSWB.
    unsigned int len = gPtr->sizeof_struct;
    if (len > MAX_REC_SIZE) {
	swack4( (char *)&gPtr->sizeof_struct, (char *)&len );
    }
    // read in the remaining portion of the SUPERSWIB
    sfile->read((char *)(data_ptr+len_gd), len - len_gd);
    int bytes_used = 0;

    ddm->map_ptrs(data_ptr, len, &bytes_used);

    // now determine how much else to read in

    len = ddm->sswb->sizeof_file < MAX_D_HEADER ?
	ddm->sswb->sizeof_file  : MAX_D_HEADER;

    // this is a bit ugly, but we need to process the entire header at
    // once, otherwise ddm->map_ptrs() won't recognize the SSWB properly.
    // although we've already read it.
    // 
    // Because of the side effects in ddm->map_ptrs(),
    // it is safest to re-read, and re-process
    // the sweepfile headers.
    // (otherwise, ddm->map_ptrs() can get confused.
    sfile->seekg(0, std::ios_base::beg);	  // rewind
    sfile->read((char *)data_ptr, len );

    if(sfile->gcount() < (int)len) {
	char message[256];
	sprintf(message, "Unable to read from sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }

    bytes_used = 0;
    ddm->map_ptrs(data_ptr, len, &bytes_used);

    if (sfile->eof())
	sfile->clear();	// EOF is OK; we're going to rewind the file anyway

    // map volume headers

    if( !ddm->found_ryib() ) {
	char message[256];
	sprintf(message, "sac:Could not complete map of sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }

    sfile->seekg( 0, std::ios::end );
    std::streampos spos_eof = sfile->tellg();
    int sizef = (int)spos_eof;
    if( (std::streampos)ddm->sswb->sizeof_file - spos_eof > 4 ) {
	char message[256];
	sprintf(message, "sac:Internal (%ld) and external (%d) filesize mismatch:\n%s\n"
		, ddm->sswb->sizeof_file, sizef, full_path_name);
	dd_Testify(message);
	return 0;
    }

    std::streampos spos = ddm->sswb->key_table[0].offset;
    int size = ddm->sswb->key_table[0].size;

    if(size > sizeof_rktb) {
	if(sizeof_rktb > 0) {
	    delete [] rotang_buf;
	    if( swapped_rktb ) {
		delete [] swapped_rktb;
		swapped_rktb = NULL;
	    }
	}
	sizeof_rktb = size + 100 * sizeof(*rte);
	rotang_buf = new char [sizeof_rktb];
    }
    rktb = (rot_ang_table *)rotang_buf;
    sfile->seekg(spos);

    // now suck in to rotation angle table

    sfile->read((char *)rktb, size);
    if(sfile->fail()) {
	char message[256];
	sprintf(message, "Unable to read RKTB from sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }
    if( strncmp( (char *)rktb, "RKTB", 4 )) {
	char message[256];
	sprintf(message, "sac:No RKTB in sweepfile:\n%s\n"
		, full_path_name);
	dd_Testify(message);
	return 0;
    }
    int offs;

    // set up to read in the first ray and map it

    if( ddm->swapped_data() ) {
	if( !swapped_rktb )
	    { swapped_rktb = new char [sizeof_rktb]; }
	ddin_crack_rktb( rotang_buf, swapped_rktb, (int)0);
	rktb = (rot_ang_table *)swapped_rktb;

	offs = rktb->angle_table_offset;
	swack_long( rotang_buf + offs, swapped_rktb + offs
		    , rktb->ndx_que_size );
	angle_zero = (long *)( swapped_rktb + offs );

	offs = rktb->first_key_offset;
	swack_long( rotang_buf + offs, swapped_rktb + offs
		    , rktb->num_rays * sizeof(rot_table_entry)/sizeof(long) );
	first_rte = rte =(rot_table_entry *)( swapped_rktb + offs);
    }
    else {
	angle_zero = (long *)(rotang_buf + rktb->angle_table_offset );
	first_rte = rte =
	    (rot_table_entry *)(rotang_buf + rktb->first_key_offset);
    }

    sfile->seekg((std::streampos)first_rte->offset);

    swp_ray_num = 0;

    // nab the first ray
    int ii = this->next_ray();

    this->ddm->dd_new_sweep = YES;

    dd_new_vol = this->ddm->dd_new_vol =
	( this->ddm->vold->volume_num != prev_vol_num );

    prev_vol_num = this->ddm->vold->volume_num;

    return ii;
}

// c---------------------------------------------------------------------------

int
dd_sweepfile_access::
next_ray()
{

    // read in the next ray

    if(swp_ray_num  >= rktb->num_rays)
	{ return LAST_RAY; }

    sfile->read((char *)data_ptr, (int)rte->size);

    if(sfile->fail()) {
	return READ_FAILURE;
    }
    int bytes_used = 0;

    ddm->map_ptrs(data_ptr, (int)rte->size, &bytes_used);

    swp_ray_num++;
    rte++;
    return bytes_used;
}

// c---------------------------------------------------------------------------

int
dd_sweepfile_access::
angle_to_ray_num( float angle )
{
    // return the ray number closest to this angle.

    int ray_num = -1;
    int rayx = 0;
    int nqs = rktb->ndx_que_size;
    int loop;

    for(; angle < 0 ; angle += 360. );
    for(; angle >= 360. ; angle -= 360. );

    int ndx0 = (int)(rktb->angle2ndx * angle);
    int ndx;
    double diff = 1.e22, diffx;

    if( (ray_num = *(angle_zero + ndx0)) >= 0 ) {
	diff = fabs
	    ( ddm->AngDiff( (float)(first_rte + ray_num)->rotation_angle
			      , angle ) );
    }
    ndx = ndx0;

    // find the next ray clockwise

    for( loop = nqs; loop-- ; ) {
	if( ++ndx >= nqs )
	    { ndx = 0; }
	if( (rayx =  *(angle_zero + ndx)) >= 0 )
	    { break; }
    }
    diffx = fabs
	( ddm->AngDiff( (float)(first_rte + rayx)->rotation_angle, angle) );

    if( diffx < diff )
	{ ray_num = rayx; diff = diffx; }

    ndx = ndx0;

    // find the next ray counter clockwise

    for( loop = nqs; loop-- ; ) {
	if( --ndx < 0 )
	    { ndx = nqs -1; }
	if(( rayx =  *(angle_zero + ndx) ) >= 0 )
	    { break; }
    }
    diffx = fabs
	( ddm->AngDiff( (float)(first_rte + rayx)->rotation_angle, angle ) );

    if( diffx < diff )
	{ return rayx; }

    return ray_num;
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

dd_mem_sweepfile::
dd_mem_sweepfile( int max_file_size, int min_rays ) {

    min_rays_per_sweep = min_rays;
    ddt = new DD_Time();

    int nb = 12 * 1024;
    local_buf = new char[nb];
    memset(local_buf, 0, nb);
    free_at = local_buf + 2 * 1024;

    sswb = new super_SWIB;
    strncpy((char *)sswb, "SSWB", 4);
    sswb->sizeof_struct = sizeof(*sswb);
    sswb->sizeof_file = 0;
    sswb->version_num = 1;
    sswb->num_key_tables = 1;
    sswb->key_table[0].type = KEYED_BY_ROT_ANG;
    sswb->key_table[0].offset = 0;
    sswb->key_table[0].size = 0;

    vold = new VOLUME;
    radd = new RADARDESC;
    swib = new SWEEPINFO;

    null_des = new generic_descriptor;
    strncpy((char *)null_des->name_struct, "NULL", 4);
    null_des->sizeof_struct = sizeof(*null_des);


    the_max_file_size = max_file_size > 0 ? max_file_size :
	6 * 1024 * 1024;
    mem_sweepfile = new char [max_file_size];
    current_offset = 0;

    max_rays = 0;
    rotang_buf = NULL;
    enlarge_rotang_table();
    swp_count_out = 0;
}
dd_mem_sweepfile::
~dd_mem_sweepfile()
{
    delete [] local_buf;
    delete [] rotang_buf;
    delete [] mem_sweepfile;
    delete ddt;
    delete sswb;
    delete swib;
    delete vold ;
    delete radd;
    delete null_des;
}



// c---------------------------------------------------------------------------

int
dd_mem_sweepfile::
begin_sweepfile(dd_mapper * ddmpr, char * directory,  char * qualifier
		, int version_num )
{
    swp_ray_num = 0;

    ddm = ddmpr;
    swp_count_out++;

    str_terminate(this->sswb->radar_name, ddm->radd->radar_name, 8);

    memcpy(swib, ddm->swib, sizeof(*swib));

# ifdef obsolete
    sprintf( filename + strlen(filename)
	     , ".%.1f_%s"
	     , swib->fixed_angle
	     , qualifier
	);
# endif
    memcpy(vold, ddm->vold, sizeof(*vold));
    vold->year = ddm->ddt->year();
    vold->month = ddm->ddt->month();
    vold->day = ddm->ddt->day();
    vold->data_set_hour = ddm->ddt->hour();
    vold->data_set_minute = ddm->ddt->minute();
    vold->data_set_second = ddm->ddt->second();

    // set todays date
    ddt->accept_time_stamp( (DTime)time_now() );
    ddt->unstamp_the_time();
    vold->gen_year = ddt->year();
    vold->gen_month = ddt->month();
    vold->gen_day = ddt->day();
 
    current_offset = 0;
    char * aa = mem_sweepfile;
    memcpy( aa, sswb, sizeof(*sswb));
    int nb = sizeof(*sswb);

    memcpy( radd, ddm->radd, sizeof( *radd ));
    radd->data_compress = 0;	// no compression for now

    memcpy( aa + nb, vold, sizeof(*vold));
    nb += sizeof(*vold);

    memcpy( aa + nb, radd, sizeof(*radd));
    nb += sizeof(*radd);

    for(int pn = 0; pn < ddm->radd->num_parameter_des; pn++) {
	memcpy( aa + nb, ddm->parms[pn], sizeof(**ddm->parms));
	nb += sizeof(**ddm->parms);
    }
    memcpy( aa + nb, ddm->celv, ddm->celv->cell_des_len);
    nb += ddm->celv->cell_des_len;
    current_offset = nb;
    offset_to_cfac = current_offset;
    
    memcpy( aa + nb, ddm->cfac, sizeof(*ddm->cfac));
    nb += sizeof(*ddm->cfac);
    current_offset = nb;
    offset_to_swib = current_offset;

    this->swib->num_rays = 0;
    this->swib->start_angle = ddm->rotation_angle();
    memcpy( aa + nb, this->swib, sizeof(*swib));
    nb += sizeof(*swib);
    current_offset = nb;

    this->sswb->sizeof_file = current_offset;
    this->sswb->d_start_time = ddm->dtime;

    reset_rotang_table();

    // leave it to the calling method to add the first beam

    return 0;
}

// c---------------------------------------------------------------------------

const char *
dd_mem_sweepfile::
end_sweepfile( int kill_it )	// may be passed a kill flag
{
    if(this->sswb->sizeof_file <= 0)
	{ return NULL; }

    current_offset = sswb->sizeof_file;

    // add the null descriptor
    memcpy( mem_sweepfile + current_offset, null_des, sizeof(*null_des));
    current_offset += sizeof(*null_des);
    
    // write out the rotation angle table
    this->sswb->key_table[0].offset = current_offset;
    this->sswb->key_table[0].size = rktb->sizeof_struct;

    memcpy( mem_sweepfile + current_offset, rktb, rktb->sizeof_struct);
    current_offset += rktb->sizeof_struct;

    this->sswb->sizeof_file = current_offset;

    // postion to write the super_SWIB
    memcpy( mem_sweepfile, this->sswb, sswb->sizeof_struct);

    // write out the correction factors again
    memcpy( mem_sweepfile + offset_to_cfac
	    , ddm->cfac, ddm->cfac->correction_des_length);

    // write out the sweep info block
    memcpy( mem_sweepfile + offset_to_swib
	    , this->swib, swib->sweep_des_length);

    current_offset = 0;
    this->sswb->sizeof_file = 0;

    // move the file to its permanent name
    
    return "";
}

// c---------------------------------------------------------------------------

int
dd_mem_sweepfile::
add_to_sweepfile()
{
    //
    char * aa = mem_sweepfile + current_offset;
    int fn, nb = 0, sos;
    int sizeof_data = LONGS_TO_BYTES
	(BYTES_TO_LONGS(ddm->celv->number_cells * sizeof(short)));

    ++this->swib->num_rays;
    ddm->ryib->sweep_num = ddm->sweep_count();
    ddm->ryib->sweep_num = swp_count_out;

    this->sswb->sizeof_file = current_offset;
    update_rotang_table();

    memcpy(aa, ddm->ryib, sizeof(*ddm->ryib));
    nb += sizeof(*ddm->ryib);
    if(ddm->new_mpb()) {
	memcpy(aa + nb, ddm->asib, sizeof(*ddm->asib));
	nb += sizeof(*ddm->asib);
    }

    for( fn = 0; fn < ddm->radd->num_parameter_des; fn++ ) {
	sos = (ddm->qdats[fn]->pdata_desc[0] == 'R')
	    ?  sizeof(**ddm->rdats) :  sizeof(**ddm->qdats);
	ddm->qdats[fn]->pdata_length = sos + sizeof_data;
	memcpy(aa + nb, ddm->qdats[fn], sos);
	nb += sizeof(**ddm->rdats);
	memcpy(aa + nb, ddm->raw_data[fn], sizeof_data);
	nb += sizeof_data;
    }
    rte->size = nb;
    current_offset += nb;
    this->sswb->d_stop_time = ddm->dtime;
    this->sswb->sizeof_file = current_offset;
    this->swib->stop_angle = ddm->rotation_angle();
    return 1;
}

// c---------------------------------------------------------------------------

int
dd_mem_sweepfile::
seek_ray( int ray_num) {
    if(ray_num < 0 || ray_num >= rktb->num_rays)
	{ return -1; }
	
    rte = first_rte + ray_num;
    swp_ray_num = ray_num;
	
    current_offset = rte->offset;

    return 1;
}

// c---------------------------------------------------------------------------

int
dd_mem_sweepfile::
reread_ray( int ray_num, char * data_ptr )
{
    // read in the next ray
    
    this->seek_ray( ray_num );
    
    data_ptr = mem_sweepfile + current_offset;
    current_offset += rte->size;
    int bytes_used = 0;
    
    ddm->map_ptrs(data_ptr, (int)rte->size, &bytes_used);
    
    swp_ray_num++;
    rte++;
    return bytes_used;
}

// c---------------------------------------------------------------------------

int
dd_mem_sweepfile::
rewrite_ray( int ray_num )
{

    this->seek_ray( ray_num );

    int fn, nb = 0, sos;
    int sizeof_data = LONGS_TO_BYTES
	(BYTES_TO_LONGS(ddm->celv->number_cells * sizeof(short)));

    char * aa = mem_sweepfile + current_offset;

    memcpy(aa, ddm->ryib, sizeof(*ddm->ryib));
    nb += sizeof(*ddm->ryib);

    if(ddm->new_mpb()) {
	memcpy(aa + nb, ddm->asib, sizeof(*ddm->asib));
	nb += sizeof(*ddm->asib);
    }
    sos = (ddm->qdats[0]->pdata_desc[0] == 'R')
	?  sizeof(**ddm->rdats) :  sizeof(**ddm->qdats);

    // no need to write out the data

    for( fn = 0; fn < ddm->radd->num_parameter_des; fn++ ) {
	ddm->qdats[fn]->pdata_length = sos + sizeof_data;
	nb += sizeof(**ddm->rdats);
	nb += sizeof_data;
    }
    current_offset += nb;
    swp_ray_num++;
    rte++;

    return nb;
}

// c---------------------------------------------------------------------------

void
dd_mem_sweepfile::
enlarge_rotang_table() {
    max_rays += 1000;
    int mm = sizeof(*rktb);
    mm = ((mm -1)/8 +1) * 8; // start tables on an 8 byte boundary 
    int nn = mm + angle_ndx_size * sizeof(long); // angle index table

    char * aa = new char [nn + max_rays * sizeof(*first_rte)];

    if(rotang_buf) {
	memcpy(aa, rotang_buf, rktb->sizeof_struct);
	delete [] rotang_buf;
	rktb = (rot_ang_table *)aa;
    }
    else {			// initializing
	rktb = (rot_ang_table *)aa;
	strncpy(rktb->name_struct, "RKTB", 4);
	rktb->ndx_que_size = angle_ndx_size;
	rktb->angle2ndx = (float)angle_ndx_size/360.;
	rktb->angle_table_offset = mm;
	rktb->first_key_offset = nn;
	rktb->sizeof_struct = rktb->first_key_offset;
    }
    rotang_buf = aa;
    angle_zero = (long *)(aa + rktb->angle_table_offset);
    first_rte = (rot_table_entry *)(aa + rktb->first_key_offset);
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

int TimeSpanDir( char * ref_dir, struct solo_list_mgmt * rlist )
{
    static DD_File_names * ddfn = NULL;
    int nr = get_swp_files( ref_dir, rlist );
    if( nr < 2 )
      { return 0; }
    DTime t1, t2;
    char * fname = solo_list_entry( rlist, nr-1 );
    ddfn->crack_file_name( fname );
    t2 = ddfn->return_dtime();
    fname = solo_list_entry( rlist, 0 );
    ddfn->crack_file_name( fname );
    t1 = ddfn->return_dtime();
    double d = t2 - t1;
    return (int)d;
}

// c---------------------------------------------------------------------------

int CleanDir( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
	, int t_span )
{
    // 
    // 
    //
    char x_link[ 256 ];
    char *bb, *fname;
    int jj, zapped = 0;
    static DD_File_names * ddfn = NULL;

    if( rcount < 1 )
	{ return zapped; }

    if( !ddfn ) {
	ddfn = new DD_File_names();
    }
    strcpy( x_link, ref_dir );
    bb = x_link + strlen( x_link );

    int nr = get_swp_files( ref_dir, rlist );
    if (nr < 1)
      { return zapped; }
    int keep = nr;

    // get the time for the latest file
    DTime t0, t1, t2;

    if( t_span > 0 ) {
	fname = solo_list_entry( rlist, nr-1 );
	ddfn->crack_file_name( fname );
	t2 = ddfn->return_dtime();
	t1 = t2 - t_span;

	for( jj=0; keep > rcount && jj < nr; jj++ ) {
	    fname = solo_list_entry( rlist, jj );
	    ddfn->crack_file_name( fname );
	    t0 = ddfn->return_dtime();
            if( t0 < t1 ) {
                strcpy( bb, fname );
                unlink( x_link );
                keep--;
                zapped++;
            }
	}
	return zapped;
    }
    int nz = nr - rcount;

    for( jj=0; jj < nz; jj++ ) {
	fname = solo_list_entry( rlist, jj );
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------

int MirrorDir( char * ref_dir, struct solo_list_mgmt * rlist
	, char * mirror_dir, struct solo_list_mgmt * mlist, int mcount )
{
    // this routine removes files/links in the mirror directory that do not
    // exist in the reference directory
    //

    char x_link[ 256 ];
    char *aa, *bb, *fname;
    int jj, kk, zapped = 0;

    if( mcount < 1 )
	{ return zapped; }

    strcpy( x_link, mirror_dir );
    bb = x_link + strlen( x_link );

    int nr = get_swp_files( ref_dir, rlist );
    int nm = get_swp_files( mirror_dir, mlist );

    if( nm < 1 )
	{ return zapped; }

    for( jj=0; jj < nm; jj++ ) {
	fname = solo_list_entry( mlist, jj );
	//	  printf( "swpfiD:fname:%s %d\n", fname, le );
	int found = 0;
	  
	for( kk = 0; kk < nr; kk++ ) {
	    aa = solo_list_entry( rlist, kk );
	    if( strcmp( fname, aa ) == 0 ) {
		//	      printf( "swpfiD:found:%s\n", aa );
		found = 1;
		break;
	    }
	}
	if( found )
	    { continue; }
    
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------

int nuCleanDir( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
	, int t_span )
{
    // 
    // 
    //
    char x_link[ 256 ];
    char *bb, *fname;
    int jj, zapped = 0;
    static Swp_File_names * ddfn = NULL;

    if( rcount < 1 )
	{ return zapped; }

    if( !ddfn ) {
	ddfn = new Swp_File_names();
    }
    strcpy( x_link, ref_dir );
    bb = x_link + strlen( x_link );

    int nr = generic_sweepfiles( ref_dir, rlist, "pqswp", "", NO );
    if (nr < 1)
      { return zapped; }
    int keep = nr;

    // get the time for the latest file
    DTime t0, t1, t2;

    if( t_span > 0 ) {
	fname = solo_list_entry( rlist, nr-1 );
	ddfn->crack_file_name( fname );
	t2 = ddfn->return_dtime();
	t1 = t2 - t_span;

	for( jj=0; keep > rcount && jj < nr; jj++ ) {
	    fname = solo_list_entry( rlist, jj );
	    ddfn->crack_file_name( fname );
	    t0 = ddfn->return_dtime();
            if( t0 < t1 ) {
                strcpy( bb, fname );
                unlink( x_link );
                keep--;
                zapped++;
            }
	}
	return zapped;
    }
    int nz = nr - rcount;

    for( jj=0; jj < nz; jj++ ) {
	fname = solo_list_entry( rlist, jj );
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------

int nuMirrorDir( char * ref_dir, struct solo_list_mgmt * rlist
	, char * mirror_dir, struct solo_list_mgmt * mlist, int mcount )
{
    // this routine removes files/links in the mirror directory that do not
    // exist in the reference directory
    //

    char x_link[ 256 ];
    char *aa, *bb, *fname;
    int jj, kk, zapped = 0;

    if( mcount < 1 )
	{ return zapped; }

    strcpy( x_link, mirror_dir );
    bb = x_link + strlen( x_link );

    int nr = generic_sweepfiles( ref_dir, rlist, "pqswp", "", NO );
    int nm = generic_sweepfiles( mirror_dir, mlist, "pqswp", "", NO );

    if( nm < 1 )
	{ return zapped; }

    for( jj=0; jj < nm; jj++ ) {
	fname = solo_list_entry( mlist, jj );
	//	  printf( "swpfiD:fname:%s %d\n", fname, le );
	int found = 0;
	  
	for( kk = 0; kk < nr; kk++ ) {
	    aa = solo_list_entry( rlist, kk );
	    if( strcmp( fname, aa ) == 0 ) {
		//	      printf( "swpfiD:found:%s\n", aa );
		found = 1;
		break;
	    }
	}
	if( found )
	    { continue; }
    
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------

int
Swp_File_names::
crack_file_name( const char * name )
{
  char zro = '\0';
  const char * under_score = "_";
  const char * aa = name;
  char *bb;
  char * cc = keep_name;

  if(!aa)
    { return 0; }

  int kn = sizeof( keep_name );
  for( size = 0; *aa && size < kn ; size++, *cc++ = *aa++ );
  *cc = zro;

  char str[256], *sptrs[32];
  strcpy(str, keep_name);
  int nt = dd_tokenz( str, sptrs, under_score );

  strcpy( the_file_type, sptrs[0] );
  strcpy( the_radar_name, sptrs[1] );

  ddt->reset();

  int yr, mon, day, hrs, min, secs;

  sscanf( sptrs[2], "%4d%2d%2d", &yr, &mon, &day );
  ddt->set_year( yr );
  ddt->set_month( mon );
  ddt->set_day( day );

  bb = strchr( sptrs[3], '.' );
  *bb++ = zro;		// mark the start of the_milliseconds

  sscanf( sptrs[3], "%2d%2d%2d", &hrs, &min, &secs );
  sscanf( bb, "%d", &the_milliseconds );

  ddt->set_additional_seconds( (DTime)D_SECS( hrs, min, secs
					      , the_milliseconds ));
  dtime = ddt->stamp_the_time();
  ttime = (long)dtime;

  bb = sptrs[4] +1;
  sscanf( bb, "%d", &vol_num );

  bb = sptrs[5] +1;
  sscanf( bb, "%d", &sweep_num );

  sscanf( sptrs[6], "%f", &the_fixed_angle );

  scan_mode = dd_scan_mode( sptrs[7] );

  if( nt >= 9 )
    { strcpy( the_qualifier, sptrs[8] ); }
	
  return 1;
}


// c---------------------------------------------------------------------------

int nab_and_sort_sweepfiles( char * dir, solo_list_mgmt *lm
    , DTime t1=0, DTime t2=ETERNITY )
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];
    char *aa, *bb, *sptrs[16];
    const char *tmp = ".tmp";
    static DD_Time * ddt = new DD_Time();
    static DD_File_names * ddfn = new DD_File_names();
    DTime dtime;
    int ii;

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	printf( "%s", mess );
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	aa = dp->d_name;
	if(strncmp(aa, "swp", 3))
	   { continue; }	/* does not compare! */
	bb = aa + strlen(aa) - strlen(tmp);
	if(!strncmp(bb, tmp, strlen(tmp)))
	    { continue; }	/* ignore ".tmp" files */

	ddfn->crack_file_name( aa );
	dtime = ddfn->return_dtime();
	if( dtime < t1 )
	    { continue; }
	if( dtime > t2 )
	    { continue; }

	ddt->accept_time_stamp( dtime );
	ddt->unstamp_the_time();
	sprintf( mess, "%d%02d%02d%02d%02d%02d.%03d"
		 , ddt->year()
		 , ddt->month()
		 , ddt->day()
		 , ddt->hour()
		 , ddt->minute()
		 , ddt->second()
		 , ddt->millisecond()
	    );
	strcat( mess, "!" );
	strcat( mess, dp->d_name );
	solo_add_list_entry(lm, mess, strlen(mess));
    }
    closedir(dir_ptr);
    if(lm->num_entries > 1) {
	solo_sort_strings(lm->list, lm->num_entries);
    }
    for( ii = 0; ii < lm->num_entries; ii++ ) {
	strcpy( mess, solo_list_entry( lm, ii ));
        dd_tokenz( mess, sptrs, "!" );
	solo_modify_list_entry( lm, sptrs[1], strlen(sptrs[1]), ii );
    }

    return(lm->num_entries);
}

// c---------------------------------------------------------------------------

int DD_File_names::crack_file_name( char * name )
    {
	char zro = '\0';
	const char * dot = ".";
	char * aa = name;
	char * cc = keep_name;

	if(!aa)
	    { return 0; }

	int kn = sizeof( keep_name );
	for( size = 0 ; *aa && size < kn ; size++, *cc++ = *aa++ );
	*cc = zro;

	char str[256], *str_ptrs[32];
	strcpy(str, keep_name);
/*	int nt = dd_tokenz( str, str_ptrs, dot ); */
	dd_tokenz( str, str_ptrs, dot );

	strcpy( the_radar_name, str_ptrs[2] );

	ddt->reset();

	aa = str_ptrs[1];	// main time stamp

	char *bb = aa + strlen( aa ) - 10; // ready to suck off all but year
	int mon, day, hrs, min, secs;

	sscanf( bb, "%2d%2d%2d%2d%2d", &mon, &day, &hrs, &min, &secs );

	ddt->set_day( day );
	ddt->set_month( mon );
	*bb = zro;
	ddt->set_year( atoi( aa ) ); // might be more than two characters

	int ms = atoi( str_ptrs[3] );
	the_milliseconds = ms % 1000;
	the_version = ms/1000;


	ddt->set_additional_seconds( (DTime)D_SECS( hrs, min, secs
						    , the_milliseconds ));

	dtime = ddt->stamp_the_time();
	ttime = (long)dtime;

	// get to the qualifier (may have dots of its own)
	int nd = 0;
	aa = keep_name;
	for( ; *aa && nd < 4; aa++ ) {
	    if( *aa == *dot )
		{ nd++; }
	}
	strcpy( the_qualifier, aa );
	strcpy (the_qualifier_no_fxd, the_qualifier);

	the_fixed_angle = -999.; // see if we can pull out a fixed angle

	if( (bb = strstr( aa, "_" ))) {
	    char *cc = str;
	    for(; aa < bb; *cc++ = *aa++);
	    *cc = '\0';
	    if( sscanf( str, "%f", &the_fixed_angle ) != 1 )
		{ the_fixed_angle = -999.; }

	    if (the_fixed_angle != -999.) {
	      strcpy (the_qualifier_no_fxd, ++aa);
	    }
	}
	return 1;
    }



// c---------------------------------------------------------------------------

