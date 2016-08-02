#ifndef SIMANGLESINC_
#define SIMANGLESINC_

/// Manage the creation of simulated angles. The class is
/// initialized with the simulation parameters. After that,
/// each call to nextAngles() returns the next simulated angle.
class SimAngles{
	public:
		/// Scan types
		enum SIMANGLESMODE {
			PPI, ///< simulate a PPI scan
			RHI	 ///< simulate an RHI scan
		};

		/// Constructor
		SimAngles(SIMANGLESMODE mode,   ///< The simulation mode
			int pulsesPerBeam,			///< The number of pulses per beam, used to compute the per pulse angle increment.
			double beamWidthdegrees,	///< The width of a beam in degrees.
			double rhiAzAngle,			///< The azimuth af an RHI simulation
			double ppiElincrement,		///< The elevation increment for a PPI simulation
			double elMinAngle,		    ///< The beginning elevation angle
			double elMaxAngle,			///< THe ending elevation angle
			double sweepIncrement,		///< The angle increment of the opposite mode during scan transitions
			int numPulsesPerTransition ///< The number of pulses in a transition, either between PPI sweeps, or when an RHI is descending
			);
		/// Destructor
		virtual ~SimAngles();
		/// Compute the next angle information
		void nextAngle(
			double& az, ///< Next azimuth
			double& el,      ///< Next elevation
			int& transition, ///< Next transition flag
			int& sweep,      ///< Next sweep
			int& volume      ///< Next volume
			);
protected:
		/// The current azimuth
		double _az;
		/// The current elevation
		double _el;
		/// the current volume
		int volume;
		/// The angle simulation mode
		SIMANGLESMODE _mode;
		/// The elevation increment between PPI sweeps
		double _ppiElIncrement;
		/// The azimuth of an RHI 
		double _rhiAzAngle;
		/// The amount to increment each angle
		double _angleInc;
		/// The minimum elevation
		double _minEl;
		/// The maximum elevation
		double _maxEl;
		/// The number of pulses in a transition
		int _numPulsesPerTransition;
		/// The transition pulse counter
		int _nTranPulses;
		/// set true if we are in a transition
		short _transition;
		/// The number of pulses in one beam. 
		int _pulsesPerBeam;
		/// The current sweep number
		short _sweep;
		/// the current volume number
		short _volume;
		/// elevation increment during transitions
		double deltaEl;
		///
		double _sweepIncrement;
};

#endif
