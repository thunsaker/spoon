var gulp = require('gulp'),
    jade = require('gulp-jade'),
    stylus = require('gulp-stylus'),
    del = require('del'),
    livereload = require('gulp-livereload'),
    ghPages = require('gulp-gh-pages');

gulp.task('default', ['clean'], function() {
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

gulp.task('copy', function() {
    // css
    gulp.src([
            'bower_components/pebble-slate/dist/css/slate.min.css'
        ])
        .pipe(gulp.dest('./dist/css'));
    // fonts
    gulp.src([
            'bower_components/pebble-slate/dist/fonts/*'
        ])
        .pipe(gulp.dest('./dist/fonts'));
    // js
    gulp.src([
            'bower_components/pebble-slate/dist/js/slate.min.js',
            'bower_components/material-design-lite/material.min.js'
        ])
        .pipe(gulp.dest('./dist/js'));

    // img
    gulp.src([
            'img/*.*'
        ])
        .pipe(gulp.dest('./dist/img'));
});

gulp.task('watch', ['default'], function() {
    gulp.watch('*.jade', ['jade']);
    gulp.watch('css/*.styl', ['stylus']);

    livereload.listen();
    gulp.watch(['.dist/**']).on('change', livereload.changed);
});

gulp.task('deploy',['default'], function() {
    return gulp.src('./dist/**/*')
                .pipe(ghPages());
});
