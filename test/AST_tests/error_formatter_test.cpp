#include "AST/SourceRange.hpp"
#include "errors-output/error-formatter.hpp"

#include <gtest/gtest.h>

TEST(ErrorFormatterTest, FormatsGnuErrorWithLocation)
{
    ast::SourceRange range;
    range.file = "sample.pcl";
    range.begin_line = 3;
    range.begin_column = 14;

    EXPECT_EQ(err::format_error(range, "boom"), "sample.pcl:3:14: error: boom");
}

TEST(ErrorFormatterTest, FormatsErrorWithoutLocation)
{
    ast::SourceRange range;

    EXPECT_EQ(err::format_error(range, "boom"), "error: boom");
}
