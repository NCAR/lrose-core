
/******************************************************************************/
/*				UxXtGets.h				      */
/******************************************************************************/

#ifndef UX_XT_GETS_INCLUDED
#define UX_XT_GETS_INCLUDED

#include <Xm/Xm.h>

static	Boolean		UxBoolValue;
static	short		UxShortValue;
static	int		UxIntValue;
static	Position	UxPosValue;
static	Dimension	UxDimValue;
static	char		*UxStrValue;
static	XmString	UxXmStrValue;

/* The following macro can be used for Boolean resources: */

#define UxGetBoolStrRes( wgt, res_name ) \
	(XtVaGetValues( wgt, res_name, &UxBoolValue, NULL ), \
	 (UxBoolValue ? "true" : "false"))

/* The following macro can be used for XmString resources: */

#define UxGetXmStringRes( wgt, res_name ) \
	(XtVaGetValues( wgt, res_name, &UxXmStrValue, NULL ), \
	 (XmStringGetLtoR( UxXmStrValue, XmSTRING_DEFAULT_CHARSET, \
				&UxStrValue ) ? UxStrValue : NULL))


#define UxGetAccelerator( wgt ) \
	(XtVaGetValues( wgt, XmNaccelerator, &UxStrValue, NULL ), UxStrValue)

#define UxGetAcceleratorText( wgt ) \
	UxGetXmStrRes( wgt, XmNacceleratorText )

#define UxGetAccelerators( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetAdjustLast( wgt ) \
	UxGetBoolStrRes( wgt, XmNadjustLast )

#define UxGetAdjustMargin( wgt ) \
	UxGetBoolStrRes( wgt, XmNadjustMargin )

#define UxGetAlignment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetAllowOverlap( wgt ) \
	UxGetBoolStrRes( wgt, XmNallowOverlap )

#define UxGetAllowResize( wgt ) \
	UxGetBoolStrRes( wgt, XmNallowResize )

#define UxGetAllowShellResize( wgt ) \
	UxGetBoolStrRes( wgt, XmNallowShellResize )

#define UxGetAncestorSensitive( wgt ) \
	UxGetBoolStrRes( wgt, XmNancestorSensitive )

#define UxGetApplyLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNapplyLabelString )

#define UxGetArgc( wgt ) \
	(XtVaGetValues( wgt, XmNargc, &UxIntValue, NULL ), UxIntValue)

#define UxGetArgv( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetArmColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetArmPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetArrowDirection( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetAutoShowCursorPosition( wgt ) \
	UxGetBoolStrRes( wgt, XmNautoShowCursorPosition )

#define UxGetAutoUnmanage( wgt ) \
	UxGetBoolStrRes( wgt, XmNautoUnmanage )

#define UxGetAutomaticSelection( wgt ) \
	UxGetBoolStrRes( wgt, XmNautomaticSelection )

#define UxGetBackground( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBackgroundPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBaseHeight( wgt ) \
	(XtVaGetValues( wgt, XmNbaseHeight, &UxIntValue, NULL ), UxIntValue)

#define UxGetBaseWidth( wgt ) \
	(XtVaGetValues( wgt, XmNbaseWidth, &UxIntValue, NULL ), UxIntValue)

#define UxGetBlinkRate( wgt ) \
	(XtVaGetValues( wgt, XmNblinkRate, &UxIntValue, NULL ), UxIntValue)

#define UxGetBorderColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBorderPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBorderWidth( wgt ) \
	(XtVaGetValues( wgt, XmNborderWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetBottomAttachment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBottomOffset( wgt ) \
	(XtVaGetValues( wgt, XmNbottomOffset, &UxIntValue, NULL ), UxIntValue)

#define UxGetBottomPosition( wgt ) \
	(XtVaGetValues( wgt, XmNbottomPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetBottomShadowColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBottomShadowPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetBottomWidget( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetButtonFontList( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetCancelButton( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetCancelLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNcancelLabelString )

#define UxGetCascadePixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetChildren( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetClipWindow( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetColormap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetColumns( wgt ) \
	(XtVaGetValues( wgt, XmNcolumns, &UxShortValue, NULL ), UxShortValue)

#define UxGetCommand( wgt ) \
	UxGetXmStrRes( wgt, XmNcommand )

#define UxGetCommandWindow( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetCommandWindowLocation( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetCreatePopupChildProc( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetCursorPosition( wgt ) \
	(XtVaGetValues( wgt, XmNcursorPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetCursorPositionVisible( wgt ) \
	UxGetBoolStrRes( wgt, XmNcursorPositionVisible )

#define UxGetDecimalPoints( wgt ) \
	(XtVaGetValues( wgt, XmNdecimalPoints, &UxShortValue, NULL ), UxShortValue)

#define UxGetDefaultButton( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDefaultButtonShadowThickness( wgt ) \
	(XtVaGetValues( wgt, XmNdefaultButtonShadowThickness, &UxDimValue, NULL ), UxDimValue)

#define UxGetDefaultButtonType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDefaultFontList( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDefaultPosition( wgt ) \
	UxGetBoolStrRes( wgt, XmNdefaultPosition )

#define UxGetDeleteResponse( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDialogStyle( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDialogTitle( wgt ) \
	UxGetXmStrRes( wgt, XmNdialogTitle )

#define UxGetDialogType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDirListItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNdirListItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetDirListItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDirListLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNdirListLabelString )

#define UxGetDirMask( wgt ) \
	UxGetXmStrRes( wgt, XmNdirMask )

#define UxGetDirSearchProc( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetDirSpec( wgt ) \
	UxGetXmStrRes( wgt, XmNdirSpec )

#define UxGetDirectory( wgt ) \
	UxGetXmStrRes( wgt, XmNdirectory )

#define UxGetDirectoryValid( wgt ) \
	UxGetBoolStrRes( wgt, XmNdirectoryValid )

#define UxGetDoubleClickInterval( wgt ) \
	(XtVaGetValues( wgt, XmNdoubleClickInterval, &UxIntValue, NULL ), UxIntValue)

#define UxGetEditMode( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetEditable( wgt ) \
	UxGetBoolStrRes( wgt, XmNeditable )

#define UxGetEntryAlignment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetEntryBorder( wgt ) \
	(XtVaGetValues( wgt, XmNentryBorder, &UxDimValue, NULL ), UxDimValue)

#define UxGetEntryClass( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetFileListItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNfileListItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetFileListItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetFileListLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNfileListLabelString )

#define UxGetFileSearchProc( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetFileTypeMask( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetFillOnArm( wgt ) \
	UxGetBoolStrRes( wgt, XmNfillOnArm )

#define UxGetFillOnSelect( wgt ) \
	UxGetBoolStrRes( wgt, XmNfillOnSelect )

#define UxGetFilterLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNfilterLabelString )

#define UxGetFontList( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetForeground( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetFractionBase( wgt ) \
	(XtVaGetValues( wgt, XmNfractionBase, &UxIntValue, NULL ), UxIntValue)

#define UxGetGeometry( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetHeight( wgt ) \
	(XtVaGetValues( wgt, XmNheight, &UxDimValue, NULL ), UxDimValue)

#define UxGetHeightInc( wgt ) \
	(XtVaGetValues( wgt, XmNheightInc, &UxIntValue, NULL ), UxIntValue)

#define UxGetHelpLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNhelpLabelString )

#define UxGetHighlightColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetHighlightOnEnter( wgt ) \
	UxGetBoolStrRes( wgt, XmNhighlightOnEnter )

#define UxGetHighlightPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetHighlightThickness( wgt ) \
	(XtVaGetValues( wgt, XmNhighlightThickness, &UxDimValue, NULL ), UxDimValue)

#define UxGetHistoryItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNhistoryItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetHistoryItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetHistoryMaxItems( wgt ) \
	(XtVaGetValues( wgt, XmNhistoryMaxItems, &UxIntValue, NULL ), UxIntValue)

#define UxGetHistoryVisibleItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNhistoryVisibleItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetHorizontalScrollBar( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetHorizontalSpacing( wgt ) \
	(XtVaGetValues( wgt, XmNhorizontalSpacing, &UxDimValue, NULL ), UxDimValue)

#define UxGetIconMask( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetIconName( wgt ) \
	(XtVaGetValues( wgt, XmNiconName, &UxStrValue, NULL ), UxStrValue)

#define UxGetIconNameEncoding( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetIconPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetIconWindow( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetIconX( wgt ) \
	(XtVaGetValues( wgt, XmNiconX, &UxIntValue, NULL ), UxIntValue)

#define UxGetIconY( wgt ) \
	(XtVaGetValues( wgt, XmNiconY, &UxIntValue, NULL ), UxIntValue)

#define UxGetIconic( wgt ) \
	UxGetBoolStrRes( wgt, XmNiconic )

#define UxGetIncrement( wgt ) \
	(XtVaGetValues( wgt, XmNincrement, &UxIntValue, NULL ), UxIntValue)

#define UxGetIndicatorOn( wgt ) \
	UxGetBoolStrRes( wgt, XmNindicatorOn )

#define UxGetIndicatorSize( wgt ) \
	(XtVaGetValues( wgt, XmNindicatorSize, &UxDimValue, NULL ), UxDimValue)

#define UxGetIndicatorType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetInitialDelay( wgt ) \
	(XtVaGetValues( wgt, XmNinitialDelay, &UxIntValue, NULL ), UxIntValue)

#define UxGetInitialState( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetInput( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetInsertPosition( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetIsAligned( wgt ) \
	UxGetBoolStrRes( wgt, XmNisAligned )

#define UxGetIsHomogeneous( wgt ) \
	UxGetBoolStrRes( wgt, XmNisHomogeneous )

#define UxGetItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNitemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetKeyboardFocusPolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLabelFontList( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLabelInsensitivePixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLabelPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNlabelString )

#define UxGetLabelType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLeftAttachment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetLeftOffset( wgt ) \
	(XtVaGetValues( wgt, XmNleftOffset, &UxIntValue, NULL ), UxIntValue)

#define UxGetLeftPosition( wgt ) \
	(XtVaGetValues( wgt, XmNleftPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetLeftWidget( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetListItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNlistItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetListItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetListLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNlistLabelString )

#define UxGetListMarginHeight( wgt ) \
	(XtVaGetValues( wgt, XmNlistMarginHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetListMarginWidth( wgt ) \
	(XtVaGetValues( wgt, XmNlistMarginWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetListSizePolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetListSpacing( wgt ) \
	(XtVaGetValues( wgt, XmNlistSpacing, &UxDimValue, NULL ), UxDimValue)

#define UxGetListUpdated( wgt ) \
	UxGetBoolStrRes( wgt, XmNlistUpdated )

#define UxGetListVisibleItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNlistVisibleItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetMainWindowMarginHeight( wgt ) \
	(XtVaGetValues( wgt, XmNmainWindowMarginHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetMainWindowMarginWidth( wgt ) \
	(XtVaGetValues( wgt, XmNmainWindowMarginWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetMappedWhenManaged( wgt ) \
	UxGetBoolStrRes( wgt, XmNmappedWhenManaged )

#define UxGetMappingDelay( wgt ) \
	(XtVaGetValues( wgt, XmNmappingDelay, &UxIntValue, NULL ), UxIntValue)

#define UxGetMargin( wgt ) \
	(XtVaGetValues( wgt, XmNmargin, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginBottom( wgt ) \
	(XtVaGetValues( wgt, XmNmarginBottom, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginHeight( wgt ) \
	(XtVaGetValues( wgt, XmNmarginHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginLeft( wgt ) \
	(XtVaGetValues( wgt, XmNmarginLeft, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginRight( wgt ) \
	(XtVaGetValues( wgt, XmNmarginRight, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginTop( wgt ) \
	(XtVaGetValues( wgt, XmNmarginTop, &UxDimValue, NULL ), UxDimValue)

#define UxGetMarginWidth( wgt ) \
	(XtVaGetValues( wgt, XmNmarginWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetMaxAspectX( wgt ) \
	(XtVaGetValues( wgt, XmNmaxAspectX, &UxIntValue, NULL ), UxIntValue)

#define UxGetMaxAspectY( wgt ) \
	(XtVaGetValues( wgt, XmNmaxAspectY, &UxIntValue, NULL ), UxIntValue)

#define UxGetMaxHeight( wgt ) \
	(XtVaGetValues( wgt, XmNmaxHeight, &UxIntValue, NULL ), UxIntValue)

#define UxGetMaxLength( wgt ) \
	(XtVaGetValues( wgt, XmNmaxLength, &UxIntValue, NULL ), UxIntValue)

#define UxGetMaxWidth( wgt ) \
	(XtVaGetValues( wgt, XmNmaxWidth, &UxIntValue, NULL ), UxIntValue)

#define UxGetMaximum( wgt ) \
	(XtVaGetValues( wgt, XmNmaximum, &UxIntValue, NULL ), UxIntValue)

#define UxGetMenuAccelerator( wgt ) \
	(XtVaGetValues( wgt, XmNmenuAccelerator, &UxStrValue, NULL ), UxStrValue)

#define UxGetMenuBar( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMenuHelpWidget( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMenuHistory( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMenuPost( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMessageAlignment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMessageString( wgt ) \
	UxGetXmStrRes( wgt, XmNmessageString )

#define UxGetMessageWindow( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMinAspectX( wgt ) \
	(XtVaGetValues( wgt, XmNminAspectX, &UxIntValue, NULL ), UxIntValue)

#define UxGetMinAspectY( wgt ) \
	(XtVaGetValues( wgt, XmNminAspectY, &UxIntValue, NULL ), UxIntValue)

#define UxGetMinHeight( wgt ) \
	(XtVaGetValues( wgt, XmNminHeight, &UxIntValue, NULL ), UxIntValue)

#define UxGetMinWidth( wgt ) \
	(XtVaGetValues( wgt, XmNminWidth, &UxIntValue, NULL ), UxIntValue)

#define UxGetMinimizeButtons( wgt ) \
	UxGetBoolStrRes( wgt, XmNminimizeButtons )

#define UxGetMinimum( wgt ) \
	(XtVaGetValues( wgt, XmNminimum, &UxIntValue, NULL ), UxIntValue)

#define UxGetMnemonic( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMnemonicCharSet( wgt ) \
	(XtVaGetValues( wgt, XmNmnemonicCharSet, &UxStrValue, NULL ), UxStrValue)

#define UxGetMultiClick( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetMustMatch( wgt ) \
	UxGetBoolStrRes( wgt, XmNmustMatch )

#define UxGetMwmDecorations( wgt ) \
	(XtVaGetValues( wgt, XmNmwmDecorations, &UxIntValue, NULL ), UxIntValue)

#define UxGetMwmFunctions( wgt ) \
	(XtVaGetValues( wgt, XmNmwmFunctions, &UxIntValue, NULL ), UxIntValue)

#define UxGetMwmInputMode( wgt ) \
	(XtVaGetValues( wgt, XmNmwmInputMode, &UxIntValue, NULL ), UxIntValue)

#define UxGetMwmMenu( wgt ) \
	(XtVaGetValues( wgt, XmNmwmMenu, &UxStrValue, NULL ), UxStrValue)

#define UxGetNavigationType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetNoMatchString( wgt ) \
	UxGetXmStrRes( wgt, XmNnoMatchString )

#define UxGetNoResize( wgt ) \
	UxGetBoolStrRes( wgt, XmNnoResize )

#define UxGetNumChildren( wgt ) \
	(XtVaGetValues( wgt, XmNnumChildren, &UxIntValue, NULL ), UxIntValue)

#define UxGetNumColumns( wgt ) \
	(XtVaGetValues( wgt, XmNnumColumns, &UxShortValue, NULL ), UxShortValue)

#define UxGetOkLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNokLabelString )

#define UxGetOrientation( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetOverrideRedirect( wgt ) \
	UxGetBoolStrRes( wgt, XmNoverrideRedirect )

#define UxGetPacking( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetPageIncrement( wgt ) \
	(XtVaGetValues( wgt, XmNpageIncrement, &UxIntValue, NULL ), UxIntValue)

#define UxGetPaneMaximum( wgt ) \
	(XtVaGetValues( wgt, XmNpaneMaximum, &UxDimValue, NULL ), UxDimValue)

#define UxGetPaneMinimum( wgt ) \
	(XtVaGetValues( wgt, XmNpaneMinimum, &UxDimValue, NULL ), UxDimValue)

#define UxGetPattern( wgt ) \
	UxGetXmStrRes( wgt, XmNpattern )

#define UxGetPendingDelete( wgt ) \
	UxGetBoolStrRes( wgt, XmNpendingDelete )

#define UxGetPopupEnabled( wgt ) \
	UxGetBoolStrRes( wgt, XmNpopupEnabled )

#define UxGetProcessingDirection( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetPromptString( wgt ) \
	UxGetXmStrRes( wgt, XmNpromptString )

#define UxGetPushButtonEnabled( wgt ) \
	UxGetBoolStrRes( wgt, XmNpushButtonEnabled )

#define UxGetQualifySearchDataProc( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetRadioAlwaysOne( wgt ) \
	UxGetBoolStrRes( wgt, XmNradioAlwaysOne )

#define UxGetRadioBehavior( wgt ) \
	UxGetBoolStrRes( wgt, XmNradioBehavior )

#define UxGetRecomputeSize( wgt ) \
	UxGetBoolStrRes( wgt, XmNrecomputeSize )

#define UxGetRefigureMode( wgt ) \
	UxGetBoolStrRes( wgt, XmNrefigureMode )

#define UxGetRepeatDelay( wgt ) \
	(XtVaGetValues( wgt, XmNrepeatDelay, &UxIntValue, NULL ), UxIntValue)

#define UxGetResizable( wgt ) \
	UxGetBoolStrRes( wgt, XmNresizable )

#define UxGetResizeHeight( wgt ) \
	UxGetBoolStrRes( wgt, XmNresizeHeight )

#define UxGetResizePolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetResizeWidth( wgt ) \
	UxGetBoolStrRes( wgt, XmNresizeWidth )

#define UxGetRightAttachment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetRightOffset( wgt ) \
	(XtVaGetValues( wgt, XmNrightOffset, &UxIntValue, NULL ), UxIntValue)

#define UxGetRightPosition( wgt ) \
	(XtVaGetValues( wgt, XmNrightPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetRightWidget( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetRowColumnType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetRows( wgt ) \
	(XtVaGetValues( wgt, XmNrows, &UxShortValue, NULL ), UxShortValue)

#define UxGetRubberPositioning( wgt ) \
	UxGetBoolStrRes( wgt, XmNrubberPositioning )

#define UxGetSashHeight( wgt ) \
	(XtVaGetValues( wgt, XmNsashHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetSashIndent( wgt ) \
	(XtVaGetValues( wgt, XmNsashIndent, &UxPosValue, NULL ), UxPosValue)

#define UxGetSashShadowThickness( wgt ) \
	(XtVaGetValues( wgt, XmNsashShadowThickness, &UxDimValue, NULL ), UxDimValue)

#define UxGetSashWidth( wgt ) \
	(XtVaGetValues( wgt, XmNsashWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetSaveUnder( wgt ) \
	UxGetBoolStrRes( wgt, XmNsaveUnder )

#define UxGetScaleHeight( wgt ) \
	(XtVaGetValues( wgt, XmNscaleHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetScaleMultiple( wgt ) \
	(XtVaGetValues( wgt, XmNscaleMultiple, &UxIntValue, NULL ), UxIntValue)

#define UxGetScaleWidth( wgt ) \
	(XtVaGetValues( wgt, XmNscaleWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetScrollBarDisplayPolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetScrollBarPlacement( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetScrollHorizontal( wgt ) \
	UxGetBoolStrRes( wgt, XmNscrollHorizontal )

#define UxGetScrollLeftSide( wgt ) \
	UxGetBoolStrRes( wgt, XmNscrollLeftSide )

#define UxGetScrollTopSide( wgt ) \
	UxGetBoolStrRes( wgt, XmNscrollTopSide )

#define UxGetScrollVertical( wgt ) \
	UxGetBoolStrRes( wgt, XmNscrollVertical )

#define UxGetScrolledWindowMarginHeight( wgt ) \
	(XtVaGetValues( wgt, XmNscrolledWindowMarginHeight, &UxDimValue, NULL ), UxDimValue)

#define UxGetScrolledWindowMarginWidth( wgt ) \
	(XtVaGetValues( wgt, XmNscrolledWindowMarginWidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetScrollingPolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectInsensitivePixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectThreshold( wgt ) \
	(XtVaGetValues( wgt, XmNselectThreshold, &UxIntValue, NULL ), UxIntValue)

#define UxGetSelectedItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNselectedItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetSelectedItems( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectionArray( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSelectionArrayCount( wgt ) \
	(XtVaGetValues( wgt, XmNselectionArrayCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetSelectionLabelString( wgt ) \
	UxGetXmStrRes( wgt, XmNselectionLabelString )

#define UxGetSelectionPolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSensitive( wgt ) \
	UxGetBoolStrRes( wgt, XmNsensitive )

#define UxGetSeparatorOn( wgt ) \
	UxGetBoolStrRes( wgt, XmNseparatorOn )

#define UxGetSeparatorType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSet( wgt ) \
	UxGetBoolStrRes( wgt, XmNset )

#define UxGetShadowThickness( wgt ) \
	(XtVaGetValues( wgt, XmNshadowThickness, &UxDimValue, NULL ), UxDimValue)

#define UxGetShadowType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetShellUnitType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetShowArrows( wgt ) \
	UxGetBoolStrRes( wgt, XmNshowArrows )

#define UxGetShowAsDefault( wgt ) \
	(XtVaGetValues( wgt, XmNshowAsDefault, &UxDimValue, NULL ), UxDimValue)

#define UxGetShowSeparator( wgt ) \
	UxGetBoolStrRes( wgt, XmNshowSeparator )

#define UxGetShowValue( wgt ) \
	UxGetBoolStrRes( wgt, XmNshowValue )

#define UxGetSkipAdjust( wgt ) \
	UxGetBoolStrRes( wgt, XmNskipAdjust )

#define UxGetSliderSize( wgt ) \
	(XtVaGetValues( wgt, XmNsliderSize, &UxIntValue, NULL ), UxIntValue)

#define UxGetSource( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSpacing( wgt ) \
	(XtVaGetValues( wgt, XmNspacing, &UxDimValue, NULL ), UxDimValue)

#define UxGetStringDirection( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSubMenuId( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetSymbolPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetText( wgt ) \
	(XmIsTextField(wgt) ? (char *)XmTextFieldGetString(wgt) : (char *)XmTextGetString(wgt))

#define UxGetTextAccelerators( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTextColumns( wgt ) \
	(XtVaGetValues( wgt, XmNtextColumns, &UxShortValue, NULL ), UxShortValue)

#define UxGetTextFontList( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTextString( wgt ) \
	UxGetXmStrRes( wgt, XmNtextString )

#define UxGetTextTranslations( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTitle( wgt ) \
	(XtVaGetValues( wgt, XmNtitle, &UxStrValue, NULL ), UxStrValue)

#define UxGetTitleEncoding( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTitleString( wgt ) \
	UxGetXmStrRes( wgt, XmNtitleString )

#define UxGetTopAttachment( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTopCharacter( wgt ) \
	(XtVaGetValues( wgt, XmNtopCharacter, &UxIntValue, NULL ), UxIntValue)

#define UxGetTopItemPosition( wgt ) \
	(XtVaGetValues( wgt, XmNtopItemPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetTopOffset( wgt ) \
	(XtVaGetValues( wgt, XmNtopOffset, &UxIntValue, NULL ), UxIntValue)

#define UxGetTopPosition( wgt ) \
	(XtVaGetValues( wgt, XmNtopPosition, &UxIntValue, NULL ), UxIntValue)

#define UxGetTopShadowColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTopShadowPixmap( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTopWidget( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTransient( wgt ) \
	UxGetBoolStrRes( wgt, XmNtransient )

#define UxGetTransientFor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTranslations( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetTraversalOn( wgt ) \
	UxGetBoolStrRes( wgt, XmNtraversalOn )

#define UxGetTroughColor( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetUnitType( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetUseAsyncGeometry( wgt ) \
	UxGetBoolStrRes( wgt, XmNuseAsyncGeometry )

#define UxGetUserData( wgt ) \
	(XtVaGetValues( wgt, XmNuserData, &UxStrValue, NULL ), UxStrValue)

#define UxGetValue( wgt ) \
	(XtVaGetValues( wgt, XmNvalue, &UxIntValue, NULL ), UxIntValue)

#define UxGetVerifyBell( wgt ) \
	UxGetBoolStrRes( wgt, XmNverifyBell )

#define UxGetVerticalScrollBar( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetVerticalSpacing( wgt ) \
	(XtVaGetValues( wgt, XmNverticalSpacing, &UxDimValue, NULL ), UxDimValue)

#define UxGetVisibleItemCount( wgt ) \
	(XtVaGetValues( wgt, XmNvisibleItemCount, &UxIntValue, NULL ), UxIntValue)

#define UxGetVisibleWhenOff( wgt ) \
	UxGetBoolStrRes( wgt, XmNvisibleWhenOff )

#define UxGetVisual( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetVisualPolicy( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetWaitForWm( wgt ) \
	UxGetBoolStrRes( wgt, XmNwaitForWm )

#define UxGetWhichButton( wgt ) \
	(XtVaGetValues( wgt, XmNwhichButton, &UxIntValue, NULL ), UxIntValue)

#define UxGetWidth( wgt ) \
	(XtVaGetValues( wgt, XmNwidth, &UxDimValue, NULL ), UxDimValue)

#define UxGetWidthInc( wgt ) \
	(XtVaGetValues( wgt, XmNwidthInc, &UxIntValue, NULL ), UxIntValue)

#define UxGetWinGravity( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetWindowGroup( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetWmTimeout( wgt ) \
	(XtVaGetValues( wgt, XmNwmTimeout, &UxIntValue, NULL ), UxIntValue)

#define UxGetWordWrap( wgt ) \
	UxGetBoolStrRes( wgt, XmNwordWrap )

#define UxGetWorkWindow( wgt ) \
	NOT_SUPPORTED_IN_XT_CODE

#define UxGetX( wgt ) \
	(XtVaGetValues( wgt, XmNx, &UxPosValue, NULL ), UxPosValue)

#define UxGetY( wgt ) \
	(XtVaGetValues( wgt, XmNy, &UxPosValue, NULL ), UxPosValue)

#endif /* UX_XT_GETS_INCLUDED */

