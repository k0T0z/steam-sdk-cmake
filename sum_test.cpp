#include "main.h"
#include <gtest/gtest.h>

namespace {
	TEST(Sumtest, PositiveNums) {
		EXPECT_EQ(5, sum(2,3));
	}
}
