#ifndef ACOUSTID_SCOPED_PTR_H_
#define ACOUSTID_SCOPED_PTR_H_

// (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
// Copyright (c) 2001, 2002 Peter Dimov
// 
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//  See http://www.boost.org/libs/smart_ptr/scoped_ptr.htm for documentation.
//
//
//  ScopedPtr mimics a built-in pointer except that it guarantees deletion
//  of the object pointed to, either on destruction of the ScopedPtr or via
//  an explicit reset(). ScopedPtr is a simple solution for simple needs;
//  use shared_ptr if your needs are more complex.
//
//  *** NOTE ***
//  If your ScopedPtr is a class member of class FOO pointing to a 
//  forward declared type BAR (as shown below), then at creation (and 
//  destruction) of an object of type FOO, BAR must be complete.  You can do
//  this by either:
//   - Making all FOO constructors and destructors non-inlined to FOO's class
//     definition, instead placing them in foo.cc below an include of bar.h
//   - Including bar.h before any creation or destruction of any object of
//     type FOO
//  The former is probably the less error-prone method, as shown below.
//
//  Example:
//
//  -- foo.h --
//  class BAR;
//
//  class FOO {
//   public:
//    FOO();   // Required for sources that instantiate class FOO to compile!
//    ~FOO();  // Required for sources that instantiate class FOO to compile!
//    
//   private:
//    ScopedPtr<BAR> bar_;
//  };
//
//  -- foo.cc --
//  #include "bar.h"
//  #include "foo.h"
//  FOO::FOO() {}  // Empty, but must be non-inlined to FOO's class definition.
//  FOO::~FOO() {} // Empty, but must be non-inlined to FOO's class definition.
//
//  ScopedPtrMalloc added in by Ray Sidney of Google.  When one of
//  these goes out of scope, instead of doing a delete or delete[], it
//  calls free().  ScopedPtrMalloc<char> is likely to see much more
//  use than any other specializations.
//
//  release() added in by Spencer Kimball of Google. Use this to conditionally
//  transfer ownership of a heap-allocated object to the caller, usually on
//  method success.

#include <cstddef>            // for std::ptrdiff_t
#include <assert.h>           // for assert
#include <stdlib.h>           // for free() decl

template <typename T>
class ScopedPtr;

template<typename T>
T** as_out_parameter(ScopedPtr<T>&);

template <typename T>
class ScopedPtr {
 private:

  T* ptr;

  // ScopedPtr's must not be copied.  We make sure of that by making the
  // copy constructor prototype private.  At the same time, there is no body
  // for this constructor.  Thus, if anything that has access to private
  // members of ScopedPtr ever (inadvertently) copies a ScopedPtr, the
  // linker will complain about missing symbols.  This is a good thing!
  ScopedPtr(ScopedPtr const &);
  ScopedPtr & operator=(ScopedPtr const &);

 public:

  typedef T element_type;

  explicit ScopedPtr(T* p = 0): ptr(p) {}

  ~ScopedPtr() {
    typedef char type_must_be_complete[sizeof(T)];
    delete ptr;
  }

  void reset(T* p = 0) {
    typedef char type_must_be_complete[sizeof(T)];

    if (ptr != p) {
      delete ptr;
      ptr = p;
    }
  }

  T& operator*() const {
    assert(ptr != 0);
    return *ptr;
  }

  T* operator->() const  {
    assert(ptr != 0);
    return ptr;
  }

  bool operator==(T* p) const {
    return ptr == p;
  }

  bool operator!=(T* p) const {
    return ptr != p;
  }

  operator bool() const {
    return ptr != 0;
  }

  T* get() const  {
    return ptr;
  }

  void swap(ScopedPtr & b) {
    T* tmp = b.ptr;
    b.ptr = ptr;
    ptr = tmp;
  }

  T* release() {
    T* tmp = ptr;
    ptr = 0;
    return tmp;
  }

 private:
  friend T** as_out_parameter<>(ScopedPtr<T>&);

  // no reason to use these: each ScopedPtr should have its own object
  template <typename U> bool operator==(ScopedPtr<U> const& p) const;
  template <typename U> bool operator!=(ScopedPtr<U> const& p) const;
};

template<typename T> inline
void swap(ScopedPtr<T>& a, ScopedPtr<T>& b) {
  a.swap(b);
}

template<typename T> inline
bool operator==(T* p, const ScopedPtr<T>& b) {
  return p == b.get();
}

template<typename T> inline
bool operator!=(T* p, const ScopedPtr<T>& b) {
  return p != b.get();
}

// Returns the internal pointer, suitable for passing to functions that
// return an object as an out parameter.
// Example:
//   int CreateObject(Foo** created);
//   ScopedPtr<Foo> foo;
//   CreateObject(as_out_parameter(foo));
template<typename T>
T** as_out_parameter(ScopedPtr<T>& p) {
  assert(p.ptr == 0);
  return &p.ptr;
}

//  ScopedArrayPtr extends ScopedPtr to arrays. Deletion of the array pointed to
//  is guaranteed, either on destruction of the ScopedArrayPtr or via an explicit
//  reset(). Use shared_array or std::vector if your needs are more complex.

template<typename T>
class ScopedArrayPtr {
 private:

  T* ptr;

  ScopedArrayPtr(ScopedArrayPtr const &);
  ScopedArrayPtr & operator=(ScopedArrayPtr const &);

 public:

  typedef T element_type;

  explicit ScopedArrayPtr(T* p = 0) : ptr(p) {}

  ~ScopedArrayPtr() {
    typedef char type_must_be_complete[sizeof(T)];
    delete[] ptr;
  }

  void reset(T* p = 0) {
    typedef char type_must_be_complete[sizeof(T)];

    if (ptr != p) {
      delete [] ptr;
      ptr = p;
    }
  }

  T& operator[](std::ptrdiff_t i) const {
    assert(ptr != 0);
    assert(i >= 0);
    return ptr[i];
  }

  bool operator==(T* p) const {
    return ptr == p;
  }

  bool operator!=(T* p) const {
    return ptr != p;
  }

  operator bool() const {
    return ptr != 0;
  }

  T* get() const {
    return ptr;
  }

  void swap(ScopedArrayPtr & b) {
    T* tmp = b.ptr;
    b.ptr = ptr;
    ptr = tmp;
  }

  T* release() {
    T* tmp = ptr;
    ptr = 0;
    return tmp;
  }

 private:

  // no reason to use these: each ScopedArrayPtr should have its own object
  template <typename U> bool operator==(ScopedArrayPtr<U> const& p) const;
  template <typename U> bool operator!=(ScopedArrayPtr<U> const& p) const;
};

template<class T> inline
void swap(ScopedArrayPtr<T>& a, ScopedArrayPtr<T>& b) {
  a.swap(b);
}

template<typename T> inline
bool operator==(T* p, const ScopedArrayPtr<T>& b) {
  return p == b.get();
}

template<typename T> inline
bool operator!=(T* p, const ScopedArrayPtr<T>& b) {
  return p != b.get();
}


// This class wraps the c library function free() in a class that can be
// passed as a template argument to ScopedPtrMalloc below.
class ScopedPtrMallocFree {
 public:
  inline void operator()(void* x) const {
    free(x);
  }
};

// ScopedPtrMalloc<> is similar to ScopedPtr<>, but it accepts a
// second template argument, the functor used to free the object.

template<typename T, typename FreeProc = ScopedPtrMallocFree>
class ScopedPtrMalloc {
 private:

  T* ptr;

  ScopedPtrMalloc(ScopedPtrMalloc const &);
  ScopedPtrMalloc & operator=(ScopedPtrMalloc const &);

 public:

  typedef T element_type;

  explicit ScopedPtrMalloc(T* p = 0): ptr(p) {}

  ~ScopedPtrMalloc() {
    free_((void*) ptr);
  }

  void reset(T* p = 0) {
    if (ptr != p) {
      free_((void*) ptr);
      ptr = p;
    }
  }

  T& operator*() const {
    assert(ptr != 0);
    return *ptr;
  }

  T* operator->() const {
    assert(ptr != 0);
    return ptr;
  }

  bool operator==(T* p) const {
    return ptr == p;
  }

  bool operator!=(T* p) const {
    return ptr != p;
  }

  operator bool() const {
    return ptr != 0;
  }

  T* get() const {
    return ptr;
  }

  void swap(ScopedPtrMalloc & b) {
    T* tmp = b.ptr;
    b.ptr = ptr;
    ptr = tmp;
  }

  T* release() {
    T* tmp = ptr;
    ptr = 0;
    return tmp;
  }

 private:

  // no reason to use these: each ScopedPtrMalloc should have its own object
  template <typename U, typename GP>
  bool operator==(ScopedPtrMalloc<U, GP> const& p) const;
  template <typename U, typename GP>
  bool operator!=(ScopedPtrMalloc<U, GP> const& p) const;

  static FreeProc const free_;
};

template<typename T, typename FP>
FP const ScopedPtrMalloc<T,FP>::free_ = FP();

template<typename T, typename FP> inline
void swap(ScopedPtrMalloc<T,FP>& a, ScopedPtrMalloc<T,FP>& b) {
  a.swap(b);
}

template<typename T, typename FP> inline
bool operator==(T* p, const ScopedPtrMalloc<T,FP>& b) {
  return p == b.get();
}

template<typename T, typename FP> inline
bool operator!=(T* p, const ScopedPtrMalloc<T,FP>& b) {
  return p != b.get();
}

#endif  // #ifndef SCOPED_PTR_H
