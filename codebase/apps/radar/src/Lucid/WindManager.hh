// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
// WindManager
//
// Coordinates the independently asynchronous U and V readers which together
// form one renderable wind layer.
//
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef WIND_MANAGER_HH
#define WIND_MANAGER_HH

#include <string>
#include <vector>

#include <toolsa/DateTime.hh>

#include "GlobalData.hh"

class WindManager
{
public:

  WindManager();

  void setEnabled(bool enabled);
  bool isEnabled() const { return _enabled; }

  void setLayerEnabled(int index, bool enabled);
  bool layerEnabled(int index) const;

  // A U/V pair is associated with one request serial. Results from an older
  // serial may finish, but cannot make the current layer ready for rendering.
  int requestData(const DateTime &midTime, double vLevel, int page);
  void invalidate();

  // Consume reader completion flags and update paired readiness. The return
  // value indicates that the cached wind overlay should be regenerated.
  bool poll();

  bool activeWindsReady() const;
  bool layerReady(int index) const;
  int layerCount() const;
  int requestSerial() const { return _requestSerial; }
  const std::string &layerError(int index) const;

private:

  struct LayerState {
    int requestSerial = -1;
    int expectedUGeneration = -1;
    int expectedVGeneration = -1;
    bool requested = false;
    bool ready = false;
    std::string error;
  };

  Params &_params;
  GlobalData &_gd;
  bool _enabled;
  int _requestSerial;
  std::vector<LayerState> _states;
  DateTime _requestedTime;
  double _requestedVLevel;
  int _requestedPage;

  bool _validatePair(int index, std::string &error) const;
  void _startPendingRequests(int index);
  static bool _nearlyEqual(double lhs, double rhs);
};

#endif
