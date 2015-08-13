$(document).ready(function() {
    var result = getToken();
    // var redirectUrl = "config.html"; // dev
    var redirectUrl = "https://thunsaker.github.io/spoon/config.html"; // prod
    // var redirectUrl = "pebblejs://close"; // auto-close

    window.location.href = redirectUrl + "#access_token=" + result.token;
});