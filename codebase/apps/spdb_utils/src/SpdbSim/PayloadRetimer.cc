#include "PayloadRetimer.hh"

#include <Spdb/Product_defines.hh>
#include <Spdb/sounding.h>
#include <dataport/bigend.h>
#include <rapformats/GenPoly.hh>
#include <rapformats/GenPt.hh>
#include <rapformats/SigAirMet.hh>
#include <rapformats/Sndg.hh>
#include <rapformats/WxObs.hh>
#include <rapformats/ac_data.h>
#include <rapformats/ac_georef.hh>
#include <rapformats/ac_posn.h>
#include <rapformats/fos.h>
#include <rapformats/ltg.h>

#include <climits>
#include <cstddef>
#include <cstring>
#include <sstream>

using namespace std;

namespace {

bool shiftTime32(si32 &value, int64_t offset,
                 const char *name, string &error)
{
  // Legacy formats use zero to mean "time unavailable"; preserve that
  // sentinel rather than turning it into a plausible replay timestamp.
  if (value == 0) {
    return true;
  }
  const int64_t shifted = (int64_t) value + offset;
  if (shifted < 0 || shifted > INT32_MAX) {
    ostringstream out;
    out << name << " is outside the SPDB 32-bit time range after retiming";
    error = out.str();
    return false;
  }
  value = (si32) shifted;
  return true;
}

bool shiftTime64(si64 &value, int64_t offset,
                 const char *name, string &error)
{
  if (value == 0) {
    return true;
  }
  if ((offset > 0 && value > INT64_MAX - offset) ||
      (offset < 0 && value < INT64_MIN - offset)) {
    error = string(name) + " overflows after retiming";
    return false;
  }
  value += offset;
  return true;
}

bool shiftTimeValue(time_t &value, int64_t offset,
                    const char *name, string &error)
{
  if (value == 0) {
    return true;
  }
  const int64_t shifted = (int64_t) value + offset;
  if (shifted < 0 || shifted > INT32_MAX) {
    error = string(name) +
            " is outside the SPDB 32-bit time range after retiming";
    return false;
  }
  value = (time_t) shifted;
  return true;
}

void copyBuffer(const void *data, int len, vector<unsigned char> &output)
{
  const unsigned char *bytes = (const unsigned char *) data;
  output.assign(bytes, bytes + len);
}

int retimeGenPt(const void *input, int len, int64_t offset,
                vector<unsigned char> &output, string &error)
{
  GenPt point;
  if (point.disassemble(input, len)) {
    error = "cannot disassemble generic point payload";
    return -1;
  }
  time_t value = point.getTime();
  if (!shiftTimeValue(value, offset, "GenPt time", error)) {
    return -1;
  }
  point.setTime(value);
  if (point.assemble()) {
    error = "cannot assemble generic point payload";
    return -1;
  }
  copyBuffer(point.getBufPtr(), point.getBufLen(), output);
  return 0;
}

int retimeWxObs(const void *input, int len, int64_t offset,
                vector<unsigned char> &output, string &error)
{
  msg_id_t msgId = STATION_REPORT;
  const bool startsWithReport = WxObs::checkForReport(input, len, msgId);
  WxObs obs;
  if (obs.disassemble(input, len)) {
    error = "cannot disassemble WxObs/station report payload";
    return -1;
  }
  time_t value = obs.getObservationTime();
  if (!shiftTimeValue(value, offset, "WxObs observation time", error)) {
    return -1;
  }
  obs.setObservationTime(value);
  // Station chunks may be legacy binary, binary-plus-XML, or bare XML.  Keep
  // the original representation because downstream readers use all three.
  if (startsWithReport) {
    obs.assembleAsReport(msgId);
  } else {
    obs.assembleAsXml();
  }
  copyBuffer(obs.getBufPtr(), obs.getBufLen(), output);
  return 0;
}

int validateAircraftPayload(int productId, int dataType2,
                            const void *input, int len,
                            vector<unsigned char> &output, string &error)
{
  // These aircraft records carry their event time only in SPDB metadata.
  // Validate their variable layouts but deliberately leave bytes unchanged.
  if (productId == SPDB_AC_POSN_ID ||
      productId == SPDB_AC_POSN_WMOD_ID) {
    const bool isWmod = productId == SPDB_AC_POSN_WMOD_ID ||
                        dataType2 == SPDB_AC_POSN_WMOD_ID;
    const size_t itemLen = isWmod ? sizeof(ac_posn_wmod_t) :
                                   sizeof(ac_posn_t);
    if (len <= 0 || (size_t) len % itemLen != 0) {
      error = "aircraft position chunk length is not a structure multiple";
      return -1;
    }
  } else if (productId == SPDB_AC_DATA_ID) {
    const size_t baseLen = offsetof(ac_data_t, client_data);
    if ((size_t) len < baseLen) {
      error = "aircraft data chunk is shorter than ac_data_t";
      return -1;
    }
    ac_data_t header;
    memset(&header, 0, sizeof(header));
    memcpy(&header, input, baseLen);
    ac_data_from_BE(&header);
    if ((uint64_t) baseLen + header.client_data_len > (uint64_t) len) {
      error = "aircraft client-data length exceeds the chunk";
      return -1;
    }
  }
  copyBuffer(input, len, output);
  return 0;
}

int retimeAcGeoref(const void *input, int len, int64_t offset,
                   vector<unsigned char> &output, string &error)
{
  if (len <= 0 || len % (int) sizeof(ac_georef_t) != 0) {
    error = "aircraft georef chunk length is not a structure multiple";
    return -1;
  }
  copyBuffer(input, len, output);
  ac_georef_t *items = (ac_georef_t *) output.data();
  const int count = len / sizeof(ac_georef_t);
  for (int ii = 0; ii < count; ++ii) {
    BE_to_ac_georef(&items[ii]);
    if (!shiftTime64(items[ii].time_secs_utc, offset,
                     "aircraft georef time", error)) {
      return -1;
    }
    BE_from_ac_georef(&items[ii]);
  }
  return 0;
}

int retimeLightning(const void *input, int len, int64_t offset,
                    vector<unsigned char> &output, string &error)
{
  if (len < (int) sizeof(ui32)) {
    error = "lightning chunk is too short";
    return -1;
  }
  ui32 cookie;
  memcpy(&cookie, input, sizeof(cookie));
  copyBuffer(input, len, output);
  // The extended cookie is byte-order invariant, so it can select the layout
  // before the per-record conversion from network byte order.
  if (cookie == LTG_EXTENDED_COOKIE) {
    if (len % (int) sizeof(LTG_extended_t) != 0) {
      error = "extended lightning chunk length is not a structure multiple";
      return -1;
    }
    LTG_extended_t *items = (LTG_extended_t *) output.data();
    const int count = len / sizeof(LTG_extended_t);
    for (int ii = 0; ii < count; ++ii) {
      LTG_extended_from_BE(&items[ii]);
      if (!shiftTime32(items[ii].time, offset,
                       "extended lightning time", error)) {
        return -1;
      }
      LTG_extended_to_BE(&items[ii]);
    }
  } else {
    if (len % (int) sizeof(LTG_strike_t) != 0) {
      error = "lightning chunk length is not a structure multiple";
      return -1;
    }
    LTG_strike_t *items = (LTG_strike_t *) output.data();
    const int count = len / sizeof(LTG_strike_t);
    for (int ii = 0; ii < count; ++ii) {
      LTG_from_BE(&items[ii]);
      if (!shiftTime32(items[ii].time, offset,
                       "lightning strike time", error)) {
        return -1;
      }
      LTG_to_BE(&items[ii]);
    }
  }
  return 0;
}

int retimeSigmet(const void *input, int len, int64_t offset,
                 vector<unsigned char> &output, string &error)
{
  const size_t fixedLen = offsetof(SIGMET_spdb_t, vertices);
  if ((size_t) len < fixedLen) {
    error = "SIGMET chunk is shorter than its fixed header";
    return -1;
  }
  copyBuffer(input, len, output);
  SIGMET_spdb_t *sigmet = (SIGMET_spdb_t *) output.data();
  const int nVertices = BE_to_si32(sigmet->num_vertices);
  const int textLen = BE_to_si32(sigmet->text_len);
  const int textOffset = BE_to_si32(sigmet->text_offset);
  if (nVertices < 0 || textLen < 0 || textOffset < 0 ||
      fixedLen + (size_t) nVertices * sizeof(SIGMET_vertex_t) >
          (size_t) len ||
      (uint64_t) textOffset + textLen > (uint64_t) len) {
    error = "SIGMET counts or offsets exceed the chunk";
    return -1;
  }
  SIGMET_spdb_from_BE(sigmet);
  if (!shiftTime32(sigmet->start_time, offset, "SIGMET start time", error) ||
      !shiftTime32(sigmet->end_time, offset, "SIGMET end time", error)) {
    return -1;
  }
  SIGMET_spdb_to_BE(sigmet);
  return 0;
}

int retimeSigAirMet(const void *input, int len, int64_t offset,
                    vector<unsigned char> &output, string &error)
{
  SigAirMet product;
  if (product.disassemble(input, len)) {
    error = "cannot disassemble SigAirMet payload";
    return -1;
  }

  time_t issue = product.getIssueTime();
  time_t start = product.getStartTime();
  time_t end = product.getEndTime();
  time_t cancel = product.getCancelTime();
  time_t obs = product.getObsTime();
  time_t forecast = product.getFcastTime();
  if (!shiftTimeValue(issue, offset, "SigAirMet issue time", error) ||
      !shiftTimeValue(start, offset, "SigAirMet start time", error) ||
      !shiftTimeValue(end, offset, "SigAirMet end time", error) ||
      !shiftTimeValue(cancel, offset, "SigAirMet cancel time", error) ||
      !shiftTimeValue(obs, offset, "SigAirMet observation time", error) ||
      !shiftTimeValue(forecast, offset, "SigAirMet forecast time", error)) {
    return -1;
  }
  product.setIssueTime(issue);
  product.setStartTime(start);
  product.setEndTime(end);
  product.setCancelTime(cancel);
  product.setObsTime(obs);
  product.setFcastTime(forecast);

  const vector<sigairmet_forecast_t> forecasts = product.getForecasts();
  product.clearForecasts();
  for (size_t ii = 0; ii < forecasts.size(); ++ii) {
    time_t value = forecasts[ii].time;
    if (!shiftTimeValue(value, offset, "SigAirMet forecast point", error)) {
      return -1;
    }
    product.addForecast(value, forecasts[ii].lat,
                        forecasts[ii].lon, forecasts[ii].id);
  }
  const vector<sigairmet_forecast_t> outlooks = product.getOutlooks();
  product.clearOutlooks();
  for (size_t ii = 0; ii < outlooks.size(); ++ii) {
    time_t value = outlooks[ii].time;
    if (!shiftTimeValue(value, offset, "SigAirMet outlook point", error)) {
      return -1;
    }
    product.addOutlook(value, outlooks[ii].lat,
                       outlooks[ii].lon, outlooks[ii].id);
  }
  product.assemble();
  copyBuffer(product.getBufPtr(), product.getBufLen(), output);
  return 0;
}

int retimeLegacySounding(const void *input, int len, int64_t offset,
                         vector<unsigned char> &output, string &error)
{
  const size_t fixedLen = offsetof(SNDG_spdb_product_t, points);
  if ((size_t) len < fixedLen) {
    error = "legacy sounding chunk is shorter than its fixed header";
    return -1;
  }
  SNDG_spdb_product_t header;
  memset(&header, 0, sizeof(header));
  memcpy(&header, input, fixedLen);
  const int nPoints = BE_to_si32(header.nPoints);
  if (nPoints < 0 ||
      fixedLen + (size_t) nPoints * sizeof(SNDG_spdb_point_t) >
          (size_t) len) {
    error = "legacy sounding point count exceeds the chunk";
    return -1;
  }
  copyBuffer(input, len, output);
  SNDG_spdb_product_t *sounding =
      (SNDG_spdb_product_t *) output.data();
  SNDG_spdb_product_from_BE(sounding);
  if (!shiftTime32(sounding->launchTime, offset,
                   "sounding launch time", error)) {
    return -1;
  }
  SNDG_spdb_product_to_BE(sounding);
  return 0;
}

int retimeSoundingPlus(const void *input, int len, int64_t offset,
                       vector<unsigned char> &output, string &error)
{
  Sndg sounding;
  if (sounding.disassemble(input, len)) {
    error = "cannot disassemble Sndg payload";
    return -1;
  }
  Sndg::header_t header = sounding.getHeader();
  if (!shiftTime32(header.launchTime, offset,
                   "Sndg launch time", error)) {
    return -1;
  }
  sounding.setHeader(header);
  sounding.assemble();
  copyBuffer(sounding.getBufPtr(), sounding.getBufLen(), output);
  return 0;
}

int retimeGenPoly(const void *input, int len, int64_t offset,
                  vector<unsigned char> &output, string &error)
{
  GenPoly polygon;
  if (!polygon.disassemble(input, len)) {
    error = "cannot disassemble generic polygon payload";
    return -1;
  }
  time_t value = polygon.getTime();
  time_t expire = polygon.getExpireTime();
  if (!shiftTimeValue(value, offset, "GenPoly time", error) ||
      !shiftTimeValue(expire, offset, "GenPoly expire time", error)) {
    return -1;
  }
  polygon.setTime(value);
  polygon.setExpireTime(expire);
  if (!polygon.assemble()) {
    error = "cannot assemble generic polygon payload";
    return -1;
  }
  copyBuffer(polygon.getBufPtr(), polygon.getBufLen(), output);
  return 0;
}

} // namespace

bool PayloadRetimer::supports(int productId, int)
{
  switch (productId) {
    case SPDB_GENERIC_POINT_ID:
    case SPDB_STATION_REPORT_ID:
    case SPDB_AC_POSN_ID:
    case SPDB_AC_POSN_WMOD_ID:
    case SPDB_AC_GEOREF_ID:
    case SPDB_AC_DATA_ID:
    case SPDB_LTG_ID:
    case SPDB_SIGMET_ID:
    case SPDB_SIGAIRMET_ID:
    case SPDB_SNDG_ID:
    case SPDB_SNDG_PLUS_ID:
    case SPDB_GENERIC_POLYLINE_ID:
      return true;
    default:
      return false;
  }
}

int PayloadRetimer::retime(int productId,
                           int dataType2,
                           const void *input,
                           int inputLen,
                           int64_t offset,
                           vector<unsigned char> &output,
                           string &error)
{
  output.clear();
  error.clear();
  if (input == NULL || inputLen <= 0) {
    error = "payload is empty";
    return -1;
  }

  switch (productId) {
    case SPDB_GENERIC_POINT_ID:
      return retimeGenPt(input, inputLen, offset, output, error);
    case SPDB_STATION_REPORT_ID:
      return retimeWxObs(input, inputLen, offset, output, error);
    case SPDB_AC_POSN_ID:
    case SPDB_AC_POSN_WMOD_ID:
    case SPDB_AC_DATA_ID:
      return validateAircraftPayload(productId, dataType2, input, inputLen,
                                     output, error);
    case SPDB_AC_GEOREF_ID:
      return retimeAcGeoref(input, inputLen, offset, output, error);
    case SPDB_LTG_ID:
      return retimeLightning(input, inputLen, offset, output, error);
    case SPDB_SIGMET_ID:
      return retimeSigmet(input, inputLen, offset, output, error);
    case SPDB_SIGAIRMET_ID:
      return retimeSigAirMet(input, inputLen, offset, output, error);
    case SPDB_SNDG_ID:
      return retimeLegacySounding(input, inputLen, offset, output, error);
    case SPDB_SNDG_PLUS_ID:
      return retimeSoundingPlus(input, inputLen, offset, output, error);
    case SPDB_GENERIC_POLYLINE_ID:
      return retimeGenPoly(input, inputLen, offset, output, error);
    default:
      error = "unsupported SPDB product ID";
      return -1;
  }
}

