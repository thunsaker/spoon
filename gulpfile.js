var gulp = require('gulp'),
    jade = require('gulp-jade'),
    stylus = require('gulp-stylus'),
    del = require('del'),
    livereload = require('gulp-livereload'),
    gutil = require('gulp-util'),
    ghPages = require('gulp-gh-pages');
    
var debug = false;

gulp.task('debug', function() {
    debug = true;
    gutil.log(gutil.colors.green('Running in DEBUG!'));
    gulp.start('default');
})

gulp.task('default', ['clean'], function() {
    debug = debug || false;
    gulp.start('render','copy');
});

gulp.task('clean', function(cb) {
    del(['dist/*'], cb)
});

gulp.task('render', ['jade','stylus']);

gulp.task('jade', function() {
    gulp.src('*.jade')
        .pipe(jade())
        .pipe(gulp.dest('./dist/'));
});

gulp.task('stylus', function() {
    gulp.src('css/*.styl')
        .pipe(stylus())
        .pipe(gulp.dest('./dist/css/'));
});

gulp.task('copy', ['copy-js', 'copy-css'], function() {
    // fonts
    gulp.src([
            'bower_components/pebble-slate/dist/fonts/*'
        ])
        .pipe(gulp.dest('./dist/fonts'));

    // img
    gulp.src([
            'img/*.*'
        ])
        .pipe(gulp.dest('./dist/img'));
        
    // misc
    gulp.src([
            'misc/**'
        ])
        .pipe(gulp.dest('./dist'));
});

gulp.task('copy-js', function() {
    // js
    gulp.src([
            'bower_components/pebble-slate/dist/js/slate.min.js',
            'bower_components/material-design-lite/material.min.js',
            'js/*.js'
        ])
        .pipe(gulp.dest('./dist/js'));
});

gulp.task('copy-css', function() {
    // css
    gulp.src([
            'bower_components/pebble-slate/dist/css/slate.min.css',
            'css/material-orange-cyan.min.css'
        ])
        .pipe(gulp.dest('./dist/css'));
});

gulp.task('watch', ['default'], function() {
    gulp.watch('*.jade', ['jade']);
    gulp.watch('css/*.styl', ['stylus']);
    gulp.watch('js/*.js', ['copy-js']);

    livereload.listen();
    gulp.watch(['.dist/**']).on('change', livereload.changed);
});

gulp.task('deploy', function() {
    debug = debug || false;
    return gulp.src('./dist/**/*')
               .pipe(ghPages());
});