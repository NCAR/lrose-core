/*  AJAX Functions
* ajax.js  Ver. 1.0
* 
* Copyright (c) 2005  University Corporation for Atmospheric Research(UCAR) 
* BSD-style license applies.
* Niles Oien from Frank Hage's April 2005 work.
* 
* This is the part that runs on the client, puts together the XML
* message from make_prod.php running on the server, and does all the right things.
*
*/


/*************************************************************************************/
var ns4up = (window.XMLHttpRequest) ? 1 : 0;  // browser sniffer
var ie4up = (window.ActiveXObject) ? 1 : 0;

var doc_height,doc_width;

if (ns4up) {
    doc_width = window.innerWidth;
    doc_height = window.innerHeight;
} else if (ie4up) {
    doc_width = document.body.clientWidth;
    doc_height = document.body.clientHeight;
}


var req;
var xml;

/*************************************************************************************
 * Example taken From: Very Dynamic Web Interfaces by Drew McLellan, http://www.xml.com
 */
function processReqChange() {
    // only if req shows "loaded"
    // alert("At critical point with : " + req.readyState + ", " + req.statusText); 
    if (req.readyState == 4) {
        // only if "OK"

/////////////////////////////////////////
//
// The following code traps an error that should not now occur.
// See comments in ciddView.php relating to the "oddball error".
//
// Niles Oien.
//
        var st_test;
        try{ 
          st_test = req.status;
        }catch(e){
          alert( "oddball error" );
          req.abort();
          return;
        }

///////////////////////////////////////////

        if (req.status == 200) {
            // ...processing statements go here...
	    //resp =  req.responseText;

	    xml = req.responseXML;
	    prod = xml.getElementsByTagName("product");
            set_radar_http( prod[0] );

        } else {
            alert("There was a problem retrieving the XML data:\n" +
                req.statusText);
        }
    }
}

/************************************************************************************
 * set_http: Set an element's innerHTML and make it visible
 *
 * Tags: 	prod_html - Contains Html to be inserted
 * 			target		Div element for insertion of the html.
 *          status_html  Contains HTML to for Displaying Status.
 */ 

function set_http(elem) {


   html = elem.getElementsByTagName("prod_html")[0].firstChild.data;
   target = elem.getElementsByTagName("target")[0].firstChild.data;
   _el(target).innerHTML = html;

   if(elem.getElementsByTagName("status_html")[0]) {
	  status_html = elem.getElementsByTagName("status_html")[0].firstChild.data;
      if(_el('status_panel')) _el('status_panel').innerHTML = status_html;
   }

	pop(_el(target));
    
}

/************************************************************************************
 * set_radar_http: Set an element's innerHTML and make it visible
 *
 * Tags: 	prod_html - Contains Html to be inserted
 *          status_html  Contains HTML to for Displaying Status.
 * 			target	Div element for insertion of the prrod_html.
 *          frame      - Ordered list of image names for anim
 */ 

function set_radar_http(elem) {

   html = elem.getElementsByTagName("prod_html")[0].firstChild.data;
   target = elem.getElementsByTagName("target")[0].firstChild.data;
   status_html = elem.getElementsByTagName("status_html")[0].firstChild.data;

   _el(target).innerHTML = html;

   if(_el('status_panel')) _el('status_panel').innerHTML = status_html;

   //alert("Going to get frames...");
//
// NOTE - in order for this to work on IE, we need the
// next line to be :
//
// var frames = elem.getElementsByTagName("frame");
//
// Not :
//
//     frames = elem.getElementsByTagName("frame");
//
// This piece of information was particularly hard-won - Niles.
//
   var frames = elem.getElementsByTagName("frame");
   nframes =  frames.length;
   //alert("Got " + nframes + " frames.");

   for(var i = 0; i < frames.length; i++) {
	   img_arr[i] = new Image();
	   img_arr[i].src = frames[i].firstChild.data;
	  // alert("Frame:" +  img_arr[i].src);
   }

   cur_frame = nframes -1;

    
}

/*************************************************************************************/
function loadXMLDoc(url) {
    // branch for native XMLHttpRequest object
    if (window.XMLHttpRequest) {
        req = new XMLHttpRequest();
        req.onreadystatechange = processReqChange;
        req.open("GET", url, true);
        req.send(null);
    // branch for IE/Windows ActiveX version
    } else if (window.ActiveXObject) {
        req = new ActiveXObject("Microsoft.XMLHTTP");
        if (req) {
            req.onreadystatechange = processReqChange;
            req.open("GET", url, true);
            req.send();
        }
    }
}


