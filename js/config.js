var redirectUrl = "pebblejs://close";
var config = {};
var token = {};

$(document).ready(function() {
    var $btnConfig = $('#config-connect-button');
    var pebbleTokenResult = getPebbleToken();
    if(pebbleTokenResult !== null && 
       pebbleTokenResult.pebble_token !== null) {
        var href = $btnConfig.attr('href');
        $btnConfig.attr('href', href + "?pebble_token=" + pebbleTokenResult.pebble_token);
    }
    
    var tokenResult = getToken();
    if(tokenResult !== null) {
        token = tokenResult;
        if(token.result === true) {
            $btnConfig.hide();
            $('#config-connect-info').show();
        } else {
            $('#config-connect-info').hide();
            $('#config-connect-error').show();
        }
    }
});


$('#btn-save').click(function() {
    config.token = token;

    var theme = $("[name='radio-theme']:checked").val();
    config.theme = theme;

    var unitValue = $("[name='tab-units'].active").text();
    var unit = unitValue.substr(unitValue.length - 3, 1);
    config.unit = unit == 'k' ? 0 : 1;
    
    var timeline = $("[name='toggle-timeline']:checked").val();
    config.timeline = timeline == 'on' ? 1 : 0;

    window.location.replace(redirectUrl + '#' + JSON.stringify(config));
});