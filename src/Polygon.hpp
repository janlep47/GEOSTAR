// Polygon.hpp
//
// by Janice Richards, Mar 17, 2018
//
//----------------------------------------
#ifndef POLYGON_HPP_
#define POLYGON_HPP_



#include <string>
#include <vector>
#include <map>


#include "H5Cpp.h"

#include "Point.hpp"
#include "RasterType.hpp"
#include "attributes.hpp"
#include "Exceptions.hpp"


namespace GeoStar {
    class Point;
/*
    clase Hole {
    public:
        bool done;
        std::vector<Point> allPoints;
        std::vector<Point> topToBottomEdges;
        std::vector<Point> bottomToTopEdges;
        
        // NOTE topToBottom refers to upperleft to lowerRight, and
        //  bottomToTop refers to lowerRight to upperLeft
        Hole(int x, int y) {
            done = false;
            allPoints.push_back(new Point(x,y));
 
            //topToBottomEdges.push_back(new Point(x,y));
            //topToBottomEdges.push_back(new Point(x,y+1));
            //topToBottomEdges.push_back(new Point(x+1,y+1));
            //bottomToTopEdges.push_back(new Point(x+1,y+1));
            //bottomToTopEdges.push_back(new Point(x+1,y));
            //bottomToTopEdges.push_back(new Point(x,y));
 
        }
        
        Hole(std::vector<Point> points) {
            done = false;
            allPoints = points;
 
            // assume (FOR NOW) all these points are consecutive points in a SINGLE column (=x value)
            //if (points == NULL) return;
            //if (points.size() == 0) return;
            //int minY = points[0].y;
            //int maxY = minY;
            //int y;
            //int x = points[0].x;
            //for (int i = 1; i < points.size(); i++) {
            //    y = points[i].y;
            //    if (y > maxY) y = maxY;
            //    else if (y < minY) y = minY;
            //}
            //topToBottomEdges.push_back(new Point(x,minY));
            //topToBottomEdges.push_back(new Point(x,maxY+1));
            //topToBottomEdges.push_back(new Point(x+1,maxY+1));
            //bottomToTopEdges.push_back(new Point(x+1,maxY+1));
            //bottomToTopEdges.push_back(new Point(x+1,minY));
            //bottomToTopEdges.push_back(new Point(x,minY));
 
        }
        
        bool addPoint(int x, int y) {
            // If this hole is already bound, can't add any more pixels to it
            if (done) return;
            
            // See if this point is to the right or above or below any current point in this "hole"
            //  polygon:
            for (int i = 0; i < allPoints.size(); i++) {
                Point pt = allPoints[i];
                if (pt.x == (x - 1) && pt.y == y) {
                    allPoints.push_back(new Point(x,y));
                    return true;
                } else if (pt.x == x && (pt.y == (y - 1) || pt.y == (y + 1))) {
                    allPoints.push_back(new Point(x,y));
                    return true;
                }
            }
            return false;   // caller will have to start a new "Hole" in the current polygon
        }
        
 
        void edgify() {
            if (done) return;
            
            
            
            // if done ...
            done = true;
            // ....
            
        }
    }
 */
    
    
    class Polygon {
        
    private:
        bool done;
        std::vector<Point> emptyVector;
        // for the "hole" polygons only:
        std::vector<Point> allPoints;

    public:
        int polygonNumber;
        
        // will only save 2 entries in pixelAddrs, x and x - 1   (if x != 0)
        // key is x-value and value is list of pixel points
        std::map<int, std::vector<Point> > pixelAddrs;
        //std::vector<Point> points;
        // edges are the pixel boundaries; these are recalculated after each full pass
        //  of a column of pixel data.  e.g. (0,2) -> (0,5)
        std::vector<Point> topToBottomEdges;
        std::vector<Point> bottomToTopEdges;
        std::vector<Polygon> holes;
        // .... later when done processing all pixels, these edges will be used to
        // COALESCE the various polygons with the same pixelValue into (possibly) larger polygons,
        // if they share pixels!
        
        
        Polygon() {
            done = false;
            this->polygonNumber = -1;
        }
        
        Polygon(int &polygonNumber, int x, int y) {
            done = false;
            this->polygonNumber = polygonNumber;
            Point pt(x,y);
            std::vector<Point> points;
            points.push_back(pt);
            pixelAddrs[x] = points;
        }
        
        Polygon(std::vector<Point> points) {
            done = false;
            //this->points = points;
            if (points.size() > 0) {
                int x = points[0].x;
                pixelAddrs[x] = points;
            }
        }

        std::vector<Point> getPoints(int x) {
            if (x < 0) return emptyVector;
            //typename std::map<int, std::vector<Point> >::iterator it;
            //typedef std::map<int, std::vector<Point> >::iterator it;
            std::map<int, std::vector<Point> >::iterator it;
            it = pixelAddrs.find(x);
            if (it == pixelAddrs.end()) 
                return emptyVector;
            else
                return it->second;
        }
        
        void addPoint(int x, int y) {
            // If this polygon/hole is already bound, can't add any more pixels to it
            if (done) return;
            
            // First see if any points with this y-value:
            //typename std::map<int, std::vector<Point> >::iterator it;
            //typedef std::map<int, std::vector<Point> >::iterator it;
            std::map<int, std::vector<Point> >::iterator it;
            it = pixelAddrs.find(x);
            // If none, add it!
            std::vector<Point> points;
            if (it != pixelAddrs.end()) {
                points = it->second;
            }
            Point pt(x,y);
            points.push_back(pt);
            pixelAddrs[x] = points;
            
            /*
            if (x == 1) {
                std::cerr << " ... pixel addresses for x = 1 " << std::endl;
                //for (int n = 0; n < pixelAddrs.size(); n++) {
                std::vector<Point> vec = pixelAddrs[x];
                for (int m = 0; m < vec.size(); m++) {
                    Point pt = vec[m];
                    int x = pt.x;
                    int y = pt.y;
                    std::cerr << "  (" << x << "," << y << ")";
                }
                //}
                std::cerr << std::endl;
            }
             */
            
            
            /*
            if (x == 0 && y == 98) {
                std::cerr << " ... pixel addresses for x = 0 (y=98)" << std::endl;
                //for (int n = 0; n < pixelAddrs.size(); n++) {
                    std::vector<Point> vec = pixelAddrs[0];
                    for (int m = 0; m < vec.size(); m++) {
                        Point pt = vec[m];
                        int x = pt.x;
                        int y = pt.y;
                        std::cerr << "  (" << x << "," << y << ")";
                    }
                //}
                std::cerr << std::endl;
            }
             */
             
            /*
            if (x == 1 && y == 0) {
                std::cerr << " ... pixel addresses for x = 0 (when x=1 and y=0)" << std::endl;
                //for (int n = 0; n < pixelAddrs.size(); n++) {
                std::vector<Point> vec = pixelAddrs[0];
                for (int m = 0; m < vec.size(); m++) {
                    Point pt = vec[m];
                    int x = pt.x;
                    int y = pt.y;
                    std::cerr << "  (" << x << "," << y << ")";
                }
                //}
                std::cerr << std::endl;

                std::cerr << " ... pixel addresses for x = 1 (y=0)" << std::endl;
                //for (int n = 0; n < pixelAddrs.size(); n++) {
                vec = pixelAddrs[1];
                for (int m = 0; m < vec.size(); m++) {
                    Point pt = vec[m];
                    int x = pt.x;
                    int y = pt.y;
                    std::cerr << "  (" << x << "," << y << ")";
                }
                //}
                std::cerr << std::endl;
            }
             */
        }
        
        
        
        
        void edgify(int x) {
            // If this polygon/hole is already bound, no need to do any more edge creation
            //  (except when all done processing the whole image, may need to test for coalescing
            //   any "done" polygons)
            if (done) return;
            
            std::cerr << "Edgifying polygon# " << polygonNumber << "  for x=" << x << std::endl;
            
            /*
            typename std::map<int, std::vector<Point> >::iterator it;
            it = pixelAddrs.find(x);
            
            if (x == 1 && it==pixelAddrs.end()) {
                std::cerr << "NO POINTS WERE FOUND FOR x=1 !!!!" << std::endl;
            }
            */
            
            std::vector<Point> points = pixelAddrs[x];
            if (x == 1) {
                if (points.size() == 0) {
                    std::cerr << "NO POINTS WERE FOUND FOR x=1 !!!!" << std::endl;
                }
            }
            
            
            // If no pixels found in this column, the polygon has been completely bounded, and is
            //  now a complete polygon:
            //if (it == pixelAddrs.end()) {
            if (points.size() == 0) {
                done = true;
                // THIS POLYGON has all it's boundaries done, spin it off to a completed polygon!!!!
                spinOffPolygon();  // also toss out any "non-done" Holes, since if they haven't been
                                   // written as "done", they're NOT REALLY HOLES!
                return;
            }
            //std::vector<Point> points = it->second;
            if (topToBottomEdges.size() == 0) {
                std::cerr << " ... for x=" << x << ", topToBottomEdges is EMPTY" << std::endl;
                // note that ALL points on the LEFT-MOST edge will ALL be contiguous
                Point firstPoint = points[0];
                Point lastPoint = points[points.size()-1];
                topToBottomEdges.push_back(firstPoint);
                // Now add 1 more to the y coordinate of the last point, to point to the LOWER LEFT of that
                //  pixel:
                lastPoint.setY(lastPoint.y+1);
                topToBottomEdges.push_back(lastPoint);
                // Now add 1 to the x coordinate of the last point, to point to the LOWER RIGHT of that pixel:
                lastPoint.setX(lastPoint.x+1);
                topToBottomEdges.push_back(lastPoint);
                bottomToTopEdges.push_back(lastPoint);  // yes ... this is a duplicate (above)
                Point secondToLast(firstPoint.x+1,firstPoint.y);
                bottomToTopEdges.push_back(secondToLast);
                bottomToTopEdges.push_back(firstPoint);
            } else {
                
                std::cerr << " ... for x=" << x << ", topToBottomEdges NOT!!!! EMPTY" << std::endl;
                
                /*
                if (x == 1) {
                    std::cerr << "BEFORE EDGIFY COLUMN 1:" << std::endl;
                    std::cerr << "   top-to-bottom edges:  ";
                    for (int n = 0; n < topToBottomEdges.size(); n++) {
                        Point pt = topToBottomEdges[n];
                        int x = pt.x;
                        int y = pt.y;
                        std::cerr << "  (" << x << "," << y << ")";
                    }
                    std::cerr << std::endl;
                }
                 */
                
                
                // Get the last edge point:
                Point lastEdgePoint = topToBottomEdges[topToBottomEdges.size()-1];
                // ... and the lowest-down y-point
                Point lowestYPoint = points[points.size()-1];
                // Assign the newest "top-to-bottom" edges
                Point nextPt1(lastEdgePoint.x,lowestYPoint.y+1);
                Point nextPt2(lowestYPoint.x+1,lowestYPoint.y+1);
                if ((lastEdgePoint.y - 1) != lowestYPoint.y) {
                    topToBottomEdges.push_back(nextPt1);
                    topToBottomEdges.push_back(nextPt2);
                } else {
                    //topToBottomEdges.erase(topToBottomEdges.size()-1);
                    topToBottomEdges.erase(topToBottomEdges.end()-1);
                    topToBottomEdges.push_back(nextPt2);
                }
                // Save the current "bottom-to-top" edges, for later, because the ones from x-1 to x=0,
                //  we want to save!
                std::vector<Point> saveBottomToTopEdges(bottomToTopEdges);
                // NOW assign the newest "bottom-to-top" edges (reassigning some of them ...)
                bottomToTopEdges[0] = topToBottomEdges[topToBottomEdges.size()-1];
                // Remove the "bottom-to-top" edges from the PREVIOUS column, since we've added a new column
                bottomToTopEdges.resize(1);
                
                // NOW go up the list of points, also looking for "holes"
                //std::vector<std::vector<Point>> holeList;
                int lastYVal = lowestYPoint.y;
                int yVal = lastYVal;
                int xVal = lowestYPoint.x;  // THIS SHOULD ALWAYS BE THE SAME, processing one column at a time
                for (int i = points.size()-2; i >= 0; i--) {
                    yVal = points[i].y;
                    if (yVal != (lastYVal - 1)) {
                        std::vector<Point> holePts(lastYVal - 1 - yVal);
                        for (int j = 0; j < holePts.size(); j++) {
                            Point pt(xVal, yVal - j);
                            holePts[j] = pt;
                        }
                        addPossibleHolePoint(holePts);
                        //holeList.push_back(holePts);
                        // assign the "jag" in the bottom-to-top edge:
                        Point jag1(xVal+1, lastYVal);
                        Point jag2(xVal, lastYVal);
                        Point jag3(xVal, yVal+1);
                        Point jag4(xVal+1, yVal+1);
                        bottomToTopEdges.push_back(jag1);
                        bottomToTopEdges.push_back(jag2);
                        bottomToTopEdges.push_back(jag3);
                        bottomToTopEdges.push_back(jag4);
                    }
                    lastYVal = yVal;
                }
                // Now add the last upper-right point for this column's pixels
                Point lastUpperRightPt(xVal+1, lastYVal);
                bottomToTopEdges.push_back(lastUpperRightPt);
                //
                // At this point, we need to add the remainder of the saved top-to-bottom edges, going from
                // xVal-1 to x=0:
                int minYVal = lowestYPoint.y;
                int saveXVal;
                int index = 0;
                Point oldBottomToTopEdgePt = saveBottomToTopEdges[index];
                Point saveLowestYPoint = oldBottomToTopEdgePt;
                while (minYVal >= oldBottomToTopEdgePt.y) {
                    if (minYVal > oldBottomToTopEdgePt.y) saveLowestYPoint = oldBottomToTopEdgePt;
                    minYVal = oldBottomToTopEdgePt.y;
                    index++;
                    if (index >= saveBottomToTopEdges.size()) {
                        index = saveBottomToTopEdges.size() - 1;
                        break;
                    } else {
                        oldBottomToTopEdgePt = saveBottomToTopEdges[index];
                    }
                }
                Point lastPointAdded = bottomToTopEdges[bottomToTopEdges.size()-1];
                
                // Start adding edges from the previously saved bottom-to-top edges, to our new
                //  bottom-to-top list:
                if (lastPointAdded.y != saveLowestYPoint.y) {
                    Point pt(saveLowestYPoint.x,lastPointAdded.y);
                    bottomToTopEdges.push_back(pt);
                    bottomToTopEdges.push_back(saveLowestYPoint);
                } else {
                    // otherwise if x's are =, just add the next saved edge point
                    //   e.g. (10,2) -> (12,2) instead of (10,2), (11,2), and (12,2)
                    index++;
                    if (index < saveBottomToTopEdges.size()) {
                        saveLowestYPoint = saveBottomToTopEdges[index];
                        Point nextPoint(saveLowestYPoint.x+1,saveLowestYPoint.y+1);
                        bottomToTopEdges.push_back(nextPoint);
                    }
                }
                
                index++;
                while (index < saveBottomToTopEdges.size()) {
                    saveLowestYPoint = saveBottomToTopEdges[index];
                    bottomToTopEdges.push_back(saveLowestYPoint);
                }
            } // endif topToBottomEdges empty/ NOT empty
                
            
            // ALSO, edgify EACH of the holes!!!
            //for (int i = 0; i < holes.size(); i++) {
            //    Polygon hole = holes[i];
            //    hole.edgify(x);
            //}     // TEMP!!!
            
            
            std::cerr << "top-to-bottom edges: ";
            for (int n = 0; n < topToBottomEdges.size(); n++) {
                Point pt = topToBottomEdges[n];
                int x = pt.x;
                int y = pt.y;
                std::cerr << "  (" << x << "," << y << ")";
            }
            std::cerr << std::endl;
            std::cerr << "bottom-to-top edges: ";
            for (int n = 0; n < bottomToTopEdges.size(); n++) {
                Point pt = bottomToTopEdges[n];
                int x = pt.x;
                int y = pt.y;
                std::cerr << "  (" << x << "," << y << ")";
            }
            std::cerr << std::endl;

            
            
            
            // REMOVE saved pixels from the PREVIOUS column, since we now have our edges!
            //std::cerr << " *** erasing points (pixelAddrs) for x=" << x << std::endl;
            //pixelAddrs.erase(x);
            //pixelAddrs[x] = emptyVector;
            if (x != 0) {
                points.clear();
                pixelAddrs[x-1] = points;
            }
            
            /*
            if (x == 0) {
            //typename std::map<int, std::vector<Point> >::iterator it;
            it = pixelAddrs.find(x);
            if (it != pixelAddrs.end()) {
                points = it->second;
                std::cerr << " ... NOW points for x=0 are: ";
                for (int n = 0; n < points.size(); n++) {
                    Point pt = points[n];
                    int x = pt.x;
                    int y = pt.y;
                    std::cerr << "  (" << x << "," << y << ")";
                }
                std::cerr << std::endl;
            }
            }
             */
        }
        
        
        void addPossibleHolePoint(std::vector<Point> points) {
            std::cerr << " --- In the 'hole' test function 'addPossibleHolePoint'" << std::endl;
            // For EACH of the current holes in this polygon, see if the FIRST of the given
            //  ADJACENT points is contiguous to it:
            if (points.size() == 0) return;
            Point pt = points[0];
            for (int i = 0; i < holes.size(); i++) {
                Polygon hole = holes[i];
                if (hole.pointIsContiguous(pt.x, pt.y)) {
                    // add the rest of these points to the hole:
                    for (int j = 1; j < points.size(); j++) {
                        pt = points[j];
                        hole.pointIsContiguous(pt.x, pt.y);  // will add the point ...
                    }
                    return;   // Ok we added these points to a current hole
                }
            }
            // NONE of the current holes has pixels adjacent to these new "hole" pixels,
            //  Create a NEW hole
            Polygon newHole(points);
            holes.push_back(newHole);
            return;
        }
        
        // Used for the "hole" polygons:
        bool pointIsContiguous(int x, int y) {
            std::cerr << " --- In the 'hole' test function 'pointIsContiguous'" << std::endl;
            Point testPt(x,y);
            // If this hole is already bound, can't add any more pixels to it
            if (done) return false;
            
            // See if this point is to the right or above or below any current point in this "hole"
            //  polygon:
            for (int i = 0; i < allPoints.size(); i++) {
                Point pt = allPoints[i];
                if (pt.x == (x - 1) && pt.y == y) {
                    allPoints.push_back(testPt);
                    return true;
                } else if (pt.x == x && (pt.y == (y - 1) || pt.y == (y + 1))) {
                    allPoints.push_back(testPt);
                    return true;
                }
            }
            return false;   // caller will have to start a new "Hole" in the current polygon
        }
        
        void spinOffPolygon() {
            // FOR NOW, just write out edges:
            printf( "  Polygon %d:  IS DONE\n", polygonNumber);
            for (int i = 0; i < topToBottomEdges.size(); i++) {
                Point pt = topToBottomEdges[i];
                //printf("    (%d,%d)\n", pt.x, pt.y );
                printf("    (%f,%f)\n", pt.x, pt.y );
            }
            for (int i = 1; i < bottomToTopEdges.size(); i++) {
                Point pt = bottomToTopEdges[i];
                //printf("    (%d,%d)\n", pt.x, pt.y );
                printf("    (%f,%f)\n", pt.x, pt.y );
            }
            // NOW print holes
            //for (int i = 0; i < holes.size(); i++) {
            //    Polygon hole = holes[i];
            //    hole.spinOffPolygon();
            //}
        }
        
    };   // end class: Polygon
    
}// end namespace GeoStar

#endif //RASTER_HPP_
