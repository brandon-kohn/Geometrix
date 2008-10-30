//
//!  Copyright (c) 2008
//!  Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef _BOOST_GEOMETRY_KD_TREE_HPP
#define _BOOST_GEOMETRY_KD_TREE_HPP
#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/range.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include "bounding_box_intersection.hpp"
#include "point_sequence_utilities.hpp"

namespace boost
{
namespace numeric
{
namespace geometry
{
    namespace detail
    {
        template <unsigned int D, typename NumberComparisonPolicy>
        struct dimension_compare
        {
            dimension_compare( const NumberComparisonPolicy& compare )
                : m_compare( compare )
                , m_lexicographicalCompare( compare )
            {}

            template <typename NumericSequence>
            typename bool operator()( const NumericSequence& lhs, const NumericSequence& rhs ) const
            {
                //! Sequences are compared by dimension specified. In the case where coordinates at D are equal, the sequences are
                //! given a total order lexicographically.
                typedef typename boost::remove_const< NumericSequence >::type sequence_type;
                if( m_compare.less_than( indexed_access_traits< sequence_type >::get<D>( lhs ), indexed_access_traits< sequence_type >::get<D>( rhs ) ) )
                {
                    return true;
                }
                else if( m_compare.equals( indexed_access_traits< sequence_type >::get<D>( lhs ), indexed_access_traits< sequence_type >::get<D>( rhs ) ) )
                {
                    return m_lexicographicalCompare( lhs, rhs );
                }
                else
                {
                    return false;
                }
            }

            NumberComparisonPolicy                          m_compare;
            lexicographical_compare<NumberComparisonPolicy> m_lexicographicalCompare;
        };  
    }

    //! \class kd-tree< NumericSequence >
    //! \brief A data structure used to store a set of points in N-dimensional space with search query functionality.
    //* A kd_tree may be used to perform queries on points within an N-dimensional orthogonal bound. The run-time complexity
    //* of queries is O( sqrt(n) * k ) where n is the total number of points in the tree and k is the number of points found
    //* in the query.
    //* \example
    //* using namespace boost::numeric::geometry;
    //* typedef point_double_3d CPoint3D;
	//* std::vector< CPoint3D > points;
    //* random_real_generator< boost::mt19937 > rnd(10.0);
    //* 
    //* for( size_t i=0; i < 1000; ++i )
    //* {
    //*     double x = rnd();
    //*     double y = rnd();
    //*     double z = rnd();
    //*     points.push_back( CPoint3D( x, y, z ) ); 
    //* }
    //*
    //* fraction_tolerance_comparison_policy<double> compare(1e-10);
    //* kd_tree< CPoint3D > tree( points, compare );
    //* 
    //* //! Specify a volume (box) with diagonal vector from 0,0,0, to 5,5,5 for the search range.
    //* orthogonal_range< CPoint3D > range( CPoint3D( 0.0, 0.0, 0.0 ), CPoint3D( 5.0, 5.0, 5.0 ) );
    //*
    //* //! Visit all the points inside the volume
    //* PointVisitor visitor( points );
    //* tree.search( range, visitor, compare );
    template <typename NumericSequence>
    class kd_tree
    {
    public:

        typedef NumericSequence                                                 sequence_type;
        typedef typename sequence_traits< sequence_type >::dimension_type       dimension_type;
        typedef typename numeric_sequence_traits< sequence_type >::numeric_type numeric_type;

        template <typename PointSequence, typename NumberComparisonPolicy>
        kd_tree( const PointSequence& pSequence, const NumberComparisonPolicy& compare, typename boost::enable_if< is_point_sequence< PointSequence > >::type* dummy=0 )
            : m_region( pSequence, compare )
        {
            build( pSequence, compare );
        }

        //! Traverse the tree on a range and visit all leaves in the specified range.
        template <typename NumericSequence, typename Visitor, typename NumberComparisonPolicy>
        inline void search( const orthogonal_range<NumericSequence>& range, Visitor& visitor, const NumberComparisonPolicy& compare ) const
        {
            if( m_pLeaf )
                visitor( *m_pLeaf );
            else
                search<0>( range, visitor, compare );
        }

    private:

        kd_tree( const orthogonal_range< sequence_type >& region )
            : m_region( region )
        {}

        template <typename NumericSequence, unsigned int Dimension, unsigned int D> 
        friend struct kd_tree_builder;
        
        template <typename NumericSequence, unsigned int Dimension, unsigned int D>
        struct kd_tree_builder
        {
            template <typename NumericSequence> 
            friend class kd_tree;

            template <typename NumberComparisonPolicy>
            static void build_tree( boost::shared_ptr< kd_tree<NumericSequence> >& pTree, std::vector< NumericSequence >& pSequence, const NumberComparisonPolicy& compare )
            {
                std::size_t pSize = pSequence.size();
                if( pSize == 1 )
                {
                    pTree->m_pLeaf.reset( new sequence_type( pSequence[0] ) );
                }
                else
                {                    
                    //! Sort on the dimension.
                    std::sort( pSequence.begin(), pSequence.end(), detail::dimension_compare<Dimension, NumberComparisonPolicy>( compare ) );
                    
                    size_t medianIndex = pSize/2;
                    pTree->m_median = indexed_access_traits< NumericSequence >::get<Dimension>( pSequence[ medianIndex ] );

                    //! Split to the left tree those that are on left or collinear of line... and to the right those on the right.
                    std::vector< sequence_type > left( pSequence.begin(), pSequence.begin() + medianIndex );
                    std::vector< sequence_type > right( pSequence.begin() + medianIndex, pSequence.end() );
                  
                    if( !left.empty() )
                    {   
                        sequence_type upperBound = pTree->m_region.get_upper_bound();
                        indexed_access_traits< sequence_type >::get<Dimension>( upperBound ) = pTree->m_median;
                        pTree->m_pLeftChild.reset( new kd_tree<NumericSequence>( orthogonal_range< sequence_type >( pTree->m_region.get_lower_bound(), upperBound ) ) );
                        kd_tree_builder<NumericSequence, (Dimension+1)%D, D>::build_tree( pTree->m_pLeftChild, left, compare );
                    }
                    if( !right.empty() )
                    {
                        sequence_type lowerBound = pTree->m_region.get_lower_bound();
                        indexed_access_traits< sequence_type >::get<Dimension>( lowerBound ) = pTree->m_median;                    
                        pTree->m_pRightChild.reset( new kd_tree<NumericSequence>( orthogonal_range< sequence_type >( lowerBound, pTree->m_region.get_upper_bound() ) ) );
                        kd_tree_builder<NumericSequence, (Dimension+1)%D, D>::build_tree( pTree->m_pRightChild, right, compare );
                    }
                }
            }
        };

        template <typename PointSequence, typename NumberComparisonPolicy>
        inline void build( const PointSequence& pSequence, const NumberComparisonPolicy& compare )
        {
            std::size_t pSize = point_sequence_traits< PointSequence >::size( pSequence );
            if( pSize == 1 )
            {
                m_pLeaf.reset( new sequence_type( point_sequence_traits< PointSequence >::get_point( pSequence, 0 ) ) );
            }
            else
            {
                std::vector< sequence_type > sortedSequence( point_sequence_traits< PointSequence >::begin( pSequence ), point_sequence_traits< PointSequence >::end( pSequence ) );

                std::sort( sortedSequence.begin(), sortedSequence.end(), detail::dimension_compare<0, NumberComparisonPolicy>( compare ) );
                size_t medianIndex = pSize/2;
                m_median = indexed_access_traits< NumericSequence >::get<0>( sortedSequence[ medianIndex ] );

                //! Split to the left tree those that are on left or collinear of line... and to the right those on the right.
                std::vector< sequence_type > left( sortedSequence.begin(), sortedSequence.begin() + medianIndex );
                std::vector< sequence_type > right( sortedSequence.begin() + medianIndex, sortedSequence.end() ); 
                 
                if( !left.empty() )
                {   
                    sequence_type upperBound = m_region.get_upper_bound();
                    indexed_access_traits< sequence_type >::get<0>( upperBound ) = m_median;
                    m_pLeftChild.reset( new kd_tree<NumericSequence>( orthogonal_range< sequence_type >( m_region.get_lower_bound(), upperBound ) ) );
                    kd_tree_builder<NumericSequence, 1, dimension_type::value>::build_tree( m_pLeftChild, left, compare );
                }
                if( !right.empty() )
                {
                    sequence_type lowerBound = m_region.get_lower_bound();
                    indexed_access_traits< sequence_type >::get<0>( lowerBound ) = m_median;                    
                    m_pRightChild.reset( new kd_tree<NumericSequence>( orthogonal_range< sequence_type >( lowerBound, m_region.get_upper_bound() ) ) );
                    kd_tree_builder<NumericSequence, 1, dimension_type::value>::build_tree( m_pRightChild, right, compare );
                }
            }
        }

        template <unsigned int Dimension, typename Visitor, typename NumberComparisonPolicy>
        inline void search( const orthogonal_range<sequence_type>& range, Visitor& visitor, const NumberComparisonPolicy& compare ) const
        {
            if( m_pLeaf )
            {
                visitor( *m_pLeaf );
                return;
            }

            //! Search the left child.
            if( m_pLeftChild )
            {
                if( range.contains( m_pLeftChild->m_region, compare ) )
                    m_pLeftChild->traverse_subtrees( visitor );
                else if( range.intersects( m_pLeftChild->m_region, compare ) )
                    m_pLeftChild->search<(Dimension+1)%dimension_type::value>( range, visitor, compare );
            }

            //! Search the right child.
            if( m_pRightChild )
            {
                if( range.contains( m_pRightChild->m_region, compare ) )
                    m_pRightChild->traverse_subtrees( visitor );
                else if( m_pRightChild && range.intersects( m_pRightChild->m_region, compare ) )
                    m_pRightChild->search<(Dimension+1)%dimension_type::value>( range, visitor, compare );
            }
        }

        template <typename Visitor>
        inline void traverse_subtrees( Visitor& v ) const
        {
            if( m_pLeaf )
            {
                v( *m_pLeaf );
                return;
            }
            
            if( m_pLeftChild )
                m_pLeftChild->traverse_subtrees( v );
            if( m_pRightChild )
                m_pRightChild->traverse_subtrees( v );
        }

        typedef boost::shared_ptr< kd_tree< sequence_type > > dimension_split;
        typedef sequence_type                                 leaf;
        typedef boost::shared_ptr< sequence_type >            leaf_ptr;
        numeric_type                                          m_median;
        dimension_split                                       m_pLeftChild;
        dimension_split                                       m_pRightChild;
        leaf_ptr                                              m_pLeaf;
        orthogonal_range< sequence_type >            m_region;

    };

}}}//namespace boost::numeric::geometry

#endif //_BOOST_GEOMETRY_KD_TREE_HPP
