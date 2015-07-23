var gulp = require('gulp'),
    jade = require('gulp-jade'),
    del = require('del'),
    livereload = require('gulp-livereload');

gulp.task('default', ['clean'], function() {
    gulp.start('jade','copy');
});

gulp.task('clean', function(cb) {
    del(['dist/*'], cb)
});

gulp.task('jade', function() {
    gulp.src('*.jade')
        .pipe(jade())
        .pipe(gulp.dest('./dist/'));
});

gulp.task('copy', function() {
    // css
    gulp.src([
            'bower_components/pebble-slate/dist/css/slate.min.css'
        ])
        .pipe(gulp.dest('./dist/css'));
    // js
    gulp.src([
            'bower_components/pebble-slate/dist/js/slate.min.js'
        ])
        .pipe(gulp.dest('./dist/js'));

    // img
    gulp.src([
            'img/*.png'
        ])
        .pipe(gulp.dest('./dist/img'));
});

gulp.task('watch', function() {
    gulp.watch('*.jade', ['jade']);
    livereload.listen();
    gulp.watch(['.dist/**']).on('change', livereload.changed);
});
