var result = getToken();
alert("Result = " + result + " token = " + result.token);
var redirectUrl = "pebblejs://close"; // auto-close
redirectUrl += "#" + encodeURIComponent(JSON.stringify(result.token));
alert(redirectUrl);

document.location = redirectUrl;