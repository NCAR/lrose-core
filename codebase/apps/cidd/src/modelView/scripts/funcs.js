/* Functions useful in image display.
 *funcs.js  Ver. 0.0
 * 
 * Copyright (c) 2005 UCAR. BSD-style license applies.
 * University Corporation for Atmospheric Research(UCAR) 
 * Niles Oien from Frank Hage's April 2005 work..
 * 
 */


/************************************************************************************
 * WSDM Pan functions:
 * Usage: Place the following div in your page:
 *
 * <DIV ID="Pan_div" STYLE="position:absolute;visibility:visible;cursor:move " 
 *      onSelectStart="return false" onMouseover=" isActive=true onMouseout="isActive=false>
 *    <!-- DIV CONTENT GOES HERE -->
 *   <IMG ID="Pan_img"  BORDER=0 SRC="foo.png">
 * </DIV>";
 */

//
// These are probably not even called anymore - I decided I did not like
// allowing the image to pan - Niles.
//
function pan_div_stop(ev) { 
    return false;
}

function pan_div_init(ev) { 
  return false;
}

function pan_div_move(ev) {
   return false;
}



/************************************************************************************
*/
function _el(i) {return document.getElementById(i);}
function _nel(i) {return document.getElementsByTagName(i);}

/************************************************************************************
*/
function popup(obj,l,t) {
	obj.style.display = "block";
	obj.style.position = "absolute";
	obj.style.left = l;
	obj.style.top = t;
}

function pop(obj) {
	obj.style.display = "block";
	obj.style.position = "absolute";
}

function hide(obj) { obj.style.display = "none";}

/************************************************************************************
*/
function popup_new_win(URL) {
  day = new Date();
  id = day.getTime();
  eval("page" + id + " = window.open(URL, '" + id + "', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=600,height=600');");
}


/************************************************************************************
*/
function retF() {return false;}





