#include "SimAngles.h"

SimAngles::SimAngles(SIMANGLESMODE mode,   
					 int pulsesPerBeam,			
					 double beamWidth,	
					 double rhiAzAngle,
					 double ppiElIncrement,
					 double elMinAngle,		    
					 double elMaxAngle,			
					 double sweepIncrement,		
					 int numPulsesPerTransition):
_mode(mode),
_pulsesPerBeam(pulsesPerBeam),
_rhiAzAngle(rhiAzAngle),
_ppiElIncrement(ppiElIncrement),
_numPulsesPerTransition(numPulsesPerTransition),
_minEl(elMinAngle),
_maxEl(elMaxAngle),
_sweepIncrement(sweepIncrement)
{
	_volume = 1;
	_sweep = 1;
	_nTranPulses = 0;
	_transition = 0;

	// initialize elevation
	_el = elMinAngle;

	// initialize azimuth
	switch (_mode) {
		case PPI:
			_az = 0;
			break;
		case RHI:
			_az = rhiAzAngle;
	}

	// compute the angle increment
	_angleInc = beamWidth / pulsesPerBeam;

}
////////////////////////////////////////////////////////////////////////////
SimAngles::~SimAngles()
{
}
////////////////////////////////////////////////////////////////////////////
void
SimAngles::nextAngle(double &az, 
					 double &el, 
					 int &transition, 
					 int& sweep,
					 int& volume)
{
	switch (_mode) {

		// PPI mode
		case PPI:
			if (_az + _angleInc < 360.0) {
				_az += _angleInc;
				_transition = 0;
				break;
			}


			// send out ingremental elevation angles as we move
			// up in elevation to the next PPI sweep, or incremental 
			// elevation angles down until we retunr to the bottom.
			if (!_transition) {
				_sweep++;
				if (_el + _ppiElIncrement < _maxEl) {
					deltaEl = _ppiElIncrement/_numPulsesPerTransition;
				} else {
					deltaEl = -(_maxEl - _minEl)/_numPulsesPerTransition;
				}
				_nTranPulses = 0;
			}
			_transition = 1;
			_el += deltaEl;			
			if (deltaEl < 0) {
				// descending transition; continue until reaching the
				// bottom elevation
				if (_el < _minEl) {
					// we ave reached the bottom
					_el = _minEl;
					_transition = 0;
					_volume++;
					_az = 0.0;
				} 
			} else {
				// ascending transition, see if we have gotten to the next level
				if (_nTranPulses++ >= _numPulsesPerTransition) {
					// have reached the next elevation
					_nTranPulses = 0;
					_transition = 0;
					_az = 0;
				}
			}
			break;

			// RHI mode
		case RHI:
			_az = _rhiAzAngle;
			if (_el + _angleInc > _maxEl && !_transition) 
			{
				// Start a descending RHI transition
				_transition = 1;
				_sweep++;
			}
			if (!_transition) {
				_el += _angleInc;
			} else {
				double deltaEl = (_maxEl - _minEl)/_numPulsesPerTransition;
				_el -= deltaEl;
				if (_el < _minEl) {
					_el = _minEl;
					_transition = 0;
					_volume++;
				}
			}
			break;
	}

	// set the values
	el = _el;
	az = _az;
	volume = _volume;
	transition = _transition;
	sweep = _sweep;
}
////////////////////////////////////////////////////////////////////////////

