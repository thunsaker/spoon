function getToken() {
    var q = window.location.hash.toString();
    if(q !== null && q.length > 0) {
        var myToken = q.substring(14);
        var result = {};
        if(myToken.length > 0) {
            result.result = true;
            result.token = myToken;
        } else {
            result.result = false;
            result.token = "";
        }
        return result;
    } else {
        return null;
    }
}

function getPebbleToken() {
    var q = window.location.search.toString();
    console.log("q: " + q);
    if(q !== null && q.length > 0) {
        var myToken = q.substring(14);
        var result = {};
        console.log("Token: " + myToken);
        if(myToken.length > 0) {
            result.pebble_token = myToken;
        } else {
            result.pebble_token = "";
        }
        return result;
    } else {
        return null;
    }
}