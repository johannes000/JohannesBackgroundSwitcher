#include <gtest/gtest.h>
#include <string>

TEST(SanityTest, Add) {
	EXPECT_EQ(2 + 2, 4);
}

TEST(SanityTest, StringIsNotEmpty) {
	std::string output = "World";
	EXPECT_FALSE(output.empty());
}
