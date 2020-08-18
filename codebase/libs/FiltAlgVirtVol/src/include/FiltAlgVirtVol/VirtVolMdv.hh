/**
 * @file VirtVolMdv.hh
 * @brief Mdv specific data handling
 * @class VirtVolMdv
 * @brief Mdv specific data handling
 */

#ifndef VIRT_VOL_MDV_HH
#define VIRT_VOL_MDV_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <vector>
#include <string>

class FiltAlgParms;
class UrlParms;
class Grid2d;
class GriddedData;
class DsMdvx;

//------------------------------------------------------------------
class VirtVolMdv
{
  friend class VirtVolVolume;
public:

  /**
   * Empty constructor
   */
  VirtVolMdv(void);

  /**
   * Destructor
   */
  virtual ~VirtVolMdv(void);

  void output2d(const time_t &t, const std::vector<Grid2d> &grids, const UrlParms &u);
  bool initialize(const time_t &t, const UrlParms &u, const std::string &fieldName, const FiltAlgParms *parms);
  bool readAllFields(const time_t &t, const std::string &url,
		     const std::vector<std::string> &fields,
		     const FiltAlgParms *parms);
  std::vector<GriddedData> initializeInputField(const std::string &field,
						const std::string &url,
						const FiltAlgParms *parms);
  
  void initializeOutput(const time_t &t, int nz);
  void addOutputField(const time_t &t, const std::string &name, const std::vector<Grid2d> &data3d,
		      double missing);
  void write(const UrlParms &u);

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

  inline std::vector<double> getVlevel(void) const {return _vlevel;}
  inline double getIthVlevel(int i) const {return _vlevel[i];}

  /**
   * @return Number of vertical levels
   */
  int nz(void) const;
  
  /**
   * @return true if positive azimuithal increment
   */
  inline bool clockwise(void) const {return _positiveDy;}
  
  inline MdvxProj proj(void) const {return _proj;}
  
  void getRadarParams(bool &hasWavelength, bool &hasAltitude,
		      double &altitude, double &wavelength) const;

  double extractAltitude(void) const;

protected:

  DsMdvx _mdvRead; /**< The object read into */
  DsMdvx _mdvWrite;  /**< The object written out from */
  Mdvx::master_header_t _masterHdr;  /**< Mdv master header */
  Mdvx::field_header_t _fieldHdr;    /**< Mdv field header (any one) */
  Mdvx::vlevel_header_t _vlevelHdr;  /**< Mdv vlevel header */
  MdvxProj _proj;                    /**< Projection */
  std::vector<MdvxChunk> _chunks;    /**< Mdv chunks */
  std::vector<double> _vlevel;       /**< The vertical levels degrees */
  int _nz, _nx, _ny;                 /**< Grid dimensions */
  bool _positiveDy;                  /**< positive or negative azimuthal increment */
  bool _hasWavelength;               /**< True if input data has wavelength */
  bool _hasAltitude;                 /**< True if input data has altitude */
  double _altitude;                  /**< Altitude if _hasAltitude */
  double _wavelength;                /**< Wavelength if _hasWavelength */
private:
   
   Mdvx::master_header_t _tweakedMasterHdr(const time_t &t,
   					  int dataDimension) const;
   Mdvx::field_header_t _tweakedFieldHdr(int dataDimension, int nx, int ny, int nz,
   					const time_t &t,  const std::string &name,
   					double badValue) const;
};

#endif
