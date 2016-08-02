#ifndef CP2VERSIONINC_
#define CP2VERSIONINC_

#include <string>

namespace CP2Lib {
	/// Provide a version number for the CP2 software

	struct CP2Version {
	public:
		static std::string revision();
	};
};

#endif
