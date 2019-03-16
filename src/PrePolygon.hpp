// PrePolygon.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef PREPOLYGON_HPP_
#define PREPOLYGON_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {
    
template <class T>
class PrePolygon {
    
private:
    int previousRowIndex;
    bool done;
    bool hasHoles;
    bool isContained;
    std::vector<PolygonPoint*> emptyVector;
    
    // edges are the pixel boundaries; these are recalculated after each full pass
    //  of a column of pixel data.  e.g. (0,2) -> (0,5)
    // Until the complete polygon boundaries have been found, the edges consist
    // of all the pixel addresses which are part of the edge, with x,y being the
    // CENTER of the pixel:
    //
    std::vector<PolygonPoint*> edges_xsorted;
    std::vector<PolygonPoint*> edges_ysorted;
    // These are the (compressed) edges used for setting up the gdal polygon
    //  - only calculated when the polygon's complete bounds have been determined:
    std::vector<PolygonPoint*> edges_list;
    
    int polygonNumber;
    T pixelValue;
    
    // will only save 2 entries in pixelAddrs, y and y - 1   (if y != 0)
    // key is y-value and value is list of pixel points
    std::map<int, std::vector<PolygonPoint*> > pixelAddrs;
    std::vector<PrePolygon*> containedPolygons;
    
    EdgeXSorter xSorter;
    EdgeYSorter ySorter;
    
    
    void checkPointOnRightHole(PolygonPoint *pt);
    void checkPointOnLeftHole(PolygonPoint *pt);
    void checkPointOnBottomHole(PolygonPoint *pt);
    void checkPointOnTopHole(PolygonPoint *pt);
    bool anyPointJustLeft(PolygonPoint *pt, int index);
    bool anyPointJustRight(PolygonPoint *pt, int index);
    bool anyPointJustAbove(PolygonPoint *pt);
    bool anyPointJustBelow(PolygonPoint *pt);
    bool anyPointAbove(PolygonPoint *pt);
    int findXsortedPoint(PolygonPoint *pt);
    int findYsortedPoint(PolygonPoint *pt);
    int findXsortedPoint(int x, int y);
    int findYsortedPoint(int x, int y);
    bool minMaxRowOrCol(PolygonPoint *pt);
    bool isPointInside(PolygonPoint *testPt);
    int getNumberThisPointsAbove(PolygonPoint *pt);
    int getNumberThisPointsLeft(PolygonPoint *pt);
    int getNumberThisPointsRight(PolygonPoint *pt);
    int getNumberThisPointsBelow(PolygonPoint *pt);
public:
    
    
    PrePolygon();
    PrePolygon(int &polygonNumber, T &pixelValue, int x, int y);
    PrePolygon(int &polygonNumber, T &pixelValue, std::vector<PolygonPoint*> &points);
    int getPolygonNumber();
    T getPixelValue();
    std::vector<PolygonPoint*> getEdgePoints();
    
    // This function is called when another polygon is coalesced into
    //  this polygon.
    void addEdgePoints(std::vector<PolygonPoint*> newEdgePoints);
    std::vector<PolygonPoint*> getPoints(int y);
    void addPoints(std::vector<PolygonPoint*> newPoints, int y);
    void addPoint(int x, int y);
    void setDone(bool done);
    bool isDone();
    void setAlreadyContained();
    bool isAlreadyContained();
    void edgify(int y);
    void findFinalBoundingPolygon();
    bool hasAnyHoles();
    bool isPolygonInside(PrePolygon *smallerPolygon);
    void addContainedPolygon(PrePolygon *insidePolygon);
    void spinOffPolygon();
    void printMoreCrap();
    
};   // end class: PrePolygon
    
}// end namespace GeoStar


#endif //PREPOLYGON_HPP_

