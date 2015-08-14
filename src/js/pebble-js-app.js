// 2015 Thomas Hunsaker @thunsaker

var maxAppMessageTries = 3;
var appMessageRetryTimeout = 1000;
var appMessageTimeout = 1000;
var appMessageQueue = [];
var venues = {};
var max_venues = 16;
var max_radius = 5000;
var isNewList = false;
var api_date = '20150612';
var api_mode = '&m=swarm';
var recentCheckinMessage = 'Last Check-in';
var sending = false;
var mToFeet = 3.2808;
var ftInMile = 5280;

Pebble.addEventListener('ready',
	function(e) {
		console.log('js app ready');
		Pebble.sendAppMessage({'ready':true});
	});

Pebble.addEventListener('showConfiguration',
	function(e) {
		// TODO: Add the existing user settings to the url
		Pebble.openURL('https://thunsaker.github.io/spoon/config.html'); // Prod
	});

Pebble.addEventListener('webviewclosed',
	function(e) {
		var configuration = JSON.parse(e.response);
		if(configuration.token.result === true) {
			if(configuration.token.token.length > 0) {
				localStorage.foursquare_token = configuration.token.token;
				notifyPebbleConnected(localStorage.foursquare_token.toString());
			}
			isNewList = true;
			getClosestVenues();
		} else {
			//Pebble.showSimpleNotificationOnPebble('Spoon', ':( Connection Failed. Try Again.');
		}
		
		if(configuration.theme !== null && configuration.theme.length > 0 && configuration.unit !== null) {
			localStorage.spoon_theme = configuration.theme;
			localStorage.spoon_unit = configuration.unit; // 0 == km | 1 == mi
			notifyPebbleConfiguration(configuration.theme);
		}
	});

function notifyPebbleConnected(token) {
	Pebble.sendAppMessage({'token':token});
}

function notifyPebbleConfiguration(theme) {
	appMessageQueue.push({'message': {'config': theme, 'name': 'The Theme!' }});
	sendAppMessage();
}

function notifyPebbleCheckinOutcome(result, message, tip) {
	appMessageQueue.push({'message': { 'result' : result ? 1 : 0, 'name' : message }});
	sendAppMessage();
}

var error = function(e) {
	Pebble.sendAppMessage({'error': e});
};

var success = function(position) {
	var userToken = localStorage.foursquare_token.toString();
	if(userToken) {	
		fetchMostRecentCheckin(userToken);
		fetchClosestVenues(userToken, position);
	}
};

function fetchClosestVenues(token, position) {
	var req = new XMLHttpRequest();
	var requestUrl = 'https://api.foursquare.com/v2/venues/search?oauth_token=' + token + '&v=' + api_date + '&ll=' +  position.coords.latitude + ',' + position.coords.longitude + '&limit=' + max_venues + '&radius=' + max_radius + api_mode;
//  console.log("requestUrl: " + requestUrl);
	req.open('GET', requestUrl, true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			if (req.status == 200) {
				if (req.responseText) {
					isNewList = true;
					var response = JSON.parse(req.responseText);
					venues = response.response.venues;
					venues.forEach(function (element, index, array) {
						var offsetIndex = index + 1;
						var venueId = element.id.replace('\'','');
						var venueName = element.name.length >= 60 ? element.name.substring(0,59).trim().replace('\'','') : element.name.replace('\'','');
						var venueAddress = element.location.address ? element.location.address.length > 20 ? element.location.address.substring(0,20).trim() : element.location.address : '(No Address)';
						if(element.location.distance) {
							var venueDistance = "";
							if(localStorage.spoon_unit === null || localStorage.spoon_unit === "0") {
								venueDistance = element.location.distance >= 1000 ? (element.location.distance/1000).toFixed(2) + " km - " : element.location.distance + " m - ";
							} else {
								var distance = element.location.distance * mToFeet;
								venueDistance = distance >= ftInMile ? (distance/ftInMile).toFixed(2) + " mi - " : distance.toFixed(0) + " ft - ";
							}
							venueAddress = venueDistance + venueAddress;
						}							

						if(isNewList) {
							appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress, 'index':offsetIndex}});
							isNewList = false;
						} else if(index == venues.length - 1) {
							appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress, 'index':offsetIndex}});
						} else {
							appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress, 'index':offsetIndex}});
						}

						// Send them in clusters of 5
						if(index % 5 == 1 || index == max_venues)
							sendAppMessage();
					});
				} else {
					//console.log('Invalid response received! ' + JSON.stringify(req));
					appMessageQueue.push({'message': {'error': 'Error: Error with request :('}});
				}
			} else {
				console.log('Request returned error code ' + req.status.toString());
			}
		}
		sendAppMessage();
	};

	req.ontimeout = function() {
		console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'error': 'Error:\nRequest timed out!'}});
		sendAppMessage();
	};
	req.onerror = function() {
		console.log('HTTP request return error');
		appMessageQueue.push({'message': {'error': 'Error:\nNo internet connection detected.'}});
		sendAppMessage();
	};
	req.send(null);
}

function fetchMostRecentCheckin(token) {
	var reqRecent = new XMLHttpRequest();
	var current_time = Math.round(new Date().getTime() / 1000);
	var requestUrlRecent = 'https://api.foursquare.com/v2/users/self/checkins?oauth_token=' + token + '&v=' + api_date + '&m=foursquare&beforeTimestamp=' + current_time + '&sort=newestfirst&limit=1';
	reqRecent.open('GET', requestUrlRecent, true);
	reqRecent.onload = function(e) {
		if (reqRecent.readyState == 4) {
			if (reqRecent.status == 200) {
				if (reqRecent.responseText) {
					var response = JSON.parse(reqRecent.responseText);
					var checkinItems = response.response.checkins.items;
					checkinItems.forEach(function (element, index, array) {
						var venueId = element.venue.id.replace('\'','');
						var venueName = element.venue.name.length > 45 ? 
							element.venue.name.substring(0,45).replace('\'','') 
						: element.venue.name.replace('\'','');
						var checkinDate = new Date(element.createdAt*1000);
						var checkinString = checkinDate !== null ? 
							"at " + checkinDate.getHours() + ":" + checkinDate.getMinutes() + " " + checkinDate.toDateString() 
							: "Sometime in the past. :)";
						appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':checkinString, 'index':0 }});
					});
				} else {
					appMessageQueue.push({'message': {'error': 'Error: Error with request :(' }});
				}
			}
		}
		sendAppMessage();
	};
	reqRecent.ontimeout = function() {
		//console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'error': 'Error:\nRequest timed out!'}});
		sendAppMessage();
	};
	reqRecent.onerror = function() {
		//console.log('HTTP request return error');
		appMessageQueue.push({'message': {'error': 'Error:\nNo internet connection detected.'}});
		sendAppMessage();
	};
	reqRecent.send(null);
}

function getClosestVenues() {
	if (navigator.geolocation) {
		navigator.geolocation.getCurrentPosition(success, error);
	} else {
		console.log('no location support');
		error('not supported');
	}		
}

function sendAppMessage() {
	if(sending === true)
		return;
	else
		sending = true;
	
	if (appMessageQueue.length > 0) {
		var currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;

		if (currentAppMessage.numTries < maxAppMessageTries) {
			console.log('Trying to send a message: ' + currentAppMessage.message.name);
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() { sendAppMessage(); }, appMessageTimeout);
					sending = false;
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() { sendAppMessage(); }, appMessageRetryTimeout);
				}
			);
		} else {
			console.log('Failed sending AppMessage after multiple attempts for transactionId:' + currentAppMessage.transactionId + '. Error: None. Here\'s the message:'  + JSON.stringify(currentAppMessage.message));
			sending = false;
		}
	} else {
		sending = false;
	}
}

function attemptCheckin(id, name, private, twitter, facebook) {
	var userToken = localStorage.foursquare_token.toString();
	if(userToken) {
		if (navigator.geolocation) {
			navigator.geolocation.getCurrentPosition(function(position) {
				var req = new XMLHttpRequest();
				var checkinRequestUrl = 'https://api.foursquare.com/v2/checkins/add?oauth_token=' + userToken + '&v=' + api_date + '&ll=' +  position.coords.latitude + ',' + position.coords.longitude + '&venueId=' + id + api_mode;
				var broadcastType = '';
				if(private == 1) {
					broadcastType = 'private';
				} else {
					if(twitter == 1) {
						broadcastType = 'twitter';
					}
					if(facebook == 1) {
						broadcastType += (broadcastType.length > 0 ? ',' : '') + 'facebook';
					}
				}
					
				if(broadcastType.length > 0) {
					checkinRequestUrl += '&broadcast=' + broadcastType;
				}
				
				req.open('POST', checkinRequestUrl, true);
				req.onload = function(e) {
					if (req.readyState == 4) {
						if (req.status == 200) {
							if (req.responseText) {
								var response = JSON.parse(req.responseText);
								//console.log('Response: ' + req.responseText);
								if(response) {
									// TODO: Maybe show the user a popular tip after checkin?
									notifyPebbleCheckinOutcome(true, name, '');
								}
							} else {
								console.log('Invalid response received! ' + JSON.stringify(req));
								notifyPebbleCheckinOutcome(false, 'Error:\nError with Request', '');
							}
						} else {
							console.log('Request returned error code ' + req.status.toString());
						}
					}
				};
				req.ontimeout = function() {
					console.log('HTTP request timed out');
					appMessageQueue.push({'message': {'error': 'Error:\nRequest timed out!'}});
					sendAppMessage();
				};
				req.onerror = function() {
					console.log('HTTP request return error');
					appMessageQueue.push({'message': {'error': 'Error:\nNo internet connection detected.'}});
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

Pebble.addEventListener('appmessage',
	function(e) {
		//console.log('received appmessage');
		if (e.payload.id) {
			attemptCheckin(e.payload.id,e.payload.name,e.payload.private,e.payload.twitter,e.payload.facebook);
		} else if (e.payload.refresh) {
			getClosestVenues();
		}
	});