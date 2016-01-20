// var result = getToken();
var redirectUrl = "pebblejs://close"; // auto-close
// redirectUrl += "#" + encodeURIComponent(JSON.stringify(result.token));
redirectUrl += window.location.hash.toString();
document.location = redirectUrl;