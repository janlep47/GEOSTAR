// Raster_flip.cpp
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

    

    
    Raster* Raster::flip(const std::string &newRasterName, short flipAxis) {
        if (flipAxis == Raster::FLIP_HORIZONTALLY ||
            flipAxis == Raster::FLIP_VERTICALLY ||
            flipAxis == Raster::FLIP_BOTH) {
            
            // Create the new Raster, to hold the transformed raster, using the same type and size as this raster:
            RasterType type = raster_datatype;
            GeoStar::Raster *rasNew;
            try {
                rasNew = image->create_raster(newRasterName, type, get_nx(), get_ny());
            } catch (RasterExistsException e) {
                rasNew = image->open_raster(newRasterName);
            }
            return flip(rasNew, flipAxis);
        } else {
            throw_FlipOptionError(std::to_string(flipAxis));
        }
        return NULL;
    }
    
    
    Raster* Raster::flip(Raster *rasNew, short flipAxis) {
        if (flipAxis == Raster::FLIP_HORIZONTALLY ||
            flipAxis == Raster::FLIP_VERTICALLY ||
            flipAxis == Raster::FLIP_BOTH) {
            
            // Verify the new raster's dimensions are ok for this flip:
            //    .... do similar to rotate() ....
            
            
            // Now, call the templated flipType<>() method, based on the new raster type:
            switch(raster_datatype) {
                case INT8U:
                    return flipType<uint8_t>(rasNew, flipAxis);
                    /*
                     case INT8S:
                     return flipType<int8_t>(rasNew, flipAxis);
                     case INT16U:
                     return flipType<uint16_t>(rasNew, flipAxis);
                     case INT16S:
                     return flipType<int16_t>(rasNew, flipAxis);
                     case INT32U:
                     return flipType<uint32_t>(rasNew, flipAxis);
                     case INT32S:
                     return flipType<int32_t>(rasNew, flipAxis);
                     case INT64U:
                     return flipType<uint64_t>(rasNew, flipAxis);
                     case INT64S:
                     return flipType<int64_t>(rasNew, flipAxis);
                     */
                case REAL32:
                    return flipType<float>(rasNew, flipAxis);
                    
                case REAL64:
                    return flipType<double>(rasNew, flipAxis);
                    //case COMPLEX_INT16:
                    //    return
                default:
                    throw_RasterUnsupportedTypeError(fullRastername);
            }
        } else {
            throw_FlipOptionError(std::to_string(flipAxis));
        }
        return NULL;
    }

    
    
    
    template <typename T>
    Raster* Raster::flipType(Raster *outRaster, const int flipAxis) {
        
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
        
        long int newNy = ny;      //sliceOut.getDeltaY();
        long int newNx = nx;      //sliceOut.getDeltaX();
        long int xOut0 = 0;                  //sliceOut.getX0();
        long int yOut0 = 0;                  //sliceOut.getY0();
        long int xOutMax = newNx - 1;        //xOut0 + newDeltaX;
        long int yOutMax = newNy - 1;        //yOut0 + newDeltaY;
        std::vector<T> newData(newNx);    // this will hold ONE ROW of the new image - output slice
        long int oldY, oldX;
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
        double halfNX = (nx * 0.5) - 0.5;
        double halfNY = (ny * 0.5) - 0.5;
        
        for (long int i = yOut0; i < yOutMax; i++) {
            if (flipAxis == FLIP_HORIZONTALLY) oldY = i;
            else {
                yVal = static_cast<double>(i);
                yVal = halfNY + (halfNY - yVal);
                oldY = static_cast<long> (yVal);
            }
            
            for (long int j = xOut0; j < xOutMax; j++) {
                if (flipAxis == FLIP_VERTICALLY) oldX = j;
                else {
                    xVal = static_cast<double>(j);
                    xVal = halfNX + (halfNX - xVal);
                    oldX = static_cast<long> (xVal);
                }
                pixelVal = getScaledPixelFromTile<T>(oldY, oldX, 0.0, 0.0, sliceInput, scalingSlice, reader);
                
                newData[j] = pixelVal;
            }
            sliceOut.setY0(i);
            rasNew->write(sliceOut,newData);
            
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "transform execution duration: " << duration << std::endl;
        
        return rasNew;
    }
    
    
    template Raster* Raster::flipType<uint8_t>(Raster*, const int);
    template Raster* Raster::flipType<float>(Raster*, const int);
    template Raster* Raster::flipType<double>(Raster*, const int);


}// end namespace GeoStar
