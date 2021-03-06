//
//! Copyright © 2018
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <geometrix/numeric/number_comparison_policy.hpp>
#include <geometrix/primitive/point.hpp>
#include <geometrix/primitive/segment.hpp>
#include <geometrix/primitive/capsule.hpp>
#include <geometrix/utility/utilities.hpp>

#include "./2d_kernel_fixture.hpp"

#include <iostream>

TEST_F(geometry_kernel_2d_fixture, construct_from_points)
{
    auto sut = capsule2(point2{1,1}, point2{2,2}, 0.2);
}

TEST_F(geometry_kernel_2d_fixture, construct_from_segment)
{
    auto sut = capsule2(segment2{point2{1,1}, point2{2,2}}, 0.2);
}
