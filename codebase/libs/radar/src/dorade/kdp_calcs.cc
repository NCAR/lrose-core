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
/* 	$Id: kdp_calcs.cc,v 1.8 2016/03/03 18:45:06 dixon Exp $	 */

# include <stdio.h>
# include <radar/dorade/dd_mapper.hh>
# include <radar/dorade/dd_utils.hh>
# include <fstream>
# include <iostream>

# define Mongoo
# define NEGATIVE_KDPS

# include <radar/dorade/kdp_calcs.hh>

void taperUpAvrg(Xfloat *, Xfloat *, int, AvrgList *);
void centeredAvrg(Xfloat *, Xfloat *, int, AvrgList *);
void defaultMFilter(Xfloat *, Xfloat *, int, MedianFilterList *);
void dd_StdDev( Xfloat *, Xfloat *, Xfloat *, int, AvrgList *, AvrgList *);

// c---------------------------------------------------------------------------

PeakMarker * kdp_calcs::
instantiate_a_peak() // first peak
{
    PeakMarker * peak = new PeakMarker;
    peak->next = peak->prev = NULL;
    peak->peak_num = 1;
    return peak;
}

// c---------------------------------------------------------------------------

PeakMarker * kdp_calcs::
instantiate_a_peak( PeakMarker * peak ) // subsequent peak
{
    peak->next = new PeakMarker;
    peak->next->prev = peak;
    peak->next->next = NULL;
    peak->next->peak_num = peak->peak_num + 1;
    return peak->next;
}

// c---------------------------------------------------------------------------

phidp_segment * kdp_calcs::
instantiate_a_segment()		// first segment
{
    phidp_segment * seg = new phidp_segment;
    seg->segment_num = 1;
    seg->next = seg->prev = NULL;

    // anticipate at least 2 peaks
    seg->firstPeak = instantiate_a_peak();
    instantiate_a_peak( seg->firstPeak );

    return seg;
}

// c---------------------------------------------------------------------------

phidp_segment * kdp_calcs:: 
instantiate_a_segment( phidp_segment * seg ) // a subsequent segment
{
    if( seg->next )
	{ return seg->next; }

    seg->next = new phidp_segment;
    seg->next->next = NULL;
    seg->next->segment_num = seg->segment_num +1;
    seg->next->prev = seg;

    // anticipate at least 2 peaks
    seg->next->firstPeak = instantiate_a_peak();
    instantiate_a_peak( seg->next->firstPeak );

    return seg->next;
}

// c---------------------------------------------------------------------------
// Initialize the kdp_calcs object
// c---------------------------------------------------------------------------

kdp_calcs::
kdp_calcs() {

    volume_ray_count = prep_vol_ray_count = 0;
    prep_vol_ray_count = csu_vol_ray_count = 0;
    Min_Good = 5;
    Max_Bad = 3;
    hail_no = 0;
    nsslMin_Good = 4;
    baseAvg = 8;
    doubleBaseAvg = 2 * baseAvg +1;
    mongoBaseAvg = 6 * baseAvg +1;
    TstPlsStartGate = 0;
    TstPlsStopGate = 0;
    preFilter = FIR3ORDER +1;
    postFilter = FIR3ORDER +1;
    ray_count = 0;
    num_phidp_segments = 0;
    catchit = 0;
    return_double = NO;

    double_arg = NULL;
    nssl_long_avg_thr = 40.;
    powerThr = -110.;
    maxKdp = 45.;
    hailTestZ = 45.;
    rho_hail = 0.8;
    rhoThr = 0.9;
    rhoThr = 0.8;
    rhoSD_hail = 0.08;
    phidpGlitchThr = 30.;	// currently 4.5 in DoradeGen
    maxPhidpDelta = 3.0;
    TstPlsStartRng = KM_TO_M( 149. );
    TstPlsStopRng = KM_TO_M( 161. );
    TstPlsStartRng = KM_TO_M( 171. );
    TstPlsStopRng = KM_TO_M( 176. );
    minStdDev = 8.;		// inital std. dev. for an NSSL segment
    minStdDev = 10.;		// inital std. dev. for an NSSL segment
    higherStdDev = 10.;		// steady state for an NSSL segment
    higherStdDev = 15.;		// steady state for an NSSL segment
    cell_spacing = -999.;
    kdp_scale = 100.;
    az1 = -2 * 360.;
    az2 = -360.;

    int size = 2 * MAXCVGATES;
    int full_size = size + preFilter + postFilter;
    mongoBaseAvg = 6 * baseAvg +1;

    avliDouble = new AvrgList( doubleBaseAvg );
    avliDoubleB = new AvrgList( doubleBaseAvg );
    avliMongo = new AvrgList( mongoBaseAvg );
    avliMongoB = new AvrgList( mongoBaseAvg );
    
    deltaRanges = new Xfloat [mongoBaseAvg];
    deltaRangesSums = new Xfloat [mongoBaseAvg];
    deltaRangesSq = new Xfloat [mongoBaseAvg];
    deltaRangesSqSums = new Xfloat [mongoBaseAvg];

    int mm = full_size * sizeof( Xfloat );
    scrA = new Xfloat [full_size];
    memset( scrA, 0, mm );
    phi_dpA = new Xfloat [full_size];
    memset( phi_dpA, 0, mm );
    phizA  = new Xfloat [full_size];
    memset( phizA, 0, mm );
    phiCopyA  = new Xfloat [full_size];
    memset( phiCopyA, 0, mm );
    phiFltrA  = new Xfloat [full_size];
    memset( phiFltrA, 0, mm );

    phi_dp0 = phi_dpA + preFilter;
    phiz0 = phizA + preFilter;
    phiCopy0 = phiCopyA + preFilter;
    phiFltr0 = phiFltrA + preFilter;

    mm = size * sizeof( Xfloat );

    Ckdp0 = new Xfloat [size];
    memset( Ckdp0, 0, mm );
    phiAvg0  = new Xfloat [size];
    memset( phiAvg0, 0, mm );
    stdDev0  = new Xfloat [size];
    memset( stdDev0, 0, mm );
    phiDouble0  = new Xfloat [size];
    memset( phiDouble0, 0, mm );
    phiMongo0  = new Xfloat [size];
    memset( phiMongo0, 0, mm );
    kdpDouble0  = new Xfloat [size];
    memset( kdpDouble0, 0, mm );
    kdpMongo0  = new Xfloat [size];
    memset( kdpMongo0, 0, mm );
    DBz0  = new Xfloat [size];
    memset( DBz0, 0, mm );
    fltrDBz0  = new Xfloat [size];
    memset( fltrDBz0, 0, mm );
    phiPreGlitch0 = new Xfloat [size];
    memset( phiPreGlitch0, 0, mm );

    mm = size * sizeof(short);

    phidp_first_segment = instantiate_a_segment();
}

// c---------------------------------------------------------------------------

void
kdp_calcs::
preprocessing()
{
    // c...mark
    prep_vol_ray_count++;

//    printf( "vrc: %d %.1f %s\n", volume_ray_count
//	    , mapr->azimuth(), mapr->ascii_time() );

    catchit = az1 >= 0 ? mapr->inSector( (float)mapr->azimuth(), az1, az2 )
	: 0;

    num_phidp_segments = 0;
    num_gates = mapr->number_of_cells();

    int gg;

    // convert phidp to floating point

    int fn_phi = mapr->field_index_num( "PHIDP" );

    if( fn_phi < 0 )
	{ fn_phi = mapr->field_index_num( "PHI" ); }
    if( fn_phi < 0 )
	{ fn_phi = mapr->field_index_num( "PH" ); } // UF
    if( fn_phi < 0 )
	{ fn_phi = mapr->field_index_num( "DP" ); } // CSU UF
    if( fn_phi < 0 ) {
	printf( "Could not find a PHIdp field\n" );
	exit(1);
    }
    short * ss = (short *)mapr->raw_data_ptr( fn_phi );
    short * ssEnd = ss + num_gates;
    Xfloat rcp_scale = 1./mapr->scale( fn_phi );
    Xfloat bias = mapr->bias( fn_phi );
    Xfloat * phi = phi_dp0;
    
    for(; ss < ssEnd; *phi++ = DD_UNSCALE( *ss++, rcp_scale, bias ));

    *( phi_dp0 -1 ) = *phi_dp0;
    *( phi_dp0 + num_gates ) = *( phi_dp0 + num_gates -1 );
    
    //
    // Average phi_dp over 17 gates for the std. dev. calc.
    //
    centeredAvrg( phi_dp0, phiAvg0, num_gates, avliDouble );
    //
    // now take the standard deviation of phi_dp
    //
    dd_StdDev( phi_dp0, phiAvg0, stdDev0, num_gates, avliDouble
	       , avliDoubleB );

    // see if we need to make sure we exclude the test pulse data

    Xfloat range_b1 = mapr->meters_to_first_cell();
    Xfloat cs = mapr->meters_between_cells();
    TstPlsStartGate = int ((TstPlsStartRng - range_b1)/cs);
    TstPlsStopGate = int ((TstPlsStopRng - range_b1)/cs +1.);

    if(TstPlsStopGate > TstPlsStartGate) {

	gg = TstPlsStartGate;
	int fn_power = mapr->field_index_num( "DM" );
	short * power = (short *)mapr->raw_data_ptr( fn_power ) + gg;
	int bad = mapr->bad_data_flag( fn_power );

	for(; gg <= TstPlsStopGate; gg++ ) {
	    *( stdDev0 + gg ) = 999.;
	    *( power + gg ) = bad;
	}
    }

    // if( catchit ) {
    //     mark = 0;
    // }

}

// c---------------------------------------------------------------------------
// NSSL algorithm
// c---------------------------------------------------------------------------

int
kdp_calcs::
do_nsslKdpx( dd_mapper * maprIn )
{
    // really generate NSSL Kdp

    mapr = maprIn;
    nssl_vol_ray_count++;

    preprocessing();

    if( mapr->meters_between_cells() != cell_spacing ) {
	cell_spacing = mapr->meters_between_cells();
	//
	// regenerate special delta range arrays for the NSSL Kdp calc.
	//
	nssl_delta_range_constants();
    }

    nsslDefineSegs();

    int fg, lg, gg;

    if( num_phidp_segments < 1 ) {
	return 0;
    }
    //
    // reaverage within the good segments
    //

    nssl_reavg_unf();

    // fill in before the first segment

    fg = phidp_first_segment->first_gate;
    lg = phidp_last_segment->endGate;

    Xfloat xx = *(phiDouble0 + fg);

    for(gg = 0; gg < fg; *(phiDouble0 + gg++) = xx);

# ifdef Mongoo
    xx = *(phiMongo0 + fg);
    for(gg = 0; gg < fg; *(phiMongo0 + gg++) = xx);
# endif
    
    // fill in after the last segment

    xx = *(phiDouble0 + lg -1);
    for(gg = lg; gg < num_gates; *(phiDouble0 + gg++) = xx);

# ifdef Mongoo
    xx = *(phiMongo0 + lg -1);
    for(gg = lg; gg < num_gates; *(phiMongo0 + gg++) = xx);
# endif

    // now we will try to generate Kdp

    NKdp_finally( phiDouble0, kdpDouble0, avliDouble );

# ifdef Mongoo
    NKdp_finally( phiMongo0, kdpMongo0, avliMongo );
//    NKdp_finally( phiDouble0, kdpMongo0, avliMongo );
# endif

    return num_gates;
}

// c---------------------------------------------------------------------------

int
kdp_calcs::
do_nsslKdp( dd_mapper * maprIn, Xfloat * Nkdp0 )
{
    // generate Kdp values using the NSSL algorithm

    int cell_count = do_nsslKdpx( maprIn );

    int gg;

    if( !cell_count ) {
	for( gg = 0; gg < num_gates; gg++ ) {
	    *( Nkdp0 + gg ) = 0;
	}
	return 0;
    }

    // combine the short and long averages if necessary

    double tot_dbl = 0, tot_mongo = 0, tot_mix = 0;

# ifndef Mongoo
    // just copy them into the destination array

    for( gg = 0; gg < num_gates; gg++ ) {
	*( Nkdp0 + gg ) = *( kdpDouble0 + gg );
    }

# else
    int fn_dbz = mapr->field_index_num( "DBZ" );
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DZ" ); }
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DB" ); }

    short * ssdbz0 = (short *)mapr->raw_data_ptr( fn_dbz );
    int thr = (int)(nssl_long_avg_thr * mapr->scale( fn_dbz ) +.5);

    // generate combined Kdp
    for( gg = 0; gg < num_gates; gg++ ) {
	if( *( ssdbz0 + gg ) < thr ) {
	    *( Nkdp0 + gg ) = *( kdpMongo0 + gg );
	}
	else {
	    *( Nkdp0 + gg ) = *( kdpDouble0 + gg );
	}
	tot_dbl += *( kdpDouble0 + gg );
	tot_mongo += *( kdpMongo0 + gg );
	tot_mix += *( Nkdp0 + gg );
    }
# endif
    //    printf( "Az: %5.1f short: %6.1f long: %6.1f mix: %6.1f\n"
    //	    , mapr->azimuth(), tot_dbl, tot_mongo, tot_mix );
    // mark = 0;
    return num_gates;
}

// c---------------------------------------------------------------------------

int
kdp_calcs::
do_nsslKdp( dd_mapper * maprIn, short * ssNkdp0 )
{
    static unsigned int total_gates = 0, long_avg_gates = 0;
    total_gates += num_gates;

    // generate Kdp values using the NSSL algorithm

    int cell_count = do_nsslKdpx( maprIn );

    int gg;

    if( !cell_count ) {
	for( gg = 0; gg < num_gates; gg++ ) {
	    *( ssNkdp0 + gg ) = 0;
	}
	return 0;
    }

# ifndef Mongoo

    for( gg = 0; gg < num_gates; gg++ ) {
	*( ssNkdp0 + gg ) = (int)(*( kdpDouble0 + gg ) * kdp_scale + .5 );
    }

# else

    int fn_dbz = mapr->field_index_num( "DBZ" );
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DZ" ); }
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DB" ); }
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "UDBZ" ); }

    short * ssdbz0 = (short *)mapr->raw_data_ptr( fn_dbz );
    int thr = (int)(nssl_long_avg_thr * mapr->scale( fn_dbz ) +.5);
    
    // generate scaled Kdp

    for( gg = 0; gg < num_gates; gg++ ) {
	if( *( ssdbz0 + gg ) < thr ) {
	    *( ssNkdp0 + gg ) = (int)(*( kdpMongo0 + gg ) * kdp_scale + .5 );
	    long_avg_gates++;
	    if( !( long_avg_gates % 5000000 )) {
		printf( "kdp_calcs:total_gates: %dK long_avg_gates: %dK\n"
			, total_gates/1000, long_avg_gates/1000 );
	    }
	}
	else {
	    *( ssNkdp0 + gg ) = (int)(*( kdpDouble0 + gg ) * kdp_scale + .5 );
	}
    }

# endif
    return num_gates;
}


// c---------------------------------------------------------------------------

void
kdp_calcs::
nsslDefineSegs()
{
    Xfloat stdDevThr = minStdDev;
    int gg = 0;
    int num_good;
    num_phidp_segments = 0;
    struct phidp_segment *seg = phidp_first_segment;


    for(; gg < num_gates; ) {

	if(num_phidp_segments) { seg = phidp_last_segment->next; }

	// find "Min_Good" consecutive good gates

	for(num_good = 0; gg < num_gates && num_good < nsslMin_Good; gg++) {

	    if( *(stdDev0 +gg) < stdDevThr )
		{ ++num_good; stdDevThr = higherStdDev; }
	    else
		{ num_good = 0; stdDevThr = minStdDev; }
	}

	if(num_good < nsslMin_Good) { break; } // get all the way out

	// we have a segment with at least nsslMin_Good gates
	//
	instantiate_a_segment( seg ); // make sure there's a next segment

	num_phidp_segments++;
	seg->first_gate = gg - nsslMin_Good;
	seg->next->first_gate = num_gates;
	phidp_last_segment = seg;

	// count the number of good gates in this segment

	for(; gg < num_gates && *(stdDev0 + gg) < higherStdDev; gg++);

	seg->endGate = gg;
	seg->gate_count = seg->endGate - seg->first_gate;
    }
}

// c---------------------------------------------------------------------------

void
kdp_calcs::
nssl_reavg_unf()
{
    //
    // reaverage within the good segments
    //
    int nn, fg, ng, ii, jj;
    struct phidp_segment *seg = phidp_first_segment;
    Xfloat val, delavg, flip = 0;

    for(nn = 0; nn < num_phidp_segments ; nn++, seg = seg->next) {
	fg = seg->first_gate;
	ng = seg->gate_count;
	centeredAvrg( phi_dp0 + fg, phiDouble0 + fg, ng, avliDouble); 

	seg->Avrg1 = *(phiDouble0 + fg);
	val = *(phiDouble0 + fg + ng -1);
	seg->Avrg2 = val > seg->Avrg1 ? val : seg->Avrg1;
	seg->Avrg2 = val;

# ifdef Mongoo
	centeredAvrg( phi_dp0 + fg, phiMongo0 + fg, ng, avliMongo); 
	seg->avrg1 = *(phiMongo0 + fg);
	val = *(phiMongo0 + fg + ng -1);
	seg->avrg2 = val > seg->avrg1 ? val : seg->avrg1;
	seg->avrg2 = val;
# endif

	if(nn) {
	    // fill the gap between the last segment and this segment
	    //
	    delavg = seg->Avrg1 - seg->prev->Avrg2;

	    if( fabs( delavg ) > 90. ) {
		// mark = 0;
	    }

	    if ( fabs( delavg ) > 100. ) {
		
		flip = delavg < 0 ? 180. : -180.;
		
		seg->Avrg1 += flip ;
		seg->Avrg2 += flip ;
		
		ii = seg->first_gate;
		jj = seg->endGate;
		
		for(; ii < jj; ii++ ) {
		    *(phiDouble0 + ii ) += flip;
		}
	    }
	    fill_gap( phiDouble0, seg->prev->endGate -1, seg->prev->Avrg2
		      , seg->first_gate, seg->Avrg1);


# ifdef Mongoo
	    delavg = seg->avrg1 - seg->prev->avrg2;

	    if ( fabs( delavg ) > 100. ) {
		
		flip = delavg < 0 ? 180. : -180.;
		
		seg->avrg1 += flip ;
		seg->avrg2 += flip ;
		
		ii = seg->first_gate;
		jj = seg->endGate;
		
		for(; ii < jj; *(phiMongo0 + ii++) += flip );
	    }
	    fill_gap( phiMongo0, seg->prev->endGate -1, seg->prev->avrg2
		      , seg->first_gate, seg->avrg1);
# endif
	}
    }
}
// c---------------------------------------------------------------------------

void
kdp_calcs::
fill_gap(Xfloat * values, int Agate, Xfloat Aval, int Bgate, Xfloat Bval)
{
    // linear interpolated fill between the Agate and the Bgate
    // exclusive of A and B

    Xfloat delta = Bval - Aval;

    int steps = Bgate - Agate;

    if(steps < 2)
	{ return; }

    Xfloat small_delta = delta/(Xfloat)steps;

    Xfloat B = Aval + small_delta;

    Xfloat *val = values + Agate + 1; // first value to be filled

    for(; --steps; B += small_delta) {
	*val++ = B;
    }
}

// c---------------------------------------------------------------------------

void
kdp_calcs::
NKdp_finally( Xfloat * phi0, Xfloat * Kdp0, AvrgList *avli )
{
    // c...mark
    avli->reset();		// we keep a running sum of the phi values

    int size = avli->returnSize();
    Xfloat *deltaR = deltaRanges;
    Xfloat sumPhi = 0;
    Xfloat sum_dr;
    Xfloat sum_drSq;
    Xfloat sum_drXphi;
    Xfloat kw, slope;
    int gate, ii, kk = 0;
    static int count = 0;

    count++;

    // seed with size/2 values
    int halfSize = size/2;

    for(gate = 0; gate <= halfSize; gate++) { // seed the running phi sum
	sumPhi = avli->updateSum(*(phi0 + gate));
    }
    kk = halfSize +1;

    // first loop grows to full sample size
    

    for(gate = 0; gate < halfSize; gate++) {

	sum_dr = *(deltaRangesSums + kk - 1);
	sum_drSq = *(deltaRangesSqSums + kk - 1);

	sum_drXphi = 0;
	kw = (Xfloat)kk;

	for(ii = 0;  ii < kk; ii++) {
	    sum_drXphi += *(deltaR + ii) * (*(phi0 + ii));
	}

	slope =
	    (kw * sum_drXphi - sum_dr * sumPhi)
	    / (kw * sum_drSq - sum_dr * sum_dr);

	*(Kdp0 + gate) = slope > 1.e-6 ? slope : 0;

	sumPhi = avli->updateSum(*(phi0 + kk)); // update running phi sum
	kk++;
    }


    // the first iteration of the next loop will be at full sample size


    int gFirst = 0;		// points to first gate in the set of samples
    int gLast =  size;		// points to last gate in the set of samples
    kw = (Xfloat)size;
    sum_dr = *(deltaRangesSums + size -1);
    sum_drSq = *(deltaRangesSqSums + size -1);

    for(; gLast < num_gates; gFirst++, gate++, gLast++) {

	// sum dr * phi for all samples
	sum_drXphi = 0;

	for(ii = 0;  ii < size; ii++) {
	    sum_drXphi += *(deltaR + ii) * (*(phi0 + gFirst + ii));
	}

	slope =
	    (kw * sum_drXphi - sum_dr * sumPhi)
	    / (kw * sum_drSq - sum_dr * sum_dr);

# ifdef NEGATIVE_KDPS
	*(Kdp0 + gate) = slope;
# else
	*(Kdp0 + gate) = slope > 1.e-6 ? slope : 0;
# endif
	sumPhi = avli->updateSum(*(phi0 + gLast)); // update running phi sum
    }

    // fill in at the end

    for(; gate < num_gates; *(Kdp0 + gate) = (*(Kdp0 + gate - 1)), gate++ );

# ifdef obsolete
    int i, j, i1, i2, k, nn, ng = num_gates;
    double *fdp = phi0, x[60], y[60], gatekm = .149896240234375;
    double sx, sy, sx2, sxy;
    k = size/2;

    for( i=0; i < ng; i++ ) {
	i1 = i-k >= 0 ? i-k : 0;
	i2 = i+k+1 < ng ? i+k+1 : ng;
	for(nn=0,j=i1; j < i2; j++, nn++ ) {
	    x[nn] = 2.*gatekm*(float)(j-i1);
	    y[nn] = *(phi0 + j);
	}
	sx = sy = sx2 = sxy = 0;

	for(ii=0; ii < nn; ii++ ) {
	    sx += x[ii];
	    sy += y[ii];
	    sx2 += x[ii] * x[ii];
	    sxy += x[ii] * y[ii];
	}
	slope = ((double)nn * sxy - sx * sy  )/((double)nn * sx2 - sx * sx );
	
	if( i >= gatex ) {
          // mark = 0;
	}
    }
# endif
    // mark = 0;
}

// c---------------------------------------------------------------------------

void
kdp_calcs::
nssl_delta_range_constants()
{

    // now generate some special delta range arrays used in the NKdp calc.

    Xfloat *dist = deltaRanges;
    Xfloat *distSq = deltaRangesSq;
    Xfloat *distSum = deltaRangesSums;
    Xfloat *distSqSum = deltaRangesSqSums;
    *distSum = *distSqSum = 0;
    Xfloat r;
    int nn;

    for(nn = 0; nn < mongoBaseAvg; nn++) {
	r = M_TO_KM( 2. * (Xfloat)nn * cell_spacing );
	*(dist + nn) = r;
	*(distSq + nn) = r * r;
	if(nn) {
	    *(distSum + nn) = *(distSum + nn -1) + r;
	    *(distSqSum + nn) = *(distSqSum + nn -1) + r * r;
	}
    }
}

// c---------------------------------------------------------------------------

void
kdp_calcs::
do_plots()
{
    static int local_count = 1;	// 1 => don't do it1
    local_count = 0;
    int ii;

    const char *dir = "/scr/hotlips/oye/spol/rain/";
    // dir = "/scr/schroeder/oye/spol/rain/";
    // dir = "/scr/ale/oye/";

    if( !catchit ) {
	return;
    }
    if( local_count++ ) {
	return;
    }

    int fn_dbz = mapr->field_index_num( "DBZ" );
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DB" ); }
    if( fn_dbz < 0 )
	{ fn_dbz = mapr->field_index_num( "DZ" ); }
    short * ssdbz = (short *)mapr->raw_data_ptr( fn_dbz );
    Xfloat rcp_dbz_scale = 1./mapr->scale( fn_dbz );
    Xfloat dbz;

    int fn_rho = mapr->field_index_num( "RHOHV" );
    if( fn_rho < 0 )
	{ fn_rho = mapr->field_index_num( "RH" ); }
//     short * ssrho = (short *)mapr->raw_data_ptr( fn_rho );
//     Xfloat rcp_rho_scale = 1./mapr->scale( fn_rho);

    int fn_zdr = mapr->field_index_num( "ZDR" );
    if( fn_zdr < 0 )
	{ fn_zdr = mapr->field_index_num( "ZD" ); }
    short * sszdr = (short *)mapr->raw_data_ptr( fn_zdr );
    Xfloat rcp_zdr_scale = 1./mapr->scale( fn_zdr);
    Xfloat zdr;

//     int fn_ncp = mapr->field_index_num( "NCP" );
//     short * ssncp = (short *)mapr->raw_data_ptr( fn_ncp );
//     Xfloat rcp_ncp_scale = 1./mapr->scale( fn_ncp);

    Xfloat * col4 = phi_dp0;
    Xfloat * col12 = kdpDouble0;

    char fname[256], str[256];;
    sprintf( fname, "%sRay%d.txt", dir, (int)az1 );
    printf( "%s\n", fname );
    std::ofstream oFile( fname, std::ios::out );

    for( ii = 0; ii < num_gates; ii++ ) {
	sprintf(str, "%10.2f", M_TO_KM(mapr->celv->dist_cells[ii]));

	dbz = *(ssdbz++) * rcp_dbz_scale;
	sprintf(str + strlen(str), " %10.2f", dbz);
	zdr = *sszdr++ * rcp_zdr_scale;
	sprintf(str + strlen(str), " %10.2f", zdr);
	sprintf(str + strlen(str), " %10.2f", *col4++);
	sprintf(str + strlen(str), " %10.2f", *col12++);

# ifdef obsolete
	rho = *ssrho++ * rcp_rho_scale;
	sprintf(str + strlen(str), " %10.3f", rho);
//	sprintf(str + strlen(str), " %10.2f", *col2++);
	//	sprintf(str + strlen(str), " %10.2f", *col5++);
	//	sprintf(str + strlen(str), " %10.2f", *col6++);
	//	sprintf(str + strlen(str), " %10.2f", *col7++);
	//	sprintf(str + strlen(str), " %10.2f", *col8++);
	sprintf(str + strlen(str), " %10.2f", *col9++);
	sprintf(str + strlen(str), " %10.2f", *col10++);
	sprintf(str + strlen(str), " %10.2f", *col11++);
	sprintf(str + strlen(str), " %10.2f", *col12++);
	sprintf(str + strlen(str), " %10.2f", *col13++);
	ncp = *ssncp++ * rcp_ncp_scale;
	sprintf(str + strlen(str), " %10.2f", ncp);
# endif
	sprintf(str + strlen(str), " \n" );
	
	oFile.write( str, strlen( str ));
    }

    oFile.close();
}

