#include <boost/test/unit_test.hpp>

#include "types.h"
#include "points.h"
#include "route.h"
#include "track.h"
#include "gridworld_track.h"
#include <iostream>

using namespace GPS;
using namespace GridWorld;

BOOST_AUTO_TEST_SUITE( Route_findPosition )

const double epsilon = 0.0001;

const Position pos1 = Position(20,2);
const Position pos2 = Position(30,3);
const Position pos3 = Position(40,4);
const Position pos4 = Position(50,5);
const Position pos5 = Position(60,6);
const Position pos6 = Position(70,7);

const RoutePoint rtpt1 = { pos1, "P1" };
const RoutePoint rtpt2 = { pos2, "P2" };
const RoutePoint rtpt3 = { pos3, "P3" };
const RoutePoint rtpt4 = { pos4, "P4" };
const RoutePoint rtpt5 = { pos5, "P5" };
const RoutePoint rtpt6 = { pos5, "P6" };

const std::vector<RoutePoint> routePoints = { rtpt1, rtpt2, rtpt3, rtpt4, rtpt5, rtpt6 };
const Route route {routePoints};

const metres horizontalGridUnit = 100000;
const metres verticalGridUnit = 0;
GridWorldModel gwNearEquator {Earth::Pontianak,horizontalGridUnit,verticalGridUnit};

///////////////////////////////////////////////////////////////////////////////

// Typical Input - Searching for a Position in the middle
BOOST_AUTO_TEST_CASE( TypicalName )
{
    const Position expectedPosition = rtpt4.position;

    const std::string name = rtpt4.name;
    Position actualPosition = route.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Typical Input - Searching for position containing non-standard characters in its' name
BOOST_AUTO_TEST_CASE( SpacesAndSymbols )
{
    const RoutePoint rtpt1 = { pos1, "P / 1" };
    const RoutePoint rtpt2 = { pos2, "P / 2" };
    const RoutePoint rtpt3 = { pos3, "P / 3" };

    const std::vector<RoutePoint> routePoints = { rtpt1, rtpt2, rtpt3};
    const Route route {routePoints};

    const Position expectedPosition = rtpt2.position;

    const std::string name = rtpt2.name;
    Position actualPosition = route.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Erroneous Input - Empty String to check if invalid argument is thrown
BOOST_AUTO_TEST_CASE( EmptyString )
{
    const std::string emptyInput = "";
    BOOST_CHECK_THROW( route.findPosition(emptyInput);, std::invalid_argument );
}

// Erroneous Input - No RoutePOint exists with given name to check if domain error is thrown
BOOST_AUTO_TEST_CASE( NameNotFound )
{
    const std::string invalidName = "Invalid";
    BOOST_CHECK_THROW( route.findPosition(invalidName);, std::domain_error );
}

// Edge Case - First Item in Route object
BOOST_AUTO_TEST_CASE( FirstItem )
{
    const Position expectedPosition = rtpt1.position;

    const std::string name = rtpt1.name;
    Position actualPosition = route.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Edge Case - Last Item in Route Object
BOOST_AUTO_TEST_CASE( LastItem )
{
    const Position expectedPosition = rtpt6.position;

    const std::string name = rtpt6.name;
    Position actualPosition = route.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Edge Case - Single Point in Route Object
BOOST_AUTO_TEST_CASE( SinglePoint )
{
    const Position expectedPosition = rtpt1.position;

    const std::string name = rtpt1.name;
    Position actualPosition = route.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Boundary Case - Multiple points with same name to check if function can find the second name
BOOST_AUTO_TEST_CASE( sharedName )
{
    const RoutePoint duplicateName = { pos2, "P1" };
    const std::vector<RoutePoint> routePoints = { rtpt1, duplicateName, rtpt3, rtpt4, rtpt5, rtpt6 };
    const Route route {routePoints};
    const Position expectedPosition = duplicateName.position;

    Position actualPosition = route.findPosition("P1");

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Check that findPosition also works correctly for Track class
BOOST_AUTO_TEST_CASE( TrackPoints )
{
    const std::vector<TrackPoint> trackPoints = GridWorldTrack("A1B1C1D1E",gwNearEquator).toTrackPoints();
    const metres granularity = horizontalGridUnit * 0.1; // Ensure no points are merged.
    const Track track {trackPoints,granularity};

    const Position expectedPosition = trackPoints[0].position;

    const std::string name = trackPoints[0].name;
    Position actualPosition = track.findPosition(name);

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

// Check that findPosition() takes into account merged points.
BOOST_AUTO_TEST_CASE( MergedPoints )
{
    const std::vector<TrackPoint> trackPoints = GridWorldTrack("A1B1C1D1E",gwNearEquator).toTrackPoints();
    const metres granularity = horizontalGridUnit * 1.5; // Every second point is merged.
    const Track track {trackPoints,granularity};

    const Position expectedPosition = trackPoints[1].position;

    const std::string name = trackPoints[1].name;
    Position actualPosition = track.findPosition(name);

    std::cout << actualPosition.latitude() << " " << actualPosition.longitude();

    BOOST_CHECK_SMALL( Position::horizontalDistanceBetween(actualPosition, expectedPosition), epsilon );
}

BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
