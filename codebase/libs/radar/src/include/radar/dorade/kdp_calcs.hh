/* 	$Id: kdp_calcs.hh,v 1.2 2011/09/14 04:28:58 dixon Exp $	 */


# define FIR3GAIN	1.044222
# define FIR3ORDER	20	/* Adaptive filter order and gain */


// c---------------------------------------------------------------------------

struct PeakMarker {
    struct PeakMarker * prev;
    struct PeakMarker * next;
    Xfloat value;
    int gate_num;
    int peak_num;
    int offset;
};

// c---------------------------------------------------------------------------

struct phidp_segment {
    Xfloat avrg1;
    Xfloat avrg2;

    struct phidp_segment *prev;
    struct phidp_segment *next;

    int first_gate;
    int gate_count;
    int endGate;
    int segment_num;
    int ray_num;

    // mark where this segment starts and ends in the working arrays
    // so we have room to initialize filters and continue them
    // slightly beyond the data

    int offset;	
    int offsEnd;
    PeakMarker * firstPeak;
    int numPeaks;

    Xfloat Avrg1;
    Xfloat Avrg2;
};

// c---------------------------------------------------------------------------

// c---------------------------------------------------------------------------

class kdp_calcs {

private:

    int prep_vol_ray_count;
    int csu_vol_ray_count;
    int nssl_vol_ray_count;
    int volume_ray_count;
    int Min_Good;
    int Max_Bad;
    int hail_no;
    int nsslMin_Good;
    int num_gates;
    int baseAvg;
    int doubleBaseAvg;
    int mongoBaseAvg;
    int TstPlsStartGate;
    int TstPlsStopGate;
    int preFilter;
    int postFilter;
    int ray_count;
    int num_phidp_segments;
    int catchit;
    int return_double;

    double * double_arg;
    Xfloat nssl_long_avg_thr;
    Xfloat powerThr;
    Xfloat maxKdp;
    Xfloat hailTestZ;
    Xfloat rho_hail;
    Xfloat rhoThr;
    Xfloat rhoSD_hail;
    Xfloat phidpGlitchThr;
    Xfloat maxPhidpDelta;
    Xfloat TstPlsStartRng;
    Xfloat TstPlsStopRng;
    Xfloat minStdDev;
    Xfloat higherStdDev;
    Xfloat cell_spacing;
    Xfloat kdp_scale;

    AvrgList *avliDouble;
    AvrgList *avliDoubleB;
    AvrgList *avliMongo;
    AvrgList *avliMongoB;

    Xfloat  *deltaRanges;
    Xfloat  *deltaRangesSums;
    Xfloat  *deltaRangesSq;
    Xfloat  *deltaRangesSqSums;

    Xfloat * phiAvg0;
    Xfloat * stdDev0;
    Xfloat * phiDouble0;
    Xfloat * phiMongo0;
    Xfloat * kdpDouble0;
    Xfloat * kdpMongo0;
    Xfloat * DBz0;
    Xfloat * fltrDBz0;

    Xfloat * scrA;
    Xfloat * phi_dpA;
    Xfloat * phizA;
    Xfloat * phiCopyA;
    Xfloat * phiFltrA;

    Xfloat * phi_dp0;
    Xfloat * phiz0;
    Xfloat * phiPreGlitch0;
    Xfloat * phiCopy0;
    Xfloat * phiFltr0;
    Xfloat * Ckdp0;

    float    az1;
    float    az2;

    dd_mapper * mapr;
    phidp_segment * phidp_first_segment;
    phidp_segment * phidp_last_segment;

    // methods

    void preprocessing();

    void nsslDefineSegs();
    void fill_gap(Xfloat *, int, Xfloat, int, Xfloat );
    void NKdp_finally( Xfloat *, Xfloat *, AvrgList * );
    void nssl_delta_range_constants();
    void nssl_reavg_unf();
    int do_nsslKdpx( dd_mapper * );

    phidp_segment * instantiate_a_segment();
    phidp_segment * instantiate_a_segment( phidp_segment * seg ); // a subsequent segment
    PeakMarker * instantiate_a_peak(); // first peak
    PeakMarker * instantiate_a_peak( PeakMarker * peak ); // subsequent peak


public:

    kdp_calcs();

    ~kdp_calcs();

    int do_nsslKdp( dd_mapper *, short * );

    int do_nsslKdp( dd_mapper *, double * );

    void do_plots();

    void set_catch( float az )
    {
	az1 = az;
	az2 = FMOD360( az + 1 );
    }

    void set_TP_limits( float start, float stop )
    { TstPlsStartRng = start; TstPlsStopRng = stop; }

    int return_caught() { return catchit; }

    double return_az1() { return az1; }

    Xfloat * return_phidp() { return phi_dp0; }

    Xfloat * return_phiSD() { return stdDev0; }

    Xfloat * return_phiDBL0() { return phiDouble0; }

    Xfloat * return_phiMON() { return phiMongo0; }

    Xfloat * return_kdpDBL0() { return kdpDouble0; }

    Xfloat * return_kdpMON() { return kdpMongo0; }

    Xfloat * return_Ckdp0() { return Ckdp0; }

};


// c---------------------------------------------------------------------------


