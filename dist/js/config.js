$(document).ready(function() {
    var config = {};
    var token = {};
    var tokenResult = getToken();
    if(tokenResult !== null) {
        token = tokenResult;
        if(token.result === true) {
            $('#config-connect-button').hide();
            $('#config-connect-info').show();
        } else {
            $('#config-connect-info').hide();
            $('#config-connect-error').show();
        }
    }

    var redirectUrl = "pebblejs://close";

    $('#btn-save').click(function() {
        config.token = token;

        var theme = $("[name='radio-theme']:checked").val();
        config.theme = +theme;

        var unitValue = $("[name='tab-units'].active").text();
        var unit = unitValue.substr(unitValue.length - 3, 1);
        config.unit = unit == 'k' ? 0 : 1;
        
        window.location.replace(redirectUrl + '#' + JSON.stringify(config));
    });
});