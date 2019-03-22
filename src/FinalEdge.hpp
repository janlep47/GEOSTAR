// FinalEdge.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef FINALEDGE_HPP_
#define FINALEDGE_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {

class FinalEdge {
private:
    bool done;
    PolygonPoint *originPt;  // northWestPt
    // Need a vector for when parts of polygons are one-pixel wide
    //  so that the edge can loop back over the same pixel!
    std::map<PolygonPoint*, std::vector<PolygonPoint*> > pointMap;
    std::vector<PolygonPoint*> edgePoints;
    std::vector<PolygonPoint*> sortedEdgeList;
    int lastPointDirection;
    AdjacentSorter adjacentSorter;
    
    void markTransition(PolygonPoint *fromPt, PolygonPoint *toPt);
    bool anyUntransitionedPtsFromOrigin();
    PolygonPoint* getNextPoint(PolygonPoint *fromPt);
    PolygonPoint* getBestNextPoint(std::vector<PolygonPoint*> alreadyTransitionedPoints);
    bool alreadyATransition(PolygonPoint *fromPt, PolygonPoint *toPt);
    bool anyTransitionFrom(PolygonPoint *fromPt);
    int determineLastDirection(PolygonPoint *pt, PolygonPoint *nextPt);
    std::vector<PolygonPoint*> getAdjacentPoints(PolygonPoint *pt);
public:
    
    FinalEdge();
    FinalEdge(std::vector<PolygonPoint*> &edges_xsorted);
    FinalEdge(PolygonPoint *originPt, std::vector<PolygonPoint*> &edges_xsorted);
    std::vector<PolygonPoint*> getFinalEdges();
    bool isDone();
}; // end class: FinalEdge
    
}// end namespace GeoStar


#endif //FINALEDGE_HPP_
