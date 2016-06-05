//
//! Copyright � 2016
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef GEOMETRIX_NUMERIC_BOOST_UNITS_QUANTITY_HPP
#define GEOMETRIX_NUMERIC_BOOST_UNITS_QUANTITY_HPP

#include <geometrix/space/affine_space_traits.hpp>
#include <geometrix/tensor/tensor_access_policy.hpp>

#include <boost/units/quantity.hpp>
#include <boost/units/limits.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace geometrix {

	//! Define numeric traits for the coordinate type.
	template <typename Unit, typename Numeric>
	struct is_numeric< boost::units::quantity< Unit, Numeric > > : boost::true_type {};

	template <typename Unit, typename Numeric>
	struct numeric_traits< boost::units::quantity< Unit, Numeric > >
	{
		typedef boost::units::quantity< Unit, Numeric >      numeric_type;
		typedef typename boost::is_float< Numeric >::type    is_float;
		typedef typename boost::is_integral< Numeric >::type is_integral;
		typedef void                                         is_numeric;
		static const unsigned int digits = std::numeric_limits<Numeric>::digits10;

		BOOST_MPL_ASSERT_MSG
		(
			(!(is_float::value && is_integral::value))
			, NUMERIC_TRAITS_CANNOT_DEFINE_FLOAT_AND_INTEGRAL
			, (boost::units::quantity< Unit, Numeric >)
		);
	};

	template <typename Unit, typename Numeric>
	struct construction_policy< boost::units::quantity< Unit, Numeric > >
	{
		template <typename T>
		static boost::units::quantity< Unit, Numeric > construct(T&& n) { return boost::units::quantity< Unit, Numeric >(boost::numeric_cast<Numeric>(geometrix::get(n)) * Unit()); }
		template <typename T, typename Unit2>
		static boost::units::quantity< Unit, Numeric > construct(const boost::units::quantity<Unit2, T>& n) { return boost::units::quantity< Unit, Numeric >(n); }
	};

	template <typename Unit, typename Numeric>
	struct construction_policy< const boost::units::quantity< Unit, Numeric > >
	{
		template <typename T>
		static const boost::units::quantity< Unit, Numeric > construct(T&& n) { return boost::units::quantity< Unit, Numeric >(boost::numeric_cast<Numeric>(geometrix::get(n)) * Unit()); }
		template <typename T, typename Unit2>
		static const boost::units::quantity< Unit, Numeric > construct(const boost::units::quantity<Unit2, T>& n) { return boost::units::quantity< Unit, Numeric >(n); }
	};

}//namespace geometrix;

#endif //GEOMETRIX_NUMERIC_BOOST_UNITS_QUANTITY_HPP