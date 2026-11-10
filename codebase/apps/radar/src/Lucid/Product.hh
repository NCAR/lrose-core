// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** BSD licence applies - see the source distribution for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////
// Product.hh
//
// One configured symbolic-product source and its current decoded snapshot.
////////////////////////////////////////////////////////////////

#ifndef Product_HH
#define Product_HH

#include <memory>
#include <vector>
#include <QByteArray>
#include "GlobalData.hh"
#include "Params.hh"

class QPainter;
class SymprodRender;
class SymprodRenderObj;
struct SymprodRenderContext;

class Product
{
public:
  explicit Product(Params::symprod_prod_info_t &prodInfo);
  ~Product();

  // Decode on the GUI thread, then atomically replace the published snapshot.
  // A bad response leaves the previous display available.
  bool installChunks(const vector<QByteArray> &chunks, string &error);

  void drawQt(QPainter &painter,
              const SymprodRenderContext &context) const;

  double pickClosestObject(double lat, double lon,
                           const SymprodRenderContext &context);
  SymprodRenderObj *getClosestObject() const { return _closestObject; }

  int _active;
  int _data_valid;
  time_t _last_collected;
  Params::symprod_prod_info_t &_prodInfo;

private:
  void _selectForDisplay(const SymprodRenderContext &context,
                         vector<SymprodRender *> &selected) const;

  vector<unique_ptr<SymprodRender>> _symprods;
  SymprodRenderObj *_closestObject;
};

#endif
