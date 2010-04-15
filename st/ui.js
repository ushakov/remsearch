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
  GEvent.addListener(map, "dragend", map_dragged);
}

$(document).ready(function() {
        $("#text").keyup(function () {
		var v = $("#text").val();
		setTimeout(function() {query_changed(v)}, 400);
	    });
	$("#more").click(more);
	$("#refresh").click(refresh);
	$("#in-titles").change(refresh);
	$("#in-vport").change(refresh);

	$("#in-titles-label").click(function() {$("#in-titles").click();});
	$("#in-vport-label").click(function() {$("#in-vport").click();});

	$("#list").hide();
	list_shown = 0;
	$("#listbutton").click(toggle_list);
    });

function query_changed(oldVal) {
    var text = $("#text").val();
    if (text != oldVal) {
        return;
    }
    send_search(text, 0);
}

function map_dragged(ev) {
    // Don't refresh if not viewport-dependent.
    if ($("#in-vport").val()) {
	refresh();
    }
}

var placemarks = [];
var cont = -2;

function send_search(q, c) {
    if (!q) {
	return;
    }
    $("#res").html('...');
    var b = map.getBounds();
    var data = {'output': 'json',
		'q': q,
		'n': 50};
    var callback = got_results;
    if (c != 0 && cont != -2) {
	if (cont != -1) {
	    data['s'] = cont;
	    callback = got_more_results;
	}
    }

    if ($("#in-titles").val()) {
	data['ts'] = 1;
    }
    if ($("#in-vport").val()) {
	data['minlat'] = b.getSouthWest().y;
	data['minlng'] = b.getSouthWest().x;
	data['maxlat'] = b.getNorthEast().y;
	data['maxlng'] = b.getNorthEast().x;
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
    for (var i = 0; i < json['data'].length; ++i) {
	var t = json['data'][i];
	var marker = makeMarker(t['lat'], t['lng'], t['title'], t['id']);
	marker.data = t;
	placemarks[placemarks.length] = marker; 
	map.addOverlay(marker);
    }
    cont = json['cont'];
    if (cont == -1) {
	$("#more").hide();
    } else {
	$("#more").show();
    }
    $("#res").html(placemarks.length);
    if (list_shown) {
	show_list();
    }
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

function show(n) {
    var marker = placemarks[n];
    var id = placemarks[n].data['id'];
    showDescription(marker, id);
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
var list_shown;
function show_list() {
    var list = $("#list");
    list.css("visibility", "visible");
    list.html('<ul>');
    for(var i = 0; i < placemarks.length; i++) {
	var t = placemarks[i].data['title'];
	var html = "<li>" +
	    "<a href=\"javascript:show(" + i + ")\">" + t + "</a>";
	list.append(html);
    }
    if (!list_shown) {
	list.fadeIn("slow");
    }
    list_shown = 1;
}

function hide_list() {
    if (list_shown) {
	$("#list").fadeOut("slow");
    }
    list_shown = 0;
}

function toggle_list() {
    if (list_shown) {
	hide_list();
    } else {
	show_list();
    }
}
