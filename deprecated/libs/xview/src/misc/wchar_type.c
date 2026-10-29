#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)wchar_type.c 1.8 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#include <xview_private/i18n_impl.h> 
#include <xview/pkg.h> 
 

/*  Warning: this routine is very Japanese locale dependent
 */

Xv_private int
wchar_type(wc)
        wchar_t *wc;
{

        /*
         * Returns:
         *      1 for ASCII word character (alphanumeric and '_')
         *      2 for Kanji English alphabet/Numeric
         *      3 for Hiragana
         *      4 for Katakana, Codeset 1
         *      5 for Kanji
         *      6 for Greek alphabet
         *      7 for Russiun alphabet
         *      8 for anything from codeset 2 (Hankaku Katakana)
         *      9 for anything from codeset 3 (Gaiji)
         *      0 for anything else
         */
        if (iswascii(*wc)) {
                return ((iswalpha(*wc) || iswdigit(*wc) || *wc == '_'));
        } else {
                if (*wc == 0xa1bc)        /* CHOUON KIGOU */
                        return (4);     /*  is KATAKANA */
                if (isenglish(*wc) || isnumber(*wc))
                        return (2);
                if (isjhira(*wc))
                        return (3);
                if (isjkata(*wc))
                        return (4);
                if (isjkanji(*wc))
                        return (5);
                if (isjgreek(*wc))
                        return (6);
                if (isjrussian(*wc))
                        return (7);
                if ((*wc & 0x0080) == 0x0080)     /* Codeset 2 */
                        return (8);
                if ((*wc & 0x8000) == 0x8000)     /* Codeset 3 */
                        return (9);
                return (0);     /* *Sigh* */
        }
}
