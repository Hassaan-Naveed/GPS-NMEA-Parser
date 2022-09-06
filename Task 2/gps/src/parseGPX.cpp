#include "parseGPX.h"
#include "consts.h"

#include <boost/algorithm/string.hpp>

namespace GPX
{

    std::string getFileContents(std::string fileAddress){
        std::ifstream file(fileAddress);
        if (file.good()){
            //If the file has no errors raised, read chars from file into content
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return content;
        } else {
            throw std::invalid_argument("Error opening source file '" + fileAddress + "'.");
        }
    }

    XML::Element getRootElement(std::string source, bool isFileName) {
        if (isFileName) {
            source = GPX::getFileContents(source);
        }
        // Seperates out XMl tags into multiple elements
        return XML::Parser(source).parseRootElement();
    }

    XML::Element checkOuterElements(XML::Element rootElement, std::string element){
        //Check that gpx exists and it contains the given element (either 'rte' or 'trk')
        if (rootElement.getName() != Consts::globalElements.gpx) throw std::domain_error("Missing '" + Consts::globalElements.gpx + "' element.");
        if (! rootElement.containsSubElement(element)) throw std::domain_error("Missing '" + element + "' element.");

        return rootElement.getSubElement(element);
    }

    GPS::Position parsePosition(XML::Element root){
        std::string lat,lon,ele = "0";

        //Check that 'lat' and 'lon' attributes exist and assign their contents to variables
        if (! root.containsAttribute(Consts::subElements.lat)) throw std::domain_error("Missing '" + Consts::subElements.lat + "' attribute.");
        if (! root.containsAttribute(Consts::subElements.lon)) throw std::domain_error("Missing '" + Consts::subElements.lon + "' attribute.");

        lat = root.getAttribute(Consts::subElements.lat);
        lon = root.getAttribute(Consts::subElements.lon);

        //'ele' attribute may not always be present
        if(root.containsSubElement(Consts::subElements.ele)){
            ele = root.getSubElement(Consts::subElements.ele).getLeafContent();
        }

        //Creates Position of the latitude, longitude, and elevation data in each point of the route or track
        return GPS::Position(lat, lon, ele);
    }

    std::string parseName(XML::Element root) {
        std::string name;

        //'name' may not always exist
        if(root.containsSubElement(Consts::subElements.name)) {
            name = root.getSubElement(Consts::subElements.name).getLeafContent();
            if(!name.empty() && !(name.find_first_not_of(' ') == std::string::npos))
                //Remove any leading or following whitespace characters from the name
                boost::trim(name);
            else name = "";
        }
        return name;
    }

    std::tm parseTime(XML::Element root){
        std::istringstream iss;

        std::string time="";
        tm datetime;

        //Check 'time' element exists and assign its contents to time variable
        if (! root.containsSubElement(Consts::subElements.time)) throw std::domain_error("Missing '" + Consts::subElements.time + "' element.");
        time = root.getSubElement(Consts::subElements.time).getLeafContent();

        //Checks time is correct format
        iss.str(time);
        iss >> std::get_time(&datetime,"%Y-%m-%dT%H:%M:%SZ");
        if (iss.fail()) throw std::domain_error("Malformed date/time content: " + time);

        return datetime;
    }


    GPS::RoutePoint parseRoutePoint(XML::Element root){
        return {parsePosition(root), parseName(root)};
    }

    std::vector<GPS::RoutePoint> getRoutePointElements(XML::Element element){
        //Checks 'trkpt' element exists and adds each TrackPoint in each 'trkpt' to results
        std::vector<GPS::RoutePoint> points;

        if (! element.containsSubElement(Consts::routeElements.rtept)) throw std::domain_error("Missing '" + Consts::routeElements.rtept + "' element.");
        for(int i = 0; i < (int)element.countSubElements(Consts::routeElements.rtept); i++){
            points.push_back(parseRoutePoint(element.getSubElement(Consts::routeElements.rtept, i)));
        }
        return points;
    }

    std::vector<GPS::RoutePoint> parseRoute(std::string source, bool isFileName){
        //Set root element to 'rte'
        XML::Element rootElement = getRootElement(source, isFileName);
        rootElement = checkOuterElements(rootElement, Consts::routeElements.rte);

        std::vector<GPS::RoutePoint> result = getRoutePointElements(rootElement);

        return result;
    }


    GPS::TrackPoint parseTrackPoint(XML::Element root) {
        return {parsePosition(root), parseName(root), parseTime(root)};
    }

    std::vector<GPS::TrackPoint> getTrackPointElements(XML::Element element, std::vector<GPS::TrackPoint> points){
        //Checks 'trkpt' element exists and adds each TrackPoint in each 'trkpt' to results
        if (! element.containsSubElement(Consts::trackElements.trkpt)) throw std::domain_error("Missing '" + Consts::trackElements.trkpt + "' element.");
        for(int i = 0; i < (int)element.countSubElements(Consts::trackElements.trkpt); i++){
            points.push_back(parseTrackPoint(element.getSubElement(Consts::trackElements.trkpt, i)));
        }
        return points;
    }

    std::vector<GPS::TrackPoint> parseTrack(std::string source, bool isFileName) {
        //Set root element to 'trk'
        XML::Element rootElement = getRootElement(source, isFileName);
        rootElement = checkOuterElements(rootElement, Consts::trackElements.trk);

        std::vector<GPS::TrackPoint> result; // struct { position, name, time }

        //If any 'trkseg' elements exist, iterate through all the 'trkpt' elements in each 'trkseg'
        if (rootElement.containsSubElement(Consts::trackElements.trkseg)) {
            for(int y = 0; y < (int)rootElement.countSubElements(Consts::trackElements.trkseg); y++) {
                result = getTrackPointElements(rootElement.getSubElement(Consts::trackElements.trkseg, y), result);
            }
        } else {
            result = getTrackPointElements(rootElement, result);
        }
        return result;
    }
}
