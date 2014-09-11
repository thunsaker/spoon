// 2014 Thomas Hunsaker @thunsaker

//  Start of Strap API
var strap_api_num_samples=10;var strap_api_url="https://api.straphq.com/create/visit/with/";var strap_api_timer_send=null;var strap_api_const={};strap_api_const.KEY_OFFSET=48e3;strap_api_const.T_TIME_BASE=1e3;strap_api_const.T_TS=1;strap_api_const.T_X=2;strap_api_const.T_Y=3;strap_api_const.T_Z=4;strap_api_const.T_DID_VIBRATE=5;strap_api_const.T_ACTIVITY=2e3;strap_api_const.T_LOG=3e3;var strap_api_can_handle_msg=function(data){var sac=strap_api_const;if((sac.KEY_OFFSET+sac.T_ACTIVITY).toString()in data){return true}if((sac.KEY_OFFSET+sac.T_LOG).toString()in data){return true}return false};var strap_api_clone=function(obj){if(null==obj||"object"!=typeof obj)return obj;var copy={};for(var attr in obj){if(obj.hasOwnProperty(attr))copy[attr]=obj[attr]}return copy};var strap_api_log=function(data,min_readings,log_params){var sac=strap_api_const;var lp=log_params;if(!((sac.KEY_OFFSET+sac.T_LOG).toString()in data)){var convData=strap_api_convAcclData(data);var tmpstore=window.localStorage["strap_accl"];if(tmpstore){tmpstore=JSON.parse(tmpstore)}else{tmpstore=[]}tmpstore=tmpstore.concat(convData);if(tmpstore.length>min_readings){window.localStorage.removeItem("strap_accl");var req=new XMLHttpRequest;req.open("POST",strap_api_url,true);var tz_offset=(new Date).getTimezoneOffset()/60*-1;var query="app_id="+lp["app_id"]+"&resolution="+(lp["resolution"]||"")+"&useragent="+(lp["useragent"]||"")+"&action_url="+"STRAP_API_ACCL"+"&visitor_id="+(lp["visitor_id"]||Pebble.getAccountToken())+"&visitor_timeoffset="+tz_offset+"&accl="+encodeURIComponent(JSON.stringify(tmpstore))+"&act="+(tmpstore.length>0?tmpstore[0].act:"UNKNOWN");req.setRequestHeader("Content-type","application/x-www-form-urlencoded");req.setRequestHeader("Content-length",query.length);req.setRequestHeader("Connection","close");req.onload=function(e){if(req.readyState==4&&req.status==200){if(req.status==200){}else{}}};req.send(query)}else{window.localStorage["strap_accl"]=JSON.stringify(tmpstore)}}else{var req=new XMLHttpRequest;req.open("POST",strap_api_url,true);var tz_offset=(new Date).getTimezoneOffset()/60*-1;var query="app_id="+lp["app_id"]+"&resolution="+(lp["resolution"]||"")+"&useragent="+(lp["useragent"]||"")+"&action_url="+data[(sac.KEY_OFFSET+sac.T_LOG).toString()]+"&visitor_id="+(lp["visitor_id"]||Pebble.getAccountToken())+"&visitor_timeoffset="+tz_offset;req.setRequestHeader("Content-type","application/x-www-form-urlencoded");req.setRequestHeader("Content-length",query.length);req.setRequestHeader("Connection","close");req.onload=function(e){if(req.readyState==4&&req.status==200){if(req.status==200){}else{}}};req.send(query)}};var strap_api_convAcclData=function(data){var sac=strap_api_const;var convData=[];var time_base=parseInt(data[(sac.KEY_OFFSET+sac.T_TIME_BASE).toString()]);for(var i=0;i<strap_api_num_samples;i++){var point=sac.KEY_OFFSET+10*i;var ad={};var key=(point+sac.T_TS).toString();ad.ts=data[key]+time_base;key=(point+sac.T_X).toString();ad.x=data[key];key=(point+sac.T_Y).toString();ad.y=data[key];key=(point+sac.T_Z).toString();ad.z=data[key];key=(point+sac.T_DID_VIBRATE).toString();ad.vib=data[key]=="1"?true:false;ad.act=data[(sac.KEY_OFFSET+sac.T_ACTIVITY).toString()];convData.push(ad)}return convData};
//  End of Strap API

var maxAppMessageTries = 3;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 100;
var appMessageQueue = [];
var venues = {};
var max_venues = 10;
var isNewList = false;
var api_date = '20140905';
var api_mode = '&m=swarm';

Pebble.addEventListener('ready',
	function(e) {
		if(localStorage.foursquare_token) {
			getClosestVenues();
		}
	}
);

Pebble.addEventListener('showConfiguration', 
	function(e) {
		var client_id = '0KM5OWM4PWMHTEVCDVSWNBPRSXNFLRMODVBP0OGX31JELKR5';
		var callback_uri = 'http%3A%2F%2Fthomashunsaker.com%2Fapps%2Fsoup%2Fspoon_callback.html';
		if(client_id && callback_uri) {
			Pebble.openURL('http://foursquare.com/oauth2/authorize?client_id=' + client_id + '&response_type=token&redirect_uri=' + callback_uri);
		} else {
			Pebble.showSimpleNotificationOnPebble('Spoon', 'Invalid authorization url, please check client_id and callback_uri variables.');
		}
	}
);

Pebble.addEventListener('webviewclosed',
	function(e) {
		var configuration = JSON.parse(e.response);
		if(configuration.result) {
			localStorage.foursquare_token = configuration.token;
			notifyPebbleConnected(localStorage.foursquare_token.toString());
			isNewList = true;
			getClosestVenues();
		} else {
			Pebble.showSimpleNotificationOnPebble('Spoon', ':( Connection Failed. Try Again.');
		}
	}
);

function notifyPebbleConnected(token) {
	Pebble.sendAppMessage( { 'token' : token },
		function(e) {
			console.log('Successfully delivered token message with transactionId=' + e.data.transactionId);
		},
		function(e) {
			console.log('Unable to deliver token message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
		}
	);
}

function notifyPebbleCheckinOutcome(result, message) {
	Pebble.sendAppMessage( { 'result' : result ? 1 : 0, 'name' : message },
		function(e) {
			console.log('Successfully delivered token message with transactionId=' + e.data.transactionId);
		},
		function(e) {
			console.log('Unable to deliver token message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
			}
		);
}

var error = function(e) {
	Pebble.sendAppMessage({'error': e});
	Pebble.showSimpleNotificationOnPebble('Spoon', 'No location');
};

var success = function(position) {
	var userToken = localStorage.foursquare_token.toString();
	if(userToken) {
		var req = new XMLHttpRequest();
		var requestUrl = 'https://api.foursquare.com/v2/venues/search?oauth_token=' + userToken + '&v=' + api_date + '&ll=' +  position.coords.latitude + ',' + position.coords.longitude + '&limit=' + max_venues + api_mode;
		req.open('GET', requestUrl, true);
		req.onload = function(e) {
			if (req.readyState == 4) {
				if (req.status == 200) {
					if (req.responseText) {
						isNewList = true;
						var response = JSON.parse(req.responseText);
						venues = response.response.venues;
						venues.forEach(function (element, index, array) {
							var venueId = element.id.replace('\'','');
							var venueName = element.name.length > 45 ? element.name.substring(0,45).replace('\'','') : element.name.replace('\'','');
							var venueAddress = element.location.address ? element.location.address.length > 20 ? element.location.address.substring(0,20) : element.location.address : '(No Address)';
							if(element.location.distance) {
								var venueDistance = element.location.distance >= 1000 ? (element.location.distance/1000).toFixed(2) + "km - " : element.location.distance + "m - ";
								venueAddress = venueDistance + venueAddress;
							}							
						
							if(isNewList) {
								appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress,'index':index, 'refresh':true }});
								isNewList = false;
							} else if(index == venues.length - 1) {
								appMessageQueue.push({'message': {'id':venueId, 'name':venueName, 'address':venueAddress,'index':index, 'last':true }});
							} else {
								appMessageQueue.push({'message': {'id': venueId, 'name': venueName, 'address': venueAddress, 'index':index}});
							}
						});
					} else {
						console.log('Invalid response received! ' + JSON.stringify(req));
						appMessageQueue.push({'message': {'error': 'Error: Error with request :(' }});
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
};

function getClosestVenues() {
	if (navigator.geolocation) {
		navigator.geolocation.getCurrentPosition(success, error);
	} else {
		console.log('no location support');
		error('not supported');
	}		
}

function sendAppMessage() {
	if (appMessageQueue.length > 0) {
		var currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;

		if (currentAppMessage.numTries < maxAppMessageTries) {
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() { sendAppMessage(); }, appMessageTimeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() { sendAppMessage(); }, appMessageRetryTimeout);
				}
			);
		} else {
			console.log('Failed sending AppMessage after multiple attempts for transactionId:' + currentAppMessage.transactionId + '. Error: None. Here\'s the message:'  + JSON.stringify(currentAppMessage.message));
		}
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
								console.log('Response: ' + response.toString());
								if(response)
									notifyPebbleCheckinOutcome(true, name);
							} else {
								console.log('Invalid response received! ' + JSON.stringify(req));
								notifyPebbleCheckinOutcome(false, 'Error:\nError with Request');
								//appMessageQueue.push({'message': {'error': 'Error with request :(' }});
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
		// console.log('Received message: ' + e.payload.toString());

		// Strap API: Developer updates these parameters to fit
		var strap_params = {
			app_id: "nRzHfYJZcx7HvZG7S",
			resolution: "144x168",
			useragent: "PEBBLE/2.0"
		};

		// -------------------------
		//  Strap API inclusion in appmessage
		//  This allows Strap to process Strap-related messages
		//  DO NOT EDIT
		// -------------------------
		if(strap_api_can_handle_msg(e.payload)) {
			clearTimeout(strap_api_timer_send);
			var params = strap_api_clone(strap_params);
			strap_api_log(e.payload, 200, params);
			strap_api_timer_send = setTimeout(function() {
				strap_api_log({}, 0, params);
			}, 10 * 1000);
		}
		// -------------------------

		if (e.payload.id) {
			attemptCheckin(e.payload.id,e.payload.name,e.payload.private,e.payload.twitter,e.payload.facebook);
		} else if (e.payload.refresh) {
			getClosestVenues();
		}
	}
);