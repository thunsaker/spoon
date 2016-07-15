var guidLength = 32;
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
    if(q !== null && q.length > 0) {
        
        var myToken = "";
        var paramIndex = q.indexOf('pebble_token');
        var startIndex = paramIndex + 13;
        var endex = startIndex + guidLength;

        myToken = q.substring(startIndex, endex);
        var result = {};
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