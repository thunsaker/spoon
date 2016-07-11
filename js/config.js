var redirectUrl = "pebblejs://close";
var config = {};
var token = {};
var pebbleToken;

// Initialize Firebase
var firebaseConfig = {
    apiKey: process.env.FIREBASE,
    databaseURL: "https://spoon-943a4.firebaseio.com"
};
var app = firebase.initializeApp(firebaseConfig);
var database = app.database();

var $btnConfig = $('#config-connect-button');
var $tabKm = $('#tab-units-km');
var $tabMi = $('#tab-units-mi');
var $selectTheme = $('#select-theme');
var $checkTimeline = $('#check-timeline');
var $msgError = $('#config-connect-error');
var $progress = $('#progress-save');

$(document).ready(function() {
    var pebbleTokenResult = getPebbleToken();
    if(pebbleTokenResult !== null && 
       pebbleTokenResult.pebble_token !== null) {
        pebbleToken = pebbleTokenResult.pebble_token;
        var href = $btnConfig.attr('href');
        $btnConfig.attr('href', href + "?pebble_token=" + pebbleToken);
        
        // Fetch data
        loadUserSettings();
    }
    
    var tokenResult = getToken();
    if(tokenResult !== null) {
        token = tokenResult;
        if(token.result === true) {
            hideAuthButton();
        } else {
            showAuthButton();
            showErrorMessage('Authorizing Foursquare failed. :( Try again.')
        }
    }
});

$('#btn-save').click(function() {
    $progress.show();
    var hasToken = false;
    if(token != null && 
       token.result !== false && 
       token.token != null && 
       token.token.length > 0) {
        config.token = token.token;
        hasToken = true;
    }
    
    var theme = $("[name='radio-theme']:checked").val();
    config.theme = theme;

    var unitValue = $("[name='tab-units'].active").text();
    var unit = unitValue.substr(unitValue.length - 3, 1);
    config.unit = unit == 'k' ? 0 : 1;
    
    var timeline = $("[name='toggle-timeline']:checked").val();
    config.timeline = timeline == 'on' ? 1 : 0;

    if(pebbleToken != null && pebbleToken.length > 0) {
        saveUserData(pebbleToken, hasToken, 
                     parseInt(theme), 
                     unit == 0 ? false : true, 
                     timeline == 0 ? false : true)
                    .then(function(result) {
                        $progress.hide();
                        window.location.replace(
                            redirectUrl + '#' + JSON.stringify(config));
                    });
        console.log('Pushing to Firebase');
    } else {
        console.log("No user...");
    }
});

$('#config-connect-reauthorize').click(function() {
    showAuthButton();
});

function loadUserSettings() {
    $progress.show();
    getUserData(pebbleToken).then(function(data) {
        if(data.val() != null) {
            // Auth Button
            if(data.val().token === true) {
                hideAuthButton();
            } else {
                showAuthButton();
            }
            
            // Distance
            if(data.val().distance == true) {
                $tabKm.addClass('active');
                $tabMi.removeClass('active');
            } else {
                $tabMi.addClass('active');
                $tabKm.removeClass('active');
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
            showAuthButton();
        }
        $progress.hide();
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