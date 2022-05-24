/**
 * @file VirtVolVolume.hh
 * @brief Volume data
 * @class VirtVolVolume
 * @brief Volume data
 */

#ifndef VIRT_VOL_VOLUME_HH
#define VIRT_VOL_VOLUME_HH

#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <FiltAlgVirtVol/VirtVolMdv.hh>
// #include <Mdv/MdvxProj.hh>
// #include <Mdv/MdvxRadar.hh>
// #include <Mdv/Mdvx.hh>
// #include <Mdv/MdvxChunk.hh>
#include <rapmath/SpecialUserData.hh>
#include <rapmath/VolumeData.hh>
#include <ctime>
#include <vector>

class DsTrigger;
// class DsMdvx;
class VirtVolSweep;
class PolarCircularTemplate;

//------------------------------------------------------------------
class VirtVolVolume : public VolumeData
{
  friend class VirtVolSweep;

public:

  /**
   * Empty constructor
   */
  VirtVolVolume(void);

  /**
   * Constructor.  Parses argc and argv to set up triggering
   *
   * @param[in] parms  Pointer to parameters (derived class)
   * @param[in] argc
   * @param[in] argv
   */
  VirtVolVolume(const FiltAlgParms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~VirtVolVolume(void);

  /**
   * @return the VirtVolVolume function definitions, which are supported
   * for any derived class
   */
  static std::vector<FunctionDef> virtVolUserUnaryOperators(void);

  #define FILTALG_BASE
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #include <FiltAlgVirtVol/VirtVolVolumeVirtualMethods.hh>
  #undef FILTALG_BASE

  /**
   * Process the input specification to create user data
   * @param[in] p  The function definition, Should be one of those supported
   *               by VirtVolVolume (see the static strings) 
   *
   * @return NULL for error, the correct user data otherwise
   */
  MathUserData *processVirtVolUserVolumeFunction(const UnaryNode &p);

  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * @param[out] t   time that was triggered.
   *
   * @return true if a time was triggered, false for no more triggering.
   */
  bool triggerVirtVol(time_t &t);

  /**
   * Clear out the _data vector
   */
  void clear(void);

  /**
   * @return pointer to vector of grids (one per field) at a height
   * @param[in] index The index to the height
   */
  const std::vector<GriddedData> *get2d(int index) const;

  /**
   * @return pointer to vector of grids (one field) at all height, in
   * ascending height order, by creating new grids and copying into return
   *
   * @param[in] name The name of the field
   */
  std::vector<GriddedData> getField3d(const std::string &name) const;

  /**
   * Take the new data grids out of the input sweep and store into the volume
   * @param[in] zIndex  Index to height for this Sweep
   * @param[in] s  The sweep
   */
  void addNewSweep(int zIndex, const VirtVolSweep &s);

  /**
   * Take the input gridded data and add it to a height as a new field
   * @param[in] zIndex  Index to height
   * @param[in] g  The grid
   */
  void addNewGrid(int zIndex, const GriddedData &g);

  /**
   * Store input math user data into the volume
   * @param[in] name  Name of user data
   * @param[in] v  Pointer to the user data
   *
   * @return true if succesfully stored
   *
   * @note the pointer v becomes owned by this object
   */
  bool storeMathUserDataVirtVol(const std::string &name, MathUserData *v);

  /**
   * Output entire volume as MDV and/or netCDF, based on internal state
   * Forms full volumes from local 2d grids, one per url
   *
   * @param[in] t  Time for output volume
   */
  void output(const time_t &t);

  /**
   * Output the named special data fields as 2 dimensional output.
   * @note it is assumed that the names point to special data that is of type VertData2d.
   * @param[in] t  Time of output
   * @param[in] u Specification of the URL and fields to write
   */
  void specialOutput2d(const time_t &t, const UrlParms &u);

  /**
   * @return number of gates
   */
  int getNGates(void) const;

  /**
   * @return gate spacing (km)
   */
  double getDeltaGateKm(void) const;

  /**
   * @return azimuthal spacing (degrees)
   */
  double getDeltaAzDeg(void) const;

  /**
   * @return distance to first gate (km)
   */
  double getStartRangeKm(void) const;

  /**
   * @return Number of vertical levels
   */
  int nz(void) const;
  
  /**
   * @return true if positive azimuithal increment
   */
  inline bool clockwise(void) const {return _mdv.clockwise();}
  
  bool isCircular(void) const;

  inline MdvxProj proj(void) const {return _mdv.proj(); }


  inline std::vector<double> getVlevel(void) const {return _mdv.getVlevel();}
  
  inline double getIthVlevel(int i) const {return _mdv.getIthVlevel(i);}

  inline void getRadarParams(bool &hasWavelength, bool &hasAltitude,
			     double &altitude, double &wavelength) const
  {
    _mdv.getRadarParams(hasWavelength, hasAltitude, altitude,
			wavelength);
  }
  inline double extractAltitude(void) const {return _mdv.extractAltitude();}

protected:

  /**
   * @class GridFields
   * @brief all fields at a height
   */
  class GridFields
  {
  public:
    /**
     * Constructor, no grids
     * @param[in] z  Height index
     */
    inline GridFields(int z) : _z(z) {}
    /**
     * Destructor
     */
    inline ~GridFields(void) {}

    /**
     * @return number of 2d grids in the volume
     */
    inline int num(void) const {return (int)_grid2d.size();}

    /**
     * @return pointer to i'th 2d grid
     * @param[in] i
     */
    inline const GriddedData *ithGrid(int i) const {return &_grid2d[i];}
    
    int _z;                            /**< The height index */
    std::vector<GriddedData> _grid2d;  /**< The grids, one per field */
  private:
  };

  time_t _time;         /**< Trigger time */
  bool _archiveChecked; /**< True if archive mode has been checked for */
  bool _isArchiveMode;  /**< True if it is archive mode, false for realtime*/
  time_t _archiveT0;    /**< Earliest time in archive mode */
  time_t _archiveT1;    /**< Latest time in archive mode */
  bool _debug;          /**< Triggering debug flag */
  DsTrigger *_T;        /**< Triggering pointer */

  // Mdvx::master_header_t _masterHdr;  /**< Mdv master header */
  // Mdvx::field_header_t _fieldHdr;    /**< Mdv field header (any one) */
  // Mdvx::vlevel_header_t _vlevelHdr;  /**< Mdv vlevel header */
  // MdvxProj _proj;                    /**< Projection */
  // std::vector<MdvxChunk> _chunks;    /**< Mdv chunks */
  // std::vector<double> _vlevel;       /**< The vertical levels degrees */
  // int _nz, _nx, _ny;                 /**< Grid dimensions */
  // bool _positiveDy;                  /**< positive or negative azimuthal increment */
  // bool _hasWavelength;               /**< True if input data has wavelength */
  // bool _hasAltitude;                 /**< True if input data has altitude */
  // double _altitude;                  /**< Altitude if _hasAltitude */
  // double _wavelength;                /**< Wavelength if _hasWavelength */
  VirtVolMdv _mdv;

  const FiltAlgParms *_parms;        /**< Pointer to derived parameters */

  std::vector<GridFields> _data;     /**< All the data at each height */
  SpecialUserData *_special;        /**< The names/ptrs to special data */

  bool _initialInitializeInput(const time_t &t, const UrlParms &u);
  bool _initializeInput(const time_t &t, const UrlParms &u);
  bool _initializeInputField(const std::string &field, 
			     const std::string &url);
  void _outputToUrl(const time_t &t, const UrlParms &u);
  // void _outputFieldToUrl(const std::string &name,
  // 			 const time_t &t, DsMdvx &out);
private:
   
  static const std::string _parmsFuzzyStr; /**< Function keyword */
  static const std::string _parmsCircularTemplateStr; /**< Function keyword */
  static const std::string _verticalConsistencyStr; /**< Function keyword */
  static const std::string _verticalClumpFiltStr; /**< Function keyword */
  static const std::string _shapeFiltStr; /**< Function keyword */
  static const std::string _shapeFixedFiltStr; /**< Function keyword */
  static const std::string _volumeTimeStr; /**< Function keyword */

  MathUserData *_computeFuzzyFunction(const std::vector<std::string> &args);
  MathUserData *_computeVerticalConsistency(const std::string &dataStr,
					    const std::string &isNyqString,
					    const std::string &tmpstr);
  MathUserData *_computeVerticalClumpFilt(const std::string &dataName,
					  const std::string &threshName,
					  const std::string &pctName);
  MathUserData *_computeShapes(const std::string &dataName, const std::string &modeStr);
  MathUserData *_computeFixedShapes(const std::string &dataName, const std::string &sizeStr);
  MathUserData *_computeParmsCircularTemplate(const std::string &rstr,
					      const std::string &minrstr);
  MathUserData *_consistency(const std::vector<GriddedData> &data,
			     const std::vector<GriddedData> &isNyquist,
			     const PolarCircularTemplate &pt);
  // Mdvx::master_header_t _tweakedMasterHdr(const time_t &t,
  // 					  int dataDimension) const;
  // Mdvx::field_header_t _tweakedFieldHdr(int dataDimension, int nx, int ny, int nz,
  // 					const time_t &t,  const std::string &name,
  // 					double badValue) const;
  // bool _outputNcf(DsMdvx &out, const UrlParms &u);
  // std::string _computeOutputPath(const DsMdvx &mdvx, const UrlParms &u);
  // void _writeLdataInfo(const DsMdvx &mdvx,
  // 		       const UrlParms &u, const string &outputPath);


  bool _missingValueForField(const std::string &name, double &missingValue) const;
  std::vector<Grid2d> _getData3d(const std::string &name) const;


};

#endif
