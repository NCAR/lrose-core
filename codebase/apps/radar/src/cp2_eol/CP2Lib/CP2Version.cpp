#include "CP2Version.h"
#include "svnInfo.h"

using namespace CP2Lib;

std::string
CP2Version::revision() {

	return std::string(SVNREVISION);

}
