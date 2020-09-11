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
#ifndef Beam_HH
#define Beam_HH

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <cmath>
#include <string>
#include <vector>
#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>

#include "ScaledLabel.hh"
#include "DisplayField.hh"
#include "DisplayFieldController.hh"
#include "Params.hh"

//#if defined(OSX_LROSE) && !defined(SINCOS_DEFN)
//#define SINCOS_DEFN
//#define sincosf(x, s, c) __sincosf(x, s, c)
//#define sincos(x, s, c) __sincos(x, s, c)
//#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/// Base class for Beam objects
///
/// Manage static data for one display beam that can
/// render multiple variables.
/// For the specified solid angle and number
/// of gates:
///
/// - a vector of vertices is created.
/// - nVars vectors, each of length matching the vertices vector,
///   is allocated.
/// 
/// The colors for each variable will be set dynamically
/// by the owner as data need to be rendered. At that time, 
/// a display list is also created (by the beam owner)
/// that uses the display list id. By having display lists
/// created for all variables, the display of beams can be
/// rapidly switched between variables just by executing
/// the correct display list.

class Beam
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] params      TDRP params
   * @param[in] n_fields    Number of fields in the beam.
   */
  
  Beam(const Params &params,
       const RadxRay *ray,
       int n_fields);
  
  /**
   * @brief Destructor
   */

  virtual ~Beam();

  /**
   * @brief Set the brush value for every gate for the given field to the
   *        given brush.
   *
   * @param[in] field Field number.
   * @param[in] brush Brush to use.
   */
  
  virtual void resetFieldBrush(size_t field, const QBrush *brush);

  /**
   * @brief Apply data and color maps to an existing beam.
   *        See PpiWidget::addBeam().
   * 
   * @param[in] beam_data        Vectors of data, one for each field.
   *                             sizes)
   * @param[in] maps             Colormaps, one for each field.
   * @param[in] background_brush Background brush of the display.
   */
  
  virtual void fillColors(const std::vector<std::vector<double> >& beamData,
			  //const std::vector<DisplayField*>& fields,
			  DisplayFieldController *displayFieldController,
			  size_t nFields,
			  const QBrush *background_brush);

  virtual void updateFillColors(const Radx::fl32 *beamData,
				size_t nData,
				//DisplayFieldController *displayFieldController,
				size_t displayFieldIdx,
				size_t nFields,
				const ColorMap *map,
				const QBrush *background_brush);

  virtual void updateFillColorsSparse(const std::vector<double>& field_data,
			  DisplayFieldController *displayFieldController,
			  size_t nFields_expected,
				      const QBrush *background_brush,
				      size_t fieldIdx);


  /**
   * @brief Paint the given field in the given painter.
   */
  
  virtual void paint(QImage *image,
                     const QTransform &transform,
                     size_t field,
                     bool useHeight = false,
                     bool drawInstHt = false) = 0;
  
  /**
   * @brief Print details of beam object
   */

  virtual void print(ostream &out) = 0;

  // get ray pointer
  
  const RadxRay *getRay() const { return _ray; }

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Check to see if the beam is being rendered for the indicated field.
   *        If the field number is invalid (< 0 or >= num fields), returns
   *        whether the beam is being rendered for any field.
   *
   * @param[in] field_num   The field index.
   *
   * @return Returns true if the beam is being rendered for this field, false
   *         otherwise.
   */

  inline bool isBeingRendered(int field_num) const
  {
    if (field_num < 0 || field_num >= (int)_nFields) {
      return isBeingRendered();
    }
    return _beingRendered[field_num];
  }
  

  /**
   * @brief Check to see if the beam is being rendered for any field.
   *
   * @return Returns true if the beam is being rendered for any field, false
   *         otherwise.
   */

  inline bool isBeingRendered() const
  {
    return _beingRenderedAnywhere;
  }
  
  /**
   * @brief Set the flags indicating that the beam is being rendered for the
   *        indicated field.  If the field number is set to an invalid value
   *        (< 0 or >= num fields), sets the flag for all of the fields.
   *
   * @param[in] field_num         The index of the field to set.
   * @param[in] being_rendered    The value of the flag.
   */

  inline void setBeingRendered(int field_num, bool being_rendered)
  {
  
  if (field_num < 0 || field_num >= (int)_nFields)
    {
      setBeingRendered(being_rendered);
      return;
    }

    // If we get here, we got a valid field number.  Set the flag for that
    // field.

    _beingRendered[field_num] = being_rendered;

    if (being_rendered) {
      _beingRenderedAnywhere = true;
    } else {
      // If we are unsetting a being rendered flag, then we need to loop through
      // the list of flags and determine if any fields are being rendered.
      _beingRenderedAnywhere = false;
      for (size_t i = 0; i < _nFields; ++i) {
	if (_beingRendered[i]) {
	  _beingRenderedAnywhere = true;
	  break;
	}
      } /* endfor - i */
    }
    
  }
  

  /**
   * @brief Set the flags indicating that the beam is being rendered for all
   of the fields.
   *
   * @param[in] being_rendered    The value of the flag.
   */

  inline void setBeingRendered(bool being_rendered)
  {
    for (size_t i = 0; i < _nFields; ++i) {
      _beingRendered[i] = being_rendered;
    }
    _beingRenderedAnywhere = being_rendered;
  }
  

  ///////////////////////////////////////////////
  /// \name Memory management:
  /// This class uses the notion of clients to decide when it should be deleted.w
  /// If removeClient() returns 0, the object should be deleted.
  //@{

  /// add a client - i.e. an object using this ray
  /// returns the number of clients using the ray
  
  int addClient() const; 
  
  /// client object no longer needs this ray
  /// returns the number of clients using the ray
  
  int removeClient() const;
  
  // set number of clients to zero

  int removeAllClients() const;

  /// delete this ray if no longer used by any client

  static void deleteIfUnused(const Beam *beam);

  void addFields(const RadxRay *ray, size_t n_fields, size_t nFields_expected);

  size_t getNFields() {return _nFields;};  
  //@}



protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  const Params &_params;

  // relevant ray
  // we use reference counting to determine when to delet the object
  
  const RadxRay *_ray;
  size_t _nGates;

  /**
   * @brief Number of fields in this beam.
   */

  size_t _nFields;

  /**
   * @brief Flags indicating that this beam is currently in a render list so
   *        shouldn't be deleted.  There is a separate flag for each field in
   *        the beam since the fields are rendered independently.
   */

  vector<bool> _beingRendered;
  
  /**
   * @brief Flag indicating whether any field of this beam is currently being
   *        rendered.  This flag will be true if any flag in _beingRendered is
   *        true.  This is used to reduce the amount of time spent looping
   *        through the _beingRendered array.
   */

  bool _beingRenderedAnywhere;
  
  /**
   * @brief The brush to use for each gate for each field.  The brush
   *        includes the color to use.
   */

  std::vector< std::vector< const QBrush* > > _brushes;

  // keeping track of reference counting clients using this object

  mutable int _nClients;
  mutable pthread_mutex_t _nClientsMutex;

};

#endif
