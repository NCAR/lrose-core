#ifndef PPIBEAMCONTROLLER_H
#define PPIBEAMCONTROLLER_H

#include "PpiBeam.hh"

class PpiBeamController
{

public:

  PpiBeamController();
  virtual ~PpiBeamController();

  void add(PpiBeam *);
  

private:

  // pointers to active beams                                                                                    

  std::vector<PpiBeam*> _ppiBeams;
 
  //  vector<PpiBeam *> _beams;
  //  vector<PpiBeam *> _working;
 
  

};

#endif
