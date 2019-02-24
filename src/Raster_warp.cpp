// Raster_warp.cpp
//
// by Janice Richards, Feb 20, 2018
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

//#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {

   
    
    
    Raster* Raster::rotateWithWarp(const float angle, const Slice &inSlice) {
        // if angle in degrees: cos(angle*PI/180) ....
        Slice in = inSlice;
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
        
        std::string name = "rotwarp_"+std::to_string(nx)+"_"+std::to_string(ny);
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(name, type, nx, ny);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        

        // NOW, create 51 "ground control points", that correspond to points in the output slice,
        //  which will map to the input slice
        
        int ngcp = 51;
        int order = 1;    // ALL ORDERS WORK !!!!!!!!
        //double rix[51], riy[51], rox[51], roy[51];
        //double rmsx[51], rmsy[51];
        std::vector<double> rix(ngcp), riy(ngcp), rox(ngcp), roy(ngcp);
        double rmsx, rmsy;

        double oldOriginX, oldOriginY, newOriginX, newOriginY;
        oldOriginX = inputX0 + inputWidth/2.0;
        oldOriginY = inputY0 + inputHeight/2.0;
        newOriginX = outX0 + nx/2.0;
        newOriginY = outY0 + ny/2.0;
        
        /*   Using these equations:
         X = x*cos(θ) - y*sin(θ)
         Y = x*sin(θ) + y*cos(θ)
         */
        
        // Point 0 - origin(s)
        rix[0] = oldOriginX; riy[0] = oldOriginY;
        rox[0] = newOriginX; roy[0] = newOriginY;
        
        double inputWidthHalf = inputWidth/2.0;
        double inputHeightHalf = inputHeight/2.0;
        int val, xvalSign,yvalSign;
        double dval;
        double xVal, yVal;
        double newXVal, newYVal;
        std::srand((unsigned)time(0));
        for (int i = 1; i <= 50; i++) {
            xvalSign = std::rand()%2;
            yvalSign = std::rand()%2;
            
            val = std::rand()%100;
            dval = val/100.0;   // 0.0 to .99
            xVal = inputWidthHalf*dval;
            
            val = std::rand()%100;
            dval = val/100.0;   // 0.0 to .99
            yVal = inputHeightHalf*dval;
            
            if (xvalSign == 1) xVal *= -1.0;
            if (yvalSign == 1) yVal *= -1.0;
            newXVal = xVal*cosAngle - yVal*sinAngle;
            newYVal = xVal*sinAngle + yVal*cosAngle;
            rix[i] = translateXPoint(oldOriginX,xVal);
            riy[i] = translateYPoint(oldOriginY,yVal);
            rox[i] = translateXPoint(newOriginX,newXVal);
            roy[i] = translateYPoint(newOriginY,newYVal);
        }
        
        GeoStar::WarpParameters warpData;
        // go backwards, because we will START with the new output coordinates, and map them to the
        //  old input coordinates:
        warpData.GCPRegression2D(rox, roy, rix, riy, &order, &rmsx, &rmsy);
        return warp(warpData, in, out, rasNew);
    }

    
    
    
    
    // warp #1:
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs) {
        // default input slice:
        Slice in(0, 0, get_nx(), get_ny());
        
        std::string name = rastername+"WARP";
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            // create new output raster with a mutable size
            rasNew = image->create_raster(name, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        return warp(inputGCPs, outputGCPs, in, rasNew);   // calls warp #6
    }
    
    
    // warp #2:
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice) {
        std::string name = rastername+"WARP";
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            // create new output raster with a mutable size
            rasNew = image->create_raster(name, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        return warp(inputGCPs, outputGCPs, inSlice, rasNew);   // calls warp #6
    }

    
    
    
    // warp #3
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const std::string &name) {
        // default input slice:
        Slice in(0, 0, get_nx(), get_ny());
        
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            // create new output raster with a mutable size
            rasNew = image->create_raster(name, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        return warp(inputGCPs, outputGCPs, in, rasNew);   // calls warp #6
    }
    
    
    
    // warp #4
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice, const std::string &name) {
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            // create new output raster with a mutable size
            rasNew = image->create_raster(name, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }
        return warp(inputGCPs, outputGCPs, inSlice, rasNew);   // calls warp #6
    }
    
    
    
    // warp #5
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, Raster *rasNew) {
        // default input slice:
        Slice in(0, 0, get_nx(), get_ny());
        return warp(inputGCPs, outputGCPs, in, rasNew);  // calls warp #6
    }

    
    
    // warp #6
    //
    // This function will determine the warpParameters object, and create a default output Raster Slice,
    //   then call warp(warpInfo, inslice, outslice, newRaster)
    //
    // IMPORTANT NOTE r/t getting WarpParameters object:
    //   the number of gcp's (ground control points) should be AN ODD NUMBER !!!!!
    //   Change the WarpParameter's code to force this (if not already ....)
    //
    // THIS version of warp uses the TileIO, to handle reading in tiles of the input raster
    //  as needed:
    //
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice inSlice, Raster *rasNew) {
        // FORCE the number of ground control points to be ODD:
        int ngcp = (inputGCPs.size()/2) * 2;
        if (ngcp == inputGCPs.size()) ngcp--;
        
        // Verify the # of input points corresponds to the # of output points:
        if (inputGCPs.size() != outputGCPs.size()) {
            // THROW AN ERROR!
            std::cerr << "CRAP!" << std::endl;
        }
        
        std::vector<double> rix(ngcp), riy(ngcp), rox(ngcp), roy(ngcp);
        double rmsx, rmsy;
        int order = 0;  // FORCE GCPRegression to calculate the best order
        
        // Now, create the warpdata object:
        for (int i = 0; i < ngcp; i++) {
            rix[i] = inputGCPs[i].x;
            riy[i] = inputGCPs[i].y;
            rox[i] = outputGCPs[i].x;
            roy[i] = outputGCPs[i].y;
        }
        
        // Since no slice info is given, default to using entire input raster; so, need to determine the output raster's
        //  cooresponding size:
        GeoStar::WarpParameters tempWarpData;
        tempWarpData.GCPRegression2D(rix, riy, rox, roy, &order, &rmsx, &rmsy);
        Point maxPt = tempWarpData.GCPTransform(get_nx(), get_ny());
        
        // Reset the output raster's dimensions if necessary (NOTE: may throw an exception if created with specific dimentions
        //   and therefore immutable):
        // THIS MAY NOT WORK IF THE warp stuff doesn't yeild a max output point, for the max input point!!!!!!
        //
        // !!!!!!!!!!!!
        //
        
        
        long int newNx = maxPt.x;
        long int newNy = maxPt.y;
        if (rasNew->get_nx() < newNx || rasNew->get_ny() < newNy)
            rasNew->setSize(newNx, newNy);
        
        GeoStar::WarpParameters warpData;
        // go backwards, because we will START with the new output coordinates, and map them to the
        //  old input coordinates:
        warpData.GCPRegression2D(rox, roy, rix, riy, &order, &rmsx, &rmsy);
        
        // default slices
        //Slice in(getX0(), getY0(), get_nx(), get_ny());
        Slice out(0,0,newNx,newNy);

        return warp(warpData, inSlice, out, rasNew);
    }
    
    
    
    // warp #7
    //
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice, const Slice &outSlice, const std::string &name) {
        RasterType type = raster_datatype;
        
        GeoStar::Raster *rasNew;
        try {
            // create new output raster with a mutable size
            rasNew = image->create_raster(name, type);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
        }

        return warp(inputGCPs, outputGCPs, inSlice, outSlice, rasNew);
    }
    
    
    
    // warp #8
    //
    // This function will determine the warpParameters object,
    //   then call warp(warpInfo, inslice, outslice, newRaster)
    //
    // IMPORTANT NOTE r/t getting WarpParameters object:
    //   the number of gcp's (ground control points) should be AN ODD NUMBER !!!!!
    //   Change the WarpParameter's code to force this (if not already ....)
    //
    // THIS version of warp uses the TileIO, to handle reading in tiles of the input raster
    //  as needed:
    //template<typename T>
    Raster* Raster::warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice, const Slice &outSlice, Raster *rasNew) {
        // FORCE the number of ground control points to be ODD:
        int ngcp = (inputGCPs.size()/2) * 2;
        if (ngcp == inputGCPs.size()) ngcp--;
        
        // Verify the # of input points corresponds to the # of output points:
        if (inputGCPs.size() != outputGCPs.size()) {
            // THROW AN ERROR!
            std::cerr << "CRAP!" << std::endl;
        }
        
        std::vector<double> rix(ngcp), riy(ngcp), rox(ngcp), roy(ngcp);
        double rmsx, rmsy;
        int order = 0;  // FORCE GCPRegression to calculate the best order
        
        // Now, create the warpdata object:
        for (int i = 0; i < ngcp; i++) {
            rix[i] = inputGCPs[i].x;
            riy[i] = inputGCPs[i].y;
            rox[i] = outputGCPs[i].x;
            roy[i] = outputGCPs[i].y;
        }
        
        GeoStar::WarpParameters warpData;
        // go backwards, because we will START with the new output coordinates, and map them to the
        //  old input coordinates:
        warpData.GCPRegression2D(rox, roy, rix, riy, &order, &rmsx, &rmsy);
        
        return warp(warpData, inSlice, outSlice, rasNew);
    }

    
    
    
    //
    // IMPORTANT NOTE r/t getting WarpParameters object:
    //   the number of gcp's (ground control points) should be AN ODD NUMBER !!!!!
    //   Change the WarpParameter's code to force this (if not already ....)
    //
    // THIS version of warp uses the TileIO, to handle reading in tiles of the input raster
    //  as needed:
    Raster* Raster::warp(const WarpParameters warpInfo, const Slice &in, const Slice &out, Raster *outRaster) {
        //RasterType  raster_datatype;
        switch(raster_datatype) {
            case INT8U:
                //std::cout << " HERE B " << std::endl;
                return warpType<uint8_t>(warpInfo, in, out, outRaster);
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
                //std::cout << "FLOAT TYPE ";
                return warpType<float>(warpInfo, in, out, outRaster);
            case REAL64:
                return warpType<double>(warpInfo, in, out, outRaster);
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }

    }
    
    
    template <typename T>
    Raster* Raster::warpType(const WarpParameters warpInfo, const Slice &in, const Slice &out, Raster *outRaster) {

        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        
        
        // Note: this raster is 512 X 1024 pixels
        //
        // Note that the old "regular" warp took 0.057570 seconds
        
        //GeoStar::Slice tileDescriptor(0,0,128,128);     // 12.134 seconds
        //int maxNumberSlicesInRam = 5;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,256,256);     // 47.859 seconds
        //int maxNumberSlicesInRam = 5;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,64,64);     //   3.237 seconds  (BUT ALSO FIXED A BUG throwing out older tiles)
        //int maxNumberSlicesInRam = 5;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,64,64);     //    3.195 seconds
        //int maxNumberSlicesInRam = 8;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,16,16);     //    0.450297 seconds  !!!! (no change in code from above 2 trys!!!!!!!
        //int maxNumberSlicesInRam = 20;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //   0.360998  seconds
        //int maxNumberSlicesInRam = 50;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //   0.394495  seconds
        //int maxNumberSlicesInRam = 100;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,4,4);     //   0.418323  seconds
        //int maxNumberSlicesInRam = 100;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,4,4);     //   0.497101  seconds
        //int maxNumberSlicesInRam = 180;
        
        
        // try AGAIN, since BUG FIXED ...
        //GeoStar::Slice tileDescriptor(0,0,128,128);     // 12.013347 seconds  STILL CRAPPY
        //int maxNumberSlicesInRam = 5;
        
        // NOW try this, since replacing slice read with pixel reads:
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //   now 0.189813 ... was 0.360998  seconds  ...
        //int maxNumberSlicesInRam = 50;
        
        // Now same tile descriptor, but BACK to reading SLICES (vs individual pixels) BUT WITH SPED-UP
        //  CHANGES:
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //   0.259461  seconds (a little more that 0.189813)
        //int maxNumberSlicesInRam = 50;
        
        // RETRY:
        //GeoStar::Slice tileDescriptor(0,0,16,16);     //   0.294543  seconds  (was 0.450297 seconds )
        //int maxNumberSlicesInRam = 20;
        
        // RETRY:
        //GeoStar::Slice tileDescriptor(0,0,128,128);     // 6.089577 sec (was 12.134 seconds)
        //int maxNumberSlicesInRam = 5;
        
        // try this ...
        //GeoStar::Slice tileDescriptor(0,0,64,64);   //  0.908807 ..earlier 1.821461 sec (prev 3.195 seconds)
        //int maxNumberSlicesInRam = 8;
        
        // try:
        //GeoStar::Slice tileDescriptor(0,0,32,32);   //  0.362923
        //int maxNumberSlicesInRam = 20;
        
        // RETRY AGAIN:
        //GeoStar::Slice tileDescriptor(0,0,16,16);   // 0.226449 ..earlier 0.294543  (prev 0.450297 seconds )
        //int maxNumberSlicesInRam = 20;
        
        // RETRY (3rd time at least):
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //  0.222582
        //int maxNumberSlicesInRam = 50;
        
        // RETRY (3rd time at least):
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //  0.271
        //int maxNumberSlicesInRam = 40;
        
        // RETRY (3rd time at least):
        //GeoStar::Slice tileDescriptor(0,0,8,8);     //  0.204153 / 0.237041 / 0.217582 / 0.211129
        //int maxNumberSlicesInRam = 70;
        
        // try
        //GeoStar::Slice tileDescriptor(0,0,16,16);   // 0.228656 / 0.232202 / 0.232242 / 0.237962
        //int maxNumberSlicesInRam = 70;
        
        // RETRY AGAIN:
        //GeoStar::Slice tileDescriptor(0,0,128,128);     // 3.268377 prev 6.089577 sec (earliest 12.134 sec)
        //int maxNumberSlicesInRam = 5;                     // 3.138226 / 3.090430 / 3.237506
        
        // AFTER FIXING A FEW MORE BUGS ...
        //GeoStar::Slice tileDescriptor(0,0,256,256);     // check for bug in image write (sl defect)
        //int maxNumberSlicesInRam = 1;                  // 12.102776 / 12.255923
        
        
        // RETRY (4th time?) AFTER FIXING A FEW MORE BUGS ...:
        GeoStar::Slice tileDescriptor(0,0,8,8);     //  0.204849 / 0.212305 / 0.211564   (SAME)
        int maxNumberSlicesInRam = 70;   // WAS ... 0.204153 / 0.237041 / 0.217582 / 0.211129
        // 0.182227 - 0.219794
        // STILL POTENTIAL CHANGES TO SPEED TileIO (or could change name to "TileReader")
        
        
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        
        GeoStar::Slice scalingSlice(0,0,2,2,4);
        
        GeoStar::WarpParameters warpData = warpInfo;
        GeoStar::Raster *rasNew = outRaster;
        Slice sliceOut = out;
        Slice sliceInput = in;
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + in.getDeltaX();
        long int yInMax = yIn0 + in.getDeltaY();
        long int newDeltaY, newDeltaX;
        newDeltaY = sliceOut.getDeltaY();
        newDeltaX = sliceOut.getDeltaX();
        long int xOut0 = sliceOut.getX0();
        long int yOut0 = sliceOut.getY0();
        long int xOutMax = xOut0 + newDeltaX;  // new
        long int yOutMax = yOut0 + newDeltaY;  // new
        
        // TileIO will now handle reading input data.
        std::vector<T> newData(newDeltaX);    // this will hold ONE ROW of the new image - output slice
        double xScaled, yScaled;
        long int oldY, oldX;
        double xDiff, yDiff;
        T pixelVal;
        
        GeoStar::Point pt;    // pt.x and pt.y are double
        double yVal, xVal;
        
        //sliceInput.setDeltaY(3);
        sliceOut.setDeltaY(1);
        
        double X, Y;
        
        //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        
        for (long int i = yOut0; i < yOutMax; i++) {
            yVal = static_cast<double>(i);
            for (long int j = xOut0; j < xOutMax; j++) {
                xVal = static_cast<double>(j);
                
                pt = warpData.GCPTransform(xVal, yVal);
                xScaled = pt.getX() + xIn0;
                
                oldX = static_cast<long> (xScaled);
                
                xDiff = xScaled - static_cast<long>(oldX);
                getNearest(oldX, xDiff);

                if (oldX < xIn0) {
                    newData[j] = 0;
                    continue;
                } else if (oldX >= xInMax) {
                    newData[j] = 0;
                    continue;
                }

                yScaled = pt.getY() + yIn0;
                
                oldY = static_cast<long> (yScaled);
                
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
            rasNew->write(sliceOut,newData);
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "warp execution duration: " << duration << std::endl;
        
        return rasNew;
    }
    
    
    
    // THIS version of warp just reads in the entire width of the input raster BY 3 pixels deep
    //   (WIDTH X 3)
    Raster* Raster::oldwarp(const WarpParameters warpInfo, const Slice &in, const Slice &out, Raster *outRaster) {
        GeoStar::WarpParameters warpData = warpInfo;
        GeoStar::Raster *rasNew = outRaster;
        Slice sliceOut = out;
        Slice sliceInput = in;
        long int y0 = sliceInput.getY0();
        long int oldDeltaY, oldDeltaX;
        oldDeltaY = sliceInput.getDeltaY();
        oldDeltaX = sliceInput.getDeltaX();
        long int newDeltaY, newDeltaX;
        newDeltaY = sliceOut.getDeltaY();
        newDeltaX = sliceOut.getDeltaX();
        long int xOut0 = sliceOut.getX0();
        long int yOut0 = sliceOut.getY0();

        std::vector<double> data(10000);             // Old data;  this will hold the input slice
        std::vector<double> newData(newDeltaX);      // New data;  this will hold ONE ROW of the new image/output slice
        
        double xScaled, yScaled;
        long int oldY, oldX;
        double xDiff, yDiff;
        double pixelVal;
        
        GeoStar::Point pt;    // pt.x and pt.y are double
        double yVal, xVal;
        
        sliceInput.setDeltaY(3);
        sliceOut.setDeltaY(1);
        
        for (long int i = yOut0; i < newDeltaY; i++) {
            xVal = static_cast<double>(xOut0);
            yVal = static_cast<double>(i);
            pt = warpData.GCPTransform(xVal, yVal);
            yScaled = pt.getY();
            oldY = static_cast<int> (yScaled);
            
            if ((oldY - 1) < y0)
                sliceInput.setY0(y0);
            else if ((oldY - 1) > (y0 + oldDeltaY - 2))
                sliceInput.setY0(y0 + oldDeltaY - 2);
            else
                sliceInput.setY0(oldY - 1);
            
            // Verify our currently read-in slice includes the necessary pixels for doing this scale;
            //  re-read in as necessary
            // verifySliceOK(r,c,xDiff,yDiff);
            try {
                this->read(sliceInput,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
            
            yDiff = yScaled - static_cast<double>(oldY);
            getNearest(oldY, yDiff);
            
            for (long int j = xOut0; j < newDeltaX; j++) {
                xVal = static_cast<double>(j);
                pt = warpData.GCPTransform(xVal, yVal);
                xScaled = pt.getX();
                oldX = static_cast<int> (xScaled);
                xDiff = xScaled - static_cast<double>(oldX);
                getNearest(oldX, xDiff);
                pixelVal = getScaledPixel(oldY, oldX, xDiff, yDiff, sliceInput, &data[0]);
                newData[j] = pixelVal;
            }
            // Current row is scaled, write it to the output raster
            rasNew->write(sliceOut,newData);
            sliceOut.setY0(i);
        }

        return rasNew;
    }

    
    template Raster* Raster::warpType<uint8_t>(const WarpParameters, const Slice&, const Slice&, Raster*);
    template Raster* Raster::warpType<float>(const WarpParameters, const Slice&, const Slice&, Raster*);
    template Raster* Raster::warpType<double>(const WarpParameters, const Slice&, const Slice&, Raster*);

}// end namespace GeoStar
