#include <gtest/gtest.h>
#include "../../incs/DataStructs.hpp"

class Vec2Test : public ::testing::Test {
	protected:
		void SetUp() override {

		}

		void TearDown() override {

		}
};

TEST_F(Vec2Test, Vec2Creation) {
	Vec2 pos{5, 10};

	EXPECT_EQ(pos.x, 5);
	EXPECT_EQ(pos.y, 10);
}

TEST_F(Vec2Test, Vec2Equality) {
	Vec2 pos1{5, 10};
	Vec2 pos2{5, 10};
	Vec2 pos3{3, 7};
	
	EXPECT_EQ(pos1.x, pos2.x);
	EXPECT_EQ(pos1.y, pos2.y);
	EXPECT_NE(pos1.x, pos3.x);
}