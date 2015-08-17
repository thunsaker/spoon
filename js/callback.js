$(document).ready(function() {
    var result = getToken();
    
    var redirectUrl = "pebblejs://close"; // auto-close
    redirectUrl += "#" + encodeURIComponent(JSON.stringify(result.token));
    alert(redirectUrl);

    document.location = redirectUrl;
});