#ifndef CONSTS_H
#define CONSTS_H

#endif // CONSTS_H
#include <string>

namespace Consts {

    //Constant variables for all elements that are needed
    struct trackElements{
        const std::string
        trkpt = "trkpt",
        trk = "trk",
        trkseg = "trkseg";
    }trackElements;

    struct routeElements{
        const std::string
        rte = "rte",
        rtept = "rtept";
    }routeElements;

    struct globalElements{
        const std::string
        gpx = "gpx";
    }globalElements;

    struct subElements{
        const std::string
        lat = "lat",
        lon = "lon",
        ele = "ele",
        name = "name",
        time = "time";
    }subElements;
}
