function getUserData(userid) {
    return database.ref('config/' + userid).once('value');
};

function saveUserData(userid, token, theme, distance, timeline) {
    return database.ref('config/' + userid).set({
        token: token,
        theme: theme,
        distance: distance,
        timeline: timeline
    });
}