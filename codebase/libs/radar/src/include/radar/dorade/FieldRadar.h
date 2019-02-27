/*
 *	$Id: FieldRadar.h,v 1.2 2019/02/27 02:59:40 dixon Exp $
 *
 *	Module:		 FieldRadar.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2019/02/27 02:59:40 $
 *
 * revision history
 * ----------------
 * Revision 1.5  1992/09/24  17:12:09  thor
 * Added new items.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/10/16  15:34:22  thor
 * Increased array size to elinimate padding.
 *
 * Revision 1.2  1991/10/15  17:55:06  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:23  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCFieldRadarh
#define INCFieldRadarh

struct field_radar_i {
    char  field_radar_info[4];	/* Identifier for a field written */
				/* radar information block */
				/* (ascii characters FRIB). */
    long field_radar_info_len;	/* Length of this field written radar */
				/* information block in bytes. */
    long data_sys_id;		/* Data system identification. */
    float loss_out;		/* Waveguide Losses between Transmitter and */
				/* antenna in db. */
    float loss_in;		/* Waveguide Losses between antenna and Low */
				/* noise amplifier in db. */
    float loss_rjoint;		/* Losses in the rotary joint in db. */
    float ant_v_dim;		/* Antenna Vertical Dimension in m. */
    float ant_h_dim;		/* Antenna Horizontal Dimension in m. */
    float ant_noise_temp;	/* Antenna Noise Temperature in degrees K. */
    float r_noise_figure;	/* Receiver noise figure in dB*/
    float xmit_power[5];        /* Nominal Peak transmitted power in dBm
                                   by channel */
    float x_band_gain;          /* X band gain in dB */
    float receiver_gain[5];     /* Measured receiver gain in dB (by channel) */
    float if_gain[5];           /* Measured IF gain in dB (by channel) */
    float conversion_gain;      /* A to D conversion gain in dB */
    float scale_factor[5];      /* Scale factor to account for differences in
                                   the individual channels, and the inherent
                                   gain due to summing over the dwell time */
    float processor_const;      /* Constant used to scale dBz to
				   units the display processors understand */
    long dly_tube_antenna;	/* Time delay from RF being applied to
				   tube and energy leaving antenna in ns. */
    long dly_rndtrip_chip_atod;/* Time delay from a chip generated in the 
				   timing module and the RF pulse entering
                                   the A to D converters.  Need to take the
                                   RF input to the HPA and inject it into 
                                   the waveguide back at the LNA to make this
                                   measurement  in ns*/
    long dly_timmod_testpulse; /* Time delay from timing Module test
				   pulse edge and test pulse arriving at
				   the A/D converter in ns. */
    long dly_modulator_on;	/* Modulator rise time (Time between
				   video on into HPA and modulator full up in
				   the high power amplifier) in ns. */
    long dly_modulator_off;	/* Modulator fall time (Time between
				   video off into the HPA
				   and modulator full off) in ns. */
    float peak_power_offset;     /*Added to the power meter reading of the
                                   peak output power this yields actual
				   peak output power (in dB) */ 
    float test_pulse_offset;    /* Added to the power meter reading of the
                                   test pulse this yields actual injected
                                   test pulse power (dB) */
    float E_plane_angle;           /* E-plane angle (tilt) this is the angle in
				   the horizontal plane (when antennas are
				   vertical) between a line normal to the
				   aircraft's longitudinal axis and the radar
				   beam in degrees.  Positive is in direction
				   of motion (fore) */
    float H_plane_angle;         /* H plane angle in degrees - this follows
				    the sign convention described in the
				    DORADE documentation for ROLL angle */
    float encoder_antenna_up;   /* Encoder reading minus IRU roll angle
				   when antenna is up and horizontal */
    float pitch_antenna_up;     /* Antenna pitch angle (measured with
				   transit) minus IRU pitch angle when
				   antenna is pointing up */
    short indepf_times_flg;	/* 0 = neither recorded, 1 = independent
				   frequency data only, 3 = independent 
				   frequency and time series data recorded */
    short indep_freq_gate;	/* gate number where the independent frequency
                                   data comes from */
    short time_series_gate;	/* gate number where the time series data come
                                   from */
    short num_base_params;      /* Number of base parameters. */
    char  file_name[80];	/* Name of this header file. */
}; /* End of Structure */


struct field_radar_iv1 {
    char  field_radar_info[4];	/* Identifier for a field written */
				/* radar information block */
				/* (ascii characters FRIB). */
    long field_radar_info_len;	/* Length of this field written radar */
				/* information block in bytes. */
    long data_sys_id;		/* Data system identification. */
    float loss_out;		/* Waveguide Losses between Transmitter and */
				/* antenna in db. */
    float loss_in;		/* Waveguide Losses between antenna and Low */
				/* noise amplifier in db. */
    float loss_rjoint;		/* Losses in the rotary joint in db. */
    float ant_v_dim;		/* Antenna Vertical Dimension in m. */
    float ant_h_dim;		/* Antenna Horizontal Dimension in m. */
    float ant_noise_temp;	/* Antenna Noise Temperature in degrees K. */
    float r_noise_figure;	/* Receiver noise figure in dB*/
    float xmit_power[5];        /* Nominal Peak transmitted power in dBm
                                   by channel */
    float x_band_gain;          /* X band gain in dB */
    float receiver_gain[5];     /* Measured receiver gain in dB (by channel) */
    float if_gain[5];           /* Measured IF gain in dB (by channel) */
    float conversion_gain;      /* A to D conversion gain in dB */
    float scale_factor[5];      /* Scale factor to account for differences in
                                   the individual channels, and the inherent
                                   gain due to summing over the dwell time */
    float processor_const;      /* Constant used to scale dBz to
				   units the display processors understand */
    long dly_tube_antenna;	/* Time delay from RF being applied to
				   tube and energy leaving antenna in ns. */
    long dly_rndtrip_chip_atod; /* Time delay from a chip generated in the 
				   timing module and the RF pulse entering
                                   the A to D converters.  Need to take the
                                   RF input to the HPA and inject it into 
                                   the waveguide back at the LNA to make this
                                   measurement  in ns*/
    long dly_timmod_testpulse;  /* Time delay from timing Module test
				   pulse edge and test pulse arriving at
				   the A/D converter in ns. */
    long dly_modulator_on;	/* Modulator rise time (Time between
				   video on into HPA and modulator full up in
				   the high power amplifier) in ns. */
    long dly_modulator_off;	/* Modulator fall time (Time between
				   video off into the HPA
				   and modulator full off) in ns. */
    float peak_power_offset;    /* Added to the power meter reading of the
                                   peak output power this yields actual
				   peak output power (in dB) */ 
    float test_pulse_offset;    /* Added to the power meter reading of the
                                   test pulse this yields actual injected
                                   test pulse power (dB) */
    float E_plane_angle;        /* E-plane angle (tilt) this is the angle in
				   the horizontal plane (when antennas are
				   vertical) between a line normal to the
				   aircraft's longitudinal axis and the radar
				   beam in degrees.  Positive is in direction
				   of motion (fore) */
    float H_plane_angle;        /* H plane angle in degrees - this follows
				   the sign convention described in the
				   DORADE documentation for ROLL angle */
    float encoder_antenna_up;   /* Encoder reading minus IRU roll angle
				   when antenna is up and horizontal */
    float pitch_antenna_up;     /* Antenna pitch angle (measured with
				   transit) minus IRU pitch angle when
				   antenna is pointing up */
    short indepf_times_flg;	/* 0 = neither recorded, 1 = independent
				   frequency data only, 3 = independent 
				   frequency and time series data recorded */
    short indep_freq_gate;	/* gate number where the independent frequency
                                   data comes from */
    short time_series_gate;	/* gate number where the time series data come
                                   from */
    short padding;
}; /* End of Structure */


struct field_radar_i_v0 {
    char  field_radar_info[4];	/* Identifier for a field written */
				/* radar information block */
				/* (ascii characters FRIB). */
    long field_radar_info_len;	/* Length of this field written radar */
				/* information block in bytes. */
    short data_sys_id;		/* Data system identification. */
    short signal_source;	/* Signal source. */
    float loss_out;		/* Losses between Transmitter and */
				/* antenna in db. */
    float loss_in;		/* Losses between antenna and Low */
				/* noise amplifier in db. */
    float ant_loss;		/* Losses in the antenna itself in db. */
    float sys_loss_0;		/* Other not yet defined system losses */
				/* #0 in db. */
    float sys_loss_1;		/* Other not yet defined system losses */
				/* #1 in db. */
    float sys_loss_2;		/* Other not yet defined system losses */
				/* #2 in db. */
    float sys_loss_3;		/* Other not yet defined system losses */
				/* #3 in db. */
    float ant_v_dim;		/* Antenna Vertical Dimension in m. */
    float ant_h_dim;		/* Antenna Horizontal Dimension in m. */
    float aperture_eff;		/* Aperture Efficiency. */
    char  filter_num[8];	/* If signal processor filter number */
				/* being used. */
    float bessel_correct;	/* Bessel Filter Correction Factor. */
    float ant_noise_temp;	/* Antenna Noise Temperature in degrees K. */
    float r_noise_figure;	/* Receiver noise figure in ?. */
    short dly_tube_antenna;	/* Time delay from RF being applied to */
				/* tube and energy leaving antenna in ns. */
    short dly_antenna_dircplr;	/* Time delay from energy entering */
				/* antenna and it reaching the test */
				/* pulse directional coupler in ns. */
    short dly_dircplr_ad[6];	/* Time delay from the test pulse. */
				/* directional coupler to the A/D */
				/* converters in ns. */
    short dly_timmod_testpulse; /* Time delay from timeing Module test */
				/* pulse edge and test pulse being */
				/* injected into directional coupler in ns. */
    short dly_modulator_on;	/* Modulator rise time (Time between */
				/* modulator on in timing module and */
				/* modulator full up in the high power */
				/* amplifier) in ns. */
    short dly_modulator_off;	/* Modulator fall time (Time between */
				/* end last chip in the timing module */
				/* and modulator full off) in ns. */
    short dly_rf_twt_on;	/* Time between a chip edge in timing */
				/* module and chip out of the TWT in ns. */
    short dly_rf_twt_off;	/* Time between a chip off edge in */
				/* timing module and chip actually off */
				/* out of the TWT in ns. */
    short indepf_times_flg;	/*  */
    short indep_freq_gate;	/*  */
    short times_series_gate;	/*  */
}; /* End of Structure */


typedef struct field_radar_i field_radar_i;
typedef struct field_radar_i FIELDRADAR;

#endif 
