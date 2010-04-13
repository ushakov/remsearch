var map = null;
var edit = null;
var icon_base = 'http://maps.google.com/mapfiles/ms/micons/';
var cache_server_base = 'http://localhost:15000/wmcache?path='
var description_div = null;
var marker = null;
var timeout_interval = 10000.0;

function initialize() {
  // Set up the copyright information
  // Each image used should indicate its copyright permissions
  var myCopyright = new GCopyrightCollection("(c) ");
  myCopyright.addCopyright(new GCopyright('Demo',
                                          new GLatLngBounds(new GLatLng(-90,-180), new GLatLng(90,180)),
                                          0,'©2007 Google'));

  map = new GMap2(document.getElementById("map_canvas"));
  map.setCenter(new GLatLng(55.75, 37.65), 14);
  map.addControl(new GSmallMapControl());
  map.addControl(new GMapTypeControl());
  map.enableScrollWheelZoom();
  GEvent.addListener(map, "dragend", refresh);
}

$(document).ready(function() {
        $("#text").keyup(function () {
		var v = $("#text").val();
		setTimeout(function() {query_changed(v)}, 400);
	    });
	$("#more").click(more);
	$("#refresh").click(refresh);
    });

function query_changed(oldVal) {
    var text = $("#text").val();
    if (text != oldVal) {
        return;
    }
    send_search(text, 0);
}

var placemarks = [];
var cont = -2;

function send_search(q, c) {
    $("#res").html('');
    var b = map.getBounds();
    var data = {'output': 'json',
		'q': q,
		'minlat': b.getSouthWest().y,
		'minlng': b.getSouthWest().x,
		'maxlat': b.getNorthEast().y,
		'maxlng': b.getNorthEast().x,
		'n': 30};
    var callback = got_results;
    if (c != 0 && cont != -2) {
	if (cont != -1) {
	    data['s'] = cont;
	    callback = got_more_results;
	}
    }
    $.getJSON("/search", data, callback);
}

function got_results(json) {
    $("#res").html('');
    for (var i = 0; i < placemarks.length; i++) {
	map.removeOverlay(placemarks[i]);
    }
    placemarks = [];
    got_more_results(json);
}

function got_more_results(json) {
    //    $("#res").append("<div>" + json['q'] + "</div>");
    for (var i = 0; i < json['data'].length; ++i) {
	var t = json['data'][i];
	//$("#res").append("<div>lat=" + t['lat'] + "; lng=" + t['lng'] + " " + t['title'] + "</div>");
	var marker = makeMarker(t['lat'], t['lng'], t['title'], t['id']);
	placemarks[placemarks.length] = marker; 
	map.addOverlay(marker);
    }
    cont = json['cont'];
    if (cont == -1) {
	$("#more").hide();
    } else {
	$("#more").show();
    }
    $("#res").html("<div>" + placemarks.length + "</div>");
}

function refresh() {
    var text = $("#text").val();
    send_search(text, 0);
}
function more() {
    var text = $("#text").val();
    send_search(text, 1);
}

function showDescription(marker, id) {
    var id_str = "" + id;
    while (id_str.length < 8) {
        id_str = "0" + id_str;
    }
    var descr_url = cache_server_base + 'wmdescr_ru_' + id_str + '.html';
    response = '<div style="overflow:auto;height:95%"><iframe frameborder=0 width=650 height=400 src="' + descr_url + '"></div>';
    marker.openInfoWindowHtml(response);
}

function makeMarker(lat, lng, title, id) {
  var myicon = new GIcon();
  myicon.image = "/search?name=icon.png";
  myicon.shadow = null;
  myicon.iconSize = new GSize(32, 32);
  myicon.iconAnchor = new GPoint(16, 32);
  myicon.infoWindowAnchor = new GPoint(16, 32);
  var point = new GLatLng(lat,lng);
  var marker = new GMarker(point, {"icon": myicon, "title": title});
  GEvent.addListener(marker, "click", function() {
          showDescription(marker, id);
      });
  return marker;
}
