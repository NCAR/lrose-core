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
#ifndef CP2_NET_HH
#define CP2_NET_HH

#include <map>
#include <vector>

namespace CP2Net {

  /// A header for each pulse of data
  typedef struct CP2PulseHeader {
    long long pulse_num;	///< Pulse number
    double az;				///< The azimuth, from the antenna controller.
    double el;				///< The elevation, from the antenna controller.
    int scanType;			///< The scan type, but we don't know what the codes are.
    int sweepNum;			///< The sweep number, from the antenna controller.
    int volNum;				///< The volume number, from the antenna controller.
    int antSize;			///< I have no idea what this is.
    int antTrans;			///< This is probably the flag indicating the antenna is in transition.
    int  channel;			///< The Piraq number.
    int  gates;				///< The number of gates, set by the host.
    int status;             ///< Status that comes from the piraq for each pulse
    bool horiz;			    ///< set true for horizontal polarization, false for vertical
    double prt;				///< The prt, in seconds
    double xmit_pw;			///< The transmited pulse width, in seconds.
  } CP2PulseHeader;

  ///Bit mask defines for the status field of CP2PulseHeader
#define PIRAQ_FIFO_EOF 1    ///< If the Piraq reported an EOF error.

  /// A header and data are combined to make one pulse.
  typedef struct CP2Pulse {
    CP2PulseHeader header;  ///< The pulse header.
    float data[2];			///< An array of data values will start here.
  } CP2Pulse;

  /// Use this class when the full pulse, with
  /// a filled data array, needs to be preserved. 
  /// Usually a pointer to CP2Pulse just points into
  /// a CP2Packet which is going to be deleted, making
  /// the CP2Pulse* invalid
  class CP2FullPulse {
  public:
    CP2FullPulse(CP2Pulse* pPulse);
    ~CP2FullPulse();
    float* data();
    CP2PulseHeader* header();
  protected:
    CP2Pulse* _cp2Pulse;
  };

  /// Identifiers for the product types.
  enum PRODUCT_TYPES {
    PROD_S_DBMHC,	///< S-band dBm horizontal co-planar
    PROD_S_DBMVC,	///< S-band dBm vertical co-planar
    PROD_S_DBZ,	    ///< S-band dBz 
    PROD_S_SNR,		///< S-band SNR
    PROD_S_VEL,		///< S-band velocity
    PROD_S_WIDTH,	///< S-band spectral width
    PROD_S_RHOHV,	///< S-band rhohv
    PROD_S_PHIDP,	///< S-band phidp
    PROD_S_ZDR,		///< S-band zdr
    PROD_X_DBMHC,	///< X-band dBm horizontal co-planar
    PROD_X_DBMVX,	///< X-band dBm vertical cross-planar
    PROD_X_DBZ ,	///< X-band dBz
    PROD_X_SNR,		///< X-band SNR
    PROD_X_VEL,		///< X-band velocity
    PROD_X_WIDTH,	///< X-band spectral width
    PROD_X_LDR		///< X-band LDR
  };

  /// A header for each beam product
  typedef struct CP2ProductHeader {
    PRODUCT_TYPES prodType;	///< The product identifier
    int  gates;				///< The number of gates, set by the host.
    long long beamNum;		///< The beam number.
    double az;				///< The azimuth
    double el;				///< The elevation
    double gateWidthKm;		///< The width of each gate, in km
  } CP2ProductHeader;

  /// A header and data are combined to make one product.
  typedef struct CP2Product {
    CP2ProductHeader header;///< The pulse header.
    double data[1];			///< An array of data values will start here.
  } CP2Product;

  /// Interface for working with CP2 network data
  /// transmission. When constructed, an area for
  /// building a network packet is allocated.
  /// A network packet will contain either CP2Pulse
  /// or CP2Product structures.
  class CP2Packet{
  public:
    CP2Packet();
    virtual ~CP2Packet();
    /// Add a pulse to the packet. This is used for constructing
    /// a packet of pulses
    void addPulse(
		  CP2PulseHeader* header,       ///< The pulse header information
		  int numDataValues,			///< The number of data values.
		  float* data					///< The data for the beam
		  );
    /// Add a product to the packet. This is used for constructing
    /// a packet of products
    void addProduct(
		    CP2ProductHeader& header,       ///< The pulse header information
		    int numDataValues,			///< The number of data values.
		    double* data					///< The data for the beam
		    );
    /// Construct a pulse packet. Used when building a packet from a datagram.
    /// @return False if the data structure was self consistent, true if an
    /// error was detected.
    bool setPulseData(
		      int size,					///< Size in bytes of the data
		      void* data					///< The data packet
		      );	
    /// Construct a product packet. Used when building a packet from a datagram.
    /// @return False if the data structure was self consistent, true if an
    /// error was detected.
    bool setProductData(
			int size,					///< Size in bytes of the data
			void* data					///< The data packet
			);	
    /// Empty the packet of data.
    void clear();
    /// @return The size (in bytes) of the CP2Packet.
    int packetSize();
    /// @return A pointer to the begining of the CP2Beam array.
    void* packetData();
    /// @return The number of pulses in the packet
    int numPulses();
    /// @return The number of produ in the packet
    int numProducts();
    /// Fetch a pulse
    /// @param i The pulse index. It must be less than 
    /// the value returned by numPulses(). 
    /// @return A pointer to beam i. If the index
    /// is illegal, null is returned.
    CP2Pulse* getPulse(int index);
    /// Fetch a product
    /// @param i The beam index. It must be less than 
    /// the value returned by numBeams(). 
    /// @return A pointer to beam i. If the index
    /// is illegal, null is returned.
    CP2Product* getProduct(int index);

  protected:
    /// The vector that will be expanded with beam data when 
    /// addBeam is called. _paketData is grown as beams are
    /// added to it. It won't be shrunk, so that space allocations
    /// are only done as necessary to met the maximum requested
    /// size.
    std::vector<unsigned char> _packetData;
    /// The amount of data currently in the _packet data
    int _dataSize;
    /// The offset to each pulse in the packet
    std::vector<int> _pulseOffset;
  };

#define PulseMap std::map<long long, CP2FullPulse*>
  /// A collator, which collects pulses on two separate queus, and then returns 
  /// pairs that
  /// have identical pulse numbers. It is used to pair up Xh and Xv
  /// pulses, which have been sent in different broadcast datagrams.
  /// 
  /// Pulses are fed into the colator; when matches occur, the 
  /// matching pulses can be retrieved from the collator. Note that
  /// CP2PulseCollator is holding only pointers to CP2FullPulse. However, it
  /// will delete pulses when its internal queue eceeds the queue size limit.
  ///
  /// Matching pulses are popped from the queue and it is up to the
  /// caller to delete them.
  class CP2PulseCollator {
  public:
    CP2PulseCollator(int maxQueueSize ///< The maximum queu size.
		     );
    virtual ~CP2PulseCollator();
    /// Add a pulse to a collator queue.
    void addPulse(CP2FullPulse* pPulse, ///< The pulse to be added to a queue
		  int queueNumber ///< The queue that the pulse will be placed on (0 or 1)
		  );
    /// Test for a match. 
    /// @returns true if matching pulses are available. If true, they are
    /// removed from the queue, and returned through pulse0 and pulse1.
    bool gotMatch(CP2FullPulse** pulse0, ///< The pulse from queue0
		  CP2FullPulse** pulse1            ///< The pulse from queue1
		  );
    /// @returns The number of pulses that have been discarded from the
    /// collator. If this is non-zero, then there is a serious problem 
    /// with one of your data streams, or the queue is not long enough to 
    /// cope with the burstiness of the data streams.
    int discards();

  protected:
    /// The maximum queue size
    int _maxQueueSize;
    /// The cumulative number of discards.
    int _discards;
    /// Queue number 0.
    PulseMap _queue0;
    /// Queue number 1.
    PulseMap _queue1;
  };
};
#endif
