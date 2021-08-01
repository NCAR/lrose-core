// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// DisplayFieldView.hh
//
// DisplayFieldView class 
//
// Brenda Javornik, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2021
//
///////////////////////////////////////////////////////////////
//
// DisplayFieldView manages the display and selection 
// of data fields.
//
///////////////////////////////////////////////////////////////

#ifndef DISPLAYFIELDVIEW_HH
#define DISPLAYFIELDVIEW_HH

#include <string>
#include <vector>

#include "ParamFile.hh"
#include "RayLoc.hh"
//#include "ContextEditingView.hh"
#include "ClickableLabelNamed.hh"
#include "ParameterColorView.hh"
#include "FieldColorController.hh"
//#include "DisplayFieldController.hh"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QStringList>

class QApplication;
class QActionGroup;
class QButtonGroup;
//class QRadioButton;
class QPushButton;
class QListWidget;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QDateTime;
class QDateTimeEdit;
class QFileDialog;

class DisplayField;
//class DisplayFieldController;

class DisplayFieldView : public QGroupBox {
  
  Q_OBJECT

public:

  DisplayFieldView(); // DisplayFieldController *controller);
  ~DisplayFieldView();

  void createFieldPanel(QWidget *parent);
  void updateFieldPanel(string rawFieldLabel, string newFieldName,
    string rawFieldShortCut);
  void clear();

  //QWidget *getViewWidget();

  void set_label_font_size(int size);
  void setHaveFilteredFields(bool value);

  // signal that user has selected fieldName for display
  void changeToField(string fieldName, bool guiMode);

public slots:
  //void _changeFieldVariable(bool value);
  void _changeFieldVariable(QString fieldName);
  void _changeField(int fieldIdx);
  void _editFieldVariable(QString fieldName);

private slots:
  void contextMenuCancel();
  void contextMenuDelete();
  void contextMenuRemove();
  void contextMenuUndo();
  void contextMenuParameterColors();  

signals:
  void selectedFieldChanged(QString newFieldName);
  void removeField(QString fieldName);

private:

  void ShowContextMenu(QString fieldName);
  void informationMessage();
  size_t _findFieldIndex(QString fieldName);

  QString _workingWithField;
  
  // QGroupBox *_fieldPanel;
  QVBoxLayout *_fieldsLayout;
  //QLabel *_selectedLabelWidget;
  QButtonGroup *_fieldGroup;
  //vector<QRadioButton *> _fieldButtons;
  vector<ClickableLabelNamed *> _fieldButtons;
  //DisplayField *_selectedField;
  //string _selectedName;
  //string _selectedLabel;
  //string _selectedUnits;
  QLabel *_valueLabel;
  //int _fieldNum;
  //int _prevFieldNum;

  int _label_font_size;
  bool _haveFilteredFields;

  //DisplayFieldController *_controller;

};

#endif
