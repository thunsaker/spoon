// 2016 Thomas Hunsaker @thunsaker

var timeline = require('timeline');
var moment = require('moment');

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
var sending = false;
var mToFeet = 3.2808;
var ftInMile = 5280;
var lang = "en";
var currentConfig = {};

var getCurrentConfig = function() {
	currentConfig.token = 
		localStorage.foursquare_token !== null && localStorage.foursquare_token.length > 0 ? 
		"[TOKEN_NOT_SHOWN]" : "";
	currentConfig.theme = parseInt(localStorage.spoon_theme);
	currentConfig.unit = localStorage.spoon_unit;
	currentConfig.timeline = parseInt(localStorage.spoon_timeline);
};

var createPin = function(id, venue, address) {
	return {
		"id": id,
		"time": new Date(),
		"duration":0,
		"layout": {
			"type": "genericPin",
			"title": venue,
			"locationName": address,
			"tinyIcon": "system://images/LOCATION"
		}
	};
};

var appendLangToUrl = function(url) {
	return url += "&locale=" + lang;
};

Pebble.addEventListener('ready',
	function(e) {
		Pebble.sendAppMessage({'ready':true});
		lang = navigator.language.substring(0,2);
		getCurrentConfig();
	});

Pebble.addEventListener('showConfiguration',
	function(e) {
 		Pebble.openURL('https://thunsaker.github.io/spoon/config');
	});

Pebble.addEventListener('webviewclosed',
	function(e) {
		var configuration = JSON.parse(e.response);
		if(configuration.token.result) {
 			localStorage.foursquare_token = configuration.token.token;
			notifyPebbleConnected(localStorage.foursquare_token.toString());
			isNewList = true;
			if(configuration.unit !== null)
				localStorage.spoon_unit = configuration.unit; // 0 == km | 1 == mi
			getClosestVenues();
		} else {
			Pebble.sendAppMessage({'error':3});
		}
		
		localStorage.spoon_unit = configuration.unit;
		var theme = parseInt(configuration.theme);
		localStorage.spoon_theme = theme;
		notifyPebbleConfiguration(theme);
		localStorage.spoon_timeline = parseInt(configuration.timeline);
		getCurrentConfig();
	}
);

function notifyPebbleConnected(token) {
// 	console.log("Sending the token: " + token);
	Pebble.sendAppMessage({'token':token});
// 	console.log("Sent the token: " + token);
}

function notifyPebbleConfiguration(theme) {
	appMessageQueue.push({'message': {'config': theme }});
	sendAppMessage();
}

function notifyPebbleCheckinOutcome(result, message, tip, error) {
	if(error > -1)
		appMessageQueue.push({'message': { 'result' : result ? 1 : 0, 'name':message, 'error':error }});
	else
		appMessageQueue.push({'message': { 'result' : result ? 1 : 0, 'name':message }});
	sendAppMessage();
}

var error = function(e) {
	Pebble.sendAppMessage({'error':2});
	console.log("Error:" + e);
};

var success = function(position) {
	var userToken = localStorage.foursquare_token !== null ?
		localStorage.foursquare_token.toString() : null;
	if(userToken) {	
		fetchMostRecentCheckin(userToken);
		fetchClosestVenues(userToken, position);
	}
};

function fetchClosestVenues(token, position) {
	var req = new XMLHttpRequest();
	var requestUrl = 'https://api.foursquare.com/v2/venues/search?oauth_token=' + token + '&v=' + api_date + '&ll=' +  position.coords.latitude + ',' + position.coords.longitude + '&limit=' + max_venues + '&radius=' + max_radius + api_mode;
//  	console.log("requestUrl: " + requestUrl);
	requestUrl = appendLangToUrl(requestUrl);
	req.open('GET', requestUrl, true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			if (req.status == 200) {
				if (req.responseText) {
					isNewList = true;
					var response = JSON.parse(req.responseText);
					venues = response.response.venues;
					venues.forEach(function (element, index, array) {
						var offsetIndex = index;
						var venueId = element.id.replace('\'','');
						var venueName = element.name.length >= 60 ? element.name.substring(0,59).trim().replace('\'','') : element.name.replace('\'','');
						var venueAddress = element.location.address ? element.location.address.length > 20 ? element.location.address.substring(0,20).trim() : element.location.address : '';
						var venueDistance = "";
						var venueDistanceUnit = 0;
						if(element.location.distance) {
							var distance = element.location.distance;
							// TODO: Once configuration is in place
 							if(localStorage.spoon_unit === null || localStorage.spoon_unit === "0") {
								venueDistanceUnit = distance >= 1000 ? 1 : 0;
								venueDistance = distance >= 1000 ? (distance/1000).toFixed(2) : distance; // Distance in m
							} else {
 								distance *= mToFeet; // Distance in Feet
								venueDistanceUnit = distance >= ftInMile ? 3 : 2;
								venueDistance = distance >= ftInMile ? (distance/ftInMile).toFixed(2) : distance.toFixed(0);
							}
						}

						if(isNewList) {
							appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress, 'distance':venueDistance.toString(), 'unit':venueDistanceUnit, 'index':offsetIndex}});
							isNewList = false;
						} else {
							appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress, 'distance':venueDistance.toString(), 'unit':venueDistanceUnit, 'index':offsetIndex}});
						}

						// Send them in clusters of 5
// 						if(index % 5 == 1 || index == max_venues/2 || index == max_venues)
							sendAppMessage();
					});
				} else {
					//console.log('Invalid response received! ' + JSON.stringify(req));
					appMessageQueue.push({'message': {'error':2}});
				}
			} else {
				console.log('Request returned error code ' + req.status.toString());
			}
		}
		sendAppMessage();
	};

	req.ontimeout = function() {
		console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'error':1}});
		sendAppMessage();
	};
	req.onerror = function() {
		console.log('HTTP request return error');
		appMessageQueue.push({'message': {'error':0}});
		sendAppMessage();
	};
	req.send(null);
}

function fetchMostRecentCheckin(token) {
	var reqRecent = new XMLHttpRequest();
	var current_time = Math.round(new Date().getTime() / 1000);
	var requestUrlRecent = 'https://api.foursquare.com/v2/users/self/checkins?oauth_token=' + token + '&v=' + api_date + '&m=foursquare&beforeTimestamp=' + current_time + '&sort=newestfirst&limit=1';
	requestUrlRecent = appendLangToUrl(requestUrlRecent);
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
						var checkinString = "";
						var checkinDate = new Date(element.createdAt*1000);
 						var minutes = checkinDate.getMinutes();
						minutes = minutes < 10 ? "0" + minutes : minutes;
						// Show relative time if the last checkin was less than 1 day ago
						if(Date.now() - checkinDate.getTime() > 86400000) {
							checkinString = checkinDate !== null ? 
 								checkinDate.getHours() + ":" + minutes + " " + checkinDate.toDateString() 
								: "";
						} else {
							checkinString = moment.unix(element.createdAt).fromNow();
						}
						appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':checkinString, 'index':-1 }});
					});
				} else {
					appMessageQueue.push({'message': {'error':2 }});
				}
			}
		}
		sendAppMessage();
	};
	reqRecent.ontimeout = function() {
		//console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'error':1}});
		sendAppMessage();
	};
	reqRecent.onerror = function() {
		//console.log('HTTP request return error');
		appMessageQueue.push({'message': {'error':0}});
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
//  		console.log('Trying to send a message: ' + currentAppMessage.message.name);
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

function attemptCheckin(id, name, broadcast) {
	var userToken = localStorage.foursquare_token.toString();
	if(userToken) {
		if (navigator.geolocation) {
			navigator.geolocation.getCurrentPosition(function(position) {
				var req = new XMLHttpRequest();
				var checkinRequestUrl = 'https://api.foursquare.com/v2/checkins/add?oauth_token=' + userToken + '&v=' + api_date + '&ll=' +  position.coords.latitude + ',' + position.coords.longitude + '&venueId=' + id + api_mode;
// 				console.log("checkinRequestUrl: " + checkinRequestUrl);
				var broadcastType = '';
				switch (broadcast) {
					case 1:
						broadcastType = 'private';
						break;
					case 2:
						broadcastType = 'twitter';
						break;
					case 3:
						broadcastType = 'facebook';
						break;
					case 4: // both
						broadcastType = 'twitter,facebook';
						break;
				}
					
				if(broadcast > 0) {
					checkinRequestUrl += '&broadcast=' + broadcastType;
				}
// 				console.log("After broadcast checkinRequestUrl: " + checkinRequestUrl);
				checkinRequestUrl = appendLangToUrl(checkinRequestUrl);
				req.open('POST', checkinRequestUrl, true);
				req.onload = function(e) {
					if (req.readyState == 4) {
						if (req.status == 200) {
							if (req.responseText) {
								var response = JSON.parse(req.responseText);
// 								console.log('Response: ' + req.responseText);
								if(response) {
									var checkin = response.response.checkin;
									var venue = checkin.venue;
									
									// TODO: Maybe show the user a popular tip after checkin?
									notifyPebbleCheckinOutcome(true, venue.name, '', -1);
									if(currentConfig.timeline === 1) {
										var pin = createPin(checkin.id,venue.name,venue.location.address);
// 										console.log('Inserting pin now: ' + JSON.stringify(pin));
										// Push the pin
										timeline.insertUserPin(pin, function(responseText) { 
											console.log('User Pin Result: ' + responseText);
										});
									}
								}
							} else {
								console.log('Invalid response received! ' + JSON.stringify(req));
								notifyPebbleCheckinOutcome(false, name, '', 2);
							}
						} else {
							console.log('Request returned error code ' + req.status.toString());
						}
					}
				};
				req.ontimeout = function() {
					console.log('HTTP request timed out');
					appMessageQueue.push({'message': {'error':1}});
					sendAppMessage();
				};
				req.onerror = function() {
					console.log('HTTP request return error');
					appMessageQueue.push({'message': {'error':0}});
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
//  		console.log('received appmessage');
		if (e.payload.id) {
// 			console.log('received appmessage id: ' + e.payload.id);
			attemptCheckin(e.payload.id,e.payload.name,e.payload.broadcast);
		} else if (e.payload.refresh) {
// 			console.log('received appmessage: ' + e.payload);
			max_venues = e.payload.refresh + 1;
			getClosestVenues();
		}
	});