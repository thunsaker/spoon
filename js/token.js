function getToken() {
    alert("Urls \n window.location.href=" + window.location.href + " window.location=" + window.location + " document.URL=" + document.URL);
    var q = window.location.hash.toString();
    alert("Hash " + q);
    if(q !== null && q.length > 0) {
        var myToken = q.substring(14);
        alert("Token = " + myToken);
        var result = {};
        if(myToken.length > 0) {
            result.result = true;
            result.token = myToken;
        } else {
            result.result = false;
            result.token = "";
        }
        alert("Result = " + result);
        return result;
    } else {
        return null;
    }
}