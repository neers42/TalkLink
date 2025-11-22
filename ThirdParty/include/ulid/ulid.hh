#ifndef ULID_HH
#define ULID_HH

// http://stackoverflow.com/a/23981011
//#ifdef __SIZEOF_INT128__
//#define ULIDUINT128
//#endif
//#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
//#include "ulid_uint128.hh"
//#else
//#include "ulid_struct.hh"
//#endif // ULIDUINT128


// ユーザーが明示的にULIDの構造体を使うよう指定
//#ifdef NO_ULID_UINT128
//#include "ulid_struct.hh"
//#else
//  // http://stackoverflow.com/a/23981011
//#ifdef __SIZEOF_INT128__
//#define ULIDUINT128
//#endif
//
//#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
//#include "ulid_uint128.hh"
//#else
//#include "ulid_struct.hh"
//#endif
//#endif


#if defined(NO_ULID_UINT128)
#include "ulid_struct.hh"
#else
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
#include "ulid_uint128.hh"
#else
#include "ulid_struct.hh"
#endif
#endif

#endif // ULID_HH
