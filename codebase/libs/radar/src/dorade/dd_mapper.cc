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
/* 	$Id: dd_mapper.cc,v 1.7 2016/03/03 18:45:06 dixon Exp $	 */


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <time.h>
# include <sys/time.h>
# include <signal.h>

# include <radar/dorade/dd_mapper.hh>



// LittleEndian is set in ../ddutils/spol_C_utils.c
extern int LittleEndian;

extern "C"
{
  int swap4( char *ov );
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

dd_mapper::
dd_mapper() {
    const int K = 1024;
    int nb = ( 4 +  MAX_PARMS*4 ) * MAXCVGATES;
    int ii;
	
    local_buf = new char[nb];
    memset(local_buf, 0, nb);
    free_at = local_buf;
    free_at += 4 * K;	// safety
    data_buf = free_at;
    free_at += 64 * K;

    sswb = new super_SWIB;
    swib = new SWEEPINFO;
    vold = new VOLUME;
    radd = new RADARDESC;
    for (ii = 0; ii < MAX_FIELDS; ii++) {
	field_names[ii][0] = '\000';
    }
    for(ii = 0; ii < MAX_RADARDESC; ii++) {
        radds[ ii ] = new RADARDESC;
    }

    for(ii = 0; ii < MAX_FIELDS; ii++) {
        parms[ii] = new PARAMETER;
        qdats[ii] = new QPARAMDATA; 
        rdats[ii] = (PARAMDATA *)qdats[ii];
        comms[ii] = NULL;
    }

    frib = fribx = 0;

    celv = new CELLVECTOR;

    celvc = new CELLVECTOR;

    cfac = new CORRECTION;

    ryib = new RAY;

    asib = new PLATFORM;
    cell_lut = 0;

    scr0 = free_at;

    int nn = (2 * MAXCVGATES) * sizeof(int);
    if( (free_at + nn) > (local_buf + nb)) {
	printf("\nExceeded the size of local_buf %p %p %p %p\n"
	    , free_at, (free_at + nn), local_buf
	    , (local_buf + nb) );
	exit(1);
    }

    ddt = new DD_Time();
    count = dd_vol_count = dd_sweep_count = dd_ray_count = 0;
    dd_sweep_ray_count = 0;
    dd_swapped_data = 0;
    dd_new_asib = NO;
    ignore_cfac = NO;
    num_radardesc = 1;
    comment_count = 0;
    constructed_parm_count = 0;
    max_scan_modes = 11;

    for( ii = 0; ii < max_scan_modes; ii++ ) {
      switch( ii ) {
      case CAL:
	scan_mode_mnes[ii] = "CAL";
	break;
      case PPI:
	scan_mode_mnes[ii] = "PPI";
	break;
      case COP:
	scan_mode_mnes[ii] = "COP";
	break;
      case RHI:
	scan_mode_mnes[ii] = "RHI";
	break;
      case VER:
	scan_mode_mnes[ii] = "VER";
	break;
      case TAR:
	scan_mode_mnes[ii] = "TAR";
	break;
      case MAN:
	scan_mode_mnes[ii] = "MAN";
	break;
      case IDL:
	scan_mode_mnes[ii] = "IDL";
	break;
      case SUR:
	scan_mode_mnes[ii] = "SUR";
	break;
      case AIR:
	scan_mode_mnes[ii] = "AIR";
	break;
      case HOR:
	scan_mode_mnes[ii] = "HOR";
	break;
      default:
	scan_mode_mnes[ii] = "";
	break;
      }
    }
    num_alias_sets = sizeof( known_aliases )/sizeof( char * );

    int num_radar_types = 10;

    for( ii = 0; ii < num_radar_types; ii++ ) {

      switch(ii) {
	
      case GROUND:
	radar_types_ascii[ii] = "GROUND";
	break;

      case AIR_FORE:
	radar_types_ascii[ii] = "AIR_FORE";
	break;

      case AIR_AFT:
	radar_types_ascii[ii] = "AIR_AFT";
	break;

      case AIR_TAIL:
	radar_types_ascii[ii] = "AIR_TAIL";
	break;

      case AIR_LF:
	radar_types_ascii[ii] = "AIR_LF";
	break;

      case SHIP:
	radar_types_ascii[ii] = "SHIP";
	break;

      case AIR_NOSE:
	radar_types_ascii[ii] = "AIR_NOSE";
	break;

      case SATELLITE:
	radar_types_ascii[ii] = "SATELLITE";
	break;

      case LIDAR_MOVING:
	radar_types_ascii[ii] = "LIDAR_MOVING";
	break;

      case LIDAR_FIXED:
	radar_types_ascii[ii] = "LIDAR_FIXED";
	break;

      default:
	radar_types_ascii[ii] = "UNKNOWN";
	break;
      }
    }

    sizeof_xstf = 0;
    this->xstf = 0;
}

// c---------------------------------------------------------------------------

dd_mapper::
~dd_mapper()
{
    delete [] local_buf;
    delete ddt;
    delete sswb;
    delete swib;
    delete vold ;
    delete radd;
    delete[] (char*)xstf;
    int ii;
    for(ii = 0; ii < MAX_RADARDESC; ii++) {
        delete radds[ ii ];
    }

    for(ii = 0; ii < MAX_FIELDS; ii++) {
        delete parms[ii];
        delete qdats[ii]; 
        delete comms[ii];
    }

    delete celv;

    delete celvc;

    delete cfac;

    delete ryib;

    delete asib;
    constructed_parm_count = 0;
}

           

// c---------------------------------------------------------------------------

int
dd_mapper::
map_ptrs( char * buf, int size, int * bytes_used )
{
    char * at = buf, *aa, *bb;
    short * ss, ns;
    int bytes_left = size, ii, mm, nn;
    float gs, rr, *fp;
    double b8, s16, dval;
    generic_descriptor * gd;
    unsigned long gdsos;
    int sizeof_data;
    int num_parms = 0, num_rdats = 0, pn, fn;
    int gotta_celv = NO, num_radds = 0;
    dd_new_vol = dd_new_sweep = dd_new_ray = dd_new_asib = NO;
    dd_complete = dd_sweep_file = NO, dd_sizeof_ray = 0;
    dd_found_ryib = NO;
    result = 0;
    comment_d * commx;
    int mcopy, offset, final_bad_count;
    dd_swapped_data = NO;
    CELLSPACINGFP * csfd;
    XTRA_STUFF *xstfo;

    char *raw_at = data_buf;	// in case we have to swap or decompress data
				// otherwise we point to the actual raw data

    count++;
    if (this->xstf)
      { this->xstf->sizeof_struct = 0; } // nonzero indicates a new xstf
    
    for( ; bytes_left > (int)sizeof(generic_descriptor) ;) {
	gd = (generic_descriptor *)at;
	gdsos = gd->sizeof_struct;
	if( gdsos > MAX_REC_SIZE) {
	   dd_swapped_data = YES;
	   swack4( (char *)&gd->sizeof_struct, (char *)&gdsos );
	   // fix size, since we use it later when processing COMM records
	   //	   gd->sizeof_struct = gdsos;
	}
	
	if(gdsos > (unsigned long)bytes_left) {
	    dd_new_ray = dd_new_vol = dd_complete = NO;
	    return dd_complete;
	}
	if(gdsos < sizeof(struct generic_descriptor) ||
	   gdsos > MAX_REC_SIZE) {
	    dd_error_state = DATA_UNDECIPHERABLE;
	    return (dd_complete = NO);
	}
	
	if(!strncmp(at, "RDAT", 4) || !strncmp(at, "QDAT", 4)) {
	    // acutal data!
	    //
	    fn = num_rdats++;
	    offset = (*at == 'R') ? sizeof(PARAMDATA)
		: this->parms[fn]->offset_to_data;

	    mcopy = (*at == 'R') ? sizeof(**rdats) : sizeof(**qdats);

	    // nab the header in the data block

	    if( dd_swapped_data ) {
		ddin_crack_qdat(at, (char *)this->qdats[fn], mcopy);
	    }
	    else {
		memcpy(this->qdats[fn], at, mcopy);
	    }
	    sizeof_data = LONGS_TO_BYTES
		(BYTES_TO_LONGS( this->celv->number_cells * sizeof(short)));

	    // deal with compression

	    if( its_8_bit[fn] ) {
		nn = this->celv->number_cells;
		sizeof_data = LONGS_TO_BYTES( BYTES_TO_LONGS( nn ));
		this->raw_data[fn] = raw_at;
		aa = at + offset;
		ss = (short *)raw_at;
		if( scale8[fn] == 1. ) {
		    b8 = bias8[fn];
		    s16 = this->parms[fn]->parameter_scale;
		    for( ii = 0; ii < nn; ii++, aa++, ss++ ) {
			dval = DD_UNSCALE( *aa, 1., b8 );
			*ss = (short)DD_SCALE( dval, s16, 0 );
		    }
		}
		else {
		    for( ii = 0; ii < nn; ii++ )
			{ *ss++ = *aa++; }
		}
		raw_at += sizeof_data * sizeof( short );
	    }
	    else if( this->radd->data_compress == NO_COMPRESSION ) {

		if( dd_swapped_data ) {
		    swack_short( at + offset, raw_at, (gdsos - mcopy) >> 1 );
		    this->raw_data[fn] = raw_at;
		    raw_at += gdsos - mcopy;
		}
		else {
		    this->raw_data[fn] = at + offset;
		}
	    }
	    else if( dd_swapped_data ) {
		nn = dd_hrd16LE_uncompressx
		     ((unsigned short *)(at + offset)
		      , (unsigned short *)raw_at
		      , (int)this->parms[fn]->bad_data
		      , &final_bad_count
		      , this->celv->number_cells);
		    this->raw_data[fn] = raw_at;
		    raw_at += nn * sizeof(short);
	    }
	    else {		// basic HRD 16-bit compression
		nn = dd_hrd16_uncompressx
		     ((unsigned short *)(at + offset)
		      , (unsigned short *)raw_at
		      , (int)this->parms[fn]->bad_data
		      , &final_bad_count
		      , this->celv->number_cells);
		    this->raw_data[fn] = raw_at;
		    raw_at += nn * sizeof(short);
	    }
	}
	else if(!strncmp(at, "RYIB", 4) && dd_new_ray) {
	    break;
	}
	else if(!strncmp(at, "RYIB", 4)) {
	    dd_found_ryib = dd_new_ray = YES;
	    if( dd_swapped_data ) {
		ddin_crack_ryib(at, (char *)this->ryib, (int)0);
	    }
	    else {
		memcpy(this->ryib, at, gdsos);
	    }
	    dd_sweep_ray_count++;
	    dd_ray_count++;

	    ddt->reset();
	    DTime dt = (this->ryib->julian_day -1) * SECONDS_PER_DAY +
		D_SECS(this->ryib->hour, this->ryib->minute, this->ryib->second
			, this->ryib->millisecond);
	    this->ddt->set_year( this->vold->year );
	    this->ddt->set_additional_seconds(dt);
	    this->ddt->stamp_the_time();
	    this->dtime = this->ddt->time_stamp();
	    this->ddt->unstamp_the_time(); // generates yy, mon, dd, hh, mm, ss, ms
	}
	else if(!strncmp(at, "XSTF", 4)) {
	  // deal with the XTRA_STUFF catch all descripter

	  if ((unsigned long)sizeof_xstf != gdsos) {
	    if (this->xstf)
	      { aa = (char *)this->xstf; delete [] aa; }
	    aa = new char [gdsos];
	    this->xstf = (XTRA_STUFF *)aa;
	    sizeof_xstf = gdsos;
	  }
	  memcpy ((char *)this->xstf, at, gdsos);

	  if( dd_swapped_data ) {
	    xstfo = (XTRA_STUFF *)at;

	    this->xstf->sizeof_struct = gdsos;
	    this->xstf->source_format =
	      swap4 ((char *)&xstfo->source_format);
	    this->xstf->offset_to_first_item =
	      swap4 ((char *)&xstfo->offset_to_first_item);
	    this->xstf->transition_flag =
	      swap4 ((char *)&xstfo->transition_flag);
	  }
	}
	else if(!strncmp(at, "ASIB", 4)) {
	    if( dd_swapped_data ) {
		ddin_crack_asib(at, (char *)this->asib, (int)0);
	    }
	    else {
		memcpy(this->asib, at, gdsos);
	    }
	    dd_latitude = this->asib->latitude;
	    dd_longitude = this->asib->longitude;
	    dd_altitude = this->asib->altitude_msl;
	    dd_new_asib = YES;
	}
	else if(!strncmp(at, "SWIB", 4)) {
	    if( dd_swapped_data ) {
		ddin_crack_swib(at, (char *)this->swib, (int)0);
	    }
	    else {
		memcpy(this->swib, at, gdsos);
	    }
	    this->swib->sweep_des_length = sizeof(*swib);
	    dd_rays_prev_sweep = dd_sweep_ray_count;
	    dd_sweep_ray_count = 0;
	    dd_sweep_count++;
	    dd_new_sweep = YES;
	}
	else if(!strncmp(at, "VOLD", 4)) {
	    dd_vol_count++;
	    dd_new_vol = YES;
	    if( dd_swapped_data ) {
		ddin_crack_vold(at, (char *)this->vold, (int)0);
	    }
	    else {
		memcpy(this->vold, at, gdsos);
	    }
	    this->vold->volume_des_length = sizeof(*vold);
	}
	else if(!strncmp(at, "RADD", 4)) {
	    mcopy = (gdsos < sizeof(*radd)) ? gdsos : sizeof(*radd);

	    if(dd_swapped_data) {
		ddin_crack_radd(at, (char *)this->radd, mcopy);
		if(gdsos < sizeof(struct radar_d))
		    { this->radd->extension_num = 0; }
	    }
	    else {
		memcpy(this->radd, at, mcopy);
	    }
	    num_radds++;
	    str_terminate( this->the_radar_name, this->radd->radar_name, 8 );
	    dd_latitude = this->radd->radar_latitude;
	    dd_longitude = this->radd->radar_longitude;
	    dd_altitude = this->radd->radar_altitude;
	    this->radd->radar_des_length = sizeof(*radd);
	    this->frib = 0;
	}
	else if(!strncmp(at, "PARM", 4)) {
	    pn = num_parms++;
	    mcopy = (gdsos < sizeof(**parms)) ? gdsos : sizeof(**parms);
	    if( dd_swapped_data ) {
	       ddin_crack_parm(at, (char *)this->parms[pn], (int)0);
	       if(gdsos < sizeof(struct parameter_d))
		 { this->parms[pn]->extension_num = 0; }
	    }
	    else {
		memcpy(this->parms[pn], at, mcopy);
	    }

	    if( this->parms[pn]->binary_format == DD_8_BITS ) {
		its_8_bit[pn] = YES;
		this->parms[pn]->binary_format = DD_16_BITS;
		scale8[pn] = this->parms[pn]->parameter_scale;
		bias8[pn] = this->parms[pn]->parameter_bias;
		if( scale8[pn] == 1. ) {
		    this->parms[pn]->parameter_scale = 100.;
		    this->parms[pn]->parameter_bias = 0;
		}
	    }
	    else
		{ its_8_bit[pn] = NO; }

	    this->parms[pn]->parameter_des_length =  sizeof(**parms);
	}
	else if(!strncmp(at, "CSFD", 4)) {
	    // condensed cell spacing information (floating pt)

	    fp = this->celv->dist_cells;
	    this->celv->number_cells = 0;
	    csfd = (CELLSPACINGFP *)at;
	    nn = sizeof(  csfd->spacing );

	    if( dd_swapped_data ) {
		aa = at + 8;	// now points to num_segments
		swack4( aa, (char *)&mm ); // num segments
		aa += 4;
		swack4( aa, (char *)&rr ); //  range to first gate in meters
		aa += 4;	// now points at the list of gate spacings
		bb = aa + sizeof( csfd->spacing ); // points list of cell count

		for(; mm-- ; aa += 4, bb += 2 ) {
		    swack4( aa, (char *)&gs );
		    swack2( bb, (char *)&ns );
		    this->celv->number_cells += ns;

		    for(; ns-- ; rr += gs ) {
			*fp++ = rr;
		    }
		}
	    }
	    else {
		mm = csfd->num_segments;
		rr = csfd->distToFirst;

		for(ii = 0; ii < mm ; ii++ ) {
		    gs = csfd->spacing[ ii ];
		    ns = csfd->num_cells[ ii ];
		    this->celv->number_cells += ns;

		    for(; ns-- ; rr += gs ) {
			*fp++ = rr;
		    }
		}
	    }
	    strncpy( this->celv->cell_spacing_des, "CELV", 4 );
	    this->celv->cell_des_len =
		this->celv->number_cells * sizeof( float ) + 12;
	    gotta_celv = YES;
	}
	else if(!strncmp(at, "CELV", 4)) {
	    if( dd_swapped_data ) {
	      ddin_crack_celv(at, (char *)this->celv, (int)0);
	      swack_long((at+12), (char *)&this->celv->dist_cells[0]
			  , (int)this->celv->number_cells);
	    }
	    else {
		memcpy(this->celv, at, gdsos);
	    }
	    gotta_celv = YES;
	}
	else if(!strncmp(at, "FRIB", 4)) {
	  if (!this->fribx)
	    { this->fribx = new FIELDRADAR; }
	  this->frib = this->fribx; // flag the presence of a FRIB

	  if( dd_swapped_data ) {
	    ddin_crack_frib(at, (char *)this->frib, (int)0);
	    this->frib->field_radar_info_len = sizeof (*this->frib);
	  }
	  else {
	    memcpy (this->frib, at, sizeof (*this->frib));
	  }
	}
	else if(!strncmp(at, "CFAC", 4)) {
	    if( dd_swapped_data ) {
		ddin_crack_cfac(at, (char *)this->cfac, (int)0);
	    }
	    else {
		memcpy(this->cfac, at, gdsos);
	    }
	    this->cfac->correction_des_length = sizeof(*cfac);
	}
	else if(!strncmp(at, "SSWB", 4)) {
	    if(dd_swapped_data) {
		if(LittleEndian) {
		    ddin_crack_sswbLE(at, (char *)this->sswb, (int)0);
		}
		else {
		    ddin_crack_sswb(at, (char *)this->sswb, (int)0);
		}
	    }
	    else {
		memcpy(this->sswb, at, gdsos);
	    }
	    dd_sweep_file = YES;
	    this->sswb->sizeof_struct = sizeof(*sswb);
	}
	else if(!strncmp(at, "COMM", 4)) {
	    int tcc = comment_count;
	    if( tcc < MAX_COMMENTS ) {
		if( !this->comms[tcc] ) {
		    commx = this->comms[tcc] = new comment_d;
		    strncpy( commx->comment_des, "COMM", 4 );
		    commx->comment_des_length = sizeof( *comm );
		}
		else
		    { this->comm = this->comms[ tcc ]; }

		memcpy( this->comms[tcc]->comment, at+8, gdsos-8 );
		this->comms[tcc]->comment_des_length = gdsos;
		comment_count++;
	    }
	}
	
	at += gdsos;
	bytes_left -= gdsos;
	if(dd_new_ray)
	    { dd_sizeof_ray += gdsos; }
    }
    *bytes_used = size - bytes_left;

    if(gotta_celv) {
	this->celvc->cell_des_len = this->celv->cell_des_len;
	this->celvc->number_cells = this->celv->number_cells;
	the_min_cell_spacing = this->celv->dist_cells[1]
	    - this->celv->dist_cells[0];
	this->celvc->dist_cells[0] =
		  this->celv->dist_cells[0] + this->cfac->range_delay_corr;

	for(int ii = 1; ii < this->celv->number_cells; ii++) {
	    float xx = this->celv->dist_cells[ii]
		- this->celv->dist_cells[ii -1];

	    if( xx < the_min_cell_spacing )
		{ the_min_cell_spacing = xx; }

	    this->celvc->dist_cells[ii] =
		  this->celv->dist_cells[ii] + this->cfac->range_delay_corr;
	}
    }
    if(dd_new_vol && dd_new_ray) {
	dd_complete = (num_parms == this->radd->num_parameter_des &&
	    num_rdats == this->radd->num_parameter_des);
    }
    else if(dd_new_vol)
	{ dd_complete = (num_parms == this->radd->num_parameter_des); }
    else if(dd_new_ray) {
	dd_complete = (num_rdats == this->radd->num_parameter_des);
    }

    if(dd_new_vol) {
	// copy out the parameter names into null terminated strings
	for( pn = 0; pn < this->radd->num_parameter_des; pn++ ) {
	    str_terminate(this->field_names[pn]
			  , this->parms[pn]->parameter_name, 8);
	}
    }
    return dd_complete;
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

void
dd_mapper::
copy_data( dd_mapper * sourceMap, char ** names_of_fields, int field_count )
{

    dd_mapper * sdm = sourceMap;

    char ** nof = names_of_fields;

    this->dtime = sdm->dtime;
    this->ddt->accept_time_stamp( this->dtime );
    this->ddt->unstamp_the_time();

    this->dd_new_vol = sdm->new_vol();
    if( this->dd_new_vol ) {
	this->vold->volume_num++;
    }
    this->dd_new_sweep = sdm->new_sweep();
    this->dd_new_ray = sdm->new_ray();
    this->swib->fixed_angle = sdm->swib->fixed_angle;
    this->dd_sweep_count = sdm->sweep_count();
    this->radd->scan_mode = sdm->radd->scan_mode;
    int changed = 0;

    if( this->celv->number_cells != sdm->celv->number_cells ) {
      changed = 1;
    }
    if( this->meters_between_cells() != sdm->meters_between_cells() ) {
      changed |= 2;
    }

    if( this->meters_to_first_cell() != sdm->meters_to_first_cell() ) {
      changed |= 4;
    }

    if( changed ) {
      memcpy(this->celv, sdm->celv, sdm->celv->cell_des_len );
      memcpy(this->celvc, sdm->celvc, sdm->celvc->cell_des_len );
      memcpy(this->cfac, sdm->cfac, sizeof(*cfac));
      memcpy(this->radd, sdm->radd, sizeof(*radd));
      if( constructed_parm_count ) {
	this->radd->num_parameter_des = constructed_parm_count;
	this->radd->total_num_des = 2 + this->radd->num_parameter_des;
      }
    }

    memcpy(this->ryib, sdm->ryib, sizeof(*ryib));

    if((this->dd_new_asib = sdm->new_mpb())) {
	memcpy(this->asib, sdm->asib, sizeof(*asib));
    }
    int gdsos;
    char *aa;

    if (sdm->xstf && sdm->xstf->sizeof_struct > 0) {
      // deal with the XTRA_STUFF catch all descripter

      gdsos = sdm->xstf->sizeof_struct;
      if (sizeof_xstf != gdsos) {
	if (this->xstf)
	  { aa = (char *)this->xstf; delete [] aa; }
	aa = new char [gdsos];
	this->xstf = (XTRA_STUFF *)aa;
	sizeof_xstf = gdsos;
      }
      memcpy ((char *)this->xstf, (char *)sdm->xstf, gdsos);
    }
    int sizeof_data = LONGS_TO_BYTES
	(BYTES_TO_LONGS(this->celv->number_cells * sizeof(short)));
    int new_pn;
    int pn;
    int nn = field_count;
    
    for( ; nn-- ; nof++) {
	pn = sdm->field_index_num( *nof );

	if(( pn = sdm->field_index_num( *nof ) ) < 0 )
	    { continue; }

	if(( new_pn = this->field_index_num( *nof ) ) < 0 )
	    { continue; }

	memcpy(this->raw_data[new_pn]
	       , sdm->raw_data[pn], sizeof_data);
    }
}

// c---------------------------------------------------------------------------

void
dd_mapper::
ray_constructor( dd_mapper * sourceMap, char * data_buf
		 , char ** names_of_fields, int field_count )
{

    dd_mapper * sdm = sourceMap;
    char * aa = data_buf;

    // copy over all the headers

    memcpy(this->sswb, sdm->sswb, sizeof(*sswb));
    memcpy(this->vold, sdm->vold, sizeof(*vold));
    memcpy(this->radd, sdm->radd, sizeof(*radd));
    this->radd->data_compress = 0;
    memcpy(this->celv, sdm->celv
	   , sdm->celv->cell_des_len );
    memcpy(this->celvc, sdm->celvc
	   , sdm->celvc->cell_des_len );
    this->the_min_cell_spacing = sdm->min_cell_spacing();
    memcpy(this->cfac, sdm->cfac, sizeof(*cfac));
    memcpy(this->swib, sdm->swib, sizeof(*swib));
    memcpy(this->ryib, sdm->ryib, sizeof(*ryib));
    memcpy(this->asib, sdm->asib, sizeof(*asib));

    ddt->accept_time_stamp( this->dtime = sdm->dtime );
    ddt->unstamp_the_time();

    char ** nof = names_of_fields;
    const char * names[64];

    for(int ii = 0; ii < field_count; names[ii++] = *nof++);

	// find the fields that already exist and copy them over

    int sizeof_data = LONGS_TO_BYTES
	(BYTES_TO_LONGS(this->celv->number_cells * sizeof(short)));
    int new_pn = 0;
    int pn;
    int nfn;
    
    for(nfn = 0; nfn < field_count ; nfn++) {

	for(pn = 0; pn < sdm->radd->num_parameter_des; pn++ ) {

	    if(!strcmp(names[nfn], "DBZ")) {

		// it might be called DZ in the source data

		if( sdm->field_index_num( "DBZ" ) < 0 ) {
		    if( sdm->field_index_num( "DZ" ) >= 0 ) {
			names[nfn] = "DZ";
		    }
		}
	    }
	    
	    if(!strcmp(names[nfn], sdm->field_names[pn])) {

		// copy over field info

		strcpy(this->field_names[new_pn]
		       , sdm->field_names[pn]);
		memcpy(this->rdats[new_pn]
		       , sdm->rdats[pn], sizeof(**rdats));
		memcpy(this->qdats[new_pn]
		       , sdm->qdats[pn], sizeof(**qdats));
		memcpy(this->parms[new_pn]
		       , sdm->parms[pn], sizeof(**parms));

		this->raw_data[new_pn] = aa;
		aa += sizeof_data;

		memcpy(this->raw_data[new_pn]
		       , sdm->raw_data[pn], sizeof_data);

		names[nfn] = NULL;
		new_pn++;
		break;
	    }
	}
    }
    radd->num_parameter_des = new_pn;

    // add each of the new fields
    int fn;

    for(nfn = 0; nfn < field_count ; nfn++) {
	if( !names[nfn] )
	    { continue; }

	fn = this->radd->num_parameter_des++;

	// copy zeroth parameter descriptor

	memcpy(this->parms[fn], sdm->parms[0], sizeof(**parms));
	parameter_info( names[nfn], this->parms[fn] );

	memcpy(this->rdats[fn], sdm->rdats[0], sizeof(**rdats));
	strncpy(this->rdats[fn]->pdata_name
		, this->parms[fn]->parameter_name, 8);
	strcpy( this->field_names[fn], names[nfn] );

	this->raw_data[fn] = aa;
	memset( this->raw_data[fn], 0, sizeof_data );
	aa += sizeof_data;
    }
    constructed_parm_count = this->radd->num_parameter_des;
    this->radd->total_num_des = 2 + this->radd->num_parameter_des;
}

// c---------------------------------------------------------------------------

int dd_mapper::
threshold_setup( int theType, float min, float max, char * name )
{
    typeofThresholding = 0;

    switch(theType) {

    case REMOVE_LT:
    case REMOVE_GT:
    case REMOVE_LT_AND_GT:
    case PRESERVE_LT_AND_GT:
	break;
    default:
	return 0;
	break;
    }
    if(!name || !strlen(name)) {
	return 0;
    }
    int fn_thr = this->field_index_num( name );
    if( fn_thr < 0 )
	{ return 0; }
    minThreshold = min;
    maxThreshold = max;
    strcpy(thrFieldName, name);
    typeofThresholding = theType;
    return 1;
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

int
dd_mapper::
set_threshold_flags( int * flags, int cell1, int cell2 )
{
    int * flg = flags;
    if( !typeofThresholding )
	{ return 0; }

    if( cell1 < 0 || cell1 >= this->celv->number_cells )
	{ return 0; }

    if( cell2 >= this->celv->number_cells )
	{ return 0; }

    if( cell2 < cell1 )
	{ cell2 = cell1; }	// do one cell

    int fn_thr = this->field_index_num( thrFieldName );
    if( fn_thr < 0 )
	{ return 0; }

    int nn = cell2 - cell1 +1;

    int scaled_min = (int)DD_SCALE( minThreshold
				    , this->parms[fn_thr]->parameter_scale
				    , this->parms[fn_thr]->parameter_bias );
    int scaled_max = (int)DD_SCALE( maxThreshold
				    , this->parms[fn_thr]->parameter_scale
				    , this->parms[fn_thr]->parameter_bias );

    short * ss = (short *)this->raw_data[fn_thr] + cell1;
    short * ssEnd = ss + nn;

    switch(typeofThresholding) {

    case REMOVE_LT:
	for( ; ss < ssEnd; flg++, ss++ ) {
	    *flg = (*ss < scaled_min) ? 1 : 0;
	}
	break;

    case REMOVE_GT:
	for( ; ss < ssEnd; flg++, ss++ ) {
	    *flg = (*ss > scaled_max) ? 1 : 0;
	}
	break;

    case REMOVE_LT_AND_GT:
	for( ; ss < ssEnd; flg++, ss++ ) {
	    *flg = (*ss < scaled_min || *ss > scaled_max) ? 1 : 0;
	}
	break;

    case PRESERVE_LT_AND_GT: // notch
	for( ; ss < ssEnd; flg++, ss++ ) {
	    *flg = (*ss < scaled_min || *ss > scaled_max) ? 0 : 1;
	}
	break;
    }
    return nn;
}

// c---------------------------------------------------------------------------

int
dd_mapper::
set_threshold_flags( int * flags )
{
    int nn =
	this->set_threshold_flags( flags, 0, this->celv->number_cells -1 );
    return nn;
}

// c---------------------------------------------------------------------------

int dd_mapper::
return_thresholded_field( char * name, float * vals, float * bad_val
    , int cell1, int cell2 )
{
    if( !typeofThresholding )
	{ return 0; }

    int fn = this->field_index_num( name );
    if( fn < 0 )
	{ return 0; }

    if( cell1 < 0 || cell1 >= this->celv->number_cells )
	{ return 0; }

    if( cell2 >= this->celv->number_cells )
	{ return 0; }

    if( cell2 < cell1 )
	{ cell2 = cell1; }	// do one cell

    int nn = cell2 - cell1 +1;

    int * flags = (int *)scr0;
    this->set_threshold_flags( flags, cell1, cell2 );

    double scale = this->parms[fn]->parameter_scale;
    double rcp_scale = scale > 0 ? 1./scale : 1.;
    double bias = this->parms[fn]->parameter_bias;
    short * ss = (short *)this->raw_data[fn] + cell1;
    short * ssEnd = ss + nn;
    *bad_val = DD_UNSCALE( this->parms[fn]->bad_data, rcp_scale, bias );

    for( ; ss < ssEnd ; ss++, flags++, vals++) {
	*vals = *flags ? *bad_val : DD_UNSCALE( *ss, rcp_scale, bias );
    }
    return nn;
}

// c---------------------------------------------------------------------------

int dd_mapper::
return_thresholded_field( char * name, float * vals, float * bad_val
    , float range1, float range2 )
{

    int cell1 = this->range_cell( range1 );
    int cell2 = this->range_cell( range2 );
    int nn = this->return_thresholded_field
	( name, vals, bad_val, cell1, cell2 );

    return nn;
}

// c---------------------------------------------------------------------------

int dd_mapper::
return_thresholded_field( char * name, float * vals, float * bad_val )
{
    int nn = this->return_thresholded_field
	( name, vals, bad_val, 0, this->celv->number_cells - 1 );

    return nn;
}

// c---------------------------------------------------------------------------

int dd_mapper::
return_field( const char * name, float * vals, float * bad_val )
{
    int fn = this->field_index_num( name );
    if( fn < 0 )
    {
      return 0;
    }

    double scale = this->parms[fn]->parameter_scale;
    double rcp_scale = scale > 0 ? 1./scale : 1.;
    double bias = this->parms[fn]->parameter_bias;
    short * ss = (short *)this->raw_data[fn];
    int nn = this->celv->number_cells;
    short * ssEnd = ss + nn;
    *bad_val = DD_UNSCALE( this->parms[fn]->bad_data, rcp_scale, bias );

    for( ; ss < ssEnd ; ss++, vals++) {
	*vals = DD_UNSCALE( *ss, rcp_scale, bias );
    }
    return nn;

}

// c---------------------------------------------------------------------------

int dd_mapper::
replace_field( char * name, float * vals, float * bad_val )
{
    float f_bad = *bad_val;
    int fn = this->field_index_num( name );
    if( fn < 0 )
	{ return 0; }

    double scale = this->parms[fn]->parameter_scale;
    double bias = this->parms[fn]->parameter_bias;
    short * ss = (short *)this->raw_data[fn];
    int nn = this->celv->number_cells;
    short * ssEnd = ss + nn;
    int bad = this->parms[fn]->bad_data;

    for( ; ss < ssEnd ; ss++, vals++) {
	*ss = ( *vals == f_bad ) ? bad
	    : (short)DD_SCALE( *vals, scale, bias );
    }
    return nn;

}

// c---------------------------------------------------------------------------

int
dd_mapper::
copy_raw_field( char * name, char * to )
{
    int fn = this->field_index_num( name );
    if( fn < 0 )
	{ return 0; }
    int nn = this->celv->number_cells;

    if( dd_swapped_data ) {
	swack_short( (char *)this->raw_data[fn], (char *)to, nn );
    }
    else {
	memcpy( to, this->raw_data[fn], nn * sizeof(short));
    }
    return nn;
}

// c---------------------------------------------------------------------------

int
dd_mapper::
return_frequencies( int field_num, double * freqs )
{
  if( field_num < 0 || field_num >= this->radd->num_parameter_des )
    { return 0; }
  // frequencies returned in GHz
  int nn = 0, mask = this->parms[field_num]->xmitted_freq;
  int probe = 1;
  float * fptr = &this->radd->freq1;

  for( int ii = 0; ii < 5; ii++, probe <<= 1 ) {
    if( probe & mask )
      { *( freqs + nn++ ) = *( fptr + ii ); }
  }
  return nn;
}

// c---------------------------------------------------------------------------

int
dd_mapper::
return_interpulse_periods( int field_num, double * ipps )
{
  if( field_num < 0 || field_num >= this->radd->num_parameter_des )
    { return 0; }

  // interpulse periods returned in seconds

  int nn = 0, mask = this->parms[field_num]->interpulse_time;
  int probe = 1;
  float * fptr = &this->radd->interpulse_per1;

  for( int ii = 0; ii < 5; ii++, probe <<= 1 ) {
    if( probe & mask ) {
      *( ipps + nn ) = *( fptr + ii ) * .001; // in the header as milliseconds
      nn++;
    }
  }
  return nn;
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------
// c---------------------------------------------------------------------------
// c---------------------------------------------------------------------------



// maps the input buffer upto and including the first ray if possible
// all descripters are copied into local memory and pointers to 
// raw data fields are updated

int dd_mapper::within_range( float range ) // in meters!
{
    int nn = (int)(( range - this->celvc->dist_cells[0] ) /
		   the_min_cell_spacing + .5 );
    if( nn < 0 || nn >= this->celvc->number_cells )
	{ return -1; }
    return nn;
}

int dd_mapper::range_cell( float range ) //  in meters!
{
    int gg = (int)(( range - this->celvc->dist_cells[0] ) /
		   the_min_cell_spacing + .5 );
    if( gg < 0 )
	{ gg = 0; }

    if( gg >= this->celv->number_cells )
	{ gg = this->celv->number_cells -1; }
    return gg;
}






double dd_mapper::rotation_angle()
{
# ifdef obsolete
    double xx = (this->radd->scan_mode == RHI)
	? CART_ANGLE(this->ryib->elevation)
	: this->ryib->azimuth;
    return FMOD360(xx);
# else
    double d_rot;

    switch(this->radd->scan_mode) {

    case AIR:
	d_rot = FMOD360( this->asib->rotation_angle +
			 this->cfac->rot_angle_corr +
			 this->roll());
	break;

    case RHI:
	d_rot = FMOD360( CART_ANGLE( this->elevation() ));
	break;

    case TAR:
	d_rot = this->radd->radar_type != GROUND ?
	    FMOD360( this->asib->rotation_angle +
		     this->cfac->rot_angle_corr) :
			 this->azimuth();
	break;

    default:
	d_rot = this->azimuth();
	break;
    }
    return d_rot;
# endif
}




char * dd_mapper::field_name(int field_num)
{
    if(field_num >= 0 && field_num < this->radd->num_parameter_des)
	{ return this->field_names[field_num]; }
    return NULL;
}



// c---------------------------------------------------------------------------

void
dd_mapper::
zero_fields()
{
  char *aa;
  int size, nn = this->celv->number_cells;


  for(int fn = 0; fn < this->radd->num_parameter_des; fn++) {
    aa = this->raw_data[fn];
    size = (this->parms[fn]->binary_format == DD_8_BITS)
      ? 1 : sizeof(short);
    memset( aa, 0, nn * size );
  }
}
// c---------------------------------------------------------------------------

int
dd_mapper::
field_index_num( const char * name ) { // c...mark
  int nn = strlen(name);
  if(nn < 1)
    { return -1; }
  for(int fn = 0; fn < this->radd->num_parameter_des; fn++) {
    if( (int)strlen( this->field_names[fn] ) != nn )
      { continue; }
    if(!strcmp( name, this->field_names[fn] ))
      { return fn; }
  }
  return -1;
}

// c---------------------------------------------------------------------------

void dd_mapper::ac_radar_angles
(double *azimuth, double *elevation, double *rotation_angle, double *tilt
 , double *ac_vel, CORRECTION *alt_cfac)
{
  /*
   * see Wen-Chau Lee's paper
   * "Mapping of the Airborne Doppler Radar Data"
   */
  double R, P, H, D, T;
  double sinP, cosP, sinD, cosD;
  CORRECTION *xcfac = (alt_cfac) ? alt_cfac : this->cfac;

  R = RADIANS (this->asib->roll +xcfac->roll_corr);
  P = RADIANS (this->asib->pitch + xcfac->pitch_corr);
  H = RADIANS (this->asib->heading + xcfac->heading_corr);
  D = RADIANS (this->asib->drift_angle + xcfac->drift_corr);
  sinP = sin(P);
  cosP = cos(P);
  sinD = sin(D);
  cosD = cos(D);
  
  T = H + D;

  double theta_a, tau_a, sin_tau_a, cos_tau_a, sin_theta_rc, cos_theta_rc;

  theta_a = RADIANS(this->asib->rotation_angle +xcfac->rot_angle_corr);
  tau_a = RADIANS(this->asib->tilt +xcfac->tilt_corr);
  sin_tau_a = sin(tau_a);
  cos_tau_a = cos(tau_a);
  sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  cos_theta_rc = cos(theta_a + R);
    
  double xsubt, ysubt, zsubt;

  xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
	   + cosD * sin_theta_rc * cos_tau_a
	   -sinD * cosP * sin_tau_a);
  ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
	   + sinD * sin_theta_rc * cos_tau_a
	   + cosP * cosD * sin_tau_a);
  zsubt = (cosP * cos_tau_a * cos_theta_rc
	   + sinP * sin_tau_a);
  
  double theta_t, tau_t, lambda_t, phi_t;
  
  *rotation_angle = theta_t = atan2(xsubt, zsubt);
  *tilt = tau_t = asin(ysubt);
  lambda_t = atan2(xsubt, ysubt);
  *azimuth = fmod(lambda_t + T, TWOPI);
  *elevation = phi_t = asin(zsubt);
  
  double vert, d;
  vert =  this->asib->vert_velocity != -999 ? this->asib->vert_velocity : 0;
  d = sqrt((double)(SQ(this->asib->ew_velocity) + SQ(this->asib->ns_velocity)));
  d += xcfac->ew_gndspd_corr; /* klooge to correct ground speed */
  *ac_vel = d*sin(*tilt) + vert*sin(*elevation);
  
  return;
}

// c---------------------------------------------------------------------------

// special convenience utilities here


double dd_mapper::AngDiff(float a1, float a2) {
    double d = a2 - a1;

    if( d < -270. ) { return(d + 360.); }
    if( d >  270. ) { return(d - 360.); }
    return(d);
    // difference between two azimuths;
    // tries to deal with crossing 360 degrees
}

int dd_mapper::inSector(float ang, float ang1, float ang2)
{
    // assumes sector defined from ang1 clockwise to ang2

    if(ang1 > ang2)		// crosses 360.
	{ return(ang >= ang1 || ang < ang2); }

    return(ang >= ang1 && ang < ang2);
}

char * dd_mapper::slashPath( char * path )
{
    int nn = strlen(path);
    if(!nn)
	{ return NULL; }
    if( *(path +nn -1) != '/' )
	{ strcat( path +nn, "/"); }
    return path;
}

int dd_mapper::dz_present()
{
    int fn;

    fn = this->field_index_num( "DBZ" );
    if( fn < 0 )
	{ fn = this->field_index_num( "DZ" ); }
    if( fn < 0 )
	{ fn = this->field_index_num( "DB" ); }
    return fn;
}

int dd_mapper::vr_present()
{
    int fn;

    fn = this->field_index_num( "VR" );
    if( fn < 0 )
	{ fn = this->field_index_num( "VE" ); }
    return fn;
}

int dd_mapper::zdr_present()
{
    int fn;

    fn = this->field_index_num( "ZDR" );
    if( fn < 0 )
	{ fn = this->field_index_num( "ZD" ); }
    if( fn < 0 )
	{ fn = this->field_index_num( "DR" ); }
    return fn;
}

int dd_mapper::ldr_present()
{
    int fn;

    fn = this->field_index_num( "LDR" );
    if( fn < 0 )
	{ fn = this->field_index_num( "LC" ); }
    if( fn < 0 )
	{ fn = this->field_index_num( "LD" ); }
    return fn;
}

int dd_mapper::rho_present()
{
    int fn;

    fn = this->field_index_num( "RHOHV" );
    if( fn < 0 )
	{ fn = this->field_index_num( "RH" ); }
    if( fn < 0 )
	{ fn = this->field_index_num( "RX" ); }
    return fn;
}

int dd_mapper::phi_present()
{
    int fn;

    fn = this->field_index_num( "PHIDP" );
    if( fn < 0 )
	{ fn = this->field_index_num( "PHI" ); }
    if( fn < 0 )
	{ fn = this->field_index_num( "PH" ); } // UF
    if( fn < 0 )
	{ fn = this->field_index_num( "DP" ); } // CSU UF
    return fn;
}

int dd_mapper::alias_index_num( char * name ) 
{
    char str[256], * sptrs[64];
    int ii, jj, kk,mm, nn, nt, ndx;
		
    if(( mm = strlen( name )) < 1 ) { 
	return -1; 	
    }
	
    for( ii = 0; ii < num_alias_sets; ii++ ) { // for each set of aliases 
	if( !strstr( known_aliases[ii], name ))
	    { continue; }		// not even close

      strcpy( str, known_aliases[ii] );
      nt = dd_tokens( str, sptrs );

      for( jj = 0; jj < nt; jj++ ) {
	nn = strlen( sptrs[jj] );
	if( mm != nn )
	  { continue; }		// lengths of possible match not the same
	if( !strcmp( name, sptrs[jj] )) {
      //
      // we have a match; now see if this mapper responds to 
      // one of these aliases
      //
	  for( kk = 0; kk < nt; kk++ ) {
	    if(( ndx = this->field_index_num( sptrs[kk] )) >= 0 )
	      { return ndx; }
	  }
	  // else see if the field is already in the data
	  return this->field_index_num(name);
	}
     }
    }
    return this->field_index_num(name);
}


// c---------------------------------------------------------------------------


index_of_mappers::index_of_mappers()
{
    for( int ii = 0; ii < max_maps; this->ddmapper[ii++] = NULL );
}

int index_of_mappers::new_mapper_index()
{
    // look for an unused map pointer

    int ii = 0;
    for(; ii < max_maps && this->ddmapper[ii] ; ii++ );
    if( ii < max_maps ) {
	this->ddmapper[ ii ] = new dd_mapper();
	return ii;
    }
    return -1;
}

dd_mapper * index_of_mappers::return_mapper( int index )
{
    if( index < 0 || index >= max_maps )
	{ return NULL; }
    return this->ddmapper[ index ];
}

// c---------------------------------------------------------------------------




