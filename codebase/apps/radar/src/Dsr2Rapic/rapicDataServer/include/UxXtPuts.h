
/******************************************************************************/
/*				UxXtPuts.h				      */
/******************************************************************************/

#ifndef UX_XT_PUTS_INCLUDED
#define UX_XT_PUTS_INCLUDED


#define UxPutAccelerator( wgt, res_value ) \
	XtVaSetValues( wgt, XmNaccelerator, res_value, NULL )

#define UxPutAcceleratorText( wgt, res_value ) \
	UxPutStrRes( wgt, XmNacceleratorText, res_value )

#define UxPutAccelerators( wgt, res_value ) \
	UxPutStrRes( wgt, XmNaccelerators, res_value )

#define UxPutAdjustLast( wgt, res_value ) \
	UxPutStrRes( wgt, XmNadjustLast, res_value )

#define UxPutAdjustMargin( wgt, res_value ) \
	UxPutStrRes( wgt, XmNadjustMargin, res_value )

#define UxPutAlignment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNalignment, res_value )

#define UxPutAllowOverlap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNallowOverlap, res_value )

#define UxPutAllowResize( wgt, res_value ) \
	UxPutStrRes( wgt, XmNallowResize, res_value )

#define UxPutAllowShellResize( wgt, res_value ) \
	UxPutStrRes( wgt, XmNallowShellResize, res_value )

#define UxPutAncestorSensitive( wgt, res_value ) \
	UxPutStrRes( wgt, XmNancestorSensitive, res_value )

#define UxPutApplyLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNapplyLabelString, res_value )

#define UxPutArgc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNargc, res_value, NULL )

#define UxPutArgv( wgt, res_value ) \
	XtVaSetValues( wgt, XmNargv, res_value, NULL )

#define UxPutArmColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNarmColor, res_value )

#define UxPutArmPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNarmPixmap, res_value )

#define UxPutArrowDirection( wgt, res_value ) \
	UxPutStrRes( wgt, XmNarrowDirection, res_value )

#define UxPutAutoShowCursorPosition( wgt, res_value ) \
	UxPutStrRes( wgt, XmNautoShowCursorPosition, res_value )

#define UxPutAutoUnmanage( wgt, res_value ) \
	UxPutStrRes( wgt, XmNautoUnmanage, res_value )

#define UxPutAutomaticSelection( wgt, res_value ) \
	UxPutStrRes( wgt, XmNautomaticSelection, res_value )

#define UxPutBackground( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbackground, res_value )

#define UxPutBackgroundPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbackgroundPixmap, res_value )

#define UxPutBaseHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNbaseHeight, res_value, NULL )

#define UxPutBaseWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNbaseWidth, res_value, NULL )

#define UxPutBlinkRate( wgt, res_value ) \
	XtVaSetValues( wgt, XmNblinkRate, res_value, NULL )

#define UxPutBorderColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNborderColor, res_value )

#define UxPutBorderPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNborderPixmap, res_value )

#define UxPutBorderWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNborderWidth, res_value, NULL )

#define UxPutBottomAttachment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbottomAttachment, res_value )

#define UxPutBottomOffset( wgt, res_value ) \
	XtVaSetValues( wgt, XmNbottomOffset, res_value, NULL )

#define UxPutBottomPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNbottomPosition, res_value, NULL )

#define UxPutBottomShadowColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbottomShadowColor, res_value )

#define UxPutBottomShadowPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbottomShadowPixmap, res_value )

#define UxPutBottomWidget( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbottomWidget, res_value )

#define UxPutButtonFontList( wgt, res_value ) \
	UxPutStrRes( wgt, XmNbuttonFontList, res_value )

#define UxPutCancelButton( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcancelButton, res_value )

#define UxPutCancelLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcancelLabelString, res_value )

#define UxPutCascadePixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcascadePixmap, res_value )

#define UxPutChildren( wgt, res_value ) \
	XtVaSetValues( wgt, XmNchildren, res_value, NULL )

#define UxPutClipWindow( wgt, res_value ) \
	UxPutStrRes( wgt, XmNclipWindow, res_value )

#define UxPutColormap( wgt, res_value ) \
	XtVaSetValues( wgt, XmNcolormap, res_value, NULL )

#define UxPutColumns( wgt, res_value ) \
	XtVaSetValues( wgt, XmNcolumns, res_value, NULL )

#define UxPutCommand( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcommand, res_value )

#define UxPutCommandWindow( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcommandWindow, res_value )

#define UxPutCommandWindowLocation( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcommandWindowLocation, res_value )

#define UxPutCreatePopupChildProc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNcreatePopupChildProc, res_value, NULL )

#define UxPutCursorPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNcursorPosition, res_value, NULL )

#define UxPutCursorPositionVisible( wgt, res_value ) \
	UxPutStrRes( wgt, XmNcursorPositionVisible, res_value )

#define UxPutDecimalPoints( wgt, res_value ) \
	XtVaSetValues( wgt, XmNdecimalPoints, res_value, NULL )

#define UxPutDefaultButton( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdefaultButton, res_value )

#define UxPutDefaultButtonShadowThickness( wgt, res_value ) \
	XtVaSetValues( wgt, XmNdefaultButtonShadowThickness, res_value, NULL )

#define UxPutDefaultButtonType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdefaultButtonType, res_value )

#define UxPutDefaultFontList( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdefaultFontList, res_value )

#define UxPutDefaultPosition( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdefaultPosition, res_value )

#define UxPutDeleteResponse( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdeleteResponse, res_value )

#define UxPutDialogStyle( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdialogStyle, res_value )

#define UxPutDialogTitle( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdialogTitle, res_value )

#define UxPutDialogType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdialogType, res_value )

#define UxPutDirListItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNdirListItemCount, res_value, NULL )

#define UxPutDirListItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirListItems, res_value )

#define UxPutDirListLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirListLabelString, res_value )

#define UxPutDirMask( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirMask, res_value )

#define UxPutDirSearchProc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNdirSearchProc, res_value, NULL )

#define UxPutDirSpec( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirSpec, res_value )

#define UxPutDirectory( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirectory, res_value )

#define UxPutDirectoryValid( wgt, res_value ) \
	UxPutStrRes( wgt, XmNdirectoryValid, res_value )

#define UxPutDoubleClickInterval( wgt, res_value ) \
	XtVaSetValues( wgt, XmNdoubleClickInterval, res_value, NULL )

#define UxPutEditMode( wgt, res_value ) \
	UxPutStrRes( wgt, XmNeditMode, res_value )

#define UxPutEditable( wgt, res_value ) \
	UxPutStrRes( wgt, XmNeditable, res_value )

#define UxPutEntryAlignment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNentryAlignment, res_value )

#define UxPutEntryBorder( wgt, res_value ) \
	XtVaSetValues( wgt, XmNentryBorder, res_value, NULL )

#define UxPutEntryClass( wgt, res_value ) \
	UxPutStrRes( wgt, XmNentryClass, res_value )

#define UxPutFileListItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNfileListItemCount, res_value, NULL )

#define UxPutFileListItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfileListItems, res_value )

#define UxPutFileListLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfileListLabelString, res_value )

#define UxPutFileSearchProc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNfileSearchProc, res_value, NULL )

#define UxPutFileTypeMask( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfileTypeMask, res_value )

#define UxPutFillOnArm( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfillOnArm, res_value )

#define UxPutFillOnSelect( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfillOnSelect, res_value )

#define UxPutFilterLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfilterLabelString, res_value )

#define UxPutFontList( wgt, res_value ) \
	UxPutStrRes( wgt, XmNfontList, res_value )

#define UxPutForeground( wgt, res_value ) \
	UxPutStrRes( wgt, XmNforeground, res_value )

#define UxPutFractionBase( wgt, res_value ) \
	XtVaSetValues( wgt, XmNfractionBase, res_value, NULL )

#define UxPutGeometry( wgt, res_value ) \
	UxPutStrRes( wgt, XmNgeometry, res_value )

#define UxPutHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNheight, res_value, NULL )

#define UxPutHeightInc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNheightInc, res_value, NULL )

#define UxPutHelpLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhelpLabelString, res_value )

#define UxPutHighlightColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhighlightColor, res_value )

#define UxPutHighlightOnEnter( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhighlightOnEnter, res_value )

#define UxPutHighlightPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhighlightPixmap, res_value )

#define UxPutHighlightThickness( wgt, res_value ) \
	XtVaSetValues( wgt, XmNhighlightThickness, res_value, NULL )

#define UxPutHistoryItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNhistoryItemCount, res_value, NULL )

#define UxPutHistoryItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhistoryItems, res_value )

#define UxPutHistoryMaxItems( wgt, res_value ) \
	XtVaSetValues( wgt, XmNhistoryMaxItems, res_value, NULL )

#define UxPutHistoryVisibleItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNhistoryVisibleItemCount, res_value, NULL )

#define UxPutHorizontalScrollBar( wgt, res_value ) \
	UxPutStrRes( wgt, XmNhorizontalScrollBar, res_value )

#define UxPutHorizontalSpacing( wgt, res_value ) \
	XtVaSetValues( wgt, XmNhorizontalSpacing, res_value, NULL )

#define UxPutIconMask( wgt, res_value ) \
	UxPutStrRes( wgt, XmNiconMask, res_value )

#define UxPutIconName( wgt, res_value ) \
	XtVaSetValues( wgt, XmNiconName, res_value, NULL )

#define UxPutIconNameEncoding( wgt, res_value ) \
	UxPutStrRes( wgt, XmNiconNameEncoding, res_value )

#define UxPutIconPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNiconPixmap, res_value )

#define UxPutIconWindow( wgt, res_value ) \
	UxPutStrRes( wgt, XmNiconWindow, res_value )

#define UxPutIconX( wgt, res_value ) \
	XtVaSetValues( wgt, XmNiconX, res_value, NULL )

#define UxPutIconY( wgt, res_value ) \
	XtVaSetValues( wgt, XmNiconY, res_value, NULL )

#define UxPutIconic( wgt, res_value ) \
	UxPutStrRes( wgt, XmNiconic, res_value )

#define UxPutIncrement( wgt, res_value ) \
	XtVaSetValues( wgt, XmNincrement, res_value, NULL )

#define UxPutIndicatorOn( wgt, res_value ) \
	UxPutStrRes( wgt, XmNindicatorOn, res_value )

#define UxPutIndicatorSize( wgt, res_value ) \
	XtVaSetValues( wgt, XmNindicatorSize, res_value, NULL )

#define UxPutIndicatorType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNindicatorType, res_value )

#define UxPutInitialDelay( wgt, res_value ) \
	XtVaSetValues( wgt, XmNinitialDelay, res_value, NULL )

#define UxPutInitialState( wgt, res_value ) \
	UxPutStrRes( wgt, XmNinitialState, res_value )

#define UxPutInput( wgt, res_value ) \
	UxPutStrRes( wgt, XmNinput, res_value )

#define UxPutInsertPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNinsertPosition, res_value, NULL )

#define UxPutIsAligned( wgt, res_value ) \
	UxPutStrRes( wgt, XmNisAligned, res_value )

#define UxPutIsHomogeneous( wgt, res_value ) \
	UxPutStrRes( wgt, XmNisHomogeneous, res_value )

#define UxPutItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNitemCount, res_value, NULL )

#define UxPutItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNitems, res_value )

#define UxPutKeyboardFocusPolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNkeyboardFocusPolicy, res_value )

#define UxPutLabelFontList( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlabelFontList, res_value )

#define UxPutLabelInsensitivePixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlabelInsensitivePixmap, res_value )

#define UxPutLabelPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlabelPixmap, res_value )

#define UxPutLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlabelString, res_value )

#define UxPutLabelType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlabelType, res_value )

#define UxPutLeftAttachment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNleftAttachment, res_value )

#define UxPutLeftOffset( wgt, res_value ) \
	XtVaSetValues( wgt, XmNleftOffset, res_value, NULL )

#define UxPutLeftPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNleftPosition, res_value, NULL )

#define UxPutLeftWidget( wgt, res_value ) \
	UxPutStrRes( wgt, XmNleftWidget, res_value )

#define UxPutListItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNlistItemCount, res_value, NULL )

#define UxPutListItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlistItems, res_value )

#define UxPutListLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlistLabelString, res_value )

#define UxPutListMarginHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNlistMarginHeight, res_value, NULL )

#define UxPutListMarginWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNlistMarginWidth, res_value, NULL )

#define UxPutListSizePolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlistSizePolicy, res_value )

#define UxPutListSpacing( wgt, res_value ) \
	XtVaSetValues( wgt, XmNlistSpacing, res_value, NULL )

#define UxPutListUpdated( wgt, res_value ) \
	UxPutStrRes( wgt, XmNlistUpdated, res_value )

#define UxPutListVisibleItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNlistVisibleItemCount, res_value, NULL )

#define UxPutMainWindowMarginHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmainWindowMarginHeight, res_value, NULL )

#define UxPutMainWindowMarginWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmainWindowMarginWidth, res_value, NULL )

#define UxPutMappedWhenManaged( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmappedWhenManaged, res_value )

#define UxPutMappingDelay( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmappingDelay, res_value, NULL )

#define UxPutMargin( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmargin, res_value, NULL )

#define UxPutMarginBottom( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginBottom, res_value, NULL )

#define UxPutMarginHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginHeight, res_value, NULL )

#define UxPutMarginLeft( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginLeft, res_value, NULL )

#define UxPutMarginRight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginRight, res_value, NULL )

#define UxPutMarginTop( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginTop, res_value, NULL )

#define UxPutMarginWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmarginWidth, res_value, NULL )

#define UxPutMaxAspectX( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaxAspectX, res_value, NULL )

#define UxPutMaxAspectY( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaxAspectY, res_value, NULL )

#define UxPutMaxHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaxHeight, res_value, NULL )

#define UxPutMaxLength( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaxLength, res_value, NULL )

#define UxPutMaxWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaxWidth, res_value, NULL )

#define UxPutMaximum( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmaximum, res_value, NULL )

#define UxPutMenuAccelerator( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmenuAccelerator, res_value, NULL )

#define UxPutMenuBar( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmenuBar, res_value )

#define UxPutMenuHelpWidget( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmenuHelpWidget, res_value )

#define UxPutMenuHistory( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmenuHistory, res_value )

#define UxPutMenuPost( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmenuPost, res_value )

#define UxPutMessageAlignment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmessageAlignment, res_value )

#define UxPutMessageString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmessageString, res_value )

#define UxPutMessageWindow( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmessageWindow, res_value )

#define UxPutMinAspectX( wgt, res_value ) \
	XtVaSetValues( wgt, XmNminAspectX, res_value, NULL )

#define UxPutMinAspectY( wgt, res_value ) \
	XtVaSetValues( wgt, XmNminAspectY, res_value, NULL )

#define UxPutMinHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNminHeight, res_value, NULL )

#define UxPutMinWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNminWidth, res_value, NULL )

#define UxPutMinimizeButtons( wgt, res_value ) \
	UxPutStrRes( wgt, XmNminimizeButtons, res_value )

#define UxPutMinimum( wgt, res_value ) \
	XtVaSetValues( wgt, XmNminimum, res_value, NULL )

#define UxPutMnemonic( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmnemonic, res_value )

#define UxPutMnemonicCharSet( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmnemonicCharSet, res_value, NULL )

#define UxPutMultiClick( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmultiClick, res_value )

#define UxPutMustMatch( wgt, res_value ) \
	UxPutStrRes( wgt, XmNmustMatch, res_value )

#define UxPutMwmDecorations( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmwmDecorations, res_value, NULL )

#define UxPutMwmFunctions( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmwmFunctions, res_value, NULL )

#define UxPutMwmInputMode( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmwmInputMode, res_value, NULL )

#define UxPutMwmMenu( wgt, res_value ) \
	XtVaSetValues( wgt, XmNmwmMenu, res_value, NULL )

#define UxPutNavigationType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNnavigationType, res_value )

#define UxPutNoMatchString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNnoMatchString, res_value )

#define UxPutNoResize( wgt, res_value ) \
	UxPutStrRes( wgt, XmNnoResize, res_value )

#define UxPutNumChildren( wgt, res_value ) \
	XtVaSetValues( wgt, XmNnumChildren, res_value, NULL )

#define UxPutNumColumns( wgt, res_value ) \
	XtVaSetValues( wgt, XmNnumColumns, res_value, NULL )

#define UxPutOkLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNokLabelString, res_value )

#define UxPutOrientation( wgt, res_value ) \
	UxPutStrRes( wgt, XmNorientation, res_value )

#define UxPutOverrideRedirect( wgt, res_value ) \
	UxPutStrRes( wgt, XmNoverrideRedirect, res_value )

#define UxPutPacking( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpacking, res_value )

#define UxPutPageIncrement( wgt, res_value ) \
	XtVaSetValues( wgt, XmNpageIncrement, res_value, NULL )

#define UxPutPaneMaximum( wgt, res_value ) \
	XtVaSetValues( wgt, XmNpaneMaximum, res_value, NULL )

#define UxPutPaneMinimum( wgt, res_value ) \
	XtVaSetValues( wgt, XmNpaneMinimum, res_value, NULL )

#define UxPutPattern( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpattern, res_value )

#define UxPutPendingDelete( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpendingDelete, res_value )

#define UxPutPopupEnabled( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpopupEnabled, res_value )

#define UxPutProcessingDirection( wgt, res_value ) \
	UxPutStrRes( wgt, XmNprocessingDirection, res_value )

#define UxPutPromptString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpromptString, res_value )

#define UxPutPushButtonEnabled( wgt, res_value ) \
	UxPutStrRes( wgt, XmNpushButtonEnabled, res_value )

#define UxPutQualifySearchDataProc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNqualifySearchDataProc, res_value, NULL )

#define UxPutRadioAlwaysOne( wgt, res_value ) \
	UxPutStrRes( wgt, XmNradioAlwaysOne, res_value )

#define UxPutRadioBehavior( wgt, res_value ) \
	UxPutStrRes( wgt, XmNradioBehavior, res_value )

#define UxPutRecomputeSize( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrecomputeSize, res_value )

#define UxPutRefigureMode( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrefigureMode, res_value )

#define UxPutRepeatDelay( wgt, res_value ) \
	XtVaSetValues( wgt, XmNrepeatDelay, res_value, NULL )

#define UxPutResizable( wgt, res_value ) \
	UxPutStrRes( wgt, XmNresizable, res_value )

#define UxPutResizeHeight( wgt, res_value ) \
	UxPutStrRes( wgt, XmNresizeHeight, res_value )

#define UxPutResizePolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNresizePolicy, res_value )

#define UxPutResizeWidth( wgt, res_value ) \
	UxPutStrRes( wgt, XmNresizeWidth, res_value )

#define UxPutRightAttachment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrightAttachment, res_value )

#define UxPutRightOffset( wgt, res_value ) \
	XtVaSetValues( wgt, XmNrightOffset, res_value, NULL )

#define UxPutRightPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNrightPosition, res_value, NULL )

#define UxPutRightWidget( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrightWidget, res_value )

#define UxPutRowColumnType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrowColumnType, res_value )

#define UxPutRows( wgt, res_value ) \
	XtVaSetValues( wgt, XmNrows, res_value, NULL )

#define UxPutRubberPositioning( wgt, res_value ) \
	UxPutStrRes( wgt, XmNrubberPositioning, res_value )

#define UxPutSashHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsashHeight, res_value, NULL )

#define UxPutSashIndent( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsashIndent, res_value, NULL )

#define UxPutSashShadowThickness( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsashShadowThickness, res_value, NULL )

#define UxPutSashWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsashWidth, res_value, NULL )

#define UxPutSaveUnder( wgt, res_value ) \
	UxPutStrRes( wgt, XmNsaveUnder, res_value )

#define UxPutScaleHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNscaleHeight, res_value, NULL )

#define UxPutScaleMultiple( wgt, res_value ) \
	XtVaSetValues( wgt, XmNscaleMultiple, res_value, NULL )

#define UxPutScaleWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNscaleWidth, res_value, NULL )

#define UxPutScrollBarDisplayPolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollBarDisplayPolicy, res_value )

#define UxPutScrollBarPlacement( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollBarPlacement, res_value )

#define UxPutScrollHorizontal( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollHorizontal, res_value )

#define UxPutScrollLeftSide( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollLeftSide, res_value )

#define UxPutScrollTopSide( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollTopSide, res_value )

#define UxPutScrollVertical( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollVertical, res_value )

#define UxPutScrolledWindowMarginHeight( wgt, res_value ) \
	XtVaSetValues( wgt, XmNscrolledWindowMarginHeight, res_value, NULL )

#define UxPutScrolledWindowMarginWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNscrolledWindowMarginWidth, res_value, NULL )

#define UxPutScrollingPolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNscrollingPolicy, res_value )

#define UxPutSelectColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectColor, res_value )

#define UxPutSelectInsensitivePixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectInsensitivePixmap, res_value )

#define UxPutSelectPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectPixmap, res_value )

#define UxPutSelectThreshold( wgt, res_value ) \
	XtVaSetValues( wgt, XmNselectThreshold, res_value, NULL )

#define UxPutSelectedItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNselectedItemCount, res_value, NULL )

#define UxPutSelectedItems( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectedItems, res_value )

#define UxPutSelectionArray( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectionArray, res_value )

#define UxPutSelectionArrayCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNselectionArrayCount, res_value, NULL )

#define UxPutSelectionLabelString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectionLabelString, res_value )

#define UxPutSelectionPolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNselectionPolicy, res_value )

#define UxPutSensitive( wgt, res_value ) \
	UxPutStrRes( wgt, XmNsensitive, res_value )

#define UxPutSeparatorOn( wgt, res_value ) \
	UxPutStrRes( wgt, XmNseparatorOn, res_value )

#define UxPutSeparatorType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNseparatorType, res_value )

#define UxPutSet( wgt, res_value ) \
	UxPutStrRes( wgt, XmNset, res_value )

#define UxPutShadowThickness( wgt, res_value ) \
	XtVaSetValues( wgt, XmNshadowThickness, res_value, NULL )

#define UxPutShadowType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNshadowType, res_value )

#define UxPutShellUnitType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNshellUnitType, res_value )

#define UxPutShowArrows( wgt, res_value ) \
	UxPutStrRes( wgt, XmNshowArrows, res_value )

#define UxPutShowAsDefault( wgt, res_value ) \
	XtVaSetValues( wgt, XmNshowAsDefault, res_value, NULL )

#define UxPutShowSeparator( wgt, res_value ) \
	UxPutStrRes( wgt, XmNshowSeparator, res_value )

#define UxPutShowValue( wgt, res_value ) \
	UxPutStrRes( wgt, XmNshowValue, res_value )

#define UxPutSkipAdjust( wgt, res_value ) \
	UxPutStrRes( wgt, XmNskipAdjust, res_value )

#define UxPutSliderSize( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsliderSize, res_value, NULL )

#define UxPutSource( wgt, res_value ) \
	XtVaSetValues( wgt, XmNsource, res_value, NULL )

#define UxPutSpacing( wgt, res_value ) \
	XtVaSetValues( wgt, XmNspacing, res_value, NULL )

#define UxPutStringDirection( wgt, res_value ) \
	UxPutStrRes( wgt, XmNstringDirection, res_value )

#define UxPutSubMenuId( wgt, res_value ) \
	UxPutStrRes( wgt, XmNsubMenuId, res_value )

#define UxPutSymbolPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNsymbolPixmap, res_value )

#define UxPutText( wgt, res_value ) \
	XtVaSetValues( wgt, XmNvalue, res_value, NULL )

#define UxPutTextAccelerators( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtextAccelerators, res_value )

#define UxPutTextColumns( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtextColumns, res_value, NULL )

#define UxPutTextFontList( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtextFontList, res_value )

#define UxPutTextString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtextString, res_value )

#define UxPutTextTranslations( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtextTranslations, res_value )

#define UxPutTitle( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtitle, res_value, NULL )

#define UxPutTitleEncoding( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtitleEncoding, res_value )

#define UxPutTitleString( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtitleString, res_value )

#define UxPutTopAttachment( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtopAttachment, res_value )

#define UxPutTopCharacter( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtopCharacter, res_value, NULL )

#define UxPutTopItemPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtopItemPosition, res_value, NULL )

#define UxPutTopOffset( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtopOffset, res_value, NULL )

#define UxPutTopPosition( wgt, res_value ) \
	XtVaSetValues( wgt, XmNtopPosition, res_value, NULL )

#define UxPutTopShadowColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtopShadowColor, res_value )

#define UxPutTopShadowPixmap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtopShadowPixmap, res_value )

#define UxPutTopWidget( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtopWidget, res_value )

#define UxPutTransient( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtransient, res_value )

#define UxPutTransientFor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtransientFor, res_value )

#define UxPutTranslations( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtranslations, res_value )

#define UxPutTraversalOn( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtraversalOn, res_value )

#define UxPutTroughColor( wgt, res_value ) \
	UxPutStrRes( wgt, XmNtroughColor, res_value )

#define UxPutUnitType( wgt, res_value ) \
	UxPutStrRes( wgt, XmNunitType, res_value )

#define UxPutUseAsyncGeometry( wgt, res_value ) \
	UxPutStrRes( wgt, XmNuseAsyncGeometry, res_value )

#define UxPutUserData( wgt, res_value ) \
	XtVaSetValues( wgt, XmNuserData, res_value, NULL )

#define UxPutValue( wgt, res_value ) \
	XtVaSetValues( wgt, XmNvalue, res_value, NULL )

#define UxPutVerifyBell( wgt, res_value ) \
	UxPutStrRes( wgt, XmNverifyBell, res_value )

#define UxPutVerticalScrollBar( wgt, res_value ) \
	UxPutStrRes( wgt, XmNverticalScrollBar, res_value )

#define UxPutVerticalSpacing( wgt, res_value ) \
	XtVaSetValues( wgt, XmNverticalSpacing, res_value, NULL )

#define UxPutVisibleItemCount( wgt, res_value ) \
	XtVaSetValues( wgt, XmNvisibleItemCount, res_value, NULL )

#define UxPutVisibleWhenOff( wgt, res_value ) \
	UxPutStrRes( wgt, XmNvisibleWhenOff, res_value )

#define UxPutVisual( wgt, res_value ) \
	XtVaSetValues( wgt, XmNvisual, res_value, NULL )

#define UxPutVisualPolicy( wgt, res_value ) \
	UxPutStrRes( wgt, XmNvisualPolicy, res_value )

#define UxPutWaitForWm( wgt, res_value ) \
	UxPutStrRes( wgt, XmNwaitForWm, res_value )

#define UxPutWhichButton( wgt, res_value ) \
	XtVaSetValues( wgt, XmNwhichButton, res_value, NULL )

#define UxPutWidth( wgt, res_value ) \
	XtVaSetValues( wgt, XmNwidth, res_value, NULL )

#define UxPutWidthInc( wgt, res_value ) \
	XtVaSetValues( wgt, XmNwidthInc, res_value, NULL )

#define UxPutWinGravity( wgt, res_value ) \
	UxPutStrRes( wgt, XmNwinGravity, res_value )

#define UxPutWindowGroup( wgt, res_value ) \
	UxPutStrRes( wgt, XmNwindowGroup, res_value )

#define UxPutWmTimeout( wgt, res_value ) \
	XtVaSetValues( wgt, XmNwmTimeout, res_value, NULL )

#define UxPutWordWrap( wgt, res_value ) \
	UxPutStrRes( wgt, XmNwordWrap, res_value )

#define UxPutWorkWindow( wgt, res_value ) \
	UxPutStrRes( wgt, XmNworkWindow, res_value )

#define UxPutX( wgt, res_value ) \
	XtVaSetValues( wgt, XmNx, res_value, NULL )

#define UxPutY( wgt, res_value ) \
	XtVaSetValues( wgt, XmNy, res_value, NULL )

#endif /* UX_XT_PUTS_INCLUDED */

