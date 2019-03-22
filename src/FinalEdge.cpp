// FinalEdge.cpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>

#include "PolygonPoint.hpp"
#include "PolygonSorters.hpp"
#include "FinalEdge.hpp"

namespace GeoStar {

    FinalEdge::FinalEdge() {
        done = false;
    }
    
    FinalEdge::FinalEdge(std::vector<PolygonPoint*> &edges_xsorted) {
        done = false;
        originPt = edges_xsorted[0];
        edgePoints.push_back(originPt);
        sortedEdgeList = edges_xsorted;
        lastPointDirection = adjacentSorter.SW;
    }
    
    FinalEdge::FinalEdge(PolygonPoint *originPt, std::vector<PolygonPoint*> &edges_xsorted) {
        done = false;
        this->originPt = originPt;
        edgePoints.push_back(originPt);
        sortedEdgeList = edges_xsorted;
        lastPointDirection = adjacentSorter.SW;
    }
    
    void FinalEdge::markTransition(PolygonPoint *fromPt, PolygonPoint *toPt) {
        std::map<PolygonPoint*, std::vector<PolygonPoint*> >::iterator it;
        it = pointMap.find(fromPt);
        if (it == pointMap.end()) {
            std::vector<PolygonPoint*> points;
            points.push_back(toPt);
            pointMap[fromPt] = points;
        } else {
            std::vector<PolygonPoint*> points = it->second;
            points.push_back(toPt);
            pointMap[fromPt] = points;
        }
        edgePoints.push_back(toPt);
        if (toPt->x == originPt->x &&
            toPt->y == originPt->y &&
            !anyUntransitionedPtsFromOrigin()) done = true;
    }
    
    bool FinalEdge::anyUntransitionedPtsFromOrigin() {
        std::vector<PolygonPoint*> originAdjacentPoints =
        getAdjacentPoints(originPt);
        int i = 0;
        while (i < originAdjacentPoints.size()) {
            PolygonPoint *nextPt = originAdjacentPoints[i];
            if (!anyTransitionFrom(nextPt)) return true;
            i++;
        }
        return false;
    }
    
    
    
    PolygonPoint* FinalEdge::getNextPoint(PolygonPoint *fromPt) {
        
        adjacentSorter.x = fromPt->x;
        adjacentSorter.y = fromPt->y;
        adjacentSorter.lastDirection = lastPointDirection;
        std::sort (sortedEdgeList.begin(), sortedEdgeList.end(), adjacentSorter);
        
        std::vector<PolygonPoint*> adjacentPoints =
        getAdjacentPoints(fromPt);
        
        std::vector<PolygonPoint*> alreadyTransitionedPoints;
        
        int i = 0;
        
        while (i < adjacentPoints.size()) {
            PolygonPoint *nextPt = adjacentPoints[i];
            
            if (alreadyATransition(fromPt, nextPt)) {
                if (i == adjacentPoints.size()-1) {
                    if (i > 0) return getBestNextPoint(alreadyTransitionedPoints);
                    else return new PolygonPoint(-1,-1); // ERROR
                } else {
                    // don't allow a DUPLICATE transition!
                    adjacentPoints.erase(adjacentPoints.begin()+i);
                    continue; // try next point (don't increment 'i')
                }
            } else {
                // if already a transition in the opposite direction,
                //  favor any other adjacent pixel (except a duplicate
                //  transition!)
                
                if (alreadyATransition(nextPt, fromPt)) {
                    if (i == adjacentPoints.size()-1) {
                        if (i > 0) {
                            alreadyTransitionedPoints.push_back(nextPt);
                            return getBestNextPoint(alreadyTransitionedPoints);
                        } else return nextPt; // only this possible transition
                    } // see if next adjacent point is better
                    alreadyTransitionedPoints.push_back(nextPt);
                } else if (anyTransitionFrom(nextPt)) {
                    if (i == adjacentPoints.size()-1) {
                        if (i > 0) {
                            alreadyTransitionedPoints.push_back(nextPt);
                            return getBestNextPoint(alreadyTransitionedPoints);
                        } else return nextPt; // only this possible transition
                    } //  see if next adjacent point is better
                    alreadyTransitionedPoints.push_back(nextPt);
                } else {
                    return nextPt;
                }
            }
            i++;
        }
        return new PolygonPoint(-1,-1); // ERROR
    }
    
    
    
    
    // Note, if there is no "ideal" next point in the adjacent point
    //  list, choose one of the adjacent already-transitioned points.
    PolygonPoint* FinalEdge::getBestNextPoint(std::vector<PolygonPoint*> alreadyTransitionedPoints) {
        
        // favor the origin point (already transitioned from):
        if (alreadyTransitionedPoints.size() > 1) {
            // favor the origin point, if any of these adjacent points
            //  is the origin point
            for (int i = 0; i < alreadyTransitionedPoints.size(); i++) {
                PolygonPoint *pt = alreadyTransitionedPoints[i];
                if (pt->x == originPt->x && pt->y == originPt->y)
                    return originPt;
            }
        }
        // If not next to the origin point, choose the first point
        //  having already been transitioned from/to:
        if (alreadyTransitionedPoints.size() >= 1) {
            return alreadyTransitionedPoints[0];
        } else return new PolygonPoint(-1,-1);  // should never get here!
    }
    
    
    
    
    bool FinalEdge::alreadyATransition(PolygonPoint *fromPt, PolygonPoint *toPt) {
        std::map<PolygonPoint*, std::vector<PolygonPoint*> >::iterator it;
        it = pointMap.find(fromPt);
        if (it == pointMap.end()) {
            return false;
        } else {
            std::vector<PolygonPoint*> points = it->second;
            for (int i = 0; i < points.size(); i++) {
                PolygonPoint *nextPt = points[i];
                if (nextPt->x == toPt->x &&
                    nextPt->y == toPt->y)
                    return true;
            }
        }
        return false;
    }
    
    bool FinalEdge::anyTransitionFrom(PolygonPoint *fromPt) {
        std::map<PolygonPoint*, std::vector<PolygonPoint*> >::iterator it;
        it = pointMap.find(fromPt);
        if (it == pointMap.end())
            return false;
        else return true;
    }
    
    
    int FinalEdge::determineLastDirection(PolygonPoint *pt, PolygonPoint *nextPt) {
        int dx, dy;
        dx = nextPt->x - pt->x;
        dy = nextPt->y - pt->y;
        if (dx == 0 && dy == -1) return adjacentSorter.N;
        else if (dx == -1 && dy == -1) return adjacentSorter.NW;
        else if (dx == -1 && dy == 0) return adjacentSorter.W;
        else if (dx == -1 && dy == 1) return adjacentSorter.SW;
        else if (dx == 0 && dy == 1) return adjacentSorter.S;
        else if (dx == 1 && dy == 1) return adjacentSorter.SE;
        else if (dx == 1 && dy == 0) return adjacentSorter.E;
        else if (dx == 1 && dy == -1) return adjacentSorter.NE;
        return adjacentSorter.N; // should never get here
    }
    
    
    std::vector<PolygonPoint*> FinalEdge::getFinalEdges() {
        PolygonPoint *pt = originPt;
        //while (!isDone()) {
        while (!done) {
            PolygonPoint *nextPt = getNextPoint(pt);
            if (nextPt->x == -1 && nextPt->y == -1) break;
            markTransition(pt,nextPt);
            // determine the direction from pt to nextPt:
            lastPointDirection = determineLastDirection(pt,nextPt);
            pt = nextPt;
            // NEW 4/19/18:
            //if (pt->x == originPt->x && pt->y == originPt->y) break;
        }
        return edgePoints;
    }
    
    bool FinalEdge::isDone() {
        return done;
    }
    
    std::vector<PolygonPoint*> FinalEdge::getAdjacentPoints(PolygonPoint *pt) {
        std::vector<PolygonPoint*> adjacentPoints;
        int index = 1;
        while (index < sortedEdgeList.size()) {
            PolygonPoint *nextPt = sortedEdgeList[index];
            int xDiff = abs(pt->x - nextPt->x);
            int yDiff = abs(pt->y - nextPt->y);
            if (xDiff <= 1 && yDiff <= 1)
                adjacentPoints.push_back(nextPt);
            else
                return adjacentPoints;
            index++;
        }
        return adjacentPoints;
    }
    
}// end namespace GeoStar
