// var result = getToken();
var redirectUrl = "pebblejs://close#"; // auto-close
// redirectUrl += "#" + encodeURIComponent(JSON.stringify(result.token));
var hash = window.location.hash.toString();
hash = hash.substring(1, hash.length);
redirectUrl += encodeURIComponent(hash);
document.location = redirectUrl;