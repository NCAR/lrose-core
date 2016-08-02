#include "CP2PulseBiQuad.h"

CP2PulseBiQuad::CP2PulseBiQuad(
							   int gates,           
							   float a11,           
							   float a12,           
							   float b10,           
							   float b11,           
							   float b12,            
							   float a21,           
							   float a22,           
							   float b20,           
							   float b21,           
							   float b22            
							   ):
_a11(a11),
_a12(a12),
_b10(b10),
_b11(b11),
_b12(b12),
_a21(a21),
_a22(a22),
_b20(b20),
_b21(b21),
_b22(b22)
{
	for (unsigned int i = 0; i < (unsigned int)2*gates; i++) {
		_filters1.push_back(new BiQuad);
		_filters1[i]->setA1(_a11);
		_filters1[i]->setA2(_a12);
		_filters1[i]->setB0(_b10);
		_filters1[i]->setB1(_b11);
		_filters1[i]->setB2(_b12);
		_filters2.push_back(new BiQuad);
		_filters2[i]->setA1(_a21);
		_filters2[i]->setA2(_a22);
		_filters2[i]->setB0(_b20);
		_filters2[i]->setB1(_b21);
		_filters2[i]->setB2(_b22);
	}
}

/////////////////////////////////////////////////////////////////////

CP2PulseBiQuad::~CP2PulseBiQuad()
{
	for (unsigned int i = 0; i < _filters1.size(); i++) {
		delete _filters1[i];
		delete _filters2[i];
	}
}

/////////////////////////////////////////////////////////////////////

void CP2PulseBiQuad::tick(CP2Pulse& pulse)
{
	for (int i = 0; i < pulse.header.gates*2; i++) {
		StkFloat f1 = _filters1[i]->tick(pulse.data[(unsigned int)i]);
		pulse.data[(unsigned int)i] = _filters2[i]->tick(f1);
	}
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

