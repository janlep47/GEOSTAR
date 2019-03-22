// LatLong.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef LATLONG_HPP_
#define LATLONG_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {
    
class LatLong {
private:
    double latitude;    // y
    double longitude;   // x
    
public:
    
    LatLong() {
        this->latitude = 0.0;
        this->longitude = 0.0;
    }
    
    LatLong(double longitude, double latitude) {
        this->longitude = longitude;
        this->latitude = latitude;
    }
    
    
    void printInfo() {
        std::cout << "(" << longitude << ","
        << latitude << ") ";
        return;
    }
};   // end class: LatLong
    
}// end namespace GeoStar


#endif //LATLONG_HPP_

