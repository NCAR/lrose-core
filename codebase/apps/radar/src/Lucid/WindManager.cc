// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
// WindManager
//
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include "WindManager.hh"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "MdvReader.hh"

using namespace std;

WindManager::WindManager() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _enabled(_params.winds_on_at_startup),
        _requestSerial(0),
        _states(_gd.layers.wind.size()),
        _requestedVLevel(0.0),
        _requestedPage(0)
{
  _gd.layers.wind_vectors = _enabled;
}

void WindManager::setEnabled(bool enabled)
{
  if (_enabled == enabled) {
    return;
  }
  _enabled = enabled;
  _gd.layers.wind_vectors = enabled;

  // A later enable is treated as a new display request. Do not expose a pair
  // retained from an earlier time or height during the intervening disabled
  // period.
  for (LayerState &state : _states) {
    state.ready = false;
    state.requested = false;
  }
}

void WindManager::setLayerEnabled(int index, bool enabled)
{
  if (index < 0 || index >= static_cast<int>(_gd.layers.wind.size())) {
    return;
  }
  _gd.layers.wind[index].active = enabled;
  LayerState &state = _states[index];
  state.ready = false;
  state.requested = false;
  state.error.clear();
}

bool WindManager::layerEnabled(int index) const
{
  return index >= 0 && index < static_cast<int>(_gd.layers.wind.size()) &&
    _gd.layers.wind[index].active;
}

int WindManager::requestData(const DateTime &midTime,
                             double vLevel,
                             int page)
{
  ++_requestSerial;
  _requestedTime = midTime;
  _requestedVLevel = vLevel;
  _requestedPage = page;
  for (int ii = 0; ii < static_cast<int>(_gd.layers.wind.size()); ++ii) {
    LayerState &state = _states[ii];
    state = LayerState();
    state.requestSerial = _requestSerial;

    wind_data_t &wind = _gd.layers.wind[ii];
    if (!_enabled || !wind.active || wind.wind_u == nullptr ||
        wind.wind_v == nullptr) {
      continue;
    }

    state.requested = true;
    _startPendingRequests(ii);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Wind request: layer=" << wind.wind_u->legend_name
           << " time=" << midTime.asString(0)
           << " level=" << vLevel
           << " U=" << wind.wind_u->field_label
           << " V=" << wind.wind_v->field_label << endl;
    }
  }
  return _requestSerial;
}

void WindManager::invalidate()
{
  ++_requestSerial;
  for (int ii = 0; ii < static_cast<int>(_gd.layers.wind.size()); ++ii) {
    LayerState &state = _states[ii];
    state = LayerState();
    wind_data_t &wind = _gd.layers.wind[ii];
    if (wind.wind_u) {
      wind.wind_u->invalidateTimeList();
    }
    if (wind.wind_v) {
      wind.wind_v->invalidateTimeList();
    }
  }
}

bool WindManager::poll()
{
  bool overlayChanged = false;

  for (int ii = 0; ii < static_cast<int>(_gd.layers.wind.size()); ++ii) {
    LayerState &state = _states[ii];
    wind_data_t &wind = _gd.layers.wind[ii];
    if (!state.requested || !_enabled || !wind.active ||
        wind.wind_u == nullptr || wind.wind_v == nullptr) {
      continue;
    }

    // A blocking MDV call cannot safely be interrupted. If display state
    // changed while a component was busy, wait for that worker to retire and
    // then start the latest stored request instead of losing the update.
    _startPendingRequests(ii);

    const bool uCompleted = wind.wind_u->isNewH();
    const bool vCompleted = wind.wind_v->isNewH();
    if (!uCompleted && !vCompleted) {
      continue;
    }

    const bool generationsComplete =
      state.expectedUGeneration >= 0 && state.expectedVGeneration >= 0 &&
      wind.wind_u->getHorizCompletedGeneration() ==
        state.expectedUGeneration &&
      wind.wind_v->getHorizCompletedGeneration() ==
        state.expectedVGeneration;

    bool ready = false;
    string error;
    if (generationsComplete) {
      ready = _validatePair(ii, error);
    }

    if (ready != state.ready || error != state.error) {
      overlayChanged = true;
    }
    state.ready = ready;
    state.error = error;

    if (ready && _params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Wind layer ready: " << wind.wind_u->legend_name
           << " dataTime=" << wind.wind_u->h_date.asString(0)
           << " grid=" << wind.wind_u->h_fhdr.nx << "x"
           << wind.wind_u->h_fhdr.ny << endl;
    }

    if (!error.empty() && _params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Wind layer " << ii << " is not renderable: "
           << error << endl;
    }
  }

  return overlayChanged;
}

void WindManager::_startPendingRequests(int index)
{
  LayerState &state = _states[index];
  wind_data_t &wind = _gd.layers.wind[index];

  // Each generation is recorded only when its worker is actually started.
  // This lets the other component finish independently without ever pairing
  // it with data from a superseded display request.
  if (state.expectedUGeneration < 0 && !wind.wind_u->getReadBusyH()) {
    state.expectedUGeneration = wind.wind_u->requestHorizPlane(
      _requestedTime, _requestedVLevel, _requestedPage);
  }
  if (state.expectedVGeneration < 0 && !wind.wind_v->getReadBusyH()) {
    state.expectedVGeneration = wind.wind_v->requestHorizPlane(
      _requestedTime, _requestedVLevel, _requestedPage);
  }
}

bool WindManager::activeWindsReady() const
{
  if (!_enabled) {
    return true;
  }
  for (int ii = 0; ii < static_cast<int>(_gd.layers.wind.size()); ++ii) {
    if (_gd.layers.wind[ii].active && !_states[ii].ready) {
      return false;
    }
  }
  return true;
}

bool WindManager::layerReady(int index) const
{
  return _enabled && index >= 0 && index < static_cast<int>(_states.size()) &&
    _gd.layers.wind[index].active && _states[index].ready;
}

int WindManager::layerCount() const
{
  return static_cast<int>(_states.size());
}

const string &WindManager::layerError(int index) const
{
  static const string empty;
  if (index < 0 || index >= static_cast<int>(_states.size())) {
    return empty;
  }
  return _states[index].error;
}

bool WindManager::_validatePair(int index, string &error) const
{
  const wind_data_t &wind = _gd.layers.wind[index];
  const MdvReader *u = wind.wind_u;
  const MdvReader *v = wind.wind_v;
  if (!u->isValidH() || !v->isValidH()) {
    error = "one or both component reads failed";
    return false;
  }

  const Mdvx::field_header_t &uh = u->h_fhdr;
  const Mdvx::field_header_t &vh = v->h_fhdr;
  if (uh.nx < 1 || uh.ny < 1 || uh.nx != vh.nx || uh.ny != vh.ny) {
    error = "U and V grid dimensions differ";
    return false;
  }
  const size_t expected = static_cast<size_t>(uh.nx) *
    static_cast<size_t>(uh.ny);
  if (u->h_fl32_data.size() != expected ||
      v->h_fl32_data.size() != expected) {
    error = "U or V data size does not match its grid";
    return false;
  }
  if (uh.proj_type != vh.proj_type ||
      u->proj.getPjgMath() != v->proj.getPjgMath() ||
      !_nearlyEqual(uh.grid_minx, vh.grid_minx) ||
      !_nearlyEqual(uh.grid_miny, vh.grid_miny) ||
      !_nearlyEqual(uh.grid_dx, vh.grid_dx) ||
      !_nearlyEqual(uh.grid_dy, vh.grid_dy) ||
      !_nearlyEqual(uh.proj_origin_lat, vh.proj_origin_lat) ||
      !_nearlyEqual(uh.proj_origin_lon, vh.proj_origin_lon)) {
    error = "U and V grid geometry differs";
    return false;
  }
  if (!_nearlyEqual(u->h_vhdr.level[0], v->h_vhdr.level[0])) {
    error = "U and V vertical levels differ";
    return false;
  }
  if (u->h_date.utime() != v->h_date.utime()) {
    ostringstream out;
    out << "U and V data times differ (" << u->h_date.utime()
        << " versus " << v->h_date.utime() << ")";
    error = out.str();
    return false;
  }
  return true;
}

bool WindManager::_nearlyEqual(double lhs, double rhs)
{
  const double scale = max({1.0, fabs(lhs), fabs(rhs)});
  return fabs(lhs - rhs) <= 1.0e-6 * scale;
}
