#ifndef CLOSEEVENTFILTER_HH
#define CLOSEEVENTFILTER_HH


#include <QObject>
#include <QEvent>
#include <toolsa/LogStream.hh>
#include "SpreadSheetView.hh"

class CloseEventFilter : public QObject
{
     Q_OBJECT
public:
     CloseEventFilter(QObject *parent) : QObject(parent) {}

protected: 
     bool eventFilter(QObject *obj, QEvent *event)
     {
          if (event->type() == QEvent::Close)
          { 
               // Do something interesting, emit a signal for instance.
            LOG(DEBUG) << "closing window";
            SpreadSheetView *spreadSheetViewObj = (SpreadSheetView *) obj;
            spreadSheetViewObj->closeEvent();
          }

          return QObject::eventFilter(obj, event);
     }

};

#endif
