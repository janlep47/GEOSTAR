// PolygonSorters.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef POLYGONSORTERS_HPP_
#define POLYGONSORTERS_HPP_


#include <algorithm>    // std::sort
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <math.h>



namespace GeoStar {
    
    /*
    struct Size_Sorter {
        bool operator() (PrePolygon *p1, PrePolygon *p2) {
            return (p1->getEdgePoints().size() < p2->getEdgePoints().size());
        }
    }; // sizeSorter;
     */
    
    struct EdgeXSorter {
        bool operator() (PolygonPoint *pt1, PolygonPoint *pt2) {
            if (pt1->x == pt2->x) return (pt1->y < pt2->y);
            else return (pt1->x < pt2->x);
        }
    }; //xSorter;

    struct EdgeYSorter {
        bool operator() (PolygonPoint *pt1, PolygonPoint *pt2) {
            if (pt1->y == pt2->y) return (pt1->x < pt2->x);
            else return (pt1->y < pt2->y);
        }
    }; //ySorter;




    struct AdjacentSorter {
        int x;
        int y;
    
        int lastDirection;
        int favoredDirection;
    
        static const int N = 1;
        static const int NW = 2;
        static const int W = 3;
        static const int SW = 4;
        static const int S = 5;
        static const int SE = 6;
        static const int E = 7;
        static const int NE = 8;
    
        bool inFavoredDirection(int xdiff, int ydiff) {
            if (favoredDirection > 8) favoredDirection -= 8;
            switch (favoredDirection) {
                case N:
                    if (xdiff == 0 && ydiff == -1) return true;
                    return false;
                case NW:
                    if (xdiff == -1 && ydiff == -1) return true;
                    return false;
                case W:
                    if (xdiff == -1 && ydiff == 0) return true;
                    return false;
                case SW:
                    if (xdiff == -1 && ydiff == 1) return true;
                    return false;
                case S:
                    if (xdiff == 0 && ydiff == 1) return true;
                    return false;
                case SE:
                    if (xdiff == 1 && ydiff == 1) return true;
                    return false;
                case E:
                    if (xdiff == 1 && ydiff == 0) return true;
                    return false;
                case NE:
                    if (xdiff == 1 && ydiff == -1) return true;
                    return false;
            }
            return false;
        }
    
    
        bool operator() (PolygonPoint *pt1, PolygonPoint *pt2) {
            // true - > return pt1 comes first
            // false -> return pt2 comes first
            int xDiff1 = abs(pt1->x - x);
            int xDiff2 = abs(pt2->x - x);
            int yDiff1 = abs(pt1->y - y);
            int yDiff2 = abs(pt2->y - y);
            int diff1 = xDiff1 + yDiff1;
            int diff2 = xDiff2 + yDiff2;
            // if both points are adjacent or kitty corner (touching
            //  diagonally), favor W,SW,S,SE,E,NE,N,NW in that order:
            if ((diff1 == 1 || diff1 == 2) &&
                (diff2 == 1 || diff2 == 2)) {
                xDiff1 = pt1->x - x;    // pt compared to (x,y)
                xDiff2 = pt2->x - x;
                yDiff1 = pt1->y - y;
                yDiff2 = pt2->y - y;
            
                // get favored sort order
                //
                favoredDirection = lastDirection - 2;
                //std::cout << "\nfavoredDirection = " << favoredDirection;
                if (favoredDirection < 0) favoredDirection += 8;
            
                int index = 1;
                while (index <= 8) {
                    if (inFavoredDirection(xDiff1, yDiff1))
                        return true;
                    else if (inFavoredDirection(xDiff2, yDiff2))
                        return false;
                    else {
                        favoredDirection++;
                        index++;
                    }
                }
                return false;
            } else if (diff1 < diff2) {
                return true;
            } else return false;
        }
    }; //adjacentSorter;
    
}// end namespace GeoStar


#endif //POLYGONSORTERS_HPP_
