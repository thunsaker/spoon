$(document).ready(function() {
    var result = getToken();
    
    var redirectUrl = "pebblejs://close"; // auto-close
    
    document.location = redirectUrl + "#" + encodeURIComponent(JSON.stringify(result.token));
});