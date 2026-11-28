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
//////////////////////////////////////////////////////////
// PrintProcsCmd.c
//
// Cmd for printing procmap output
//
//////////////////////////////////////////////////////////

#include "PrintProcsCmd.h"
#include <MotifApp/Application.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include "Procinfo.h"
#include <stream.h>

PrintProcsCmd::PrintProcsCmd ( char *name, 
			       char mnemonic,
			       int  active) :
  NoUndoCmd ( name, mnemonic, active )
{
  
  _dialog = NULL;

}

void PrintProcsCmd::createWidgets()

{

  // get position of button interface

  Position x, y;
  XtVaGetValues(_ci[0]->baseWidget(),
		XmNx, &x, XmNy, &y, NULL);

  cerr << "x, y for button: " << x << ", " <<  y << "\n";

  _dialog =
    XtVaCreateWidget ("Procmap info",
   		      xmDialogShellWidgetClass, _ci[0]->baseWidget(),
		      NULL);
  
  // Create a Form widget to manage the button and text area

  _form =
    XtVaCreateManagedWidget ("form", xmFormWidgetClass, _dialog,
 			     XmNwidth, 500,
 			     XmNheight, 400,
 			     XmNx, 500,
 			     XmNy, 500,
 			     NULL);
  
//   _form =
//     XtVaCreateManagedWidget ("form", xmRowColumnWidgetClass, _dialog,
// 			     XmNwidth, 500,
// 			     XmNheight, 400,
// 			     XmNx, 500,
// 			     XmNy, 500,
// 			     NULL);
  
  // Create a text widget at the top of the form

  Widget _text = XmCreateScrolledText(_form, "text", NULL, 0);
  
  XtVaSetValues(_text,
		XmNrows, 30,
		XmNcolumns, 80,
		XmNeditable, False,
		XmNeditMode, XmMULTI_LINE_EDIT,
		// XmNcursorPositionVisible, False,
 		NULL);

  XtManageChild(_text);

//   Widget scrollw =
//     XtVaCreateManagedWidget ("scroll", xmScrolledWindowWidgetClass, _form,
// 			     NULL);
  
  //  _text =
  // XtVaCreateManagedWidget ("text", xmTextWidgetClass, _form,
  // 			     NULL);

  // Create a dismiss button at the bottom of the form

  Widget dismiss =
    XtVaCreateManagedWidget ("Dismiss", xmPushButtonWidgetClass, _form,
			     NULL);

  // add callback for dismiss button

  XtAddCallback (dismiss,
 		 XmNactivateCallback, 
 		 &PrintProcsCmd::dismissCallback, 
 		 (XtPointer) this );

  // Set up the attachments
  
   XtVaSetValues ( dismiss, 
 		  XmNtopAttachment,    XmATTACH_NONE,
 		  XmNleftAttachment,   XmATTACH_FORM,
 		  XmNbottomAttachment, XmATTACH_FORM,
 		  NULL );
  
   XtVaSetValues ( XtParent(_text),
		   XmNtopAttachment,    XmATTACH_FORM,
		   XmNleftAttachment,   XmATTACH_FORM,
		   XmNrightAttachment,  XmATTACH_FORM,
		   XmNbottomWidget,     dismiss,
		   XmNbottomAttachment, XmATTACH_WIDGET,
		   NULL );
  
}

void PrintProcsCmd::doit()
{

  // create widgets if needed

  if (_dialog == NULL) {
    createWidgets();
  }
  
  // Display the dialog

  XtManageChild (_form);

  // Register with Procinfo

  Procinfo::Inst()->registerTextWidget(_text);

}

void PrintProcsCmd::dismissCallback ( Widget    w,
				      XtPointer clientData,
				      XtPointer callData )
{

  cerr << "Dismiss\n";

  PrintProcsCmd * obj = ( PrintProcsCmd * ) clientData;
  
  // unmanage the widget

  XtUnmanageChild ( obj->_form );

  // un-register this widget with Procinfo

  Procinfo::Inst()->unregisterTextWidget();

  // keep compiler quiet about unused args

  Widget w1;
  w1 = w;
  XtPointer c;
  c = callData;
  
}


