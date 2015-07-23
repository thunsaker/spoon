var gulp = require('gulp');
var jade = require('gulp-jade');
var del = require('del');

gulp.task('default', ['clean','render']);

gulp.task('clean', function() {
    del(['dist/*'])
});

gulp.task('render', function() {
    gulp.src('*.jade')
        .pipe(jade())
        .pipe(gulp.dest('./dist/'))
});
