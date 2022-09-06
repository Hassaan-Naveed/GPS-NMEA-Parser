#ifndef PARSEGPX_H_201220
#define PARSEGPX_H_201220

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "points.h"
#include "xml/parser.h"

namespace GPX
{   
    /* Locates a file from its address and outputs its contents as a string
     */
    std::string getFileContents(std::string fileAddress);

    /* Seperates file into multiple elements
     */
    XML::Element getRootElement(std::string source, bool isFileName);

    /* Checks if the gpx and rte or trk elements exist
     */
    XML::Element checkOuterElements(XML::Element rootElement, std::string element);

    /* Returns the Position within the rtept or trkpt elements
     */
    GPS::Position parsePosition(XML::Element root);

    /* Returns the name within the rtept or trkpt elements
     */
    std::string parseName(XML::Element root);

    /* Returns the time within the trkpt element
     */
    std::tm parseTime(XML::Element root);

    /* Returns a RoutePoint containing the Position and name of the point
     */
    GPS::RoutePoint parseRoutePoint(XML::Element element);

    /* Returns vector of all RoutePoint elements
     */
    std::vector<GPS::RoutePoint> getRoutePointElements(XML::Element element);

    /* Parse GPX data containing a route.
    *  The source data can be provided as a string, or from a file; which one is determined by the bool parameter.
    */
    std::vector<GPS::RoutePoint> parseRoute(std::string source, bool isFileName);

    /* Returns a TrackPoint containing the Position, name, and time of the point
     */
    GPS::TrackPoint parseTrackPoint(XML::Element element);

    /* Returns vector of all TrackPoint elements
     */
    std::vector<GPS::TrackPoint> getTrackPointElements(XML::Element element, std::vector<GPS::TrackPoint> points);

    /* Parse GPX data containing a track.
    *  The source data can be provided as a string, or from a file; which one is determined by the bool parameter.
    */
    std::vector<GPS::TrackPoint> parseTrack(std::string source, bool isFileName);
}

#endif
