var result = getToken();
var redirectUrl = "pebblejs://close#"; // auto-close
redirectUrl += "#" + encodeURIComponent(JSON.stringify(result.token));
document.location = redirectUrl;