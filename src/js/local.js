// Localization (l10n.js - https://github.com/eligrey/l10n.js/)
var l = function(toTranslate) {
    return toTranslate.toLocaleString({
        "es" : { // Spanish
            "invalid-auth":"Página de autorización no válido.",
            "error":"Error:\n",
            "error-connection":":( La conexión falló. Inténtalo de nuevo",
            "error-request":"Error con la pedida :(",
            "error-timeout":"Se falló :(",
            "error-no-internet":"¡No hay internet!",
            "last-checkin-at":"a",
            "last-checkin-no-date":"En algún momento. :)",
            "last-checkin-miles":"mi -",
            "last-checkin-feet":"pies -",
			"last-checkin-kilometers":"km -",
			"last-checkin-meters":"m -",
			"address-none":"(No Hay Dirección)"
        },
        "en-US" : {
            "invalid-auth":"Invalid authorization url",
            "error":"Error:\n",
            "error-connection":":( Connection Failed. Try Again.",
            "error-request":"Error with request :(",
            "error-timeout":"Request timed out!",
            "error-no-internet":"No internet connection detected.",
            "last-checkin-at":"at",
            "last-checkin-no-date":"Sometime in the past. :)",
            "last-checkin-miles":"mi -",
            "last-checkin-feet":"ft -",
			"last-checkin-kilometers":"km -",
			"last-checkin-meters":"m -",
			"address-none":"(No Address)"
        }
    });
};