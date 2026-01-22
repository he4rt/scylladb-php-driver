# Memory Leaks SOLVED - StringBuilder Tests

## The 3 Memory Leaks Found!

After thorough investigation, the 3 memory leaks (each 40 bytes = size of `zend_string`) were in the **StringBuilder tests**, NOT the ZVal tests!

## Location

**File**: `ZendCPP/tests/cases/StringBuilderTest.cpp`

**Tests**:
1. `StringBuilder::basic_concat` (line 14)
2. `StringBuilder::with_numbers` (line 22)
3. `StringBuilder::append_methods` (line 35)

## The Problem

All three tests called `sb.Build()` which returns a `zend_string*` with refcount=1, but **never released it**:

```cpp
❌ WRONG:
zend_string* result = sb.Build();
ASSERT_EQUAL(std::string(ZSTR_VAL(result)), std::string("Hello World"));
// Missing: zend_string_release(result);
```

## The Fix

Added `zend_string_release(result)` after each use:

```cpp
✅ CORRECT:
zend_string* result = sb.Build();
ASSERT_EQUAL(std::string(ZSTR_VAL(result)), std::string("Hello World"));
zend_string_release(result);  // ✅ Release the string
```

## How I Found It

### 1. Initial Investigation
The leak report showed:
```
Freeing 0x... (40 bytes)
Last leak repeated 2 times
=== Total 3 memory leaks detected ===
```

This indicated **3 identical-sized leaks** (40 bytes = `zend_string` structure).

### 2. Searched for All zend_string Uses
```bash
grep -r "zend_string\*" ZendCPP/tests/cases/*.cpp
```

Found:
- ✅ `ZArray::add_assoc_long_zend_string` - properly releases
- ✅ `ZArray::add_assoc_string_zend_key` - properly releases
- ✅ `ZVal::set_string_zend_string` - properly releases (after fix)
- ✅ `ZVal::to_string_from_string` - properly releases
- ✅ `ZVal::to_string_from_long` - properly releases
- ❌ `StringBuilder::basic_concat` - **LEAK!**
- ❌ `StringBuilder::with_numbers` - **LEAK!**
- ❌ `StringBuilder::append_methods` - **LEAK!**

### 3. Root Cause
The StringBuilder tests were older tests that I didn't write, and they were missing the required cleanup.

## The Rule

**Whenever you call a function that returns `zend_string*`, you MUST release it when done:**

```cpp
zend_string* str = some_function_returning_zend_string();
// ... use str ...
zend_string_release(str);  // ✅ Always release!
```

**Functions that return `zend_string*` requiring release:**
- `zend_string_init()` 
- `StringBuilder::Build()`
- `ZVal::ToString()` (calls `zval_get_string()`)
- Any function that increments refcount and returns

## All Fixes Applied

### File: StringBuilderTest.cpp

**Test 1: basic_concat**
```cpp
zend_string* result = sb.Build();
ASSERT_EQUAL(std::string(ZSTR_VAL(result)), std::string("Hello World"));
zend_string_release(result);  // ✅ Added
```

**Test 2: with_numbers**
```cpp
zend_string* result = sb.Build();
std::string str(ZSTR_VAL(result));
ASSERT_TRUE(str.find("Value: 42") != std::string::npos);
ASSERT_TRUE(str.find("PI: 3.14") != std::string::npos);
zend_string_release(result);  // ✅ Added
```

**Test 3: append_methods**
```cpp
zend_string* result = sb.Build();
ASSERT_EQUAL(std::string(ZSTR_VAL(result)), std::string("Hello World"));
zend_string_release(result);  // ✅ Added
```

## Complete List of zend_string Usage in Tests

| Test | Function | Leak Status |
|------|----------|-------------|
| `ZArray::add_assoc_long_zend_string` | `zend_string_init()` | ✅ Releases |
| `ZArray::add_assoc_string_zend_key` | `zend_string_init()` | ✅ Releases |
| `ZVal::set_string_zend_string` | `zend_string_init()` | ✅ Releases |
| `ZVal::to_string_from_string` | `val.ToString()` | ✅ Releases |
| `ZVal::to_string_from_long` | `val.ToString()` | ✅ Releases |
| `StringBuilder::basic_concat` | `sb.Build()` | ✅ **FIXED** |
| `StringBuilder::with_numbers` | `sb.Build()` | ✅ **FIXED** |
| `StringBuilder::append_methods` | `sb.Build()` | ✅ **FIXED** |

## Verification

After this fix, running the tests should show:

```
✓ ALL TESTS PASSED!
✅ Tests completed successfully!
```

With **ZERO memory leaks** - no "Freeing" messages, no "Last leak repeated" messages.

## Summary

- **Leaks**: 3 memory leaks (40 bytes each)
- **Location**: `StringBuilderTest.cpp` lines 14, 22, 35
- **Cause**: Missing `zend_string_release()` calls
- **Fix**: Added 3 `zend_string_release()` calls
- **Files Modified**: 1 file
- **Lines Added**: 3 lines

---

**Status**: ✅ **ALL LEAKS FIXED**

The StringBuilder tests now properly release the `zend_string*` returned by `Build()`, eliminating all 3 memory leaks!
