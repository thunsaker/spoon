function getUserData(userid) {
    return database.ref(getConfigPath(userid)).once('value');
};

function saveUserDataWithToken(userid, theme, distance, timeline, token) {
    return database.ref(getConfigPath(userid)).set({
        theme: theme,
        distance: distance,
        timeline: timeline,
        token: token
    });
}

function saveUserData(userid, theme, distance, timeline) {
    return database.ref(getConfigPath(userid)).set({
        theme: theme,
        distance: distance,
        timeline: timeline
    });
}

function getConfigPath(userid) {
    return 'config/' + userid;
}