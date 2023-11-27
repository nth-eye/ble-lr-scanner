// #include "test.h"
// #include "nth/io/print_v2.h"

// namespace nth::v1 {

// TEST(IoPrint, ParseFmtTest)
// {
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check(""));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{:}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{:arg}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{0}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{0:}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{0:arg}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{}{}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{:}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{:arg}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0:}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{{"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("}}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{{}}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{{{}{}}}text"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{{{}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{}}}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{ {{ }"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{ }} }"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{}}{{}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("{0:{{}}}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{}text{0:arg}"));
// #if (NTH_IO_PRINT_VECTOR_CACHE)
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{:}"));
//     ASSERT_EQ(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{arg}"));
// #else
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{}"));
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{:}"));
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("text{0:arg}text{arg}"));
// #endif
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("{0: {} }"));  // TODO: Make work!
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("{ {} }"));    // TODO: Make work!
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("{"));
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("}"));
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("text { text"));
//     ASSERT_NE(imp::fmt_error::ok, imp::fmt_check("text } text"));
// }

// }
