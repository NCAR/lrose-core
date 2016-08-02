#ifndef CP2PULSEBIQUADINC_
#define CP2PULSEBIQUADINC_

#include "BiQuad.h"
#include "CP2Net.h"
using namespace CP2Net;
#include <vector>

/**
@brief A biquad IIR filter which will operate on CP2Pulses.

CP2Pulses are passed in, sent to the biqaud filter, and the
filtered data replace the incoming data. CP2PulseBiQuad is
running 2*ngates filters; i.e. one filter for each i and q
at each gate.
**/
class CP2PulseBiQuad: BiQuad {
public:
	CP2PulseBiQuad(int gates,///< The number of gates that the filter will operate on
		float a11,           ///< coeffcient filter1 a1
		float a12,           ///< coeffcient filter1 a2
		float b10,           ///< coeffcient filter1 b0
		float b11,           ///< coeffcient filter1 b1
		float b12,           ///< coeffcient filter1 b2
		float a21,           ///< coeffcient filter2 a1
		float a22,           ///< coeffcient filter2 a2
		float b20,           ///< coeffcient filter2 b0
		float b21,           ///< coeffcient filter2 b1
		float b22            ///< coeffcient filter2 b2
		);
	virtual ~CP2PulseBiQuad();
	void tick(CP2Pulse& pulse ///< A pulse to be processed
		);

protected:

	float _a11;           ///< filter1 coeffcient a1
	float _a12;           ///< filter1 coeffcient a2
	float _b10;           ///< filter1 coeffcient b0
	float _b11;           ///< filter1 coeffcient b1
	float _b12;           ///< filter1 coeffcient b2

	float _a21;           ///< filter2 coeffcient a1
	float _a22;           ///< filter2 coeffcient a2
	float _b20;           ///< filter2 coeffcient b0
	float _b21;           ///< filter2 coeffcient b1
	float _b22;           ///< filter2 coeffcient b2

	/// One first stage filter will be created for each i and q at each gate.
	std::vector<BiQuad*> _filters1;
	/// One second stage filter will be created for each i and q at each gate.
	std::vector<BiQuad*> _filters2;
};

#endif
