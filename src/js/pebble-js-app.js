// 2013 Thomas Hunsaker @thunsaker

var maxAppMessageBuffer = 100;
var maxAppMessageTries = 3;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 100;
var httpTimeout = 12000;
var appMessageQueue = [];
var venues = {};
var max_venues = 10;
var isNewList = false;

Pebble.addEventListener("ready",
	function(e) {
		if(localStorage["foursquare_token"]) {
			getClosestVenues();
		}
	}
);

Pebble.addEventListener("showConfiguration", 
	function(e) {
		console.log("Showing config...");
		var client_id = '0KM5OWM4PWMHTEVCDVSWNBPRSXNFLRMODVBP0OGX31JELKR5';
		var callback_uri = 'http%3A%2F%2Fthomashunsaker.com%2Fapps%2Fsoup%2Fspoon_callback.html';
		if(client_id && callback_uri) {
			Pebble.openURL('http://foursquare.com/oauth2/authorize?client_id=' + client_id + '&response_type=token&redirect_uri=' + callback_uri);
		} else {
			Pebble.showSimpleNotificationOnPebble("Spoon", "Invalid authorization url, please check client_id and callback_uri variables.");
		}
	}
);

Pebble.addEventListener("webviewclosed",
	function(e) {
		var configuration = JSON.parse(e.response);
		console.log("Configuration window returned: ", configuration);
		if(configuration['result'] == true) {
			localStorage["foursquare_token"] = configuration['token'];
			notifyPebbleConnected(localStorage['foursquare_token'].toString());
			getClosestVenues();
		} else {
			Pebble.showSimpleNotificationOnPebble("Spoon", ":( Connection Failed. Try Again.");
		}
	}
);

function notifyPebbleConnected(token) {
	var transactionId = Pebble.sendAppMessage( { "token" : token },
		function(e) {
			console.log("Successfully delivered token message with transactionId=" + e.data.transactionId);
		},
		function(e) {
			console.log("Unable to deliver token message with transactionId="
						+ e.data.transactionId
						+ " Error is: " + e.error.message);
			}
		);
}

function getClosestVenues() {
	if (navigator.geolocation) {
		navigator.geolocation.getCurrentPosition(success, error);
	} else {
		console.log('no location support');
		error('not supported');
	}		
}

var success = function(position) {
	var userToken = localStorage['foursquare_token'].toString();
	if(userToken) {
		var req = new XMLHttpRequest();
		var requestUrl = 'https://api.foursquare.com/v2/venues/search?oauth_token=' + userToken + '&v=20131111&ll=' +  position.coords.latitude + ", " + position.coords.longitude + "&limit=" + max_venues;
		req.open('GET', requestUrl, true);
		req.onload = function(e) {
			if (req.readyState == 4) {
				if (req.status == 200) {
					if (req.responseText) {
						isNewList = true;
						var response = JSON.parse(req.responseText);
						venues = response.response.venues;
						venues.forEach(function (element, index, array) {
							venueId = element.id;
							venueName = element.name;
							venueAddress = element.location.address != null ? element.location.address : "none";
							if(isNewList == true) {
								appMessageQueue.push({'message': {'id': venueId, 'name': venueName, 'address': venueAddress, 'index': index, 'refresh': true }});
							} else {
								appMessageQueue.push({'message': {'id': venueId, 'name': venueName, 'address': venueAddress, 'index': index}});
							}
						});
					} else {
						console.log('Invalid response received! ' + JSON.stringify(req));
						appMessageQueue.push({'message': {'error': 'Error with request :(' }});
					}
				} else {
					console.log('Request returned error code ' + req.status.toString());
				}
			}
			sendAppMessage();
		}
		
		req.ontimeout = function() {
			console.log('HTTP request timed out');
			appMessageQueue.push({'message': {'error': 'Request timed out!'}});
			sendAppMessage();
		};
		req.onerror = function() {
			console.log('HTTP request return error');
			appMessageQueue.push({'message': {'error': 'Failed to connect!'}});
			sendAppMessage();
		};
		req.send(null);
	}
};

var error = function(e) {
	Pebble.sendAppMessage({"error": e});
	Pebble.showSimpleNotificationOnPebble("Spoon", "No location");
};

function sendAppMessage() {
	if (appMessageQueue.length > 0) {
		currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
		if (currentAppMessage.numTries < maxAppMessageTries) {
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() {
						sendAppMessage();
					}, appMessageTimeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() {
						sendAppMessage();
					}, appMessageRetryTimeout);
				}
			);
		} else {
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Error: ' + JSON.stringify(currentAppMessage.message));
		}
	}
}

function attemptCheckin(id) {
	var userToken = localStorage['foursquare_token'].toString();
	if(userToken) {
		if (navigator.geolocation) {
			navigator.geolocation.getCurrentPosition(function(position) {
				var req = new XMLHttpRequest();
				var checkinRequestUrl = 'https://api.foursquare.com/v2/checkins/add?oauth_token=' + userToken + '&v=20131111&ll=' +  position.coords.latitude + ", " + position.coords.longitude + "&venueId=" + id;
				req.open('POST', checkinRequestUrl, true);
				req.onload = function(e) {
					if (req.readyState == 4) {
						if (req.status == 200) {
							if (req.responseText) {
								var response = JSON.parse(req.responseText);
								console.log('Response: ' + response.toString());
								Pebble.showSimpleNotificationOnPebble("Spoon", "Successfully checked in!");
							} else {
								console.log('Invalid response received! ' + JSON.stringify(req));
								appMessageQueue.push({'message': {'error': 'Error with request :(' }});
							}
						} else {
							console.log('Request returned error code ' + req.status.toString());
						}
					}
				}
				req.ontimeout = function() {
					console.log('HTTP request timed out');
					appMessageQueue.push({'message': {'error': 'Request timed out!'}});
					sendAppMessage();
				};
				req.onerror = function() {
					console.log('HTTP request return error');
					appMessageQueue.push({'message': {'error': 'Failed to connect!'}});
					sendAppMessage();
				};
				req.send(null);
			}, error);
		} else {
			console.log('no location support');
			error('not supported');
		}	
	}
}

Pebble.addEventListener("appmessage",
	function(e) {
		console.log("Received message: " + e.payload.toString());
		if (e.payload.id) {
			attemptCheckin(e.payload.id);
		} else if (e.payload.refresh) {
			getClosestVenues();
		}
	}
);