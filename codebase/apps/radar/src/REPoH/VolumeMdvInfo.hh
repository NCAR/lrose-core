/**
 * @file VolumeMdvInfo.hh
 * @brief Mdv related information for a volume
 * @class VolumeMdvInfo
 * @brief Mdv related information for a volume
 */

#ifndef VOLUME_MDV_INFO_HH
#define VOLUME_MDV_INFO_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/Mdvx.hh>
#include <vector>

class UrlParms;
class RepohParams;
class GridFieldsAll;

//------------------------------------------------------------------
class VolumeMdvInfo
{
public:

  /**
   * Constructor
   */
  VolumeMdvInfo(void);

  /**
   * Destructor
   */
  virtual ~VolumeMdvInfo(void);

  inline int nx(void) const {return _nx;}
  inline int ny(void) const {return _ny;}
  inline int nz(void) const {return _nz;}
  inline int numData(void) const {return _nx*_ny;}
  
  inline const MdvxProj &proj(void) const {return _proj;}
  inline const std::vector<double> &vlevel(void) const {return _vlevel;}
  inline double vlevel(int index) const {return _vlevel[index];}
  inline double dx(void) const {return _coord.dx;}
  
    
  std::vector<double> noisePerRange(double noiseAt100Km) const;

  bool initialInitializeInput(const time_t &t, const UrlParms &u,
			      const RepohParams &parms);

  
  bool initializeInput(const time_t &t, const UrlParms &u,
		       const RepohParams &parms, GridFieldsAll &data);

  void outputToUrl(const time_t &t, const UrlParms &u,
		   const GridFieldsAll &data);



protected:
private:

  Mdvx::master_header_t _masterHdr;
  Mdvx::field_header_t _fieldHdr;
  Mdvx::vlevel_header_t _vlevelHdr;
  MdvxProj _proj;
  Mdvx::coord_t _coord;
  bool _hasWavelength;
  bool _hasAltitude;
  double _altitude, _wavelength;
  std::vector<double> _vlevel;
  int _nz, _nx, _ny;

  void _outputFieldToUrl(const std::string &name,
			 const time_t &t,
			 const GridFieldsAll &data,
			 DsMdvx &out);
};

#endif
