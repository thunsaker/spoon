var redirectUrl = "pebblejs://close";
var config = {};
var token = {};
var pebbleToken;
var hasToken = false;

var configs = {
    databaseURL: "https://spoon-943a4.firebaseio.com"
};
var app = firebase.initializeApp(configs);
var database = app.database();

var $btnConfig = $('#config-connect-button');
var $tabKm = $('#tab-units-km');
var $tabMi = $('#tab-units-mi');
var $selectTheme = $('#select-theme');
var $checkTimeline = $('#check-timeline');
var $msgError = $('#config-connect-error');
var $progress = $('#progress-save');
var $welcome = $('#message-welcome');

$(document).ready(function() {
    var tokenResult = getToken();
    if(tokenResult !== null) {
        token = tokenResult;
        if(token.result === true) {
            hasToken = true;
            hideAuthButton();
        } else {
            hasToken = false;
            showAuthButton();
            showErrorMessage('Authorizing Foursquare failed. :( Try again.')
        }
    }
    
    var pebbleTokenResult = getPebbleToken();
    if(pebbleTokenResult !== null && 
       pebbleTokenResult.pebble_token !== null) {
        pebbleToken = pebbleTokenResult.pebble_token;
        var href = $btnConfig.attr('href');
        $btnConfig.attr('href', href + "?pebble_token=" + pebbleToken);
        
        // Fetch data
        loadUserSettings();
    }
});

$('#btn-save').click(function() {
    $progress.show();
    var hasNewToken = false;
    if(token != null && 
       token.result !== false && 
       token.token != null && 
       token.token.length > 0) {
        hasNewToken = true;
    }
    
    if(hasNewToken == true) {
        config.token = token.token;
    }
    
    var theme = $("[name='radio-theme']:checked").val();
    config.theme = theme;

    var unitValue = $("[name='tab-units'].active").text();
    var unit = unitValue.substr(unitValue.length - 3, 1);
    config.unit = unit == 'k' ? 0 : 1;
    
    var timeline = $("[name='toggle-timeline']:checked").val();
    config.timeline = timeline == 'on' ? 1 : 0;

    if(pebbleToken != null && pebbleToken.length > 0) {
        if(hasNewToken) {
            saveUserDataWithToken(pebbleToken,
                     parseInt(config.theme),
                     Boolean(config.unit), 
                     Boolean(config.timeline),
                     hasToken)
                    .then(function(result) {
                        close(config);
                    });
        } else {
            saveUserData(pebbleToken,
                     parseInt(config.theme),
                     Boolean(config.unit), 
                     Boolean(config.timeline))
                    .then(function(result) {
                        close(config);
                    });
        }
    } else {
        console.log("No User...");
    }
});

$('#config-connect-reauthorize').click(function() {
    showAuthButton();
});

$welcome.click(function() {
    $welcome.hide();
});

function loadUserSettings() {
    $progress.show();
    getUserData(pebbleToken).then(function(data) {
        if(data.val() != null) {
            // Auth Button
            if(data.val().token === true || hasToken) {
                hideAuthButton();
            } else {
                showAuthButton();
            }
            
            // Distance
            if(data.val().distance == true) {
                $tabMi.addClass('active');
                $tabKm.removeClass('active');
            } else {
                $tabKm.addClass('active');
                $tabMi.removeClass('active');
            }
            
            // Theme
            if(data.val().theme > 0) {
                $selectTheme.val(data.val().theme);
            }
            
            // Timeline
            if(data.val().timeline == true) {
                $checkTimeline.prop('checked', true);
            } else {
                $checkTimeline.prop('checked', false);
            }
        } else {
            // Possibly a new user
            $welcome.show();
            showAuthButton();
        }
        $progress.hide();
    }, function(error) {
        console.log("Error retrieving data");
        $progress.hide();
        resetAll();
    });
}

function showAuthButton() {
    $btnConfig.show();
    $('#config-connect-wrapper').hide();
    $msgError.hide();
}

function hideAuthButton() {
    $btnConfig.hide();
    $('#config-connect-wrapper').show();
}

function showErrorMessage(message) {
    $msgError.show();
    $msgError.html("Error saving user settings");
}

function resetAll() {
    showAuthButton();

    $tabKm.addClass('active');
    $tabMi.removeClass('active');
    
    $selectTheme.val(0);
    
    $checkTimeline.prop('checked', false);
}

function close(config) {
    $progress.hide();
    window.location.replace(
        redirectUrl + '#' + JSON.stringify(config));
}