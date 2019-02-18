// EdgeStrokes.cpp
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
#include "EdgeStrokes.hpp"


namespace GeoStar {
    
    EdgeStrokes::EdgeStrokes() {
        lastStrokePosn = NW;
        upperLeftLat = 0.0;
        upperLeftLong = 0.0;
        currentLatitude = upperLeftLat;
        currentLongitude = upperLeftLong;
        distancePerPixel = 1.0;
    }
    
    EdgeStrokes::EdgeStrokes(PolygonPoint *originPt, double upperLeftLat, double upperLeftLong, double distancePerPixel) {
        this->originPt = originPt;
        
        lastStrokePosn = NW;  // originPoint, always at northwest corner
        // At the upper-left corner of the origin pixel:
        this->upperLeftLat = upperLeftLat;
        this->upperLeftLong = upperLeftLong;
        currentLatitude = upperLeftLat;
        currentLongitude = upperLeftLong;
        this->distancePerPixel = distancePerPixel;
    }
    
    
    
    void EdgeStrokes::createStrokes(std::vector<PolygonPoint*> edgePoints) {
        PolygonPoint *pt = originPt;
        PolygonPoint *prevPt = new PolygonPoint(-1,-1);  //initialize "null"
        int index = 1;
        // Create the first stroke:
        strokes.push_back(new LatLong(currentLongitude, currentLatitude));
        PolygonPoint *nextPt;
        //int pointPosn;
        while (index < edgePoints.size()) {
            nextPt = edgePoints[index];
            //pointPosn = relativePosn(pt,nextPt);
            //getStrokes(pt, nextPt, pointPosn);
            getStrokes(prevPt, pt, nextPt);
            prevPt = pt;
            pt = nextPt;
            index++;
        }
        
        
        if (edgePoints.size() > 1) {
            // Make a stroke back to the original starting point
            //  (NE corner of the originPt pixel)
            // lastStrokePosn now refers to the originPt pixel:
            if (lastStrokePosn == NE) {
                goLeftOne();
            } else if (lastStrokePosn == SE) {
                goUpOne();
                goLeftOne();
            } else if (lastStrokePosn == SW) {
                goRightOne();
                goUpOne();
                goLeftOne();
            } else if (lastStrokePosn == NW) {
                // OK, this is where we want to be
            }
        }
        return;
    }
    
    
    
    
    int EdgeStrokes::relativePosn(PolygonPoint *first, PolygonPoint *second) {
        int xDiff = second->x - first->x;
        int yDiff = second->y - first->y;
        if (xDiff == 0 && yDiff == 1) return S;
        else if (xDiff == 1 && yDiff == 1) return SE;
        else if (xDiff == 1 && yDiff == 0) return E;
        else if (xDiff == 1 && yDiff == -1) return NE;
        else if (xDiff == 0 && yDiff == -1) return N;
        else if (xDiff == -1 && yDiff == -1) return NW;
        else if (xDiff == -1 && yDiff == 0) return W;
        else if (xDiff == -1 && yDiff == 1) return SW;
        else return -1; // ERROR
    }
    
    
    
    void EdgeStrokes::getStrokes( PolygonPoint *prevPt, PolygonPoint *pt,
                                 PolygonPoint *nextPt) {
        int newPointPosn = relativePosn(pt,nextPt);
        
        //printStrokeInfo(pt,nextPt,newPointPosn);
        
        // If the last stroke is ALREADY touching the NEXT pixel,
        //  we need to do a stroke around the current pixel, until
        //  we are touching the new one, again.  This should only be when
        //  we're backtracking to the previous pixel from 'pt'
        //
        //  If we do a SINGLE (counter-clockwise) stroke, it should not
        //   be touching the next pixel anymore, and then we can stroke
        //   until we hit it, again
        if (lastStrokeAtNewPt(newPointPosn)) {
            if (prevPt->x == nextPt->x && prevPt->y == nextPt->y)
                doOneStroke();  // just need to know lastStrokePosn, obj data
            else {
                reAssignLastStrokePosn(pt,nextPt,newPointPosn);
                return;  // just advance to the NEXT next point
            }
            //printStrokeInfo(pt,nextPt,newPointPosn);
        }
        // Stroke counter-clockwise, from the lastStrokePosn to the NEAREST
        // corner where pt and nextPt meet:
        do {
            switch (lastStrokePosn) {
                case NW:
                    goDownOne();
                    lastStrokePosn = SW;
                    break;
                case SW:
                    goRightOne();
                    lastStrokePosn = SE;
                    break;
                case SE:
                    goUpOne();
                    lastStrokePosn = NE;
                    break;
                case NE:
                    goLeftOne();
                    lastStrokePosn = NW;
                    break;
                default:
                    // SHOULDN'T GET HERE ... throw an exception LATER...
                    break;
            } // end switch last stroke position
            //printStrokeInfo(pt,nextPt,newPointPosn);
        } while (!lastStrokeAtNewPt(newPointPosn));
        
        // assign the lastStrokePosn with respect to the NEW point
        reAssignLastStrokePosn(pt,nextPt,newPointPosn);
        //std::cout << "NEW REASSIGNED lastStrokePosn = "
        //          << getPrintablePosn(lastStrokePosn)
        //          << "\n";
    }
    
    
    void EdgeStrokes::printStrokeInfo(PolygonPoint *pt,PolygonPoint *nextPt,
                                      int newPointPosn) {
        std::cout << " (" << pt->x << "," << pt->y << ") -> ";
        std::cout << "(" << nextPt->x << "," << nextPt->y << ")  ";
        std::cout << " lastStrokePosn " << getPrintablePosn(lastStrokePosn)
        << " and pointPosn " << getPrintablePosn(newPointPosn)
        << "\n";
    }
    
    
    std::string EdgeStrokes::getPrintablePosn(int posn) {
        switch (posn) {
            case NW:
                return "NW";
            case W:
                return "W";
            case SW:
                return "SW";
            case S:
                return "S";
            case SE:
                return "SE";
            case E:
                return "E";
            case NE:
                return "NE";
            case N:
                return "N";
        }
        return "XX";
    }
    
    
    bool EdgeStrokes::lastStrokeAtNewPt(int newPointPosn) {
        switch (lastStrokePosn) {
            case NW:
                if (newPointPosn == NW || newPointPosn == W)
                    return true;
                return false;
            case SW:
                if (newPointPosn == SW || newPointPosn == S)
                    return true;
                return false;
            case SE:
                if (newPointPosn == SE || newPointPosn == E)
                    return true;
                return false;
            case NE:
                if (newPointPosn == NE || newPointPosn == N)
                    return true;
                return false;
        }
        return false;
    }
    
    
    void EdgeStrokes::doOneStroke() {
        switch (lastStrokePosn) {
            case NW:
                goDownOne();
                lastStrokePosn = SW;
                break;
            case SW:
                goRightOne();
                lastStrokePosn = SE;
                break;
            case SE:
                goUpOne();
                lastStrokePosn = NE;
                break;
            case NE:
                goLeftOne();
                lastStrokePosn = NW;
                break;
        }
    }
    
    void EdgeStrokes::reAssignLastStrokePosn(PolygonPoint *pt,PolygonPoint *nextPt,
                                int newPointPosn) {
        switch (lastStrokePosn) {
            case NW:
                if (newPointPosn == NW)
                    lastStrokePosn = SE;
                else if (newPointPosn == W)
                    lastStrokePosn = NE;
                // else should never get here!
                break;
            case SW:
                if (newPointPosn == SW)
                    lastStrokePosn = NE;
                else if (newPointPosn == S)
                    lastStrokePosn = NW;
                // else should never get here!
                break;
            case SE:
                if (newPointPosn == SE)
                    lastStrokePosn = NW;
                else if (newPointPosn == E)
                    lastStrokePosn = SW;
                // else should never get here!
                break;
            case NE:
                if (newPointPosn == NE)
                    lastStrokePosn = SW;
                else if (newPointPosn == N)
                    lastStrokePosn = SE;
                // else should never get here!
                break;
        }
    }
    
    
    void EdgeStrokes::goUpOne() {
        currentLatitude -= distancePerPixel;
        strokes.push_back(new LatLong(currentLongitude, currentLatitude));
    }
    
    void EdgeStrokes::goDownOne() {
        currentLatitude += distancePerPixel;
        strokes.push_back(new LatLong(currentLongitude, currentLatitude));
    }
    
    void EdgeStrokes::goLeftOne() {
        currentLongitude -= distancePerPixel;
        strokes.push_back(new LatLong(currentLongitude, currentLatitude));
    }
    
    void EdgeStrokes::goRightOne() {
        currentLongitude += distancePerPixel;
        strokes.push_back(new LatLong(currentLongitude, currentLatitude));
    }
    
    
    std::vector<LatLong*> EdgeStrokes::getStrokes() {
        return strokes;
    }
    
}// end namespace GeoStar
