// EdgeStrokes.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef EDGESTROKES_HPP_
#define EDGESTROKES_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {
    
class EdgeStrokes {
private:
    double distancePerPixel;
    double upperLeftLat;
    double upperLeftLong;
    PolygonPoint *originPt;
    
    // next 3 are related to the most recent stroke position
    double currentLatitude;
    double currentLongitude;
    int lastStrokePosn;  // N, NW, ... etc. r/t currentPt position
    std::vector<LatLong*> strokes;
    
    int relativePosn(PolygonPoint *first, PolygonPoint *second);
    void getStrokes( PolygonPoint *prevPt, PolygonPoint *pt,
                    PolygonPoint *nextPt);
    bool lastStrokeAtNewPt(int newPointPosn);
    void doOneStroke();
    void reAssignLastStrokePosn(PolygonPoint *pt,PolygonPoint *nextPt,
                                int newPointPosn);
    void goUpOne();
    void goDownOne();
    void goLeftOne();
    void goRightOne();
public:
    // next for are the general direction the strokes are going,
    //  depending on where in the polygon ... use the 4 "corners"
    //  as the guide
    
    static const int N = 1;
    static const int NW = 2;
    static const int W = 3;
    static const int SW = 4;
    static const int S = 5;
    static const int SE = 6;
    static const int E = 7;
    static const int NE = 8;
    
    EdgeStrokes();
    EdgeStrokes(PolygonPoint *originPt, double upperLeftLat, double upperLeftLong,
                double distancePerPixel);
    void createStrokes(std::vector<PolygonPoint*> edgePoints);
    void printStrokeInfo(PolygonPoint *pt,PolygonPoint *nextPt,
                         int newPointPosn);
    std::string getPrintablePosn(int posn);
    std::vector<LatLong*> getStrokes();
};
    
}// end namespace GeoStar


#endif //EDGESTROKES_HPP_
