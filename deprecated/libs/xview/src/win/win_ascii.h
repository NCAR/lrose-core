/*	@(#)win_ascii.h 1.1 89/11/20 SMI	*/

/* 
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/* win_ascii_data.h -- pre-parsed keymap data */

#ifndef win_ascii_data_DEFINED
#define win_ascii_data_DEFINED

unsigned char		win_ascii_sem[] = {

/*
 *		No Modifiers   (Range 0-255)
 */
	NULL,
	NULL,						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	NULL,						/* XK_a               */
	NULL,						/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	NULL,						/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Control Modifier   (Range 256-511)
 */
	NULL,
	NULL,						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	ACTION_SELECT & 0xFF,				/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	ACTION_GO_LINE_FORWARD & 0xFF,			/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	ACTION_GO_WORD_BACKWARD & 0xFF,			/* XK_comma           */
	NULL,						/* XK_minus           */
	ACTION_GO_WORD_END & 0xFF,			/* XK_period          */
	ACTION_GO_WORD_FORWARD & 0xFF,			/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	ACTION_GO_WORD_FORWARD & 0xFF,			/* XK_less            */
	NULL,						/* XK_equal           */
	ACTION_GO_WORD_BACKWARD & 0xFF,			/* XK_greater         */
	ACTION_INPUT_FOCUS_HELP & 0xFF,			/* XK_question        */
	NULL,						/* XK_at              */
	ACTION_GO_LINE_END & 0xFF,			/* XK_A               */
	ACTION_GO_CHAR_FORWARD & 0xFF,			/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	ACTION_GO_LINE_BACKWARD & 0xFF,			/* XK_E               */
	ACTION_GO_CHAR_BACKWARD & 0xFF,			/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	ACTION_GO_COLUMN_BACKWARD & 0xFF,		/* XK_N               */
	NULL,						/* XK_O               */
	ACTION_GO_COLUMN_FORWARD & 0xFF,		/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	ACTION_ERASE_LINE_END & 0xFF,			/* XK_U               */
	NULL,						/* XK_V               */
	ACTION_ERASE_WORD_FORWARD & 0xFF,		/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	ACTION_PANEL_START & 0xFF,			/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	ACTION_PANEL_END & 0xFF,			/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	ACTION_GO_LINE_BACKWARD & 0xFF,			/* XK_a               */
	ACTION_GO_CHAR_BACKWARD & 0xFF,			/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	ACTION_GO_LINE_END & 0xFF,			/* XK_e               */
	ACTION_GO_CHAR_FORWARD & 0xFF,			/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	ACTION_GO_COLUMN_FORWARD & 0xFF,		/* XK_n               */
	NULL,						/* XK_o               */
	ACTION_GO_COLUMN_BACKWARD & 0xFF,		/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	ACTION_ERASE_LINE_BACKWARD & 0xFF,		/* XK_u               */
	NULL,						/* XK_v               */
	ACTION_ERASE_WORD_BACKWARD & 0xFF,		/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Meta Modifier   (Range 512-767)
 */
	NULL,
	NULL, 						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	ACTION_FIND_BACKWARD & 0xFF,			/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	ACTION_AGAIN & 0xFF,				/* XK_a               */
	NULL,						/* XK_b               */
	ACTION_COPY & 0xFF,				/* XK_c               */
	ACTION_MATCH_DELIMITER & 0xFF,			/* XK_d               */
	ACTION_EMPTY & 0xFF,				/* XK_e               */
	ACTION_FIND_FORWARD & 0xFF,			/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	ACTION_INCLUDE_FILE & 0xFF,			/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	ACTION_LOAD & 0xFF,				/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	ACTION_COPY_THEN_PASTE & 0xFF,			/* XK_p               */
	ACTION_QUOTE & 0xFF,				/* XK_q               */
	NULL,						/* XK_r               */
	ACTION_STORE & 0xFF,				/* XK_s               */
	NULL,						/* XK_t               */
	ACTION_UNDO & 0xFF,				/* XK_u               */
	ACTION_PASTE & 0xFF,				/* XK_v               */
	NULL,						/* XK_w               */
	ACTION_CUT & 0xFF,				/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Control/Meta Modifier   (Range 768-1023)
 */
	NULL,
	NULL, 						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	ACTION_AGAIN & 0xFF,				/* XK_a               */
	NULL,						/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	ACTION_EMPTY & 0xFF,				/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Alt Modifier   (Range 1024-1279)
 */
	NULL,
	NULL,						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	ACTION_ADJUST & 0xFF,				/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	NULL,						/* XK_a               */
	ACTION_GO_BACKGROUND & 0xFF,			/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	NULL,						/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	ACTION_JUMP_MOUSE_TO_INPUT_FOCUS & 0xFF,	/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	ACTION_MENU & 0xFF,				/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	ACTION_QUOTE_NEXT_KEY & 0xFF,			/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Control/Alt Modifier   (Range 1280-1535)
 */
	NULL,
	NULL,						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	ACTION_RESET_KEYMAP & 0xFF,			/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	NULL,						/* XK_a               */
	NULL,						/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	NULL,						/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Meta/Alt Modifier   (Range 1536-1791)
 */
	NULL,
	NULL, 						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	NULL,						/* XK_a               */
	NULL,						/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	NULL,						/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL, 						/* XK_ydiaeresis      */
/*
 *		Control/Meta/Alt Modifier   (Range 1792-2047)
 */
	NULL,
	NULL, 						/* Control A          */
	NULL, 						/* Control B          */
	NULL, 						/* Control C          */
	NULL, 						/* Control D          */
	NULL, 						/* Control E          */
	NULL, 						/* Control F          */
	NULL, 						/* Control G          */
	NULL, 						/* Control H          */
	NULL, 						/* Control I          */
	NULL, 						/* Control J          */
	NULL, 						/* Control K          */
	NULL, 						/* Control L          */
	NULL, 						/* Control M          */
	NULL, 						/* Control N          */
	NULL, 						/* Control O          */
	NULL, 						/* Control P          */
	NULL, 						/* Control Q          */
	NULL, 						/* Control R          */
	NULL, 						/* Control S          */
	NULL, 						/* Control T          */
	NULL, 						/* Control U          */
	NULL, 						/* Control V          */
	NULL, 						/* Control W          */
	NULL, 						/* Control X          */
	NULL, 						/* Control Y          */
	NULL, 						/* Control Z          */
	NULL, 						/* Esc                */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,						/* XK_space           */
	NULL,						/* XK_exclam          */
	NULL,						/* XK_quotedbl        */
	NULL,						/* XK_numbersign      */
	NULL,						/* XK_dollar          */
	NULL,						/* XK_percent         */
	NULL,						/* XK_ampersand       */
	NULL,						/* XK_apostrophe      */
	NULL,						/* XK_parenleft       */
	NULL,						/* XK_parenright      */
	NULL,						/* XK_asterisk        */
	NULL,						/* XK_plus            */
	NULL,						/* XK_comma           */
	NULL,						/* XK_minus           */
	NULL,						/* XK_period          */
	NULL,						/* XK_slash           */
	NULL,						/* XK_0               */
	NULL,						/* XK_1               */
	NULL,						/* XK_2               */
	NULL,						/* XK_3               */
	NULL,						/* XK_4               */
	NULL,						/* XK_5               */
	NULL,						/* XK_6               */
	NULL,						/* XK_7               */
	NULL,						/* XK_8               */
	NULL,						/* XK_9               */
	NULL,						/* XK_colon           */
	NULL,						/* XK_semicolon       */
	NULL,						/* XK_less            */
	NULL,						/* XK_equal           */
	NULL,						/* XK_greater         */
	NULL,						/* XK_question        */
	NULL,						/* XK_at              */
	NULL,						/* XK_A               */
	NULL,						/* XK_B               */
	NULL,						/* XK_C               */
	NULL,						/* XK_D               */
	NULL,						/* XK_E               */
	NULL,						/* XK_F               */
	NULL,						/* XK_G               */
	NULL,						/* XK_H               */
	NULL,						/* XK_I               */
	NULL,						/* XK_J               */
	NULL,						/* XK_K               */
	NULL,						/* XK_L               */
	NULL,						/* XK_M               */
	NULL,						/* XK_N               */
	NULL,						/* XK_O               */
	NULL,						/* XK_P               */
	NULL,						/* XK_Q               */
	NULL,						/* XK_R               */
	NULL,						/* XK_S               */
	NULL,						/* XK_T               */
	NULL,						/* XK_U               */
	NULL,						/* XK_V               */
	NULL,						/* XK_W               */
	NULL,						/* XK_X               */
	NULL,						/* XK_Y               */
	NULL,						/* XK_Z               */
	NULL,						/* XK_bracketleft     */
	NULL,						/* XK_backslash       */
	NULL,						/* XK_bracketright    */
	NULL,						/* XK_asciicircum     */
	NULL,						/* XK_underscore      */
	NULL,						/* XK_grave           */
	NULL,						/* XK_a               */
	NULL,						/* XK_b               */
	NULL,						/* XK_c               */
	NULL,						/* XK_d               */
	NULL,						/* XK_e               */
	NULL,						/* XK_f               */
	NULL,						/* XK_g               */
	NULL,						/* XK_h               */
	NULL,						/* XK_i               */
	NULL,						/* XK_j               */
	NULL,						/* XK_k               */
	NULL,						/* XK_l               */
	NULL,						/* XK_m               */
	NULL,						/* XK_n               */
	NULL,						/* XK_o               */
	NULL,						/* XK_p               */
	NULL,						/* XK_q               */
	NULL,						/* XK_r               */
	NULL,						/* XK_s               */
	NULL,						/* XK_t               */
	NULL,						/* XK_u               */
	NULL,						/* XK_v               */
	NULL,						/* XK_w               */
	NULL,						/* XK_x               */
	NULL,						/* XK_y               */
	NULL,						/* XK_z               */
	NULL,						/* XK_braceleft       */
	NULL,						/* XK_bar             */
	NULL,						/* XK_braceright      */
	NULL,						/* XK_asciitilde      */

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL,

	NULL,						/* XK_nobreakspace    */
	NULL,						/* XK_exclamdown      */
	NULL,						/* XK_cent            */
	NULL,						/* XK_sterling        */
	NULL,						/* XK_currency        */
	NULL,						/* XK_yen             */
	NULL,						/* XK_brokenbar       */
	NULL,						/* XK_section         */
	NULL,						/* XK_diaeresis       */
	NULL,						/* XK_copyright       */
	NULL,						/* XK_ordfeminine     */
	NULL,						/* XK_guillemotleft   */	
	NULL,						/* XK_notsign         */
	NULL,						/* XK_hyphen          */
	NULL,						/* XK_registered      */
	NULL,						/* XK_macron          */
	NULL,						/* XK_degree          */
	NULL,						/* XK_plusminus       */
	NULL,						/* XK_twosuperior     */
	NULL,						/* XK_threesuperior   */
	NULL,						/* XK_acute           */
	NULL,						/* XK_mu              */
	NULL,						/* XK_paragraph       */
	NULL,						/* XK_periodcentered  */
	NULL,						/* XK_cedilla         */
	NULL,						/* XK_onesuperior     */
	NULL,						/* XK_masculine       */
	NULL,						/* XK_guillemotright  */
	NULL,						/* XK_onequarter      */
	NULL,						/* XK_onehalf         */
	NULL,						/* XK_threequarters   */
	NULL,						/* XK_questiondown    */
	NULL,						/* XK_Agrave          */
	NULL,						/* XK_Aacute          */
	NULL,						/* XK_Acircumflex     */
	NULL,						/* XK_Atilde          */
	NULL,						/* XK_Adiaeresis      */
	NULL,						/* XK_Aring           */
	NULL,						/* XK_AE              */
	NULL,						/* XK_Ccedilla        */
	NULL,						/* XK_Egrave          */
	NULL,						/* XK_Eacute          */
	NULL,						/* XK_Ecircumflex     */
	NULL,						/* XK_Ediaeresis      */
	NULL,						/* XK_Igrave          */
	NULL,						/* XK_Iacute          */
	NULL,						/* XK_Icircumflex     */
	NULL,						/* XK_Idiaeresis      */
	NULL,						/* XK_ETH             */
	NULL,						/* XK_Ntilde          */
	NULL,						/* XK_Ograve          */
	NULL,						/* XK_Oacute          */
	NULL,						/* XK_Ocircumflex     */
	NULL,						/* XK_Otilde          */
	NULL,						/* XK_Odiaeresis      */
	NULL,						/* XK_multiply        */
	NULL,						/* XK_Ooblique        */
	NULL,						/* XK_Ugrave          */
	NULL,						/* XK_Uacute          */
	NULL,						/* XK_Ucircumflex     */
	NULL,						/* XK_Udiaeresis      */
	NULL,						/* XK_Yacute          */
	NULL,						/* XK_THORN           */
	NULL,						/* XK_ssharp          */
	NULL,						/* XK_agrave          */
	NULL,						/* XK_aacute          */
	NULL,						/* XK_acircumflex     */
	NULL,						/* XK_atilde          */
	NULL,						/* XK_adiaeresis      */
	NULL,						/* XK_aring           */
	NULL,						/* XK_ae              */
	NULL,						/* XK_ccedilla        */
	NULL,						/* XK_egrave          */
	NULL,						/* XK_eacute          */
	NULL,						/* XK_ecircumflex     */
	NULL,						/* XK_ediaeresis      */
	NULL,						/* XK_igrave          */
	NULL,						/* XK_iacute          */
	NULL,						/* XK_icircumflex     */
	NULL,						/* XK_idiaeresis      */
	NULL,						/* XK_eth             */
	NULL,						/* XK_ntilde          */
	NULL,						/* XK_ograve          */
	NULL,						/* XK_oacute          */
	NULL,						/* XK_ocircumflex     */
	NULL,						/* XK_otilde          */
	NULL,						/* XK_odiaeresis      */
	NULL,						/* XK_division        */
	NULL,						/* XK_oslash          */
	NULL,						/* XK_ugrave          */
	NULL,						/* XK_uacute          */
	NULL,						/* XK_ucircumflex     */
	NULL,						/* XK_udiaeresis      */
	NULL,						/* XK_yacute          */
	NULL,						/* XK_thorn           */
	NULL 						/* XK_ydiaeresis      */
};


#endif win_ascii_data_DEFINED
