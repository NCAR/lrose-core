/**
 * @file VolumeMdv.hh
 * @brief Volume data, with Mdv as the IO
 * @class VolumeMdv
 * @brief Volume data, with Mdv as the IO
 */

#ifndef VOLUME_MDV_HH
#define VOLUME_MDV_HH

#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/Mdvx.hh>
#include <rapmath/SpecialUserData.hh>
#include <rapmath/VolumeData.hh>
#include <ctime>
#include <vector>

class DsTrigger;
class DsMdvx;
class SweepMdv;

//------------------------------------------------------------------
class VolumeMdv : public VolumeData
{
  friend class SweepMdv;
public:

  /**
   * Empty constructor
   */
  VolumeMdv(void);

  /**
   * Constructor.  Parses argc and argv to set up triggering
   *
   * @param[in] parms  Pointer to parameters (derived class)
   * @param[in] argc
   * @param[in] argv
   */
  VolumeMdv(const FiltAlgParms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~VolumeMdv(void);


  #define FILTALG_BASE
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_BASE

  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * @param[out] t   time that was triggered.
   *
   * @return true if a time was triggered, false for no more triggering.
   */
  bool triggerMdv(time_t &t);

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
  void addNewMdv(int zIndex, const SweepMdv &s);

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
  bool storeMathUserDataMdv(const std::string &name, MathUserData *v);

  /**
   * Output entire volume as MDV, based on internal state
   * @param[in] t  Time for output volume
   */
  void output(const time_t &t);

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
  inline bool clockwise(void) const {return _positiveDy;}
  
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

    inline int num(void) const {return (int)_grid2d.size();}
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

  Mdvx::master_header_t _masterHdr;  /**< Mdv master header */
  Mdvx::field_header_t _fieldHdr;    /**< Mdv field header (any one) */
  Mdvx::vlevel_header_t _vlevelHdr;  /**< Mdv vlevel header */
  MdvxProj _proj;                    /**< Projection */
  std::vector<double> _vlevel;       /**< The vertical levels degrees */
  int _nz, _nx, _ny;                 /**< Grid dimensions */
  bool _positiveDy;                  /**< positive or negative azimuthal increment */
  bool _hasWavelength;               /**< True if input data has wavelength */
  bool _hasAltitude;                 /**< True if input data has altitude */
  double _altitude;                  /**< Altitude if _hasAltitude */
  double _wavelength;                /**< Wavelength if _hasWavelength */
  const FiltAlgParms *_parms;        /**< Pointer to derived parameters */

  std::vector<GridFields> _data;     /**< All the data at each height */
  SpecialUserData *_special;        /**< The names/ptrs to special data */

  bool _initialInitializeInput(const time_t &t, const UrlSpec &u);
  bool _initializeInput(const time_t &t, const UrlSpec &u);
  void _outputToUrl(const time_t &t, const UrlSpec &u);
  void _outputFieldToUrl(const std::string &internalName,
			 const std::string &externalName,
			 const time_t &t, DsMdvx &out);

private:
};

#endif
