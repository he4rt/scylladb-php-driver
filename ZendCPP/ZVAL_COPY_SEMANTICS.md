# ZVal Copy Semantics Implementation

## Summary

The `ZVal` class now supports proper copy construction and copy assignment with automatic reference counting, making it safe to copy zvals containing objects, arrays, and other reference-counted types.

## Changes Made

### Before (Copy Disabled)

```cpp
// Delete copy operations
ZVal(const ZVal&) = delete;
ZVal& operator=(const ZVal&) = delete;
```

**Problem**: Could not copy ZVal instances, even though zvals are designed to be copyable with proper refcount management.

### After (Copy Enabled with Refcounting)

```cpp
// Copy operations - properly handle refcounting
inline ZVal(const ZVal& other) noexcept : val_(other.val_) {
  zval_copy_ctor(&val_);
}

inline ZVal& operator=(const ZVal& other) noexcept {
  if (this != &other) {
    zval_ptr_dtor(&val_);
    val_ = other.val_;
    zval_copy_ctor(&val_);
  }
  return *this;
}
```

**Benefits**: Can now safely copy ZVal instances with automatic refcount handling.

## How It Works

### `zval_copy_ctor(&val_)`

This Zend function handles the complexity of copying different zval types:

1. **Simple types** (IS_LONG, IS_DOUBLE, IS_TRUE, IS_FALSE, IS_NULL):
   - Just copies the value (no refcounting needed)

2. **Strings** (IS_STRING):
   - Increments the `zend_string` refcount
   - Or duplicates if not refcounted

3. **Arrays** (IS_ARRAY):
   - Increments the `HashTable` refcount
   - Or duplicates if not refcounted

4. **Objects** (IS_OBJECT):
   - Increments the `zend_object` refcount

5. **Resources** (IS_RESOURCE):
   - Increments the resource refcount

6. **References** (IS_REFERENCE):
   - Increments the reference refcount

## Usage Examples

### Example 1: Copy ZVal

```cpp
ZVal original;
original.SetLong(42);

// Copy constructor
ZVal copy1(original);  // ✅ Now works! copy1 contains 42

// Copy assignment
ZVal copy2;
copy2 = original;      // ✅ Now works! copy2 contains 42
```

### Example 2: Copy ZVal with String

```cpp
ZVal original;
original.SetString("test string");

// Copy constructor - refcount incremented
ZVal copy(original);

// Both point to the same zend_string with refcount = 2
// When destroyed, each decrements refcount
```

### Example 3: Copy ZVal with Array

```cpp
ZArray arr;
arr.AddAssocLong("count", 42);

ZVal original;
original.SetArray(arr.Release());

// Copy constructor - array refcount incremented
ZVal copy(original);

// Both share the same HashTable with refcount = 2
// Copy-on-write semantics apply if modified
```

### Example 4: Copy ZVal with Object

```cpp
ZVal original;
original.SetObject(some_object);

// Copy constructor - object refcount incremented
ZVal copy(original);

// Both hold references to the same object
// Object won't be destroyed until both ZVals are gone
```

### Example 5: Storing ZVals in Containers

```cpp
std::vector<ZVal> values;

ZVal v1;
v1.SetLong(1);

ZVal v2;
v2.SetString("test");

// Now possible - copies are made
values.push_back(v1);
values.push_back(v2);
values.push_back(ZVal());  // Default constructed

// All copies are independent with proper refcounting
```

### Example 6: Return by Value

```cpp
ZVal CreateConfigValue() {
  ZVal val;
  val.SetString("config_value");
  return val;  // Move semantics (no copy)
}

ZVal GetCachedValue() {
  static ZVal cached;
  cached.SetLong(42);
  return cached;  // Copy made, refcount incremented
}
```

## Reference Counting Details

### Copy Constructor Flow

```cpp
ZVal copy(original);
```

1. Copy the zval structure: `val_(other.val_)`
2. Increment refcounts: `zval_copy_ctor(&val_)`

### Copy Assignment Flow

```cpp
copy = original;
```

1. Check for self-assignment: `if (this != &other)`
2. Destroy current value: `zval_ptr_dtor(&val_)`
3. Copy the zval structure: `val_ = other.val_`
4. Increment refcounts: `zval_copy_ctor(&val_)`

### Reference Count Examples

**Before copy:**
```
original: zend_string("test") refcount=1
```

**After copy:**
```
original: zend_string("test") refcount=2
copy:     zend_string("test") refcount=2  (same pointer)
```

**After destroying copy:**
```
original: zend_string("test") refcount=1
copy:     (destroyed, refcount decremented)
```

**After destroying original:**
```
(zend_string freed, refcount reached 0)
```

## Copy-on-Write (COW) Semantics

PHP uses copy-on-write for arrays and strings. When you copy a ZVal:

```cpp
ZVal original;
original.SetString("test");

ZVal copy(original);  // Shares the same zend_string

// If you modify copy, PHP will duplicate the string first
// This is handled automatically by Zend
```

## Performance Considerations

### When Copy is Cheap

- **Simple types** (int, float, bool, null): Just value copy
- **Strings/Arrays/Objects**: Only refcount increment (very fast)

### When Copy Triggers Work

- If refcount is 1 and string/array is modified after copy
- Copy-on-write duplication happens automatically

### Move vs Copy

```cpp
// Move (no refcount change, fastest)
ZVal v1 = CreateValue();

// Copy (refcount increment)
ZVal v2 = v1;

// Prefer move when possible
ZVal v3 = std::move(v1);  // v1 is now undefined
```

## Memory Safety

### Self-Assignment Protection

```cpp
ZVal val;
val.SetLong(42);
val = val;  // Safe! Detected and handled
```

The check `if (this != &other)` prevents:
- Destroying the value before copying
- Double-free issues
- Unnecessary work

### Exception Safety

All operations are `noexcept`:
- `zval_copy_ctor` never throws
- `zval_ptr_dtor` never throws
- Memory management is safe

## Comparison: Rule of Five

The ZVal class now follows the **Rule of Five** properly:

1. ✅ **Destructor**: `~ZVal()` - Decrements refcounts
2. ✅ **Copy Constructor**: `ZVal(const ZVal&)` - Increments refcounts
3. ✅ **Copy Assignment**: `operator=(const ZVal&)` - Proper refcount handling
4. ✅ **Move Constructor**: `ZVal(ZVal&&)` - Transfers ownership
5. ✅ **Move Assignment**: `operator=(ZVal&&)` - Transfers ownership

## Use Cases Enabled

### 1. Store in Standard Containers

```cpp
std::vector<ZVal> array;
std::map<std::string, ZVal> map;
std::unordered_map<int, ZVal> hash;
```

### 2. Return by Value

```cpp
ZVal GetValue() {
  ZVal val;
  val.SetLong(42);
  return val;  // Safe
}
```

### 3. Pass by Value

```cpp
void ProcessValue(ZVal val) {  // Copy made
  // Work with copy
}

ZVal original;
ProcessValue(original);  // original unchanged
```

### 4. Store as Member

```cpp
class MyClass {
  ZVal cached_value_;  // Can be copied when MyClass is copied
  
public:
  MyClass(const MyClass&) = default;  // Now safe!
};
```

## Testing

### Basic Copy Test

```cpp
ZVal v1;
v1.SetLong(42);

ZVal v2(v1);
assert(v2.ToLong() == 42);

ZVal v3;
v3 = v1;
assert(v3.ToLong() == 42);
```

### String Copy Test

```cpp
ZVal v1;
v1.SetString("test");

ZVal v2(v1);
zend_string* s1 = v1.ToString();
zend_string* s2 = v2.ToString();
assert(s1 == s2);  // Same pointer (shared)
```

### Array Copy Test

```cpp
ZArray arr;
arr.AddAssocLong("key", 123);

ZVal v1;
v1.SetArray(arr.Release());

ZVal v2(v1);
// Both v1 and v2 reference the same array
```

---

**Status**: ✅ **Complete**

The `ZVal` class now supports proper copy semantics with automatic reference counting, making it safe to copy zvals containing any PHP type while maintaining proper memory management.
