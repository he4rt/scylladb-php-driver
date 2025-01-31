---
sidebar_position: 2
---

# Contribution Guide

## Reporting bugs

It is inevitable that code will have bugs. There are a few things you can do to help the maintainer to fix the bug that
you've discovered quicker:

* Use the [ScyllaDB He4rt PHP Driver Issues Tab](https://github.com/he4rt/scylladb-php-driver/issues) to report all
  issues and bugs.
* Include the version of the Driver, PHP and ScyllaDB/Cassandra or DSE in your issue.
* Include the Driver dependency versions as well (e.g. libuv, C/C++ Driver, ...etc)
* Include a complete stack trace of the failure as well as any available logs.
* Include any additional information you think is relevant - the description of your setup, any non-default Driver
  configuration, etc.
* Write a failing test. The PHP Driver uses PEST test framework. A reliably failing test is the fastest way to
  demonstrate and fix a problem.

## Pull Requests

If you're able to fix a bug yourself, you can **fork the repository** and submit
a [pull request](https://github.com/he4rt/scylladb-php-driver/pulls) with the fix.
