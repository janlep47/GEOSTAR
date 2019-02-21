// Raster_bitmap.cpp
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
#include "RasterFunction.hpp"

namespace GeoStar {

    long int Raster::countBitmapOnPixels() {
        Slice in(0,0,get_nx(),get_ny());
        return countBitmapOnPixels(in);
    }

    long int Raster::countBitmapOnPixels(Slice in) {
        //if (!isBitmap) {
        //    throw_RasterNotABitmapError(fullRastername);
        //}
        
        // Now, call the templated flipType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
                return countBitmapOnPixelsType<uint8_t>(in, true);
                /*
                 case INT8S:
                 return countBitmapOnPixelsType<int8_t>(in, false);
                 case INT16U:
                 return countBitmapOnPixelsType<uint16_t>(in, true);
                 case INT16S:
                 return countBitmapOnPixelsType<int16_t>(in, false);
                 case INT32U:
                 return countBitmapOnPixelsType<uint32_t>(in, true);
                 case INT32S:
                 return countBitmapOnPixelsType<int32_t>(in, false);
                 case INT64U:
                 return countBitmapOnPixelsType<uint64_t>(in, true);
                 case INT64S:
                 return countBitmapOnPixelsType<int64_t>(in, false);
                 */
            case REAL32:
                return countBitmapOnPixelsType<float>(in, false);
                
            case REAL64:
                return countBitmapOnPixelsType<double>(in, false);
                
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        return 0;
    }
    
    
    template <typename T>
    long int Raster::countBitmapOnPixelsType(Slice in, bool isUnsigned) {
        // NOTE this new bitmap raster will have the same size as this raster!
        /*
        long int nx = get_nx();
        long int ny = get_ny();
        long int xIn0 = 0;   //in.getX0();   ... for Slice in;
        long int yIn0 = 0;   //in.getY0();
        long int xInMax = nx - 1;       //xIn0 + in.getDeltaX();
        long int yInMax = ny - 1;       //yIn0 + in.getDeltaY();
        
        Slice readSlice(0,0,nx,1);
         */
        long int nx = in.getDeltaX();
        long int ny = in.getDeltaY();
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + nx;
        long int yInMax = yIn0 + ny;
        
        std::vector<T> data(nx);             // Old data;  this will hold the input slice
        T bitval;
        long int countOn = 0;
        in.setDeltaY(1);
        
        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->read(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
            if (isUnsigned) {
                for (int j = 0; j < nx; j++) {
                    bitval = data[j];
                    if (bitval != 0) countOn++;
                }
            } else {
                for (int j = 0; j < nx; j++) {
                    bitval = data[j];
                    if (bitval > 0) countOn++;
                }
            }
        }
        return countOn;
    }
    
    
    
    //H5::PredType::NATIVE_HBOOL   -->GeoStar::BOOL
    //
    // IMPORTANT NOTE:  converting a vector to a pointer to data (used by Raster::read()/write()),
    //   (theVector<type> to &theVector[0])    DOES NOT WORK FOR vector<bool>  !!!!!!
    //  should never do vector<bool> anyway, but instead use this:
    //   std::deque<bool> boolVec  --> this works for passing as a (void *)&buffer[0] to rasterobj->write()
    //
    // BUT we are no longer using BOOL types, but instead will use unsigned 8 bit, or INT8U
    //
    
    Raster* Raster::createBitmap(const std::string &newRasterName, RasterFunction *bitmapFn) {
        
        // Create the new Raster, to hold the resulting bitmap, using the same size as this raster,
        //  with a type of unsigned int 8, because bit types NOT actually written to HDF5 files:
        RasterType type = INT8U;
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(newRasterName, type, get_nx(), get_ny());
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(newRasterName);
        }
        return createBitmap(rasNew, bitmapFn);
        return NULL;
    }
    
    
    Raster* Raster::createBitmap(Raster *rasNew, RasterFunction *bitmapFn) {
        rasNew->isBitmap = true;
        // Verify the new raster's dimensions are ok for this bitmap operation:
        //    .... do similar to rotate() ....
            
        
        // Now, call the templated flipType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
                return bitmapType<uint8_t>(rasNew, bitmapFn);
                /*
                    case INT8S:
                    return bitmapType<int8_t>(rasNew, bitmapFn);
                    case INT16U:
                    return bitmapType<uint16_t>(rasNew, bitmapFn);
                    case INT16S:
                    return bitmapType<int16_t>(rasNew, bitmapFn);
                    case INT32U:
                    return bitmapType<uint32_t>(rasNew, bitmapFn);
                    case INT32S:
                    return bitmapType<int32_t>(rasNew, bitmapFn);
                    case INT64U:
                    return bitmapType<uint64_t>(rasNew, bitmapFn);
                    case INT64S:
                    return bitmapType<int64_t>(rasNew, bitmapFn);
                    */
            case REAL32:
                return bitmapType<float>(rasNew, bitmapFn);
                    
            case REAL64:
                return bitmapType<double>(rasNew, bitmapFn);

                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        return NULL;
    }

    
    
    
    template <typename T>
    Raster* Raster::bitmapType(Raster *outRaster, RasterFunction *bitmapFn) {
        
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        GeoStar::Slice tileDescriptor(0,0,8,8);
        int maxNumberSlicesInRam = 70;
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        
        GeoStar::Slice scalingSlice(0,0,2,2,4);
        
        GeoStar::Raster *rasNew = outRaster;
        
        // NOTE this new bitmap raster will have the same size as this raster!
        long int nx = get_nx();
        long int ny = get_ny();
        long int xIn0 = 0;   //in.getX0();   ... for Slice in;
        long int yIn0 = 0;   //in.getY0();
        long int xInMax = nx - 1;       //xIn0 + in.getDeltaX();
        long int yInMax = ny - 1;       //yIn0 + in.getDeltaY();
        
        long int xOut0 = 0;                  //sliceOut.getX0();
        long int yOut0 = 0;                  //sliceOut.getY0();
        long int xOutMax = nx - 1;        //xIn0 + in.getDeltaX();
        long int yOutMax = ny - 1;        //yIn0 + in.getDeltaY();
        //std::deque<bool> newData(nx);    // this will hold ONE ROW of the new image - output slice
        std::vector<uint8_t> newData(nx);    // this will hold ONE ROW of the new image - output slice
        long int oldY, oldX;
        T pixelVal;
        
        GeoStar::Point pt;    // pt.x and pt.y are double
        double yVal, xVal;
        
        // MAY want to pass in sliceInput/sliceOut LATER ... and use as in above comments.
        Slice sliceInput(0,0,nx,ny);
        Slice sliceOut(0,0,nx,ny);
        //sliceInput.setDeltaY(3);
        sliceOut.setDeltaY(1);
        
        for (long int i = yOut0; i < yOutMax; i++) {
            oldY = i;
            
            for (long int j = xOut0; j < xOutMax; j++) {
                oldX = j;
                pixelVal = getScaledPixelFromTile<T>(oldY, oldX, 0.0, 0.0, sliceInput, scalingSlice, reader);
                newData[j] = (*bitmapFn)(pixelVal);
            }
            sliceOut.setY0(i);
            rasNew->write(sliceOut,newData);
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "transform execution duration: " << duration << std::endl;
        
        return rasNew;
    }

    
    Raster* Raster::copyUnderBitmap(Raster *bitmap, const std::string &newRasterName) {
        
        // Create the new Raster, to hold the resulting bitmap, using the same size as this raster,
        //  with a type of unsigned int 8, because bit types NOT actually written to HDF5 files:
        RasterType type = raster_datatype;
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(newRasterName, type, get_nx(), get_ny());
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(newRasterName);
            // IF THIS raster doesn't have a BOOL type, throw an ERROR!!!!
            
            
        }
        return copyUnderBitmap(bitmap, rasNew);
        return NULL;
    }

    
    
    Raster* Raster::copyUnderBitmap(Raster *bitmap, Raster *rasNew) {
        // Verify the new raster's dimensions are ok for this bitmap operation:
        //    .... do similar to rotate() ....
        
        
        // Now, call the templated flipType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
            return copyUnderBitmapType<uint8_t>(bitmap, rasNew);
            /*
             case INT8S:
             return bitmapType<int8_t>(rasNew);
             case INT16U:
             return bitmapType<uint16_t>(rasNew);
             case INT16S:
             return bitmapType<int16_t>(rasNew);
             case INT32U:
             return bitmapType<uint32_t>(rasNew);
             case INT32S:
             return bitmapType<int32_t>(rasNew);
             case INT64U:
             return bitmapType<uint64_t>(rasNew);
             case INT64S:
             return bitmapType<int64_t>(rasNew);
             */
            case REAL32:
            return copyUnderBitmapType<float>(bitmap, rasNew);
            
            case REAL64:
            return copyUnderBitmapType<double>(bitmap, rasNew);
            
            //case COMPLEX_INT16:
            //    return
            default:
            throw_RasterUnsupportedTypeError(fullRastername);
        }
        return NULL;
    }
    
    
    template <typename T>
    Raster* Raster::copyUnderBitmapType(Raster *bitmap, Raster *outRaster) {
        
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        GeoStar::Slice tileDescriptor(0,0,8,8);
        int maxNumberSlicesInRam = 70;
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        GeoStar::TileIO<uint8_t> readerBitMap(bitmap, tileDescriptor, maxNumberSlicesInRam);
        GeoStar::TileIO<T> readerDest(outRaster, tileDescriptor, maxNumberSlicesInRam);

        GeoStar::Slice scalingSlice(0,0,2,2,4);
        
        GeoStar::Raster *rasNew = outRaster;
        
        // NOTE this new bitmap raster will have the same size as this raster!
        long int nx = get_nx();
        long int ny = get_ny();
        long int xIn0 = 0;   //in.getX0();   ... for Slice in;
        long int yIn0 = 0;   //in.getY0();
        long int xInMax = nx - 1;       //xIn0 + in.getDeltaX();
        long int yInMax = ny - 1;       //yIn0 + in.getDeltaY();
        
        long int xOut0 = 0;                  //sliceOut.getX0();
        long int yOut0 = 0;                  //sliceOut.getY0();
        long int xOutMax = nx - 1;        //xIn0 + in.getDeltaX();
        long int yOutMax = ny - 1;        //yIn0 + in.getDeltaY();
        //std::deque<bool> newData(nx);    // this will hold ONE ROW of the new image - output slice
        std::vector<T> newData(nx);    // this will hold ONE ROW of the new image - output slice
        long int oldY, oldX;
        T srcPixelVal;
        uint8_t bitmapVal;
        T destPixelVal;
        
        GeoStar::Point pt;    // pt.x and pt.y are double
        double yVal, xVal;
        
        // MAY want to pass in sliceInput/sliceOut LATER ... and use as in above comments.
        Slice sliceInput(0,0,nx,ny);
        Slice sliceOut(0,0,nx,ny);
        //sliceInput.setDeltaY(3);
        sliceOut.setDeltaY(1);
        
        
        for (long int i = yOut0; i < yOutMax; i++) {
            oldY = i;
            
            for (long int j = xOut0; j < xOutMax; j++) {
                oldX = j;
                srcPixelVal = getScaledPixelFromTile<T>(oldY, oldX, 0.0, 0.0, sliceInput, scalingSlice,
                                                        reader);
                bitmapVal = getScaledPixelFromTile<uint8_t>(oldY,oldX,0.0,0.0,sliceInput,
                                                               scalingSlice,readerBitMap);
                destPixelVal = getScaledPixelFromTile<T>(oldY,oldX,0.0,0.0,sliceInput,
                                                                 scalingSlice,readerDest);
                // the bitmap is '1' replace the destination pixel value with this raster's pixel value:
                if (bitmapVal == 1) newData[j] = srcPixelVal;
                else newData[j] = destPixelVal;
            }
            sliceOut.setY0(i);
            rasNew->write(sliceOut,newData);
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "transform execution duration: " << duration << std::endl;
        
        return rasNew;
    }

    template long int Raster::countBitmapOnPixelsType<uint8_t>(Slice, bool);
    template long int Raster::countBitmapOnPixelsType<float>(Slice, bool);
    template long int Raster::countBitmapOnPixelsType<double>(Slice, bool);

    template Raster* Raster::bitmapType<uint8_t>(Raster*, RasterFunction*);
    template Raster* Raster::bitmapType<float>(Raster*, RasterFunction*);
    template Raster* Raster::bitmapType<double>(Raster*, RasterFunction*);
    
    template Raster* Raster::copyUnderBitmapType<uint8_t>(Raster*, Raster*);
    template Raster* Raster::copyUnderBitmapType<float>(Raster*, Raster*);
    template Raster* Raster::copyUnderBitmapType<double>(Raster*, Raster*);


}// end namespace GeoStar
