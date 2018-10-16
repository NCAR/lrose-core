/**
 * @file VolumeInfo.hh 
 * @brief Information about the volume as regards location, dimensions,
 *        number of elevation angles, radar characteristics
 * @class VolumeInfo
 * @brief Information about the volume as regards location, dimensions,
 *        number of elevation angles, radar characteristics
 *
 * Members public as its a 'struct-like' class
 */

#ifndef VOLUME_INFO_H
#define VOLUME_INFO_H

#include <vector>
#include <string>

//------------------------------------------------------------------
class VolumeInfo 
{
public:

  /**
   * Constructor empty
   */
  VolumeInfo(void);

  /**
   * Constructor that fills in all values, using inputs as is
   * @param[in] nx   Number of gridpoints x (radial)
   * @param[in] ny   Number of gridpoints y (azimuthal)
   * @param[in] dx   Distance between gridpoints x (radial)  km
   * @param[in] dy   Distance between gridpoints y (azimuthal)  degrees
   * @param[in] x0   Lower left grid location x (km from 0.0)
   * @param[in] y0   Lower ldef grid location y (degrees from 0.0)
   * @param[in] proj Projection type, in MDV enumerated field header units
   * @param[in] lat  Latitude of (0,0) point, degrees
   * @param[in] lon  Longitude of (0,0) point, degrees
   * @param[in] hasWavelength  True if it is radar data and wavelength is set
   * @param[in] wavelengthCm   Wavelength (cm) when hasWavelength=true
   * @param[in] altitude Altitude (km) of sensor
   * @param[in] vlevels  Elevation angles, degrees, low to high
   */
  VolumeInfo(int nx, int ny, double dx, double dy, double x0, double y0,
	     int proj, double lat, double lon, bool hasWavelength,
	     double wavelengthCm, double altitude,
	     const std::vector<double> &vlevels);
  /**
   * Destructor
   */
  virtual ~VolumeInfo(void);


  /**
   * operator==
   * @param[in] g  Object to compare to
   */
  bool operator==(const VolumeInfo &g) const;

  /**
   * operator!=
   * @param[in] g  Object to compare to
   */
  bool operator!=(const VolumeInfo &g) const;

  /**
   * operator== without the radar related comparisons (altitude/wavelength)
   * @param[in] g  Object to compare to
   */
  bool equalGrids(const VolumeInfo &g) const;

  /**
   * @return true if the number of gridpoints, deltas, and vlevels are the same
   * @param[in] g  Object to compare to
   */
  bool equalSizes(const VolumeInfo &g) const;

  /**
   * LOG(PRINT) the contents
   */
  void printInfo(void) const;

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * Debug print
   * @return string with information in it
   */
  std::string sprint(void) const;

  /**
   * @return true if its a full circle (assumes polar data, azimuth=y)
   */
  bool isCircle(void) const;

  /**
   * @return azimuth value for input index, in range 0 to 359
   *
   * @param[in] ibeam  the index
   */
  double azimuth0to359(int ibeam) const;


  /**
   * @return vertical level (degrees) for input index, -1 for error
   *
   * @param[in] index  0,1,...
   */
  double verticalLevel(int index) const;

  /**
   * return vertical level for input index and whether it is first or last
   *
   * @param[in] index   0,1,...
   * @param[out] isLast  true if this is the highest (last) vertical level 
   *                     in the volume
   * @param[out] isFirst  true if this is the lowest (first) vertical level 
   *                      in the volume
   *
   * @return vertical level in degrees
   */
  double setNext(int index, bool &isLast, bool &isFirst) const;

  /**
   * @return true if an index is out of bounds (too large) with respect to
   * vertical level indices
   * @param[in] index
   */
  bool indexTooBig(int index) const;

  /**
   * @return true if an index is for the last (highest) vertical level
   * in the volume
   * @param[in] index
   */
  bool isLastVlevel(int index) const;

  /**
   * @return the vertical levels (degrees) lowest to highest
   */
  inline std::vector<double> getVlevels(void) const { return _vlevels; }

  /**
   * @return the wavelength in centimeters
   *
   * @param[in] wavelengthDefaultCm  Default value to use if overriding
   *                                 data or no wavelength in data
   * @param[in] overrideWavelength  True to not use the data wavelength,
   *                                and do use wavelengthDefaultCm
   */
  double setWavelengthCm(double wavelengthDefaultCm,
			 bool overrideWavelength) const;

  /**
   * @return the sensor height in km
   *
   * @param[in] heightDefaultKm  Default value to use if overriding
   *                             data or no height in data
   * @param[in] overrideHeight   True to not use the data height,
   *                             and do use HeightDefaultKm
   */
  double setHeightKm(double heightDefaultKm, bool overrideHeight) const;


  /**
   * Shift azimuth grid specification by a fixed number (index), with
   * wraparound assumed for 360s
   * @param[in] n  Number of azimuthal indices to shift
   */
  void shiftAzimuthInfo(int n);

  /**
   * Synchronize some radar params local using input info when input is better
   * @param[in] info
   */
  void synchRadarParams(const VolumeInfo &info);

  int _nx;     /**< Number of gridpoints x (radial) */
  int _ny;     /**< Number of gridpoints y (azimuthal) */
  double _dx;  /**< gridpoint size x (km) */
  double _dy;  /**< gridpoint size y (degrees) */
  double _x0;  /**< lower left grid location x (km from 0) */
  double _y0;  /**< lower left grid location y (degrees from 0) */
  int    _proj; /**< Projection type */
  double _lat; /**< Latitude of x=0,y=0 (degrees) */
  double _lon; /**< Longitude of x=0,y=0 (degrees) */

  bool _hasAltitude; /**< True if sensor altitude is set */
  double _altitudeKm;  /**< Altitude (km) of sensor, when _hasAltitude=true */
  bool _hasWavelength; /**< True if it is radar data and wavelength is set */
  double _wavelengthCm; /**< Wavelength (cm) when _hasWavelength=true */

  std::vector<double> _vlevels; /**< Elevation angles, degrees, low to high */

protected:
private:

};

#endif
