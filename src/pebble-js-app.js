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

var themes = {"Dark": 0,
              "Light": 1,
              "Green": 2};
var aligns = {"Left": 0,
              "Center": 1,
              "Right": 2};
var fonts = {"Helvetica": 0,
             "Avería": 1,
             "Ubuntu": 2};

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  options.theme = themes[options.theme];
  options.align = aligns[options.align];
  options.font = fonts[options.font];
  console.log("Options = " + options.theme + options.align + options.font);
  Pebble.sendAppMessage(options, appMessageAck, appMessageNak);
});
