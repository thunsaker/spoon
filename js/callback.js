$(document).ready(function() {
    var result = getToken();
    // var redirectUrl = "config.html"; // dev
    // var redirectUrl = "https://thunsaker.github.io/spoon/config.html"; // prod
    // window.location.href = redirectUrl + "#access_token=" + result.token;
    
    var redirectUrl = "pebblejs://close"; // auto-close
    
    var config = {};
    var token = {};
    config.token = result;

    window.location.href = redirectUrl + "#" + JSON.stringify(config);
});