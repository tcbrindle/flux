
#ifndef FLUX_TEST_CATCH_HPP_INCLUDED
#define FLUX_TEST_CATCH_HPP_INCLUDED

#include <catch2/catch_test_macros.hpp>

// Hack: we already have a macro called STATIC_CHECK which is used thousands
// of times, so undef the version provided by the latest Catch
#ifdef STATIC_CHECK
#undef STATIC_CHECK
#endif

#endif
