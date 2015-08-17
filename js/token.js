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