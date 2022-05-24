
#include "CloseEventFilter.hh"
#include "SpreadSheetView.hh"
#include "BoundaryPointEditorView.hh"
#include "PolarManager.hh"

bool CloseEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::Close)
  { 
    // Do something interesting, emit a signal for instance.
    LOG(DEBUG) << "closing window";

    const char *className = obj->metaObject()->className(); 
    if (className[0] == 'S') {
       //case 'S': // SpreadSheetView
         SpreadSheetView *spreadSheetViewObj = (SpreadSheetView *) obj;
         spreadSheetViewObj->closeEvent();
      //   break;
    } else if (className[0] == 'B') {
      // case 'B': // BoundaryPointEditorView
         BoundaryPointEditorView *boundaryPointEditorViewObj = (BoundaryPointEditorView *) obj;
         boundaryPointEditorViewObj->closeEvent();
         //break;
    } else {
      // default:  // PolarManager
         PolarManager *polarManagerObj = (PolarManager *) obj;
         polarManagerObj->closeEvent(event);
    }

  }
return false;
  //return QObject::eventFilter(obj, event);
}

