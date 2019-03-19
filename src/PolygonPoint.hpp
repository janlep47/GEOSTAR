// PolygonPoint.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef POLYGONPOINT_HPP_
#define POLYGONPOINT_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {

class PolygonPoint {
public:
    int x;
    int y;
    int status;
    static const int NORMAL_POINT = 0;
    static const int HOLE_POINT = 1;
    static const int EDGE_POINT = 2;
    static const int MARKED_FOR_DELETION = 3;
    //bool markedForDeletion;
    //bool abuttingHole;
    
    
    PolygonPoint() {
        x = 0;
        y = 0;
        status = NORMAL_POINT;
        //markedForDeletion = false;
        //abuttingHole = false;
    }
    
    PolygonPoint(int x, int y) {
        this->x = x;
        this->y = y;
        status = NORMAL_POINT;
        //markedForDeletion = false;
        //abuttingHole = false;
    }
    
    void markForDeletion() {
        status = MARKED_FOR_DELETION;
        //markedForDeletion = true;
    }
    
    bool isMarkedForDeletion() {
        if (status == MARKED_FOR_DELETION) return true;
        return false;
        //return markedForDeletion;
    }
    
    bool isAbuttingHole() {
        if (status == HOLE_POINT) return true;
        return false;
        //return abuttingHole;
    }
    
    void markAbuttingHole() {
        status = HOLE_POINT;
        //abuttingHole = true;
    }
    
    bool isEdgePoint() {
        if (status == EDGE_POINT) return true;
        return false;
    }
    
    void setRegularEdgePoint() {
        status = EDGE_POINT;
        //abuttingHole = false;
    }
};
    
}// end namespace GeoStar


#endif //POLYGONPOINT_HPP_

