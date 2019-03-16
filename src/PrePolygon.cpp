// PrePolygon.cpp
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
#include "LatLong.hpp"
#include "PolygonSorters.hpp"
#include "PrePolygon.hpp"
#include "FinalEdge.hpp"
#include "EdgeStrokes.hpp"

namespace GeoStar {
    
    template <class T>
    PrePolygon<T>::PrePolygon() {
        previousRowIndex = 0;
        hasHoles = false;
        done = false;
        isContained = false;
        this->polygonNumber = -1;
    }
    
    template <class T>
    PrePolygon<T>::PrePolygon(int &polygonNumber, T &pixelValue, int x, int y) {
        previousRowIndex = 0;
        hasHoles = false;
        done = false;
        isContained = false;
        this->polygonNumber = polygonNumber;
        this->pixelValue = pixelValue;
        PolygonPoint *pt = new PolygonPoint(x,y);
        std::vector<PolygonPoint*> points;
        points.push_back(pt);
        pixelAddrs[y] = points;
        
        // ALSO add to edges:
        edges_xsorted.push_back(pt);
        edges_ysorted.push_back(pt);
    }
    
    template <class T>
    PrePolygon<T>::PrePolygon(int &polygonNumber, T &pixelValue, std::vector<PolygonPoint*> &points) {
        previousRowIndex = 0;
        hasHoles = false;
        done = false;
        isContained = false;
        this->polygonNumber = polygonNumber;
        this->pixelValue = pixelValue;
        if (points.size() > 0) {
            int y = points[0]->y;
            pixelAddrs[y] = points;
            
            // ALSO add to edges:
            edges_xsorted = points;
            edges_ysorted = points;
        }
    }
    
    
    template <class T>
    int PrePolygon<T>::getPolygonNumber() {
        return polygonNumber;
    }
    
    template <class T>
    T PrePolygon<T>::getPixelValue() {
        return pixelValue;
    }
    
    template <class T>
    std::vector<PolygonPoint*> PrePolygon<T>::getEdgePoints() {
        return edges_xsorted;
    }
    
    
    
    // This function is called when another polygon is coalesced into
    //  this polygon.
    template <class T>
    void PrePolygon<T>::addEdgePoints(std::vector<PolygonPoint*> newEdgePoints) {
        std::cout << "adding these EDGE points to polygon#"
        << polygonNumber << "\n";
        for (int z = 0; z < newEdgePoints.size(); z++) {
            PolygonPoint* pt = newEdgePoints[z];
            std::cout << "(" << pt->x << "," << pt->y << ") ";
        }
        std::cout << "\n";
        
        edges_xsorted.insert(edges_xsorted.end(), newEdgePoints.begin(), newEdgePoints.end());
        edges_ysorted.insert(edges_ysorted.end(), newEdgePoints.begin(), newEdgePoints.end());
        previousRowIndex = 0;  // reset to re-edgify ENTIRE polygon
    }
    
    
    template <class T>
    std::vector<PolygonPoint*> PrePolygon<T>::getPoints(int y) {
        if (y < 0) return emptyVector;
        std::map<int, std::vector<PolygonPoint*> >::iterator it;
        it = pixelAddrs.find(y);
        if (it == pixelAddrs.end())
            return emptyVector;
        else
            return it->second;
    }
    
    
    
    template <class T>
    void PrePolygon<T>::addPoints(std::vector<PolygonPoint*> newPoints, int y) {
        std::cout << "adding these POINTS to polygon#"
        << polygonNumber << "\n";
        for (int z = 0; z < newPoints.size(); z++) {
            PolygonPoint* pt = newPoints[z];
            std::cout << "(" << pt->x << "," << pt->y << ") ";
        }
        std::cout << "\n";
        
        // First see if any points with this y-value:
        std::map<int, std::vector<PolygonPoint*> >::iterator it;
        it = pixelAddrs.find(y);
        // If none, add it!
        std::vector<PolygonPoint*> points;
        if (it != pixelAddrs.end()) {
            points = it->second;
        }
        for (int i = 0; i < newPoints.size(); i++) {
            PolygonPoint *newPt = newPoints[i];
            points.push_back(newPt);
        }
        
        // NOW sort the points on y:
        std::sort(points.begin(), points.end(), ySorter);
        pixelAddrs[y] = points;
        done = false; // in case earlier set to true!
        previousRowIndex = 0;  // reset to re-edgify ENTIRE polygon
        
        // DEBUG:
        std::cout << "any duplicates in this list?\n";
        for (int z = 0; z < points.size(); z++) {
            PolygonPoint* pt = points[z];
            std::cout << "(" << pt->x << "," << pt->y << ") ";
        }
        std::cout << "\n";
    }
    
    
    
    template <class T>
    void PrePolygon<T>::addPoint(int x, int y) {
        // If this polygon is already bound, can't add any more pixels to it
        //if (done) return;
        
        // First see if any points with this x-value:
        std::map<int, std::vector<PolygonPoint*> >::iterator it;
        it = pixelAddrs.find(y);
        // If none, add it!
        std::vector<PolygonPoint*> points;
        if (it != pixelAddrs.end()) {
            points = it->second;
        }
        PolygonPoint *pt = new PolygonPoint(x,y);
        points.push_back(pt);
        pixelAddrs[y] = points;
        
        // ALSO add to edges:
        edges_xsorted.push_back(pt);
        edges_ysorted.push_back(pt);
        
        return;
    }
    
    
    
    template <class T>
    void PrePolygon<T>::setDone(bool done) {
        this->done = done;
    }
    
    
    template <class T>
    bool PrePolygon<T>::isDone() {
        return done;
    }
    
    template <class T>
    void PrePolygon<T>::setAlreadyContained() {
        isContained = true;
    }
    
    template <class T>
    bool PrePolygon<T>::isAlreadyContained() {
        return isContained;
    }
    
    
    template <class T>
    void PrePolygon<T>::edgify(int y) {
        
        // If this polygon is already bound, no need to do any more edge creation
        //  (except when all done processing the whole image, may need to test for coalescing
        //   any "done" polygons)
        //if (done) return;
        
        //if (polygonNumber == 0)
        //    std::cout << "\nEdgifying row " << y << "\n";
        
        std::vector<PolygonPoint*> points = pixelAddrs[y];
        
        // If no pixels found in this column, the polygon has been completely bounded, and is
        //  now a complete polygon:
        //if (points.size() == 0) {
        //    done = true;
        //    // THIS POLYGON has all it's boundaries done, spin it off to a completed polygon!!!!
        //    spinOffPolygon();
        //
        //    return;
        //}
        
        // Remove any interior points of this polygon:
        //
        std::sort (edges_xsorted.begin(), edges_xsorted.end(), xSorter);
        std::sort (edges_ysorted.begin(), edges_ysorted.end(), ySorter);
        
        
        // Look for gaps in the current column of pixels, and mark any
        // pixels to the left, or above, or below these gaps, as
        // "abutting a hole" (potentially a hole):
        if (points.size() > 1) {
            PolygonPoint *pt = points[0];
            for (int n = 1; n < points.size(); n++) {
                PolygonPoint *nextPt = points[n];
                
                if (nextPt->x > (pt->x+1)) {
                    // a gap; mark both the previous point, and this
                    //  next point as abutting a hole:
                    pt->markAbuttingHole();
                    nextPt->markAbuttingHole();
                    hasHoles = true;
                    
                    // Now, find all the points in the previous row,
                    //  which are above this gap, and mark as
                    //  abutting a hole:
                    int ypt = pt->y - 1;
                    int xpt = pt->x + 1;
                    if (ypt >= 0) {
                        while (xpt < nextPt->x) {
                            int index = findXsortedPoint(xpt,ypt);
                            if (index != -1)
                                edges_xsorted[index]->markAbuttingHole();
                            xpt++;
                        }
                    }
                }
                pt = nextPt;
            }
        }
        
        
        // DEBUG:
        /*
         if (polygonNumber == 0) {
         std::cout << "  ... edges_ysorted WAS: ";
         for (int z = 0; z < edges_ysorted.size(); z++) {
         PolygonPoint* pt = edges_ysorted[z];
         if (pt->isAbuttingHole()) std::cout << "HOLE";
         std::cout << "(" << pt->x << "," << pt->y << ") ";
         }
         }
         */
        /*
         std::cout << "\n";
         std::cout << "  ... edges_ysorted WAS: ";
         for (int z = 0; z < edges_ysorted.size(); z++) {
         PolygonPoint* pt = edges_ysorted[z];
         std::cout << "(" << pt->x << "," << pt->y << ") ";
         }
         std::cout << "\n";
         }
         */
        
        // Prune the edge points:
        int i = 0;
        //int i = previousRowIndex;
        while (i < edges_ysorted.size()) {
            PolygonPoint *edgePt = edges_ysorted[i];
            
            // Don't bother with the most recently added points!
            //  They ALL need to be kept, until a lower row's points
            //  are added
            if (edgePt->y == y) break;
            
            
            if (!edgePt->isAbuttingHole() && !edgePt->isEdgePoint()) {
                checkPointOnRightHole(edgePt);
                if (edgePt->isEdgePoint()) {
                    i++;
                    continue;
                }
                checkPointOnLeftHole(edgePt);
                if (edgePt->isEdgePoint()) {
                    i++;
                    continue;
                }
                //} else if (!edgePt->isEdgePoint()) {
                checkPointOnTopHole(edgePt);
                if (edgePt->isEdgePoint()) {
                    i++;
                    continue;
                }
                checkPointOnBottomHole(edgePt);
                if (edgePt->isEdgePoint()) {
                    i++;
                    continue;
                }
            }
            
            
            // NEW: (1st 2 lines)
            if (edgePt->isAbuttingHole() || edgePt->isEdgePoint()) {
                i++;
            } else if (!anyPointAbove(edgePt)) {
                edgePt->setRegularEdgePoint();
                i++;
            } else if (!anyPointJustRight(edgePt,i)) {
                edgePt->setRegularEdgePoint();
                i++;
            } else if (!anyPointJustLeft(edgePt,i)) {
                edgePt->setRegularEdgePoint();
                i++;
            } else if (!anyPointJustBelow(edgePt)) {
                edgePt->setRegularEdgePoint();
                i++;
            } else {
                // Don't actually erase points yet, or above algorithm
                //  won't work!
                i++;
                edgePt->markForDeletion();
            }
        }
        
        
        
        // Now clear out the points marked for deletion:
        i = 0;
        while (i < edges_ysorted.size()) {
            PolygonPoint *edgePt = edges_ysorted[i];
            if (edgePt->isMarkedForDeletion()) {
                edges_ysorted.erase(edges_ysorted.begin()+i);
                int i2 = findXsortedPoint(edgePt);
                edges_xsorted.erase(edges_xsorted.begin()+i2);
            } else {
                i++;
            }
        }
        
        // DEBUG:
        /*
         if (polygonNumber == 0) {
         std::cout << "\n  ... edges_ysorted NOW: ";
         for (int z = 0; z < edges_ysorted.size(); z++) {
         PolygonPoint* pt = edges_ysorted[z];
         if (pt->isAbuttingHole()) std::cout << "HOLE";
         std::cout << "(" << pt->x << "," << pt->y << ") ";
         }
         std::cout << "\n";
         }
         */
        /*
         std::cout << "  ... edges_ysorted NOW: ";
         for (int z = 0; z < edges_ysorted.size(); z++) {
         PolygonPoint* pt = edges_ysorted[z];
         std::cout << "(" << pt->x << "," << pt->y << ") ";
         }
         std::cout << "\n";
         }
         */
        
        
        // NOW clear out the previous row of saved points:
        if (y != 0)
            pixelAddrs[y-1].clear();
    }
    
    
    
    
    
    template <class T>
    void PrePolygon<T>::checkPointOnRightHole(PolygonPoint *pt) {
        // on a hole, if
        //  1. there is a pixel above MORE THAN 1 up, but there is
        //     NO pixels on the RIGHT of ANY y value BETWEEN these 2 pixels
        //  2. there is a pixel below MORE THAN 1 down, but there is
        //     NO pixels on the RIGHT of ANY y value BETWEEN these 2 pixels!
        //
        // No point found JUST below.  If there are any points farther
        //  below this point, mark it as "abutting-a-hole", for further
        //  edge-trimming info
        if (pt->isAbuttingHole()) return;
        int x = pt->x;
        int y = pt->y;
        int index, saveIndex;
        // 1.
        index = findXsortedPoint(pt);
        saveIndex = index;
        index--;
        bool holeFound = true;
        if (index >= 0) {
            PolygonPoint *abovePt = edges_xsorted[index];
            if (abovePt->x == x && abovePt->y < (y-1)) {
                int checkX = x;
                int checkY = y-1;
                index++; // skip over original point
                index++;
                // now, check if ANY points with x>checkX and y==checkY
                while (index < edges_xsorted.size()) {
                    PolygonPoint *testPt = edges_xsorted[index];
                    if (testPt->x > x && testPt->y == checkY) {
                        holeFound = false;  // NOT abutting a hole
                        break;  // need to do test 2.!
                    }
                    index++;
                }
                // No pixels to right of the Gap!
                if (holeFound) {
                    //pt->markAbuttingHole();
                    pt->setRegularEdgePoint();
                    return;
                }
                //return;
            }
        }
        // 2.
        index = saveIndex;
        index++;
        if (index < edges_xsorted.size()) {
            PolygonPoint *belowPt = edges_xsorted[index];
            if (belowPt->x == x && belowPt->y > (y+1)) {
                int checkX = x;
                int checkY = y+1;
                index++;
                // now, check if ANY points with x>checkX and y==checkY
                while (index < edges_xsorted.size()) {
                    PolygonPoint *testPt = edges_xsorted[index];
                    if (testPt->x > x && testPt->y == checkY)
                        return;  // NOT abutting a hole
                    index++;
                }
                // No pixels to right of the Gap!
                //pt->markAbuttingHole();
                pt->setRegularEdgePoint();
                return;
            }
        }
    }
    
    
    
    
    
    template <class T>
    void PrePolygon<T>::checkPointOnLeftHole(PolygonPoint *pt) {
        // on a hole, if
        //  1. there is a pixel above MORE THAN 1 up, but there is
        //     NO pixels on the LEFT of ANY y value BETWEEN these 2 pixels
        //  2. there is a pixel below MORE THAN 1 down, but there is
        //     NO pixels on the LEFT of ANY y value BETWEEN these 2 pixels!
        //
        // No point found JUST below.  If there are any points farther
        //  below this point, mark it as "abutting-a-hole", for further
        //  edge-trimming info
        if (pt->isAbuttingHole()) return;
        int x = pt->x;
        int y = pt->y;
        int index, saveIndex;
        // 1.
        index = findXsortedPoint(pt);
        saveIndex = index;
        index--;
        bool holeFound = true;
        if (index >= 0) {
            PolygonPoint *abovePt = edges_xsorted[index];
            if (abovePt->x == x && abovePt->y < (y-1)) {
                int checkX = x;
                int checkY = y-1;
                index--;
                // now, check if ANY points with x<checkX and y==checkY
                while (index >= 0) {
                    PolygonPoint *testPt = edges_xsorted[index];
                    if (testPt->x < x && testPt->y == checkY) {
                        holeFound = false;  // NOT abutting a hole
                        break;  // need to do test 2.!
                    }
                    index--;
                }
                // No pixels to left of the Gap!
                if (holeFound) {
                    //pt->markAbuttingHole();
                    pt->setRegularEdgePoint();
                    return;
                }
                //return;
            }
        }
        // 2.
        index = saveIndex;
        index++;
        if (index < edges_xsorted.size()) {
            PolygonPoint *belowPt = edges_xsorted[index];
            if (belowPt->x == x && belowPt->y > (y+1)) {
                int checkX = x;
                int checkY = y+1;
                index--; // skip over original point
                index--;
                // now, check if ANY points with x<checkX and y==checkY
                while (index >= 0) {
                    PolygonPoint *testPt = edges_xsorted[index];
                    if (testPt->x < x && testPt->y == checkY)
                        return;  // NOT abutting a hole
                    index--;
                }
                // No pixels to right of the Gap!
                //pt->markAbuttingHole();
                pt->setRegularEdgePoint();
                return;
            }
        }
    }
    
    
    
    
    
    
    
    template <class T>
    void PrePolygon<T>::checkPointOnBottomHole(PolygonPoint *pt) {
        // on a hole, if
        //  1. there is a pixel MORE THAN 1 to the left, but there is
        //     NO pixels ABOVE any x value BETWEEN these 2 pixels
        //  2. there is a pixel MORE THAN 1 to the right, but there is
        //     NO pixels ABOVE any x value BETWEEN these 2 pixels!
        //
        // No point found JUST below.  If there are any points farther
        //  below this point, mark it as "abutting-a-hole", for further
        //  edge-trimming info
        
        int x = pt->x;
        int y = pt->y;
        
        int index, saveIndex;
        // 1.
        index = findYsortedPoint(pt);
        saveIndex = index;
        index--;
        bool holeFound = true;
        if (index >= 0) {
            PolygonPoint *leftPt = edges_ysorted[index];
            if (leftPt->y == y && leftPt->x < (x-1)) {
                int checkY = y;
                int checkX = x-1;
                index++; // skip over original point
                index++;
                // now, check if ANY points with x==checkX and y>checkY
                while (index < edges_ysorted.size()) {
                    PolygonPoint *testPt = edges_ysorted[index];
                    if (testPt->y > y && testPt->x == checkX) {
                        holeFound = false;  // NOT abutting a hole
                        break;  // need to do test 2.!
                    }
                    index++;
                }
                // No pixels to left of the Gap!
                if (holeFound) {
                    //pt->markAbuttingHole();
                    pt->setRegularEdgePoint();
                    return;
                }
                //return;
            }
        }
        // 2.
        index = saveIndex;
        index++;
        if (index < edges_ysorted.size()) {
            PolygonPoint *rightPt = edges_ysorted[index];
            if (rightPt->y == y && rightPt->x > (x+1)) {
                int checkY = y;
                int checkX = x+1;
                index++;
                // now, check if ANY points with y>checkY and x==checkX
                while (index < edges_ysorted.size()) {
                    PolygonPoint *testPt = edges_ysorted[index];
                    if (testPt->y > y && testPt->x == checkX)
                        return;  // NOT abutting a hole
                    index++;
                }
                // No pixels to right of the Gap!
                //pt->markAbuttingHole();
                pt->setRegularEdgePoint();
                return;
            }
        }
    }
    
    
    
    
    
    template <class T>
    void PrePolygon<T>::checkPointOnTopHole(PolygonPoint *pt) {
        // on a hole, if
        //  1. there is a pixel MORE THAN 1 to the left, but there is
        //     NO pixels BELOW any x value BETWEEN these 2 pixels
        //  2. there is a pixel MORE THAN 1 to the right, but there is
        //     NO pixels BELOW any x value BETWEEN these 2 pixels!
        //
        // No point found JUST below.  If there are any points farther
        //  below this point, mark it as "abutting-a-hole", for further
        //  edge-trimming info
        //if (pt->isAbuttingHole()) return;
        int x = pt->x;
        int y = pt->y;
        int index, saveIndex;
        // 1.
        index = findYsortedPoint(pt);
        saveIndex = index;
        index--;
        bool holeFound = true;
        if (index >= 0) {
            PolygonPoint *leftPt = edges_ysorted[index];
            if (leftPt->x == x && leftPt->y < (y-1)) {
                int checkY = y;
                int checkX = x-1;
                index--;
                // now, check if ANY points with y<checkY and x==checkX
                while (index >= 0) {
                    PolygonPoint *testPt = edges_ysorted[index];
                    if (testPt->y < y && testPt->x == checkX) {
                        holeFound = false;  // NOT abutting a hole
                        break;  // need to do test 2.!
                    }
                    index--;
                }
                // No pixels to left of the Gap!
                if (holeFound) {
                    //pt->markAbuttingHole();
                    pt->setRegularEdgePoint();
                    return;
                }
                //return;
            }
        }
        // 2.
        index = saveIndex;
        index++;
        if (index < edges_ysorted.size()) {
            PolygonPoint *rightPt = edges_ysorted[index];
            if (rightPt->x == x && rightPt->y > (y+1)) {
                int checkY = y;
                int checkX = x+1;
                index--; // skip over original point
                index--;
                // now, check if ANY points with y<checkY and x==checkX
                while (index >= 0) {
                    PolygonPoint *testPt = edges_ysorted[index];
                    if (testPt->y < y && testPt->x == checkX)
                        return;  // NOT abutting a hole
                    index--;
                }
                // No pixels to right of the Gap!
                //pt->markAbuttingHole();
                pt->setRegularEdgePoint();
                return;
            }
        }
    }
    
    
    
    
    // use the y-sorted edge list:
    template <class T>
    bool PrePolygon<T>::anyPointJustLeft(PolygonPoint *pt, int index) {
        int x = pt->x;
        int y = pt->y;
        index--;
        if (index >= 0) {
            PolygonPoint *leftPt = edges_ysorted[index];
            if (leftPt->x == (x-1) && leftPt->y == y) return true;
            //else return false;
        }
        return false;
    }
    
    
    
    
    // use the y-sorted edge list:
    template <class T>
    bool PrePolygon<T>::anyPointJustRight(PolygonPoint *pt, int index) {
        int x = pt->x;
        int y = pt->y;
        index++;
        if (index < edges_ysorted.size()) {
            PolygonPoint *rightPt = edges_ysorted[index];
            if (rightPt->x == (x+1) && rightPt->y == y) return true;
            //else return false;
        }
        return false;
    }
    
    
    
    template <class T>
    bool PrePolygon<T>::anyPointJustAbove(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int index = findXsortedPoint(pt);
        index--;
        if (index >= 0) {
            PolygonPoint *abovePt = edges_xsorted[index];
            if (abovePt->y == (y-1) && abovePt->x == x) return true;
            //else return false;
        }
        return false;
    }
    
    
    
    template <class T>
    bool PrePolygon<T>::anyPointJustBelow(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int index = findXsortedPoint(pt);
        index++;
        if (index < edges_xsorted.size()) {
            PolygonPoint *belowPt = edges_xsorted[index];
            if (belowPt->y == (y+1) && belowPt->x == x) return true;
            //else return false;
        }
        return false;
    }
    
    
    // use the y-sorted edge list:
    template <class T>
    bool PrePolygon<T>::anyPointAbove(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int index = findXsortedPoint(pt);
        index--;
        while (index >= 0) {
            PolygonPoint *abovePt = edges_xsorted[index];
            // ignore points marked for deletion:
            if (abovePt->x == -1 || abovePt->y == -1)  {
                index--;
                continue;
            }
            if (abovePt->y < y && abovePt->x == x) return true;
            else if (abovePt->x < x) return false;
            index--;
        }
        return false;
    }
    
    
    
    template <class T>
    int PrePolygon<T>::findXsortedPoint(PolygonPoint *pt) {
        for (int i = 0; i < edges_xsorted.size(); i++) {
            PolygonPoint *edgePt = edges_xsorted[i];
            if (edgePt->x == pt->x && edgePt->y == pt->y)
                return i;
        }
        return 0;  // SHOULD NEVER GET HERE!
    }
    
    
    template <class T>
    int PrePolygon<T>::findXsortedPoint(int x, int y) {
        for (int i = 0; i < edges_xsorted.size(); i++) {
            PolygonPoint *edgePt = edges_xsorted[i];
            if (edgePt->x == x && edgePt->y == y)
                return i;
        }
        return -1;  // the given point was not found
    }
    
    
    
    template <class T>
    int PrePolygon<T>::findYsortedPoint(PolygonPoint *pt) {
        for (int i = 0; i < edges_ysorted.size(); i++) {
            PolygonPoint *edgePt = edges_ysorted[i];
            if (edgePt->x == pt->x && edgePt->y == pt->y)
                return i;
        }
        return 0;  // SHOULD NEVER GET HERE!
    }
    
    
    
    template <class T>
    int PrePolygon<T>::findYsortedPoint(int x, int y) {
        for (int i = 0; i < edges_ysorted.size(); i++) {
            PolygonPoint *edgePt = edges_ysorted[i];
            if (edgePt->x == x && edgePt->y == y)
                return i;
        }
        return 0;  // SHOULD NEVER GET HERE!
    }
    
    
    
    template <class T>
    void PrePolygon<T>::findFinalBoundingPolygon() {
        // DEBUG:
        
        if (polygonNumber == 0) {
            // sort edge points on x and y  (MAY NOT NEED THIS?)
            std::sort(edges_xsorted.begin(), edges_xsorted.end(), xSorter);
            std::sort(edges_ysorted.begin(), edges_ysorted.end(), ySorter);
            std::cout << "\n  ... edges_ysorted at END, before removing 'hole' points: ";
            for (int z = 0; z < edges_ysorted.size(); z++) {
                PolygonPoint* pt = edges_ysorted[z];
                if (pt->isAbuttingHole())
                    std::cout << "HOLE";
                std::cout << "(" << pt->x << "," << pt->y << ") ";
            }
            std::cout << "\n";
        }
        
        // END DEBUG
        
        // For any "hole" points, either remove them if they're contained
        //  in this polygon, or set them as "regular edge" points if they
        //  are NOT contained:
        if (hasHoles) {
            hasHoles = false;
            // sort edge points on x and y  (MAY NOT NEED THIS?)
            std::sort(edges_xsorted.begin(), edges_xsorted.end(), xSorter);
            std::sort(edges_ysorted.begin(), edges_ysorted.end(), ySorter);
            int i = 0;
            while (i < edges_xsorted.size()) {
                PolygonPoint *testPt = edges_xsorted[i];
                
                // TEST EVERY POINT
                if (testPt->isAbuttingHole()) {
                    std::cout << "\n testing hole point (" << testPt->x << "," << testPt->y
                    << ") to see if REALLY an edge point ... ";
                    
                    
                    // NEW: check if 'hole' point is really on a gap
                    //  if so, the check functions will set the 'hole'
                    //  point to a regular 'edge' point:
                    checkPointOnTopHole(testPt);
                    if (testPt->isEdgePoint()) {
                        i++;
                        continue;
                    }
                    checkPointOnBottomHole(testPt);
                    if (testPt->isEdgePoint()) {
                        i++;
                        continue;
                    }
                    
                    
                    
                    
                    if (minMaxRowOrCol(testPt)) {
                        std::cout << "  it IS \n";
                        testPt->setRegularEdgePoint();
                        i++;
                    } else {
                        std::cout << "  it's being DELETED! \n";
                        // remove this point from list!
                        edges_xsorted.erase(edges_xsorted.begin()+i);
                        int i2 = findYsortedPoint(testPt);
                        edges_ysorted.erase(edges_ysorted.begin()+i2);
                        hasHoles = true; // at least ONE real hole point!
                    }
                } else {
                    i++;
                }
            }
        }
        
        // DEBUG:
        
        if (polygonNumber == 0) {
            // sort edge points on x and y  (MAY NOT NEED THIS?)
            std::sort(edges_xsorted.begin(), edges_xsorted.end(), xSorter);
            std::sort(edges_ysorted.begin(), edges_ysorted.end(), ySorter);
            std::cout << "\n  --> edges_ysorted at END, AFTER removing 'hole' points: ";
            for (int z = 0; z < edges_ysorted.size(); z++) {
                PolygonPoint* pt = edges_ysorted[z];
                if (pt->isAbuttingHole())
                    std::cout << "HOLE";
                std::cout << "(" << pt->x << "," << pt->y << ") ";
            }
            std::cout << "\n";
            if (hasHoles) std::cout << " HAS HOLES IS TRUE!\n";
            else std::cout << " HAS HOLES IS FALSE!\n";
        }
        
        
        
        // DEBUG:
        /*
         if (polygonNumber == 0) {
         std::cout << "\n  ... after dealing w/ hole pts, edges_ysorted: ";
         for (int i = 0; i < edges_ysorted.size(); i++) {
         PolygonPoint *crp = edges_ysorted[i];
         std::cout << "(" << crp->x << "," << crp->y << ") ";
         }
         std::cout << "\n ";
         }
         */
        /*
         std::cout << "\n\n ****   edges_xsorted:  ****\n";
         for (int i = 0; i < edges_xsorted.size(); i++) {
         PolygonPoint *crp = edges_xsorted[i];
         std::cout << "(" << crp->x << "," << crp->y << ") ";
         }
         std::cout << " END edges_xsorted:\n\n\n ";
         */
        
        // Get the counter-clockwise list of edge points, in the order
        //  they will be stroked.
        std::vector<PolygonPoint*> edges_list;
        FinalEdge finalEdge(edges_xsorted);
        edges_list = finalEdge.getFinalEdges();
        
        // DEBUG:
        std::cout << "\n\n ****  FINAL edges_list:  ****\n";
        for (int i = 0; i < edges_list.size(); i++) {
            PolygonPoint *crp = edges_list[i];
            std::cout << "(" << crp->x << "," << crp->y << ") ";
        }
        std::cout << " END FINAL edges_list:\n\n\n ";
        
        double upperLeftLat = 1.0;
        double upperLeftLong = 1.0;
        double distancePerPixel = 1.0;
        PolygonPoint *originPt = edges_xsorted[0];
        
        // Stroke the outer edge, creating a new counter-clockwise list
        //  of edge points:
        EdgeStrokes edgeStrokes(originPt, upperLeftLat, upperLeftLong,
                                distancePerPixel);
        edgeStrokes.createStrokes(edges_list);
        std::vector<LatLong*> strokes = edgeStrokes.getStrokes();
        
        // DEBUG:
        std::cout << "\n\n ****  STROKES:  ****\n";
        for (int i = 0; i < strokes.size(); i++) {
            LatLong *crp = strokes[i];
            crp->printInfo();
        }
        std::cout << " END STROKES:\n\n\n ";
    }
    
    
    
    template <class T>
    bool PrePolygon<T>::minMaxRowOrCol(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        
        // first check if this point is at the min or max X-value
        //  for row = y:
        bool atY = false;
        bool pastY = false;
        int maxXatY = 0;
        int index = 0;
        
        PolygonPoint *maxPt;
        while (index < edges_ysorted.size()) {
            PolygonPoint *testPt = edges_ysorted[index];
            if (testPt->y == y) {
                if (!atY) {
                    if (x == testPt->x) return true; // at min X
                    atY = true;
                    maxXatY = testPt->x;    // first (lowest) x-value for column 'y'
                } else {
                    //if (!testPt->isAbuttingHole())
                    maxXatY = testPt->x;
                    maxPt = testPt;
                }
            } else if (testPt->y > y) {
                pastY = true;
                //if (x == maxXatY) return true; // at max X
                break; // test min/max y next
            }
            index++;
        }
        if (atY || pastY) {
            if (maxPt->isAbuttingHole()) maxPt->setRegularEdgePoint();
            if (x >= maxXatY) return true; // at max X
        }
        
        // Point is NOT at min/max X for its Y-value. Check if it is
        //  a min/max Y for its X-value:
        bool atX = false;
        bool pastX = false;
        int maxYatX = 0;
        index = 0;
        while (index < edges_xsorted.size()) {
            PolygonPoint *testPt = edges_xsorted[index];
            if (testPt->x == x) {
                if (!atX) {
                    if (y == testPt->y) return true;  // at min y
                    atX = true;
                    maxYatX = testPt->y;
                } else {
                    //if (!testPt->isAbuttingHole())
                    maxYatX = testPt->y;
                    maxPt = testPt;
                }
            } else if (testPt->x > x) {
                pastX = true;
                //if (y == maxYatX) return true;  // at max Y
                //return false; // not at ANY min/max
                break;
            }
            index++;
        }
        if (atX || pastX) {
            if (maxPt->isAbuttingHole()) maxPt->setRegularEdgePoint();
            if (y >= maxYatX) return true; // at max Y
        }
        return false;  // point not at a min/max x or y value
    }
    
    
    
    
    template <class T>
    bool PrePolygon<T>::hasAnyHoles() {
        return hasHoles;
    }
    
    
    
    template <class T>
    bool PrePolygon<T>::isPolygonInside(PrePolygon *smallerPolygon) {
        // sort edge points on x and y
        std::sort(edges_xsorted.begin(), edges_xsorted.end(), xSorter);
        std::sort(edges_ysorted.begin(), edges_ysorted.end(), ySorter);
        
        // Only need to test ONE point in the smallerPolygon:
        std::vector<PolygonPoint*> smallerPolygonEdges = smallerPolygon->getEdgePoints();
        PolygonPoint *testPt = smallerPolygonEdges[0];
        
        return isPointInside(testPt);
    }
    
    
    template <class T>
    bool PrePolygon<T>::isPointInside(PolygonPoint *testPt) {
        // Check the row and column of test point: if at least
        //  one pixel is above and below, left and right, with the
        //  number of pixels in each of those 4 cases being ODD,
        //  then this smaller polygon is INSIDE THIS POLYGON
        //std::cout << "testing if point inside (" << testPt->x << ","
        //          << testPt->y << ") \n";
        int numberPointsAbove = getNumberThisPointsAbove(testPt);
        //std::cout << " ... # pts ABOVE = " << numberPointsAbove;
        if (numberPointsAbove == 0) return false;
        else if (numberPointsAbove % 2 == 0) return false;
        int numberPointsBelow = getNumberThisPointsBelow(testPt);
        //std::cout << " ... # pts BELOW = " << numberPointsBelow;
        if (numberPointsBelow == 0) return false;
        else if (numberPointsBelow % 2 == 0) return false;
        int numberPointsLeft = getNumberThisPointsLeft(testPt);
        //std::cout << " ... # pts LEFT = " << numberPointsLeft;
        if (numberPointsLeft == 0) return false;
        else if (numberPointsLeft % 2 == 0) return false;
        int numberPointsRight = getNumberThisPointsRight(testPt);
        //std::cout << " ... # pts RIGHT = " << numberPointsRight;
        if (numberPointsRight == 0) return false;
        else if (numberPointsRight % 2 == 0) return false;
        return true;
    }
    
    
    
    
    // use the y-sorted edge list:
    template <class T>
    int PrePolygon<T>::getNumberThisPointsAbove(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int numberPoints = 0;
        int index = edges_xsorted.size() - 1;
        while (index >= 0) {
            PolygonPoint *abovePt = edges_xsorted[index];
            if (abovePt->x > x) {
                index--;
                continue;
            }
            if (abovePt->y < y && abovePt->x == x)
                numberPoints++;
            else if (abovePt->x < x) return numberPoints;
            index--;
        }
        return numberPoints;
    }
    
    
    // use the y-sorted edge list:
    template <class T>
    int PrePolygon<T>::getNumberThisPointsLeft(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int numberPoints = 0;
        int index = edges_ysorted.size() - 1;
        while (index >= 0) {
            PolygonPoint *leftPt = edges_ysorted[index];
            if (leftPt->y > y) {
                index--;
                continue;
            }
            if (leftPt->x < x && leftPt->y == y)
                numberPoints++;
            else if (leftPt->y < y) return numberPoints;
            index--;
        }
        return numberPoints;
    }
    
    
    // use the y-sorted edge list:
    template <class T>
    int PrePolygon<T>::getNumberThisPointsRight(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int index = 0;
        int numberPoints = 0;
        while (index < edges_ysorted.size()) {
            PolygonPoint *rightPt = edges_ysorted[index];
            if (rightPt->y < y) {
                index++;
                continue;
            }
            if (rightPt->x > x && rightPt->y == y)
                numberPoints++;
            else if (rightPt->y > y) return numberPoints;
            index++;
        }
        return numberPoints;
    }
    
    
    
    // use the y-sorted edge list:
    template <class T>
    int PrePolygon<T>::getNumberThisPointsBelow(PolygonPoint *pt) {
        int x = pt->x;
        int y = pt->y;
        int index = 0;
        int numberPoints = 0;
        while (index < edges_xsorted.size()) {
            PolygonPoint *belowPt = edges_xsorted[index];
            if (belowPt->x < x) {
                index++;
                continue;
            }
            if (belowPt->y > y && belowPt->x == x &&
                !belowPt->isAbuttingHole())
                numberPoints++;
            else if (belowPt->x > x) return numberPoints;
            index++;
        }
        return numberPoints;
    }
    
    
    
    template <class T>
    void PrePolygon<T>::addContainedPolygon(PrePolygon *insidePolygon) {
        std::cout << " ... good, added polygon#"
        << insidePolygon->getPolygonNumber() << " to this polygon#"
        << polygonNumber << "\n";
        containedPolygons.push_back(insidePolygon);
    }
    
    
    template <class T>
    void PrePolygon<T>::spinOffPolygon() {
        // FOR NOW, just write out edges:
        //printf( "  Polygon %d:  IS DONE\n", polygonNumber);
        //for (int i = 0; i < edges_xsorted.size(); i++) {
        //    PolygonPoint *pt = edges_xsorted[i];
        //    printf("    (%d,%d)\n", pt->x, pt->y );
        //}
        // empty out the pixelAddrs, to save RAM
        std::map<int, std::vector<PolygonPoint*> >::iterator it;
        it = pixelAddrs.begin();
        while (it != pixelAddrs.end()) {
            std::vector<PolygonPoint*> points = it->second;
            points.clear();
            it++;
        }
    }
    
    
    
    template <class T>
    void PrePolygon<T>::printMoreCrap() {
        std::cout << " Polygon #" << polygonNumber << " contains ";
        for (int i = 0; i < containedPolygons.size(); i++) {
            PrePolygon *polygon = containedPolygons[i];
            std::cout << "polygon#" << polygon->getPolygonNumber() << " ";
        }
        std::cout << "\n";
    }
   
    // explicit instantiations
    template class PrePolygon<uint8_t>;
    template class PrePolygon<float>;
    template class PrePolygon<double>;
    
}// end namespace GeoStar
