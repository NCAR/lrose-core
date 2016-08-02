#include "CP2Net.h"
#include <iostream>

using namespace CP2Net;

CP2Packet::CP2Packet():
_dataSize(0)
{

}

//////////////////////////////////////////////////////////////////////

CP2Packet::~CP2Packet() {

}

//////////////////////////////////////////////////////////////////////
void 
CP2Packet::addPulse(CP2PulseHeader* header,   
						int numDataValues,			
						float* data)
{
	// resize _packetData if it is not big enough
	int addDataSize = sizeof(CP2PulseHeader)+ sizeof(float)*numDataValues;
	int currentSize = _packetData.size();
	if (currentSize < (_dataSize + addDataSize)) {
		_packetData.resize(_packetData.size()+addDataSize);
	}

	// save the offset to the start of this pulse
	_pulseOffset.push_back(_dataSize);

	// append the new data to it.
	memcpy(&_packetData[_dataSize], header, sizeof(CP2PulseHeader));
	_dataSize += sizeof(CP2PulseHeader);
	memcpy(&_packetData[_dataSize], data, sizeof(float)*numDataValues);
	_dataSize += sizeof(float)*numDataValues;
}

void 
CP2Packet::addProduct(CP2ProductHeader& header,   
						int numDataValues,			
						double* data)
{
	// resize _packetData if it is not big enough
	int addDataSize = sizeof(header)+ sizeof(double)*numDataValues;
	int currentSize = _packetData.size();
	if (currentSize < (_dataSize + addDataSize)) {
		_packetData.resize(_packetData.size()+addDataSize);
	}

	// save the offset to the start of this pulse
	_pulseOffset.push_back(_dataSize);

	// append the new data to it.
	memcpy(&_packetData[_dataSize], &header, sizeof(header));
	_dataSize += sizeof(header);
	memcpy(&_packetData[_dataSize], data, sizeof(double)*numDataValues);
	_dataSize += sizeof(double)*numDataValues;
}

//////////////////////////////////////////////////////////////////////

void 
CP2Packet::clear()
{
	_dataSize = 0;
	_pulseOffset.clear();
}
//////////////////////////////////////////////////////////////////////

int 
CP2Packet::numPulses()
{
	return _pulseOffset.size();
}
//////////////////////////////////////////////////////////////////////

int 
CP2Packet::numProducts()
{
	return _pulseOffset.size();
}
//////////////////////////////////////////////////////////////////////

int 
CP2Packet::packetSize()
{
	return _dataSize;
}

//////////////////////////////////////////////////////////////////////

void*
CP2Packet::packetData()
{
	return &_packetData[0];
}

//////////////////////////////////////////////////////////////////////

CP2Pulse* 
CP2Packet::getPulse(int i)
{
	if (i >= _pulseOffset.size())
		return 0;

	// space into the pulse
	return (CP2Pulse*)(((char*)&_packetData[0])+_pulseOffset[i]);
}
//////////////////////////////////////////////////////////////////////

CP2Product* 
CP2Packet::getProduct(int i)
{
	if (i >= _pulseOffset.size())
		return 0;

	// space into the pulse
	return (CP2Product*)(((char*)&_packetData[0])+_pulseOffset[i]);
}
//////////////////////////////////////////////////////////////////////

bool 
CP2Packet::setPulseData(
		int size,					///< Size in bytes of the data packet
		void* data					///< The data packet
		)
{
	// clear the current accounting
	clear();

	// resize the data packet to match the incoming
	// packet. We will assume there that if the new size
	// is the same as the existing size, the resize() 
	// does not do any work.
	if (_packetData.size() != size)
		_packetData.resize(size);

	// copy the packet in.
	memcpy(&_packetData[0], data, size);

	// figure out the offsets.
	while(1) {
		_pulseOffset.push_back(_dataSize);
		CP2PulseHeader* pHeader = (CP2PulseHeader*)(((char*)&_packetData[0])+_dataSize);
		int gates = pHeader->gates;
		_dataSize += sizeof(CP2PulseHeader) + 2 * gates * sizeof(float);
		if (_dataSize >= size)
			break;
	}

	// if the computed data size does not match the length of
	// the packet, there has been an error
	if (_dataSize != size) {
		clear();
		return true;
	}

	// looks like it worked out.
	return false;
}
//////////////////////////////////////////////////////////////////////

bool 
CP2Packet::setProductData(
		int size,					///< Size in bytes of the data packet
		void* data					///< The data packet
		)
{
	// clear the current accounting
	clear();

	// resize the data packet to match the incoming
	// packet. We will assume there that if the new size
	// is the same as the existing size, the resize() 
	// does not do any work.
	if (_packetData.size() != size)
		_packetData.resize(size);

	// copy the packet in.
	memcpy(&_packetData[0], data, size);

	// figure out the offsets.
	while(1) {
		_pulseOffset.push_back(_dataSize);
		CP2ProductHeader* pHeader = (CP2ProductHeader*)(((char*)&_packetData[0])+_dataSize);
		int gates = pHeader->gates;
		_dataSize += sizeof(CP2ProductHeader) + gates * sizeof(double);
		if (_dataSize >= size)
			break;
	}

	// if the computed data size does not match the length of
	// the packet, there has been an error
	if (_dataSize != size) {
		clear();
		return true;
	}

	// looks like it worked out.
	return false;
}
//////////////////////////////////////////////////////////////////////
CP2FullPulse::CP2FullPulse(CP2Pulse* pPulse) 
{
	_cp2Pulse = (CP2Pulse*) new char[sizeof (CP2PulseHeader) + pPulse->header.gates*2*sizeof(float)];

	_cp2Pulse->header = pPulse->header;	
	memcpy(&_cp2Pulse->data, &pPulse->data, pPulse->header.gates*2*sizeof(float));
}

//////////////////////////////////////////////////////////////////////
CP2FullPulse::~CP2FullPulse()
{
	delete [] _cp2Pulse;
}
//////////////////////////////////////////////////////////////////////
CP2PulseHeader*
CP2FullPulse::header()
{
	return &_cp2Pulse->header;
}
//////////////////////////////////////////////////////////////////////
float*
CP2FullPulse::data()
{
	return &_cp2Pulse->data[0];
}
//////////////////////////////////////////////////////////////////////

CP2PulseCollator::CP2PulseCollator(int maxQueueSize):
_maxQueueSize(maxQueueSize),
_discards(0)
{
}

////////////////////////////////////////////////////
CP2PulseCollator::~CP2PulseCollator()
{
}

////////////////////////////////////////////////////
void 
CP2PulseCollator::addPulse(CP2FullPulse* pPulse, int queueNumber)
{
	long long pulseNum = pPulse->header()->pulse_num;

	switch (queueNumber) {
	case 0:
		if (_queue0.find(pulseNum) == _queue0.end()) {
			_queue0[pulseNum] = pPulse; 
		} else {
			// insertion failed, key already present. Get rid of the pulse
			/// @todo we should notify the caller that the insertion failed
			delete pPulse;
		} 
		while (_queue0.size() >= _maxQueueSize) {
			delete _queue0.begin()->second;
			_queue0.erase(_queue0.begin());
			_discards++;
		}
		break;
	case 1:
		if (_queue1.find(pulseNum) == _queue1.end()) {
			_queue1[pulseNum] = pPulse; 
		} else {
			// insertion failed, key already present. Get rid of the pulse
			/// @todo we should notify the caller that the insertion failed
			delete pPulse;
		} 
		while (_queue1.size() >= _maxQueueSize) {
			delete _queue1.begin()->second;
			_queue1.erase(_queue1.begin());
			_discards++;
		}
		break;
	default:
		printf("CP2PulseCollator called with a queue index of %d\n", queueNumber);
		delete pPulse;
	} 

}


////////////////////////////////////////////////////
bool 
CP2PulseCollator::gotMatch(CP2FullPulse** pulse0, CP2FullPulse** pulse1)
{
	if ((_queue0.size() > 0) && (_queue1.size() > 0)) {
		PulseMap::iterator i0;
		PulseMap::iterator i1;
		for (i0 = _queue0.begin(); i0 != _queue0.end(); i0++) 
		{
			i1 = _queue1.find(i0->first);
			if (i1 != _queue1.end()) {
				break;
			}
		}
		if (i0 == _queue0.end()) {
			// no match
			return false;
		}
		// i0 and i1 point to matching time tags.
		// remove all queue items before them
		PulseMap::iterator ii;
		for (ii = _queue0.begin(); ii != i0; ii++) {
			delete ii->second;
		}
		ii = i0--;
		_queue0.erase(_queue0.begin(), ii);

		for (ii = _queue1.begin(); ii != i1; ii++) {
			delete ii->second;
		}
		ii = i1--;
		_queue1.erase(_queue1.begin(), ii);

		// now i0 and i1 should point to the begining of their
		// respective queues, and there should be matching
		// timetags. Return these items, and remove from the queues
		long long t0 = _queue0.begin()->first;
		long long t1 = _queue1.begin()->first;
		long long tdiff = t0 - t1;
		if (tdiff)
			printf("timetags not matching in CP2PulseCollator\n");
		*pulse0 = _queue0.begin()->second;
		*pulse1 = _queue1.begin()->second;
		_queue0.erase(_queue0.begin());
		_queue1.erase(_queue1.begin());
		return true;
	}

	return false;

}
////////////////////////////////////////////////////
int 
CP2PulseCollator::discards()
{
	return _discards;
}
////////////////////////////////////////////////////
////////////////////////////////////////////////////


