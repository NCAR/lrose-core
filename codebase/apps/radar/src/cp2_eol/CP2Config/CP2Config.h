#ifndef CP2CONFIGINC_
#define CP2CONFIGINC_

#include <QSettings>
#include <string>
#include <vector>

/// Provide an interface to configuration
// management. Configuration items are specified
/// as key:value pairs.
/// Calls which fetch a configuration value 
/// also specify a default value for that key.
/// The configuration is written with each fetch,
/// so that a configuration will contain the default values
/// if they haven't been set.
/// Configurations are always synced when a value is written.
class CP2Config {
public:
	CP2Config(const std::string organization, const std::string application);
	virtual ~CP2Config();

	void sync();

	void setString(std::string key, std::string t);
	std::string getString(std::string key, std::string defaultValue);

	void setDouble(std::string key, double d);
	double getDouble(std::string key, double defaultValue);

	void setFloat(std::string key, float d);
	float getFloat(std::string key, float defaultValue);

	void setInt(std::string key, int i);
	int getInt(std::string key, int defaultValue);

	void setBool(std::string key, bool b);
	bool getBool(std::string key, bool defaultValue);

	void setArray(std::string key, 
		std::string subKey,
		std::vector<std::vector<int> > values);

	std::vector<std::vector<int> > getArray(std::string key, 
		std::string subKey,
		std::vector<std::vector<int> > defaultValues);

	std::vector<std::string>
		childGroups(std::string topGroup);

protected:
	/// The configuration permanent store
	QSettings _settings;

};


#endif
