## testres

[![Build Status](https://travis-ci.org/ligurio/testres.svg?branch=master)](https://travis-ci.org/ligurio/testres)

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
- estimating of testing convergence
- reducing [Mean Time to Diagnosis](https://www.joecolantonio.com/alan-page-principles-lessons-learned-at-microsoft/)
- conversion of binary test reports to a textual form for indexing, see [Swish-e](http://www.esa.org/tiee/search/html/swish-config.html#document_filter_directives)
- calculating of testcase metrics (Average Time Execution, Average Percentage of Fault Detected (APFD) and [more](http://www.iosrjournals.org/iosr-jce/papers/Vol16-issue4/Version-1/G016414751.pdf))
- keep track of the results with [`git-test`](https://github.com/ligurio/git-test)

### Usage

```
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$ cmake .. -DCMAKE_BUILD_TYPE=DEBUG
$ make test
$ bin/testres -s samples/junit.xml
```

### Authors

Developed with passion by [Sergey Bronnikov](https://bronevichok.ru/) and great
open source [contributors](https://github.com/ligurio/testres/contributors).
