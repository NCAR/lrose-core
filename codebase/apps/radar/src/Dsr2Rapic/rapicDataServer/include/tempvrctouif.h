#ifndef __TEMPVRCTOUIF_H__
#define __TEMPVRCTOUIF_H__

/*
  vrctouif.h
*/

#ifdef __cplusplus
extern "C" {
#endif

  int DBListStns(Widget wid, struct DBBrowseVars *DBVars);
  int DBListStnDays(Widget wid,struct DBBrowseVars *DBVars);
  int DBListStnDayTimes(Widget wid,struct DBBrowseVars *DBVars);
  int ConnListStns(Widget wid);
  int ConnListProducts(Widget wid, int stn);
  int SchedEditListEntries(Widget wid);
  int ReqEditListEntries(Widget wid, char *selreqstr);
  void SetSeqScrollW(Widget scrollwidget);
  void SetSeqTextW(Widget textwidget);

#ifdef __cplusplus
}
#endif

#endif
