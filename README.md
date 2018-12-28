## testres

A hyperfast web frontend for software testing results written in C.

It builds and runs on OpenBSD, Linux, and Mac OS X.

If you have any comments or patches, please feel free to post them here or
notify me by e-mail.

### Features

- CPU and memory consumption is zero in idle (CGI application)
- Support of SubUnit, TAP (Test Anything Protocol) and JUnit formats

### Usage scenarios:

- evaluating of current test coverage
- evaluating of tests stability and effectiveness
- evaluating of features stability in a project
- estimation of test convergence
- reducing [Mean Time to Diagnosis](https://www.joecolantonio.com/alan-page-principles-lessons-learned-at-microsoft/)
- conversion of binary test reports to a textual form for indexing, see [swish-e](http://www.esa.org/tiee/search/html/swish-config.html#document_filter_directives)
- test selection, [see](http://www.iosrjournals.org/iosr-jce/papers/Vol16-issue4/Version-1/G016414751.pdf)

### Usage

```
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$ cmake .. -DCMAKE_BUILD_TYPE=DEBUG
$ make
$ bin/testres -s samples/junit.xml
```

### Authors

Developed with passion by [Sergey Bronnikov](https://bronevichok.ru/) and great
open source [contributors](https://github.com/ligurio/testres/contributors).
