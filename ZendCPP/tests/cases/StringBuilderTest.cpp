/**
 * StringBuilder tests
 */

#include <ZendCPP/String/Builder.h>

#include "../framework/TestFramework.hpp"

using namespace ZendCPPTest;

ZENDCPP_TEST(StringBuilder, basic_concat) {
  ZendCPP::StringBuilder sb;
  sb << "Hello " << "World";

  auto result = sb.BuildZString();
  ASSERT_EQUAL(result.ToString(), std::string("Hello World"));
}

ZENDCPP_TEST(StringBuilder, with_numbers) {
  ZendCPP::StringBuilder sb;
  sb << "Value: " << 42 << ", PI: " << 3.14;

  const auto str = sb.BuildZString().ToString();

  ASSERT_TRUE(str.find("Value: 42") != std::string::npos);
  ASSERT_TRUE(str.find("PI: 3.14") != std::string::npos);
}

ZENDCPP_TEST(StringBuilder, append_methods) {
  ZendCPP::StringBuilder sb;
  sb.Append("Hello");
  sb.Append(" ");
  sb.Append("World", 5);

  auto result = sb.BuildZString().ToString();
  ASSERT_EQUAL(result, std::string("Hello World"));
}

ZENDCPP_TEST(StringBuilder, length) {
  ZendCPP::StringBuilder sb;

  ASSERT_EQUAL(sb.Length(), 0);

  sb << "Hello";
  ASSERT_EQUAL(sb.Length(), 5);

  sb << " World";
  ASSERT_EQUAL(sb.Length(), 11);
}
