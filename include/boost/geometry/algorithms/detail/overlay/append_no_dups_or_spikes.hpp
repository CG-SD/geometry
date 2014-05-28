// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP

#include <boost/range.hpp>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// TODO: move this / rename this
template <typename Point1, typename Point2, typename RobustPolicy>
inline bool points_equal_or_close(Point1 const& point1,
        Point2 const& point2,
        RobustPolicy const& robust_policy)
{
    if (detail::equals::equals_point_point(point1, point2))
    {
        return true;
    }

    if (! RobustPolicy::enabled)
    {
        return false;
    }

    // Try using specified robust policy
    typedef typename geometry::robust_point_type
    <
        Point1,
        RobustPolicy
    >::type robust_point_type;

    robust_point_type point1_rob, point2_rob;
    geometry::recalculate(point1_rob, point1, robust_policy);
    geometry::recalculate(point2_rob, point2, robust_policy);

    return detail::equals::equals_point_point(point1_rob, point2_rob);
}


template <typename Range, typename Point, typename RobustPolicy>
inline void append_no_dups_or_spikes(Range& range, Point const& point,
        RobustPolicy const& robust_policy)
{
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    std::cout << "  add: ("
        << geometry::get<0>(point) << ", " << geometry::get<1>(point) << ")"
        << std::endl;
#endif
    // The code below thies condition checks all spikes/dups
    // for geometries >= 3 points.
    // So we have to check the first potential duplicate differently
    if (boost::size(range) == 1
        && points_equal_or_close(*(boost::begin(range)), point, robust_policy))
    {
        return;
    }

    traits::push_back<Range>::apply(range, point);

    // If a point is equal, or forming a spike, remove the pen-ultimate point
    // because this one caused the spike.
    // If so, the now-new-pen-ultimate point can again cause a spike
    // (possibly at a corner). So keep doing this.
    // Besides spikes it will also avoid adding duplicates.
    while(boost::size(range) >= 3
            && point_is_spike_or_equal(point,
                *(boost::end(range) - 3),
                *(boost::end(range) - 2),
                robust_policy))
    {
        // Use the Concept/traits, so resize and append again
        traits::resize<Range>::apply(range, boost::size(range) - 2);
        traits::push_back<Range>::apply(range, point);
    }
}

template <typename Range, typename RobustPolicy>
inline void clean_closing_dups_and_spikes(Range& range,
                RobustPolicy const& robust_policy)
{
    int const minsize
        = core_detail::closure::minimum_ring_size
            <
                geometry::closure<Range>::value
            >::value;

    if (boost::size(range) <= minsize)
    {
        return;
    }

    typedef typename boost::range_iterator<Range>::type iterator_type;
    const bool closed = geometry::closure<Range>::value == geometry::closed;
    bool found = false;
    do
    {
        found = false;
        iterator_type first = boost::begin(range);
        iterator_type second = first + 1;
        iterator_type ultimate = boost::end(range) - 1;
        if (closed)
        {
            ultimate--;
        }

        // Check if closing point is a spike (this is so if the second point is
        // considered as a spike w.r.t. the last segment)
        if (point_is_spike_or_equal(*second, *ultimate, *first, robust_policy))
        {
            range.erase(first);
            if (closed)
            {
                // Remove closing last point
                traits::resize<Range>::apply(range, boost::size(range) - 1);
                // Add new closing point
                traits::push_back<Range>::apply(range, *boost::begin(range));
            }
            found = true;
        }
    } while(found && boost::size(range) > minsize);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP
