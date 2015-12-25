var initialized = false;

function appMessageAck(e) {
  console.log("Configuration sent");
}

function appMessageNak(e) {
  console.log("Configuration not sent: ", e);
}

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('http://woozle.org/neale/misc/twatch-config/fonts.html');
});

var fonts = {"Helvetica": 0,
             "Avería": 1,
             "Ubuntu": 2};

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed:" + decodeURIComponent(e.response));
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  var colors = options.theme.split(",");
  var out = {};
  console.log(options.theme);
  
  out.color_bg = parseInt(colors[0], 16);
  out.color_date = parseInt(colors[1], 16);
  out.color_time = parseInt(colors[2], 16);
  out.font = fonts[options.font];
  
  console.log(colors[1], out.color_date);
  Pebble.sendAppMessage(out, appMessageAck, appMessageNak);
});
