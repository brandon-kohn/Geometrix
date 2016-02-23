//
//! Copyright � 2008-2016
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef GEOMETRIX_MOVING_SPHERE_PLANE_INTERSECTION_HPP
#define GEOMETRIX_MOVING_SPHERE_PLANE_INTERSECTION_HPP

#include <geometrix/primitive/sphere_traits.hpp>
#include <geometrix/primitive/plane_traits.hpp>
#include <geometrix/tensor/vector.hpp>
#include <geometrix/algebra/expression.hpp>
#include <geometrix/algebra/dot_product.hpp>
#include <cstdint>

/////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE
//
/////////////////////////////////////////////////////////////////////////////
namespace geometrix {

	struct moving_sphere_plane_intersection_result
	{
		typedef void (moving_sphere_plane_intersection_result::*bool_type)() const;

		moving_sphere_plane_intersection_result()
		{}

		moving_sphere_plane_intersection_result( bool isIntersecting, bool isPenetrating, bool isMovingAway )
			: result( (isIntersecting ? e_is_intersecting : 0) | (isPenetrating ? e_is_penetrating : 0) | (isMovingAway ? e_is_moving_away : 0) )
		{}

		bool is_intersecting() const { return result & e_is_intersecting; }
		bool is_penetrating() const { return result & e_is_penetrating; }
		bool is_touching() const { return is_intersecting() && !is_penetrating(); }
		bool is_moving_away() const { return result & e_is_moving_away; }

		operator bool_type() const {
			return is_intersecting() ? &moving_sphere_plane_intersection_result::bool_type_mfn : 0;
		}

	private:

		void bool_type_mfn() const{}

		enum Flags
		{
			  e_is_intersecting = 1
			, e_is_penetrating = 2
			, e_is_moving_away = 4
		};

		std::uint32_t result{0};

	};

	// Intersect sphere s with movement vector v with plane p. If intersecting 
	// return time t of collision and point q at which sphere hits plane 
	// If already intersecting, q is the closest point between the center of the sphere
	// and the plane.
	template <typename Sphere, typename Vector, typename Plane, typename ArithmeticType, typename Point, typename NumberComparisonPolicy>
	inline moving_sphere_plane_intersection_result moving_sphere_plane_intersection( const Sphere& s, const Vector& velocity, const Plane& p, ArithmeticType &t, Point& q, const NumberComparisonPolicy& cmp )
	{
		// Compute distance of sphere center to plane 
		ArithmeticType dist = dot_product(p.get_normal_vector(), get_center(s)) - p.get_distance_to_origin(); 
		ArithmeticType denom = dot_product( p.get_normal_vector(), velocity );
		bool isMovingAway = cmp.greater_than_or_equal( denom * dist, 0 );
		if( cmp.less_than( math::abs( dist ), get_radius( s ) ) )
		{
			// The sphere is already overlapping the plane. Set time of 
			// intersection to zero and q to closest point on plane.
			t = 0;
			auto tclosest = dist / dot_product( p.get_normal_vector(), p.get_normal_vector() );
			assign( q, get_center( s ) - tclosest * p.get_normal_vector() );
			return moving_sphere_plane_intersection_result( true, true, isMovingAway );
		}
		else if( cmp.equals(math::abs( dist ), get_radius(s)) )
		{
			// The sphere is touching the plane. Set time of 
			// intersection to zero and q to touch-point.
			t = 0;
			auto tclosest = dist / dot_product( p.get_normal_vector(), p.get_normal_vector() );			
			assign( q, get_center( s ) - tclosest * p.get_normal_vector() );			
			return moving_sphere_plane_intersection_result( true, false, isMovingAway );
		}
		else 
		{
			if( isMovingAway )
			{
				// No intersection as sphere moving parallel to or away from plane 
				return moving_sphere_plane_intersection_result(false, false, true); 
			}
			else 
			{
				// Sphere is moving towards the plane
				// Use +r in computations if sphere in front of plane, else -r 
				ArithmeticType r = dist > 0 ? get_radius(s) : -get_radius(s);
				t = (r - dist) / denom;
				assign(q, get_center(s) + t * velocity - r * p.get_normal_vector());
				return moving_sphere_plane_intersection_result( true, true, false );
			}
		}
	}
}//namespace geometrix;

#endif //GEOMETRIX_MOVING_SPHERE_PLANE_INTERSECTION_HPP
