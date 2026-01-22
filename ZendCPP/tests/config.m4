PHP_ARG_ENABLE(zendcpp_test, whether to enable zendcpp_test support,
[  --enable-zendcpp_test          Enable zendcpp_test support])

if test "$PHP_ZENDCPP_TEST" != "no"; then
  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, 1, ZENDCPP_TEST_SHARED_LIBADD)

  # Main test file
  TEST_SOURCES="test_main.cpp ../String/Builder.cpp"

  PHP_NEW_EXTENSION(zendcpp_test,
    $TEST_SOURCES,
    $ext_shared,,
    -std=c++23 -g -O0 -Wall -Wextra)

  PHP_SUBST(ZENDCPP_TEST_SHARED_LIBADD)
  PHP_ADD_INCLUDE([$ext_srcdir/..])
  PHP_ADD_INCLUDE([$ext_srcdir])
fi
