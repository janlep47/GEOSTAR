// Raster_reproject.cpp
//
// by Janice Richards, Feb 20, 2018
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {

    

    
    Point Raster::geographicCoordinateTransform( std::string &srcWKT, std::string &targetWKT, Point pt) {
        GDALAllRegister();  // NEED HERE ??????~!!!!!!!
        OGRSpatialReference oSourceSRS, oTargetSRS;
        OGRCoordinateTransformation *poCT;
        
        // This method will wipe the existing SRS definition, and reassign it based on the contents of the passed WKT
        // string. Only as much of the input string as needed to construct this SRS is consumed from the input string,
        // and the input string pointer is then updated to point to the remaining (unused) input.
        //
        // the calling param to importFromWkt MUST be a char**
        //
        
        char* srcWktChars = new char[srcWKT.length()+1];
        char* targetWktChars = new char[targetWKT.length()+1];
        strcpy(srcWktChars, srcWKT.c_str());
        strcpy(targetWktChars, targetWKT.c_str());
        
        oSourceSRS.importFromWkt(&srcWktChars);
        oTargetSRS.importFromWkt(&targetWktChars);
        poCT = OGRCreateCoordinateTransformation( &oSourceSRS,
                                                &oTargetSRS );
        double x, y;
        x = pt.getX();
        y = pt.getY();
        
        Point transformedPt;
        
        if( poCT == NULL || !poCT->Transform( 1, &x, &y ) ) {
            throw_CoordinateTransformError("point: "+std::to_string(x)+","+std::to_string(y)+"\nsource WKT: " + srcWKT +
                                           "\ndestination WKT: " + targetWKT);
        } else {
            //printf( "(%f,%f) -> (%f,%f)\n",
            //       pt.getX(), pt.getY(),
            //       x, y );
            transformedPt.setX(x);
            transformedPt.setY(y);
        }
        return transformedPt;
    }

    
    Point Raster::geographicCoordinateTransform( OGRCoordinateTransformation *poCT, Point pt) {
        double x, y;
        x = pt.getX();
        y = pt.getY();
        /*
        Point transformedPt;
        
        if( poCT == NULL || !poCT->Transform( 1, &x, &y ) ) {
            char* srcWktChars;
            char* targetWktChars;
            poCT->GetSourceCS()->exportToPrettyWkt(&srcWktChars);
            poCT->GetTargetCS()->exportToPrettyWkt(&targetWktChars);
            std::string srcWkt(srcWktChars), targetWkt(targetWktChars);
            throw_CoordinateTransformError("point: "+std::to_string(x)+","+std::to_string(y)+"\nsource WKT: " + srcWkt +
                                           "\ndestination WKT: " + targetWkt);
        } else {
            //printf( "(%f,%f) -> (%f,%f)\n",
            //       pt.getX(), pt.getY(),
            //       x, y );
            transformedPt.setX(x);
            transformedPt.setY(y);
        }
        return transformedPt;
         */
        return geographicCoordinateTransform(poCT, x, y);
    }

    
    Point Raster::geographicCoordinateTransform( OGRCoordinateTransformation *poCT, const double xval, const double yval) {
        double x, y;
        x = xval;
        y = yval;
        
        Point transformedPt;
        
        if( poCT == NULL || !poCT->Transform( 1, &x, &y ) ) {
            char* srcWktChars;
            char* targetWktChars;
            poCT->GetSourceCS()->exportToPrettyWkt(&srcWktChars);
            poCT->GetTargetCS()->exportToPrettyWkt(&targetWktChars);
            std::string srcWkt(srcWktChars), targetWkt(targetWktChars);
            throw_CoordinateTransformError("point: "+std::to_string(x)+","+std::to_string(y)+"\nsource WKT: " + srcWkt +
                                           "\ndestination WKT: " + targetWkt);
        } else {
            //printf( "(%f,%f) -> (%f,%f)\n",
            //       pt.getX(), pt.getY(),
            //       x, y );
            transformedPt.setX(x);
            transformedPt.setY(y);
        }
        return transformedPt;
    }

    
    
    Raster* Raster::transform(std::string &newWKT, std::string &newRasterName, double newDeltaX, double newDeltaY) {
        // Create the new Raster, to hold the transformed raster, using the same type as this raster:
        RasterType type = raster_datatype;
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(newRasterName, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(newRasterName);
        }
        return transform(newWKT, newDeltaX, newDeltaY, rasNew);
    }
    
    
    Raster* Raster::transform(std::string &newWKT, double newDeltaX, double newDeltaY, Raster *rasNew) {
        // Find the current (0,0) geographic point in the current raster, and the
        //  current deltaX and deltaY for this raster (WKT coords/per raster pixel),
        //  which is the "location" attribute:
        //
        // later, do a try/catch here, after fixing getLocationAttributes() to throw parseErrors!
        if (!getLocationAttributes()) throw_AttributeDoesNotExistError(fullRastername, "location");

        int nx, ny;
        nx = get_nx();
        ny = get_ny();
        //std::cerr << "  nx = " << nx << "  ny = " << ny << std::endl;
        
        // get the coordinates (of current WKT) for this raster, for all 4 corners:
        Point ur, ll, lr;
        Point ul(x0, y0);
        ur.x = ul.x + (nx*deltaX);
        ur.y = ul.y;
        ll.x = ul.x;
        ll.y = ul.y + (ny*deltaY);
        lr.x = ur.x;
        lr.y = ll.y;
        
        
        GDALAllRegister();  // NEED HERE ??????~!!!!!!!
        OGRSpatialReference oSourceSRS, oTargetSRS;
        OGRCoordinateTransformation *poCT;

        // try {
        std::string wkt = getWKT();
        // } catch (NoWKTdefinedError e) {
        // }

        char* srcWktChars = new char[wkt.length()+1];
        char* targetWktChars = new char[newWKT.length()+1];
        strcpy(srcWktChars, wkt.c_str());
        strcpy(targetWktChars, newWKT.c_str());
        
        oSourceSRS.importFromWkt(&srcWktChars);
        oTargetSRS.importFromWkt(&targetWktChars);
        poCT = OGRCreateCoordinateTransformation( &oSourceSRS,
                                                 &oTargetSRS );
        
        // get the coordinates of the new raster, using the new WKT, and the current 4 corners:
        Point newUL = geographicCoordinateTransform(poCT, ul);
        Point newUR = geographicCoordinateTransform(poCT, ur);
        Point newLL = geographicCoordinateTransform(poCT, ll);
        Point newLR = geographicCoordinateTransform(poCT, lr);

        // For now, we're passing in newDeltaX and newDeltaY ... but later we can calculate based on doing
        // a geographicCoordinateTransform() for a pixel in the center of this raster, and for the pixel one
        // over to the right, and one down (dx = 1, dy = 1) ... and calculating the corresponding new points'
        //  delta x's and y's.
        
        // Find the bounding box for the new raster corners, b/c it MAY NOT be a perfect rectangle!
        double maxNXinCoords = getMaxDiff(newUL.x, newUR.x, newLL.x, newLR.x);
        double maxNYinCoords = getMaxDiff(newUL.y, newUR.y, newLL.y, newLR.y);
        // convert the deltaX/Y from coords to pixels:
        int newNx = abs(maxNXinCoords / newDeltaX);
        int newNy = abs(maxNYinCoords / newDeltaY);
        
        
        // reset output raster's nx and ny to calculated values:
        if (rasNew->get_nx() != newNx || rasNew->get_ny() != newNy)
            rasNew->setSize(newNx, newNy);
        

        
        // ASK LEE ... when actual UL point of new raster is different than the calculated newUL (for when the resulting
        //   new raster is skewed a bit from the input raster (this one), should this be recalculated and set in the
        //   new raster's location attribute?  Even though the raster data at this point is 0?  Or should I just set x0
        //   in the new raster's location attribute to the originally converted newUL point?
        //
        //  recalculate newUL ... newUL.x = min(newUL.x, newLL.x) ... newUL.y = min(newUL.y, newLL.y)  ??
        // ASK LEE ... !!!!!!!!!!!!!!!
        
        
        
        
        
        
        
        
        
        
        
        // Set the new transformed raster's attributes:
        rasNew->setWKT(newWKT);
        rasNew->setLocationAttributes(newUL.x, newUL.y, newDeltaX, newDeltaY);
        
        //  NOW, reset the coordinateTransformation object to switch it's order, from src->target, to
        //   target->src.  This is decide for each pixel of the target raster, which of these (src) pixels it
        //   should be mapped from.
        poCT = OGRCreateCoordinateTransformation(&oTargetSRS,
                                                 &oSourceSRS );

        // Now, call the templated tranformType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
                return transformType<uint8_t>(rasNew, poCT, ul, newUL, newDeltaX, newDeltaY);
                /*
                 case INT8S:
                 return transformType<int8_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT16U:
                 return transformType<uint16_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT16S:
                 return transformType<int16_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT32U:
                 return transformType<uint32_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT32S:
                 return transformType<int32_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT64U:
                 return transformType<uint64_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 case INT64S:
                 return transformType<int64_t>(rasNew, poCT, newUL, newDeltaX, newDeltaY);
                 */
            case REAL32:
                return transformType<float>(rasNew, poCT, ul, newUL, newDeltaX, newDeltaY);
                
            case REAL64:
                return transformType<double>(rasNew, poCT, ul, newUL, newDeltaX, newDeltaY);
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        return NULL;
    }
    
    
    
    
    
    // ul is the upper-left wkt-coordinates for this entire raster .... later we could allow just a portion of it
    //  (specifying an input slice), so that ul would be the upper-left wkt-coordinates of that portion ...
    // newUL is the upper-left wkt-coordinates for the new raster, which coorespond to the upper-left of the
    //  input (this) raster, being transormed to the new raster.
    // deltaX and deltaY are the changes in wkt-coordinates for each pixel, in the x- and y- directions, respectively.
    // newDeltaX and newDeltaY similarly, are for the new raster being transformed from this raster.
    template <typename T>
    Raster* Raster::transformType(Raster *outRaster, OGRCoordinateTransformation *poCT, const Point ul,
                                  const Point newUL, const double newDeltaX, const double newDeltaY) {
        
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        GeoStar::Slice tileDescriptor(0,0,8,8);
        int maxNumberSlicesInRam = 70;
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        
        GeoStar::Slice scalingSlice(0,0,2,2,4);
        
        GeoStar::Raster *rasNew = outRaster;
        long int nx = get_nx();
        long int ny = get_ny();
        long int xIn0 = 0;   //in.getX0();   ... for Slice in;
        long int yIn0 = 0;   //in.getY0();
        long int xInMax = nx - 1;       //xIn0 + in.getDeltaX();
        long int yInMax = ny - 1;       //yIn0 + in.getDeltaY();
        
        long int newNy = outRaster->get_ny();      //sliceOut.getDeltaY();
        long int newNx = outRaster->get_nx();      //sliceOut.getDeltaX();
        long int xOut0 = 0;                  //sliceOut.getX0();
        long int yOut0 = 0;                  //sliceOut.getY0();
        long int xOutMax = newNx - 1;        //xOut0 + newDeltaX;
        long int yOutMax = newNy - 1;        //yOut0 + newDeltaY;
        
        // TileIO will now handle reading input data.
        std::vector<T> newData(newNx);    // this will hold ONE ROW of the new image - output slice
        double xScaled, yScaled;
        long int oldY, oldX;
        double xDiff, yDiff;
        T pixelVal;
        
        GeoStar::Point pt;    // pt.x and pt.y are double
        double yVal, xVal;
        
        // MAY want to pass in sliceInput/sliceOut LATER ... and use as in above comments.
        Slice sliceInput(0,0,nx,ny);
        Slice sliceOut(0,0,newNx,newNy);
        //sliceInput.setDeltaY(3);
        sliceOut.setDeltaY(1);
        
        // NOTE: deltaX and deltaY are the wkt-coordinate differences for each pixel in this raster, for the
        //       x- and y- directions.  And, newDeltaX and newDeltaY are the wkt-coordinate differences for this
        //       newly created raster's pixels (with it's own different wkt).
        //
        double halfNewDeltaX = newDeltaX * 0.5;
        double halfNewDeltaY = newDeltaY * 0.5;
        double oneOverDeltaX = 1.0/deltaX;
        double oneOverDeltaY = 1.0/deltaY;
        double halfDeltaX = deltaX * 0.5;
        double halfDeltaY = deltaY * 0.5;
        
        double xcoord, ycoord;  // the (new)wkt coordinates of the current pixel being written to the new raster
        
        for (long int i = yOut0; i < yOutMax; i++) {
            yVal = static_cast<double>(i);
            // convert this pixel coordinate to the wkt coordinates
            ycoord = newUL.y + (halfNewDeltaY) + (newDeltaY*i);
            for (long int j = xOut0; j < xOutMax; j++) {
                xVal = static_cast<double>(j);
                // convert this pixel coordinate to the wkt coordinates
                xcoord = newUL.x + (halfNewDeltaX) + (newDeltaX*j);

                pt = geographicCoordinateTransform(poCT, xcoord, ycoord);
                
                // Now that we have the cooresponding wkt-coordinates for this raster, we need to convert them to
                //  this raster's pixel coordinates:
                xScaled = ((pt.x - ul.x) - halfDeltaX) * oneOverDeltaX;
                
                oldX = static_cast<long> (xScaled);
                
                // IMPORTANT ... getNearest() can sometimes add 1 to the pixel coordinate (oldX/oldY) ...
                //  so if it does that, and it is one pixel beyond the current raster coordinates, then need to do
                //  the OUT-OF-BOUNDS test AFTER calling getNearest():
                xDiff = xScaled - static_cast<long>(oldX);
                getNearest(oldX, xDiff);

                // Test for pixel coordinate OUT-OF-BOUNDS:
                if (oldX < xIn0) {
                    newData[j] = 0;
                    continue;
                } else if (oldX >= xInMax) {
                    newData[j] = 0;
                    continue;
                }
                
                yScaled = ((pt.y - ul.y) - halfDeltaY) * oneOverDeltaY;
                
                oldY = static_cast<long> (yScaled);
                
                // IMPORTANT ... getNearest() can sometimes add 1 to the pixel coordinate (oldX/oldY) ...
                //  so if it does that, and it is one pixel beyond the current raster coordinates, then need to do
                //  the OUT-OF-BOUNDS test AFTER calling getNearest():
                yDiff = yScaled - static_cast<long>(oldY);
                getNearest(oldY, yDiff);

                if (oldY < yIn0) {
                    newData[j] = 0;
                    continue;
                } else if (oldY >= yInMax) {
                    newData[j] = 0;
                    continue;
                }
                
                pixelVal = getScaledPixelFromTile<T>(oldY, oldX, xDiff, yDiff, sliceInput, scalingSlice, reader);
                
                newData[j] = pixelVal;
           }
            sliceOut.setY0(i);
            //std::cerr << " * " << i << std::endl;
            rasNew->write(sliceOut,newData);
            //std::cerr << " here " << std::endl;

        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "transform execution duration: " << duration << std::endl;
        
        return rasNew;
    }

    
    template Raster* Raster::transformType<uint8_t>(Raster*, OGRCoordinateTransformation*, const Point,
                                                    const Point, const double, const double);
    template Raster* Raster::transformType<float>(Raster*, OGRCoordinateTransformation*, const Point,
                                                  const Point, const double, const double);
    template Raster* Raster::transformType<double>(Raster*, OGRCoordinateTransformation*, const Point,
                                                   const Point, const double, const double);

}// end namespace GeoStar
