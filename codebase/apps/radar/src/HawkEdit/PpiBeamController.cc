
#include "toolsa/LogStream.hh"
#include "PpiBeamController.hh"
#include "DisplayFieldController.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"


PpiBeamController::PpiBeamController()
{
  LOG(DEBUG) << "enter";
  // _ppiBeams;
  LOG(DEBUG) << "exit";
}

PpiBeamController::~PpiBeamController() {
  clear();
}

void PpiBeamController::clear() {
  // delete all of the dynamically created beams                                           
  LOG(DEBUG) << "enter";

  for (size_t i = 0; i < _ppiBeams.size(); ++i) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  _ppiBeams.clear();
  LOG(DEBUG) << "exit";

}

void PpiBeamController::addBeam(const RadxRay *ray,
			   const float start_angle,
			   const float stop_angle,
			   const std::vector< std::vector< double > > &beam_data,
                          DisplayFieldController *displayFieldController)
{
  // TODO: send the controller or the model?
  _beams->addBeam(ray, start_angle, stop_angle, beam_data, displayFieldController);

}




