/**
 * @file RadxAppVolume.hh
 * @brief Volume data, with Radx underlying IO
 * @class RadxAppVolume
 * @brief Volume data, with Radx underlying IO
 */

#ifndef RadxAppVolume_HH
#define RadxAppVolume_HH

#include <radar/RadxAppParms.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
#include <rapmath/VolumeData.hh>
#include <didss/LdataInfo.hh>

//------------------------------------------------------------------
class RadxAppVolume : public VolumeData
{
  friend class RadxAppRayData;
  friend class RadxAppSweepData;

public:

  /**
   * Empty constructor
   */
  RadxAppVolume(void);

  /**
   * Constructor - parses command line args to set up triggering
   *
   * @param[in] parms  Pointer to derived parameters
   * @param[in] argc
   * @param[in] argv
   */
  RadxAppVolume(const RadxAppParms *parms, int argc, char **argv);


  /**
   * Copy constructor
   * @param[in] v
   */ 
  RadxAppVolume(const RadxAppVolume &v);

  /**
   * @param[in] v
   */ 
  RadxAppVolume &operator=(const RadxAppVolume &v);
  

  /**
   * Destructor
   */
  virtual ~RadxAppVolume(void);

  #define FILTALG_BASE
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #include <radar/RadxAppVolumeVirtualMethods.hh>
  #undef FILTALG_BASE


  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * @param[out] path   Path of the file that was triggered
   *
   * @return true if something was triggered, false for no more triggering.
   */
  bool triggerRadxVolume(std::string &path);
  
  /**
   * @return reference to the local RadxVol
   */
  inline const RadxVol & getVolRef(void) const {return _vol;}

  /**
   * @return pointer to the local RadxRay pointers vector
   */
  const std::vector<RadxRay *> *getRaysPtr(void) const {return _rays;}

  /**
   * @return reference to the local RadxVol
   */
  inline RadxVol & getVolRef(void) {return _vol;}
  
  /**
   * @return the volume time
   */
  inline time_t getTime(void) const {return _time;}

  /**
   * Trim down to only wanted output fields
   */
  void trim(void);

  /**
   * Write volume out
   */
  bool write(void);

  /**
   * Write volume out to a path
   * @param[in] path
   */
  bool write(const std::string &path);

  /**
   * Rewind the state so that path index points to the first file
   */
  void rewind(void);

  /**
   * Fast forward state so that path index points to beyond the last file
   */
  void fastForward(void);
  
protected:

  RadxVol _vol;  /**< Volume */
  time_t _time;             /**< Trigger time */
  const std::vector<RadxRay *> *_rays;  /**< Pointer to each ray in vol */
  const vector<RadxSweep *> *_sweeps;   /**< Pointer to each sweep */
  RadxRay *_ray;                       /**< Pointer to current ray */
  const RadxAppParms *_parms;         /**< Pointer to derived params */

private:

  RadxAppConfig::Group _activeGroup;  /**< Used when reading stuff */
  int _pathIndex;                     /**< index to next file,
				       * archive or filelist modes*/
  std::vector<std::string> _paths;    /**< The files, archive or filelist mode*/
  bool _realtime;                     /**< True for real time mode */
  LdataInfo _ldata;                   /**< Triggering mechanism for REALTIME */

  /**
   * @return true if object is good
   */
  inline bool isOk(void) const {return _ok;}

private:
  bool _ok;      /**< Object status */

  bool _processFile(const string &path);
  void _setupRead(RadxFile &file);
  void _setupSecondaryRead(RadxFile &file);
  bool _mergeVol(const RadxVol &secondaryVol);
  void _mergeRay(RadxRay &primaryRay, const RadxRay &secondaryRay);
  void _setupWrite(RadxFile &file);
};

#endif
