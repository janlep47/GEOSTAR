// Raster_rotate.cpp
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

    
    
    Raster* Raster::rotate(const float angle) {
        Slice in(0,0,get_nx(),get_ny());
        return rotate(angle, in);
    }


    
    Raster* Raster::rotate(const float angle, const Slice &inSlice) {
        // if angle in degrees: cos(angle*PI/180) ....
        Slice in = inSlice;
        // Make sure the x0,y0 and deltaX/Y values are valid for this raster:
        verifySlice(in);
        
        int inputX0 = in.getX0();
        int inputY0 = in.getY0();
        int inputWidth = in.getDeltaX() - inputX0;
        int inputHeight = in.getDeltaY() - inputY0;
        double cosAngle = cos(angle);
        double sinAngle = sin(angle);
        int outX0 = 0;
        int outY0 = 0;
        int nx = (cosAngle * inputWidth) + (sinAngle * inputHeight);
        int ny = (cosAngle * inputHeight) + (sinAngle * inputWidth);
        Slice out(outX0,outY0,nx,ny);
        
        std::string name = "rotate_"+std::to_string(nx)+"_"+std::to_string(ny);
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(name, type, nx, ny);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        
        return rotate(angle, in, rasNew);
    }

    
    
    
    Raster* Raster::rotate(const float angle, Raster *outRaster) {
        // if angle in degrees: cos(angle*PI/180) ....
        Slice in(0,0,get_nx(),get_ny());
        return rotate(angle, in, outRaster);
    }

    
    
    
    Raster* Raster::rotate(const float angle, const Slice &inSlice, Raster *outRaster) {
        // if angle in degrees: cos(angle*PI/180) ....
        Slice in = inSlice;
        // Make sure the x0,y0 and deltaX/Y values are valid for this raster:
        verifySlice(in);
        
        int inputX0 = in.getX0();
        int inputY0 = in.getY0();
        int inputWidth = in.getDeltaX();
        int inputHeight = in.getDeltaY();
        double cosAngle = cos(angle);
        double sinAngle = sin(angle);
        int outX0 = 0;
        int outY0 = 0;
        int nx = (cosAngle * inputWidth) + (sinAngle * inputHeight);
        int ny = (cosAngle * inputHeight) + (sinAngle * inputWidth);
        Slice out(outX0,outY0,nx,ny);
        
        std::string name = "rotate_"+std::to_string(nx)+"_"+std::to_string(ny);
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew = outRaster;
        // reset output raster's nx and ny to calculated values:
        if (rasNew->get_nx() != nx || rasNew->get_ny() != ny)
            rasNew->setSize(nx, ny);
        
        switch(raster_datatype) {
            case INT8U:
                return rotateType<uint8_t>(angle, in, out, rasNew);
                /*
                 case INT8S:
                 return warpType<int8_t>(warpInfo, in, out, outRaster);
                 case INT16U:
                 return warpType<uint16_t>(warpInfo, in, out, outRaster);
                 case INT16S:
                 return warpType<int16_t>(warpInfo, in, out, outRaster);
                 case INT32U:
                 return warpType<uint32_t>(warpInfo, in, out, outRaster);
                 case INT32S:
                 return warpType<int32_t>(warpInfo, in, out, outRaster);
                 case INT64U:
                 return warpType<uint64_t>(warpInfo, in, out, outRaster);
                 case INT64S:
                 return warpType<int64_t>(warpInfo, in, out, outRaster);
                 */
            case REAL32:
                return rotateType<float>(angle, in, out, rasNew);
                
            case REAL64:
                return rotateType<double>(angle, in, out, rasNew);
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
    }

    
        
    template <typename T>
    Raster* Raster::rotateType(const float angle, const Slice &in, const Slice &out, Raster *outRaster) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        GeoStar::Slice tileDescriptor(0,0,8,8);
        int maxNumberSlicesInRam = 70;
        
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        GeoStar::Slice scalingSlice(0,0,2,2,4);
        
        // if angle in degrees: cos(angle*PI/180) ....

        GeoStar::Raster *rasNew = outRaster;
        Slice sliceOut = out;
        Slice sliceInput = in;
        long int xIn0 = sliceInput.getX0();
        long int yIn0 = sliceInput.getY0();
        long int oldDeltaY, oldDeltaX;
        oldDeltaX = sliceInput.getDeltaX();
        oldDeltaY = sliceInput.getDeltaY();
        long int xInMax = xIn0 + oldDeltaX;
        long int yInMax = yIn0 + oldDeltaY;
        
        long int newDeltaY, newDeltaX;
        newDeltaX = sliceOut.getDeltaX();
        newDeltaY = sliceOut.getDeltaY();
        long int xOut0 = sliceOut.getX0();
        long int yOut0 = sliceOut.getY0();
        long int xOutMax = xOut0 + newDeltaX;
        long int yOutMax = yOut0 + newDeltaY;
        
        double xOriginOld, yOriginOld, xOriginNew, yOriginNew;
        xOriginOld = xIn0 + (oldDeltaX/2.0);
        yOriginOld = yIn0 + (oldDeltaY/2.0);
        xOriginNew = xOut0 + (newDeltaX/2.0);
        yOriginNew = yOut0 + (newDeltaY/2.0);


        // TileIO will now handle reading input data.
        std::vector<T> newData(newDeltaX);    // this will hold ONE ROW of the new image - output slice
        long int X, Y;   // new raster coordinates, with origin in center of raster
        long int x, y;   // old pre-rotated raster slice coordinates, with origin in center of raster slice
        long int oldY, oldX;  // old pre-rotated raster slice coordinates, of actual raster pixels
        double xDiff, yDiff;  // fractional part of where the actual rotated pixel should be read, from the old raster
        
        T pixelVal;
        
        sliceOut.setDeltaY(1);
        
        /*   Using these equations:
        X = x*cos(θ) - y*sin(θ)
        Y = x*sin(θ) + y*cos(θ)
         
         ... go backwards (from new coordinates to old), by simply plugging in -ve theta:
         
         x = X*cos(-θ) - Y*sin(-θ)
         y = X*sin(-θ) + Y*cos(-θ)
         */
        
        double cosTheta = cos(-angle);
        double sinTheta = sin(-angle);
        
        double ySinTheta, yCosTheta;
        
        
        //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        for (long int i = yOut0; i < yOutMax; i++) {
            Y = yOriginNew - static_cast<double>(i);
            
            ySinTheta = Y*sinTheta;
            yCosTheta = Y*cosTheta;
            
            for (long int j = xOut0; j < xOutMax; j++) {
                X = static_cast<double>(j - xOriginNew);
                //y = X*sinTheta + Y*cosTheta;  // new
                y = X*sinTheta + yCosTheta;
                oldY = yOriginOld - static_cast<long> (y);    // long was int
                
                yDiff = y - static_cast<long>(y);
                getNearest(oldY, yDiff);

                if (oldY < yIn0) {
                    newData[j] = 0;
                    continue;
                } else if (oldY >= yInMax) {
                    newData[j] = 0;
                    continue;
                }

                //x = X*cosTheta - Y*sinTheta;
                x = X*cosTheta - ySinTheta;
                oldX = xOriginOld + static_cast<long> (x);    // long was int
                
                xDiff = x - static_cast<long>(x);
                getNearest(oldX, xDiff);

                if (oldX < xIn0) {
                    newData[j] = 0;
                    continue;
                } else if (oldX >= xInMax) {
                    newData[j] = 0;
                    continue;
                }
                pixelVal = getScaledPixelFromTile<T>(oldY, oldX, xDiff, yDiff, sliceInput, scalingSlice, reader);
                newData[j] = pixelVal;
            }
            // Current row is rotated, write it to the output raster
            rasNew->write(sliceOut,newData);
            sliceOut.setY0(i);
        }
        
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "rotate execution duration: " << duration << std::endl;
        
        return rasNew;
    }

    template Raster* Raster::rotateType<uint8_t>(const float, const Slice&, const Slice&, Raster*);
    template Raster* Raster::rotateType<float>(const float, const Slice&, const Slice&, Raster*);
    template Raster* Raster::rotateType<double>(const float, const Slice&, const Slice&, Raster*);

}// end namespace GeoStar
