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
/**
 *
 * @file JazzXml.hh
 *
 * @class JazzXml
 *
 * Jazz XML handler.
 *  
 * @date 9/24/2010
 *
 */

#ifndef JazzXml_HH
#define JazzXml_HH

#include <euclid/PjgMath.hh>

#include "CiddParamFile.hh"

using namespace std;


class JazzXml
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  JazzXml();
  
  /**
   * @brief Destructor
   */

  virtual ~JazzXml(void);
  
  /**
   * @brief Initialize the parameters.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  
  /**
   * @brief Generate the Jazz XML for the given CIDD parameter file.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool generateXml(const CiddParamFile &cidd_param_file) const;
  
 protected:

  /**
   * @brief Generate the header for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateHeader(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the layers section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateLayers(const CiddParamFile &cidd_param_file) const;
  
  bool _generateMdvLayer(const CiddParamFile &cidd_param_file,
			 const GridField &field,
			 const bool vis_flag) const;
  
  bool _generateMdvwindLayer(const CiddParamFile &cidd_param_file,
			     const WindField &field) const;
  
  bool _generateSymprodLayer(const CiddParamFile &cidd_param_file,
			     const SymprodField &field) const;
  
  bool _generateCiddmapLayer(const CiddParamFile &cidd_param_file,
			     const MapField &field) const;

  /**
   * @brief Generate the menu groups section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateMenuGroups(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the time configuration section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateTimeConfiguration(const CiddParamFile &cidd_param_file) const;
  
  void _generateTimeOffset(const int offset_secs) const;
  
  void _generateTimeInterval(const int seconds) const;
  
  void _generateTime(const DateTime &value) const;
  
  /**
   * @brief Generate the animation configuration section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateAnimationConfiguration(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the altitude configuration section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateAltitudeConfiguration(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the view configuration section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateViewConfiguration(const CiddParamFile &cidd_param_file) const;
  
  void _generateDefaultView() const;
  
  void _generateLatLonView(const PjgMath &math) const;

  void _generateFlatView(const PjgMath &math) const;

  void _generateLambertConfView(const PjgMath &math) const;

  void _generateObliqueStereoView(const PjgMath &math) const;
  
  void _generateAlbersView(const PjgMath &math) const;
  
  void _generateLambertAzimuthalView(const PjgMath &math) const;
  
  void _generateMercatorView(const PjgMath &math) const;
  
  void _generatePolarStereographicView(const PjgMath &math) const;
  
  void _generateTransMercatorView(const PjgMath &math) const;
  
  void _generateVerticalPerspectiveView(const PjgMath &math) const;
  

  /**
   * @brief Generate the areas of interest section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateAreasOfInterest(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the window parameters section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateWindowParameters(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the cross-sections section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateXSections(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the skew-t parameters section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateSkewTParameters(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the optional tool parameters section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateOptionalToolParameters(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the color scales section for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateColorScales(const CiddParamFile &cidd_param_file) const;
  
  /**
   * @brief Generate the footer for the XML.
   *
   * @param[in] cidd_param_file The CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _generateFooter(const CiddParamFile &cidd_param_file) const;
  

};


#endif
