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
/////////////////////////////////////////////////////////////
// NcarParticleId.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////

/**
 * @file NcarParticleId.hh
 * @class NcarParticleId
 * @brief NcarParticleId takes dual pol moments,
 *        and computes particle ID
 */
#ifndef NcarParticleId_HH
#define NcarParticleId_HH

#include <string>
#include <vector>
#include <toolsa/TaArray.hh>
#include <radar/NcarPidParams.hh>
#include <radar/PidImapManager.hh>
#include <radar/PhidpProc.hh>
#include <radar/TempProfile.hh>
#include <radar/InterestMap.hh>
using namespace std;

////////////////////////
// This class

class NcarParticleId {
  
public:

  // particle IDs

  const static int CLOUD = 1;
  const static int DRIZZLE = 2;
  const static int LIGHT_RAIN = 3;
  const static int MODERATE_RAIN = 4;
  const static int HEAVY_RAIN = 5;
  const static int HAIL = 6;
  const static int RAIN_HAIL_MIXTURE = 7;
  const static int GRAUPEL_SMALL_HAIL = 8;
  const static int GRAUPEL_RAIN = 9;
  const static int DRY_SNOW = 10;
  const static int WET_SNOW = 11;
  const static int ICE_CRYSTALS = 12;
  const static int IRREG_ICE_CRYSTALS = 13;
  const static int SUPERCOOLED_DROPS = 14;
  const static int FLYING_INSECTS = 15;
  const static int SECOND_TRIP = 16;
  const static int GROUND_CLUTTER = 17;
  const static int SATURATED_SNR = 18;
  const static int CHAFF = 19;
  const static int MISC = 20;

  // category of the particle

  typedef enum {
    CATEGORY_ICE,
    CATEGORY_HAIL,
    CATEGORY_MIXED,
    CATEGORY_RAIN,
    CATEGORY_UNKNOWN
  } category_t;
  
  //////////////////////////
  // Interior class: Particle

  /**
   * @class Particle
   *   An object holding interest maps and radar variable
   *   weight information for a single particle type. Computes
   *   various interest fields for a given set of radar
   *   variable values
   */
  class Particle {
  
  public:
    
    /**
     * Constructor
     * @param[in] lbl The label for this particle type
     * @param[in] desc A description of this particle type
     * @param[in] idd The particle id number
     */
    Particle(const string &lbl, const string desc, int idd);
    
    
    /**
     * Destructor
     */
    ~Particle();

    /**
     * Create interest map managers
     * @param[in] tmpWt Weight given to temperature
     * @param[in] zhWt Weight given to horizontal reflectivity
     * @param[in] zdrWt Weight given to differential reflectivity
     * @param[in] kdpWt Weight given to specific differential phase
     * @param[in] ldrWt Weight given to the linear depolarization ratio
     * @param[in] rhvWt Weight given to the correlation coefficient
     * @param[in] sdzdrWt Weight given to the std deviation of zdr
     * @param[in] sphiWt Weight given to the std deviation of the differential phase
     */
    void createImapManagers(double tmpWt,
			    double zhWt,
			    double zdrWt,
			    double kdpWt,
			    double ldrWt,
			    double rhvWt,
			    double sdzdrWt,
			    double sphiWt);

    // add interest map
   
    /**
     * Add an interest map for this particle type
     * @param[in] field The name of the radar variable to analyze. Currently supported
     *                  variables are zh, zdr, ldr, kdp, rhv, tmp, sdzdr, sphi
     * @param[in] minZh Minumum valid horizontal reflectivity value (dbz)
     * @param[in] maxZh Maximum valid horizontal reflectivity value (dbz)
     * @param[in] map A vector of points defining the membership function for this interest map
     */
    void addInterestmap(const string &field,
			double minZh,
			double maxZh,
			const vector<PidInterestMap::ImPoint> map);


    /**
     * Compute interest score based on input values
     * and interest maps set up for this particle type
     * @param[in] dbz The dbz value for this gate
     * @param[in] tempC The tempC value for this gate
     * @param[in] zdr  The zdr value for this gate
     * @param[in] kdp The kdp value for this gate
     * @param[in] ldr The ldr value for this gate
     * @param[in] rhohv The rhohv value for this gate
     * @param[in] sdzdr The sdzdr value for this gate
     * @param[in] sdphidp The sdphidp value for this gate
     */
    void computeInterest(double dbz,
			 double tempC,
			 double zdr,
			 double kdp,
			 double ldr,
			 double rhohv,
			 double sdzdr,
			 double sdphidp);

    /**
     * Print the thresholds and interest maps for this particle type
     * @param[out] out The stream to print to
     */
    void print(ostream &out);

    /**
     * Allocate interest array for gate data
     * @param[in] nGates The number of gates to allocate
     */
    void allocGateInterest(int nGates);

    // data
    
    string label;       /**< Particle type label */
    string description; /**< Particle type description */
    int id;             /**< Integer ID for this particle */

    // limits

    double minZh;       /**< Minimum allowable horizontal reflectivity */
    double maxZh;       /**< Maximum allowable horizontal reflectivity */
    double minTmp;      /**< Minimum allowable temperature */
    double maxTmp;      /**< Maximum allowable temperature */
    double minZdr;      /**< Minimum allowable zdr value */
    double maxZdr;      /**< Maximum allowable zdr value */
    double minLdr;      /**< Minimum allowable LDR value */
    double maxLdr;      /**< Maximum allowable LDR value */
    double minSdZdr;    /**< Minimum allowable std deviation of zdr */
    double maxSdZdr;    /**< Maximum allowable std deviation of zdr */
    double minRhv;      /**< Minimum allowable rhohv */
    double maxRhv;      /**< Maximum allowable rhohv */
    double minKdp;      /**< Minimum allowable kdp */
    double maxKdp;      /**< Maximum allowable kdp */
    
    // interest maps
    
    PidImapManager* _imapZh;      /**< Interest map for zh variable */
    PidImapManager* _imapZdr;     /**< Interest map for zdr variable */
    PidImapManager* _imapLdr;     /**< Interest map for ldr variable */
    PidImapManager* _imapKdp;     /**< Interest map for kdp variable */
    PidImapManager* _imapRhohv;   /**< Interest map for rhohv variable */
    PidImapManager* _imapTmp;     /**< Interest map for temperature variable */
    PidImapManager* _imapSdZdr;   /**< Interest map for std deviation of zdr */
    PidImapManager* _imapSdPhidp; /**< Interest map for std deviation of phidp */
    
    vector<PidImapManager*> _imaps;  /**< Vector to hold the various interest maps */

    // interest value

    double sumWeightedInterest;   /**< The weighted sum of all computed interest fields */
    double sumWeights;            /**< The sum of the weights for the various interest fields */
    double meanWeightedInterest;  /**< The overall interest score for this particle */

    // array for storing interest value at each gate
    
    TaArray<double> gateInterest_; /**< Array for storing interest value at each gate */
    double *gateInterest;          /**< Pointer to the gate interest array */

  };

  /**
   * Constructor
   */
  NcarParticleId();

  /**
   * Destructor
   */ 
  ~NcarParticleId();
  
  /**
   * Free up memory
   */
  void clear();

  /**
   * Set processing options from params object
   * returns 0 on success, -1 on failure
   */

  int setFromParams(const NcarPidParams &params);

  /**
   * Set debugging on
   * @param[in] state True if debugging should be turned on
   */
  void setDebug(bool state) { _debug = state; }
  void setVerbose(bool state) {
    _verbose = state;
    if (state) {
      _debug = true;
    }
  }
  
  /**
   * Set the beam wavelength in cm
   * @param[in] cm The wavelength (cm)
   */
  void setWavelengthCm(double cm) { _wavelengthCm = cm; }

  /**
   * Set the SNR threshold for censoring
   * @param[in] dB - the threshold in Db, gates below this threshold are censored
   */
  void setSnrThresholdDb(double val) { _snrThreshold = val; }

  /**
   * Set the upper SNR threshold for flagging
   * @param[in] dB - the threshold in Db, gates above this are flagged
   * as saturated.
   */
  void setSnrUpperThresholdDb(double val) { _snrUpperThreshold = val; }

  /**
   * Set median filtering on input fields - default is off
   * @param[in] filter_len The number of gates to use for the filter
   */
  void setApplyMedianFilterToDbz(int filter_len) {
    _applyMedianFilterToDbz = true;
    _dbzMedianFilterLen = filter_len;
  }

  /**
   * Apply median filter to ZDR
   * @param[in] filter_len The number of gates to use for the filter
   */
  void setApplyMedianFilterToZdr(int filter_len) {
    _applyMedianFilterToZdr = true;
    _zdrMedianFilterLen = filter_len;
  }

  /**
   * Apply median filter to LDR
   * @param[in] filter_len The number of gates to use for the filter
   */
  void setApplyMedianFilterToLdr(int filter_len) {
    _applyMedianFilterToLdr = true;
    _ldrMedianFilterLen = filter_len;
  }

  /**
   * Apply median filter to rhohv 
   * @param[in] filter_len The number of gates to use for the filter
   */
  void setApplyMedianFilterToRhohv(int filter_len) {
    _applyMedianFilterToRhohv = true;
    _rhohvMedianFilterLen = filter_len;
  }
 
  /** 
   * Option to set LDR to specified value, if missing.
   *
   * When the SNR gets low, LDR is unreliable since there is not
   * sufficient dynamic range to provide an accurate cross-polar power
   * measurement. In these cases, it makes sense to replace LDR with a
   * neutral value, such as 0.0, so that we do not reject gates at
   * which valuable data is available.
   * @param[in] value The value to use for LDR, when unavailable
   */
  void setReplaceMissingLdr(double value = 0.0) {
    _replaceMissingLdr = true;
    _missingLdrReplacementValue = value;
  }

  /**
   * Set median filtering on PID output - default is off
   * @param[in] filter_len The number of gates to use for the filter
   */
  void setApplyMedianFilterToPid(int filter_len) {
    _applyMedianFilterToPid = true;
    _pidMedianFilterLen = filter_len;
  }

  /**
   * Set number of gates for computing standard deviation
   * @param[in] ngates Nmber of gates for computing standard deviation
   */
  void setNgatesSdev(int ngates) {
    _ngatesSdev = ngates;
  }

  /**
   * Set minimum valid interest value
   * If interest value is below this threshold, the pid value is
   * set to missing i.e. 0
   * @param[in] val The minimum valid interest value
   */
  void setMinValidInterest(double val) {
    _minValidInterest = val;
  }

  /**
   * Read in thresholds from file
   * @param[in] path The path to the thresholds file
   * @return 0 on success, -1 on failure
   */
  int readThresholdsFromFile(const string &path);

  /**
   * Load the temperature profile for the specified time.
   * This reads in a new sounding, if available, if the time has changed
   * by more that 900 secs. If this fails, the profile from
   * the thresholds file is used.
   * @param[in] dataTime: the time for the current data
   * @return 0 on success, -1 on failure
   */
  
  int loadTempProfile(time_t dataTime);
  
  /**
   * Set the temperature profile.
   * This is used to override the temperature profile in the
   * thresholds file, for example from a sounding.
   * @param
   */

  void setTempProfile(const TempProfile &profile);

  // Get the temperature profile
  
  const TempProfile &getTempProfile() const { return _tempProfile; }

  // set flag to compute the melting layer
  // Follows Giangrande et al. - Automatic Designation of the
  // Melting Layer with Polarimitric Prototype of WSR-88D Radar.
  // AMS JAMC, Vol47, 2008.

  void setComputeMeltingLayer(bool val) { _computeMl = val; }
 
  /** 
   * Get temperature at a given height
   * @param[in] htKm The height (in km)
   * @return The temperature (C)
   */
  double getTmpC(double htKm);

  /**
   * Initialize the object arrays for later use.
   * Do this if you need access to the arrays, but have not yet called
   * computePidBeam(), and do not plan to do so.
   * For example, you may want to output missing fields that you have
   * not computed, but the memory needs to be there.
   */ 

  void initializeArrays(int nGates);

  /**
   * Compute PID for a beam. Input fields at a gate should be set to 
   * _missingDouble if they are not valid for that gate. Results are
   * stored in local arrays on this class. Use get() methods to retieve them.
   * @param[in] nGates Number of gates
   * @param[in] snr SNR for input data, used for censoring decision
   * @param[in] dbz Reflectivity array
   * @param[in] zdr Differential reflectivity array
   * @param[in] kdp Phidp slope array
   * @param[in] ldr Linear depolarization ratio array 
   * @param[in] rhohv Correlation coeff array
   * @param[in] phidp Phase difference array
   * @param[in] tempC Temperature at each gate, in deg C
   */ 
  void computePidBeam(int nGates,
                      const double *snr,
                      const double *dbz,
                      const double *zdr,
                      const double *kdp,
                      const double *ldr,
                      const double *rhohv,
                      const double *phidp,
                      const double *tempC);
  
  /**
   * Compute PID for a single gate, and related interest value.
   * Also compute second most likely pid and related interest value.
   * @param[in] dbz Reflectivity array
   * @param[in] tempC Temperature at each gate, in deg C
   * @param[in] zdr Differential reflectivity array
   * @param[in] kdp Phidp slope array
   * @param[in] ldr Linear depolarization ratio array 
   * @param[in] rhohv Correlation coeff array
   * @param[in] sdzdr Standard dev. of zdr
   * @param[in] sdphidp Standard dev. of phidp
   * @param[out] pid The primary particle id computed
   * @param[out] interest The interest level of the primary particle
   * @param[out] pid2 The secondary particle id computed
   * @param[out] interest2 The interest level of the secondary particle
   * @param[out] confidence The confidence of the identification, which is expressed
   *    as the difference in interest scores between the primary and secondary pids
   */
  void computePid(double snr,
                  double dbz,
                  double tempC,
                  double zdr,
                  double kdp,
                  double ldr,
                  double rhohv,
                  double sdzdr,
                  double sdphidp,
                  int &pid,
                  double &interest,
                  int &pid2,
                  double &interest2,
                  double &confidence);

  // get fields after calling computePidBeam()

  /**
   * Get snr field after calling computePidBeam()
   * @return Pointer to an array of modified snr values
   */
  const double *getSnr() const { return _snr; }

  /**
   * Get dbz field after calling computePidBeam()
   * @return Pointer to an array of modified dbz values
   */
  const double *getDbz() const { return _dbz; }

  /**
   * Get zdr field after calling computePidBeam()
   * @return Pointer to an array of modified zdr values
   */
  const double *getZdr() const { return _zdr; }

  /**
   * Get kdp field after calling computePidBeam()
   * @return Pointer to an array of modified kdp values
   */
  const double *getKdp() const { return _kdp; }

  /**
   * Get ldr field after calling computePidBeam()
   * @return Pointer to an array of modified ldr values
   */
  const double *getLdr() const { return _ldr; }

  /**
   * Get rhohv field after calling computePidBeam()
   * @return Pointer to an array of modified rhohv values
   */
  const double *getRhohv() const { return _rhohv; }

  /**
   * Get phidp field after calling computePidBeam()
   * @return Pointer to an array of modified phidp values
   */
  const double *getPhidp() const { return _phidp; }

  /**
   * Get temperature field after calling computePidBeam()
   * @return Pointer to an array of modified temperature values
   */
  const double *getTempC() const { return _tempC; }

  /**
   * Get standard dev. of zdr field after calling computePidBeam()
   * @return Pointer to an array of standard dev. of zdr values
   */
  const double *getSdzdr() const { return _sdzdr; }

  /**
   * Get standard dev. of phidp field after calling computePidBeam()
   * @return Pointer to an array of standard dev. of phidp values
   */
  const double *getSdphidp() const { return _sdphidp; }
  
  /**
   * Get primary particle id field after calling computePidBeam()
   * @return Pointer to an array of primary particle ids
   */
  const int *getPid() const { return _pid; }
  
  /**
   * Get category of primary particle id
   */
  const category_t *getCategory() const { return _category; }
  
  /**
   * Get primary particle interest field after calling computePidBeam()
   * @return Pointer to an array of primary particle interest scores
   */
  const double *getInterest() const { return _interest; }
  
  /**
   * Get secondary particle id field after calling computePidBeam()
   * @return Pointer to an array of secondary particle ids
   */
  const int *getPid2() const { return _pid2; }
  
  /**
   * Get secondary particle interest field after calling computePidBeam()
   * @return Pointer to an array of secondary particle interest scores
   */
  const double *getInterest2() const { return _interest2; }
  
  /**
   * Get confidence level of PID after calling computePidBeam()
   * @return Pointer to an array of PID confidence levels, which are expressed
   *    as the difference in interest scores between the primary and secondary pids
   *    at each gate
   */
  const double *getConfidence() const { return _confidence; }
  
  /**
   * Get censor flag applied to PID during call to computePidBeam()
   * @return Pointer to an array of booleans.
   *    If true, censoring was applied to that gate.
   */
  const bool *getCensorFlags() const { return _cflags; }
  
  /**
   * Get melting layer fields
   */

  const double *getMlInterest() const { return _mlInterest; }
  
  /**
   * Get list of possible particle types used by the algorithm, along
   * with interest functions.
   * @return A vector of pointers to Particle objects, one for each possible particle type
   */
  const vector<Particle*> getParticleList() const { return _particleList; }

  /**
   * Get individual particle arrays
   * @return pointers to Particle objects, one for each possible particle type
   */

  const Particle *getParticleCloud() const { return _cl; }
  const Particle *getParticleDrizzle() const { return _drz; }
  const Particle *getParticleLightRain() const { return _lr; }
  const Particle *getParticleModerateRain() const { return _mr; }
  const Particle *getParticleHeavyRain() const { return _hr; }
  const Particle *getParticleHail() const { return _ha; }
  const Particle *getParticleRainHail() const { return _rh; }
  const Particle *getParticleGraupelSmallHail() const { return _gsh; }
  const Particle *getParticleGraupelRain() const { return _grr; }
  const Particle *getParticleDrySnow() const { return _ds; }
  const Particle *getParticleWetSnow() const { return _ws; }
  const Particle *getParticleIce() const { return _ic; }
  const Particle *getParticleIrregIce() const { return _iic; }
  const Particle *getParticleSuperCooledDrops() const { return _sld; }
  const Particle *getParticleBugs() const { return _bgs; }
  const Particle *getParticleSecondTrip() const { return _trip2; }
  const Particle *getParticleClutter() const { return _gcl; }
  const Particle *getParticleChaff() const { return _chaff; }
  const Particle *getParticleMisc() const { return _misc; }
  
  /**
   * Print status 
   * @param[out] out The stream to print to
   */
  void print(ostream &out);

  /**
   * Set the missing data value to use
   * @param[in] missing The missing data value to use
   */ 
  void setMissingDouble(double missing) { _missingDouble = missing; }

  /**
   * Fill a temperature array, for a radar beam elevation
   * @param[in] radarHtKm The radar height in Km MSL
   * @param[in] elevDeg The elevation angle of the radar beam
   * @param[in] nGates The number of gates in the radar beam
   * @param[in] startRangeKm The starting gate for the temperature array
   * @param[in] gateSpacingKm The spacing between gates (km)
   * @param[out] temp The filled temperature array
   */
  void fillTempArray(double radarHtKm,
                     bool setPseudoRadiusRatio,
                     double pseudoRadiusRatio,
                     double elevDeg, int nGates,
                     double startRangeKm, double gateSpacingKm,
                     double *tempC);

  const static double pseudoEarthDiamKm; /**< pseudo earth diameter - for computing radar beam heights */

protected:
private:

  static double _missingDouble; /**< The value to use for missing data */
  NcarPidParams _params;
  bool _debug;              /**< Flag to indicate whether debug messages should be printed */
  bool _verbose;            /**< Flag to indicate whether verbose messages should be printed */
  double _wavelengthCm;     /**< The wavelength (cm) of the radar beam */

  // range geometry
  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;

  // particle types
  Particle* _cl;    /**< Cloud particle type */
  Particle* _drz;   /**< Drizzle particle type */
  Particle* _lr;    /**< Light_Rain particle type */
  Particle* _mr;    /**< Moderate_Rain particle type */
  Particle* _hr;    /**< Heavy_Rain particle type */
  Particle* _ha;    /**< Hail particle type */
  Particle* _rh;    /**< Rain_Hail_Mixture particle type */
  Particle* _gsh;   /**< Graupel_Small_Hail particle type */
  Particle* _grr;   /**< Graupel_Rain particle type */
  Particle* _ds;    /**< Dry_Snow particle type */
  Particle* _ws;    /**< Wet_Snow particle type */
  Particle* _ic;    /**< Ice_Crystals particle type */
  Particle* _iic;   /**< Irreg_Ice_Crystals particle type */
  Particle* _sld;   /**< Supercooled_Liquid_Droplets particle type */
  Particle* _bgs;   /**< Flying_Insects particle type */
  Particle* _trip2; /**< Second trip particle type */
  Particle* _gcl;   /**< Ground_Clutter particle type */
  Particle* _chaff; /**< chaff for radar countermeasures */
  Particle* _misc;  /**< miscellaneous particle type */

  vector<Particle*> _particleList; /**< A vector of pointers to Particle
                                    ** objects, one for each
                                    ** possible particle type */
  
  // temperature profile

  TempProfile _tempProfile;
  time_t _prevProfileDataTime;  /**< Time of previous request
                                 ** to retrieve temp profile */

  TaArray<double> _tempHtLookup_; /**< Lookup array of temp profile heights */
  double *_tempHtLookup;          /**< Lookup array of temp profile heights */

  int _tmpMinHtMeters;          /**< Mimimum height of temp profile (m) */
  int _tmpMaxHtMeters;          /**< Maximum height of temp profile (m) */
  double _tmpBottomC;           /**< Temperature at the base of the profile */
  double _tmpTopC;              /**< Temperature at the top of the profile */

  // weights

  double _tmpWt;   /**< Weight for temperature feature field */
  double _zhWt;    /**< Weight for horiz polarized dbz feature field */              
  double _zdrWt;   /**< Weight for zdr feature field */
  double _kdpWt;   /**< Weight for kdp feature field */
  double _ldrWt;   /**< Weight for ldr feature field */
  double _rhvWt;   /**< Weight for rhohv feature field */
  double _sdzdrWt; /**< Weight for std. dev of zdr feature field */
  double _sphiWt;  /**< Weight for std. dev of phidp feature field */
  
  // store input data in local arrays
  // this data is censored and filtered
  
  TaArray<double> _snr_;   /**< Array of snr values */
  double *_snr;            /**< Pointer to the array of snr values */
  
  TaArray<double> _dbz_;   /**< Array of censored, filtered dbz values */
  double *_dbz;            /**< Pointer to the array of censored, filtered dbz values */
  
  TaArray<double> _zdr_;   /**< Array of censored, filtered zdr values */
  double *_zdr;            /**< Pointer to the array of censored, filtered zdr values */
  
  TaArray<double> _kdp_;   /**< Array of censored, filtered kdp values */
  double *_kdp;            /**< Pointer to the array of censored, filtered kdp values */
  
  TaArray<double> _ldr_;   /**< Array of censored, filtered ldr values */
  double *_ldr;            /**< Pointer to the array of censored, filtered ldr values */

  TaArray<double> _rhohv_; /**< Array of censored, filtered rhohv values */
  double *_rhohv;          /**< Pointer to the array of censored, filtered rhohv values */
  
  TaArray<double> _phidp_; /**< Array of censored, filtered phidp values */
  double *_phidp;          /**< Pointer to the array of censored, filtered phidp values */
  
  TaArray<double> _tempC_; /**< Array of censored, filtered temperature values */
  double *_tempC;          /**< Pointer to the array of censored, filtered temperature values */

  // store computed fields in local arrays
  TaArray<int> _pid_;            /**< Array of computed primary particle ids */
  int *_pid;                     /**< Pointer to the array of computed primary particle ids */
  
  TaArray<double> _interest_;    /**< Array of computed primary particle interest scores */
  double *_interest;             /**< Pointer to the array of computed primary
                                  ** particle interest scores */
  
  TaArray<category_t> _category_; /**< Categories of particle id */
  category_t *_category;          /**< Pointer to the array of categories */
  
  TaArray<int> _pid2_;           /**< Array of computed secondary particle ids */
  int *_pid2;                    /**< Pointer to the array of computed secondary particle ids */
  
  TaArray<double> _interest2_;   /**< Array of computed secondary particle interest scores */
  double *_interest2;            /**< Pointer to the array of computed primary secondary interest scores */
  
  TaArray<double> _confidence_;  /**< Array of computed pid confidence levels */
  double *_confidence;           /**< Pointer to the array of computed pid confidence levels */
  
  TaArray<double> _sdzdr_;       /**< Array of computed std. dev of zdr values */
  double *_sdzdr;                /**< Pointer to the array of computed std. dev of zdr values */
  
  TaArray<double> _sdphidp_;     /**< Array of computed std. dev of phidp values */
  double *_sdphidp;              /**< Pointer to the array of computed std. dev of phidp values */

  // censoring based on SNR

  double _snrThreshold;          /**< Gates with SNR less than this are censored */
  TaArray<bool> _cflags_;        /**< Array of censoring flags */
  bool *_cflags;                 /**< Pointer to the array of censoring flags */
  
  // flag upper SNR threshold

  double _snrUpperThreshold;     /**< Gates with SNR less than this are censored */

  // apply median filtering to input fields
  
  bool _applyMedianFilterToDbz;   /**< Flag to indicate whether median filter is used for dbz field */
  int _dbzMedianFilterLen;        /**< Length (in gates) of dbz median filter (if used) */
  
  bool _applyMedianFilterToZdr;   /**< Flag to indicate whether median filter is used for zdr field */
  int _zdrMedianFilterLen;        /**< Length (in gates) of zdr median filter (if used) */

  bool _applyMedianFilterToLdr;   /**< Flag to indicate whether median filter is used for ldr field */
  int _ldrMedianFilterLen;        /**< Length (in gates) of ldr median filter (if used) */

  bool _applyMedianFilterToRhohv; /**< Flag to indicate whether median filter is used for rhohv field */
  int _rhohvMedianFilterLen;      /**< Length (in gates) of rhohv median filter (if used) */

  // replace missing LDR with specified value

  bool _replaceMissingLdr;             /**< Flag to indicate whether missing LDR should be filled in */
  double _missingLdrReplacementValue;  /**< The value to use for missing LDR data */

  // median filtering to resulting PID field

  bool _applyMedianFilterToPid;   /**< Flag to indicate whether median filter is used for pid field */
  int _pidMedianFilterLen;        /**< Length (in gates) of pid median filter (if used) */

  int _ngatesSdev;                /**< Number of gates for standard deviations */

  double _minValidInterest;       /**< Min valid interest value. If interest value is below this threshold,
                                       the pid value is set to missing i.e. 0 */

  string _thresholdsFilePath;     /**< File path for thresholds file */

  // compute phidp standard deviation

  PhidpProc _phidpProc;

  // melting layer

  bool _computeMl;

  TaArray<double> _mlInterest_;  /**< Combined interest value for melting layer */
  double *_mlInterest;

  InterestMap *_mlDbzInterest;
  InterestMap *_mlZdrInterest;
  InterestMap *_mlRhohvInterest;
  InterestMap *_mlTempInterest;
  
  // allocate the required arrays

  void _allocArrays(int nGates);

  /**
   * Set the particle ID from a line in the thresholds file 
   * @param[out] part The particle whose ID will be set
   * @param[in] line The thresholds file line to parse for the particle ID
   * @return 0 on success, -1 on error
   */
  int _setId(Particle *part, const char *line);

  /**
   * Parse the default temperature profile from a line in the thresholds file 
   * @param[in] line The thresholds file line to parse for the temperature profile
   * @return 0 on success, -1 on error
   */
  int _parseTempProfileLine(const char *line);

  
  /**
   * Get temp profile from spdb
   */
  int _getTempProfileFromSpdb(time_t dataTime);

  /**
   * Compute membership lookup table for temperature 
   */
  void _computeTempHtLookup();

  /**
   * Set the weight of each radar variable from a line in the thresholds file 
   * @param[in] line The thresholds file line to parse for the radar variable weights
   * @return 0 on success, -1 on error
   */
  int _setWeights(const char *line);

  /**
   * Set the valid limits of each radar variable for a particle type 
   * @param[out] part The particle whose limits will be set
   * @param[in] line The thresholds file line to parse for the variable limits
   * @return 0 on success, -1 on error
   */
  int _setLimits(Particle *part, const char *line);

  /**
   * Add an interest map for a particle type and radar variable from
   * a line in the thresholds file 
   * @param[out] part The particle to add an interest map to
   * @param[in] line The thresholds file line to parse for the interest map
   * @return 0 on success, -1 on error
   */
  int _setInterestMaps(Particle *part, const char *line);

  // compute melting layer
  
  void _mlInit();
  void _mlCompute();
  
};

#endif

