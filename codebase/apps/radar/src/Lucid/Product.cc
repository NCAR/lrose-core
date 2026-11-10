// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** BSD licence applies - see the source distribution for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////
// Product.cc
//
// One configured symbolic-product source and its current decoded snapshot.
////////////////////////////////////////////////////////////////

#include "Product.hh"
#include "SymprodRender.hh"
#include "SymprodRenderContext.hh"
#include <QPainter>
#include <limits>

Product::Product(Params::symprod_prod_info_t &prodInfo) :
        _active(0),
        _data_valid(0),
        _last_collected(0),
        _prodInfo(prodInfo),
        _closestObject(nullptr)
{
}

Product::~Product() = default;

bool Product::installChunks(const vector<QByteArray> &chunks, string &error)
{
  vector<unique_ptr<SymprodRender>> decoded;
  decoded.reserve(chunks.size());
  for (const QByteArray &chunk : chunks) {
    if (chunk.isEmpty()) {
      continue;
    }
    unique_ptr<SymprodRender> symprod = make_unique<SymprodRender>(*this);
    if (symprod->deserialize(const_cast<char *>(chunk.constData()),
                             chunk.size()) != 0) {
      error = symprod->getErrStr();
      return false;
    }
    decoded.push_back(std::move(symprod));
  }

  // Publication is an all-or-nothing GUI-thread operation. This keeps every
  // pointer in the displayed snapshot valid until its replacement is ready.
  _closestObject = nullptr;
  _symprods.swap(decoded);
  _data_valid = 1;
  _last_collected = time(nullptr);
  return true;
}

void Product::_selectForDisplay(const SymprodRenderContext &context,
                                vector<SymprodRender *> &selected) const
{
  selected.clear();
  if (!_active || !_data_valid || _symprods.empty()) {
    return;
  }

  const bool reverseOrder =
    _symprods.front()->getProps().start_time >
    _symprods.back()->getProps().start_time;
  vector<Symprod::prod_hdr_props_t> remembered;
  for (size_t ii = 0; ii < _symprods.size(); ++ii) {
    SymprodRender *symprod =
      _symprods[reverseOrder ? _symprods.size() - 1 - ii : ii].get();
    const Symprod::prod_hdr_props_t props = symprod->getProps();
    bool alreadyDone = false;
    for (const Symprod::prod_hdr_props_t &previous : remembered) {
      if (previous.start_time != props.start_time &&
          previous.data_type == props.data_type) {
        alreadyDone = true;
        break;
      }
    }

    bool display = false;
    bool remember = false;
    switch (_prodInfo.render_type) {
      case Params::SYMPROD_RENDER_ALL:
      case Params::SYMPROD_RENDER_GET_VALID:
      case Params::SYMPROD_RENDER_GET_VALID_AT_FRAME_TIME:
      case Params::SYMPROD_RENDER_FIRST_BEFORE_FRAME_TIME:
        display = true;
        break;
      case Params::SYMPROD_RENDER_ALL_VALID:
        display = props.start_time <= context.frameEnd +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.expire_time >= context.frameStart -
                    _prodInfo.minutes_allow_before * 60;
        break;
      case Params::SYMPROD_RENDER_VALID_IN_LAST_FRAME:
        display = props.start_time <= context.epochEnd +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.expire_time >= context.epochEnd -
                    (context.frameEnd - context.frameStart) -
                    _prodInfo.minutes_allow_before * 60;
        break;
      case Params::SYMPROD_RENDER_LATEST_IN_FRAME:
        display = !alreadyDone &&
                  props.start_time <= context.frameEnd +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.expire_time >= context.frameStart -
                    _prodInfo.minutes_allow_before * 60;
        remember = display;
        break;
      case Params::SYMPROD_RENDER_LATEST_IN_LOOP:
        display = !alreadyDone &&
                  props.start_time <= context.epochEnd +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.expire_time >= context.epochStart -
                    _prodInfo.minutes_allow_before * 60;
        remember = display;
        break;
      case Params::SYMPROD_RENDER_FIRST_AFTER_DATA_TIME:
        display = !alreadyDone &&
                  props.start_time <= context.dataTime +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.expire_time >= context.epochStart -
                    _prodInfo.minutes_allow_before * 60;
        remember = display;
        break;
      case Params::SYMPROD_RENDER_FIRST_BEFORE_DATA_TIME:
        display = !alreadyDone &&
                  props.start_time >= context.dataTime -
                    _prodInfo.minutes_allow_before * 60 &&
                  props.start_time <= context.epochEnd +
                    _prodInfo.minutes_allow_after * 60;
        remember = display;
        break;
      case Params::SYMPROD_RENDER_ALL_BEFORE_DATA_TIME:
        display = props.start_time <= context.dataTime +
                    _prodInfo.minutes_allow_after * 60 &&
                  props.start_time >= context.epochStart -
                    _prodInfo.minutes_allow_before * 60;
        break;
      case Params::SYMPROD_RENDER_ALL_AFTER_DATA_TIME:
        display = props.start_time >= context.dataTime -
                    _prodInfo.minutes_allow_before * 60 &&
                  props.start_time <= context.epochEnd +
                    _prodInfo.minutes_allow_after * 60;
        remember = display;
        break;
    }
    if (display) {
      selected.push_back(symprod);
      if (remember) {
        remembered.push_back(props);
      }
    }
  }
}

void Product::drawQt(QPainter &painter,
                     const SymprodRenderContext &context) const
{
  vector<SymprodRender *> selected;
  _selectForDisplay(context, selected);
  for (SymprodRender *symprod : selected) {
    symprod->drawQt(painter, context);
  }
}

double Product::pickClosestObject(double lat, double lon,
                                  const SymprodRenderContext &context)
{
  _closestObject = nullptr;
  double closestDistance = std::numeric_limits<double>::max();
  vector<SymprodRender *> selected;
  _selectForDisplay(context, selected);
  for (SymprodRender *symprod : selected) {
    const double distance = symprod->pick_closest_obj(lat, lon);
    if (distance < closestDistance) {
      closestDistance = distance;
      _closestObject = symprod->get_closest_obj();
    }
  }
  return closestDistance;
}
