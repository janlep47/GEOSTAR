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
#include "RasterFunction.hpp"

namespace GeoStar {

    uint8_t Raster::minPixel() {
        Slice in(0,0,get_nx(),get_ny());
        return minPixel(in);
    }

    uint8_t Raster::minPixel(Slice in) {
        // Now, call the templated flipType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
                return minPixelType<uint8_t>(in, true);
                /*
                 case INT8S:
                 return minPixelType<int8_t>(in, false);
                 case INT16U:
                 return minPixelType<uint16_t>(in, true);
                 case INT16S:
                 return minPixelType<int16_t>(in, false);
                 case INT32U:
                 return minPixelType<uint32_t>(in, true);
                 case INT32S:
                 return minPixelType<int32_t>(in, false);
                 case INT64U:
                 return minPixelType<uint64_t>(in, true);
                 case INT64S:
                 return minPixelType<int64_t>(in, false);
                 */
            case REAL32:
                return minPixelType<float>(in, false);
                
            case REAL64:
                return minPixelType<double>(in, false);
                
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        return 0;
    }
    

    uint8_t Raster::maxPixel() {
        Slice in(0,0,get_nx(),get_ny());
        return maxPixel(in);
    }
    
    uint8_t Raster::maxPixel(Slice in) {
        // Now, call the templated flipType<>() method, based on the new raster type:
        switch(raster_datatype) {
            case INT8U:
                return maxPixelType<uint8_t>(in, true);
                /*
                 case INT8S:
                 return maxPixelType<int8_t>(in, false);
                 case INT16U:
                 return maxPixelType<uint16_t>(in, true);
                 case INT16S:
                 return maxPixelType<int16_t>(in, false);
                 case INT32U:
                 return maxPixelType<uint32_t>(in, true);
                 case INT32S:
                 return maxPixelType<int32_t>(in, false);
                 case INT64U:
                 return maxPixelType<uint64_t>(in, true);
                 case INT64S:
                 return maxPixelType<int64_t>(in, false);
                 */
            case REAL32:
                return maxPixelType<float>(in, false);
                
            case REAL64:
                return maxPixelType<double>(in, false);
                
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        return 0;
    }
    

    template <typename T>
    uint8_t Raster::minPixelType(Slice in, bool isUnsigned) {
        long int nx = in.getDeltaX();
        long int ny = in.getDeltaY();
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + nx;
        long int yInMax = yIn0 + ny;
        
        std::vector<T> data(nx);             // Old data;  this will hold the input slice
        T bitval;
        uint8_t minPixelVal = 255;  // max unsigned 8 bit value;
        in.setDeltaY(1);
        
        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->read(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
            for (int j = 0; j < nx; j++) {
                if (minPixelVal > data[j])
                    minPixelVal = data[j];
            }
            /*
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
             */
        }
        return minPixelVal;
    }
    
    
    template <typename T>
    uint8_t Raster::maxPixelType(Slice in, bool isUnsigned) {
        long int nx = in.getDeltaX();
        long int ny = in.getDeltaY();
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + nx;
        long int yInMax = yIn0 + ny;
        
        std::vector<T> data(nx);             // Old data;  this will hold the input slice
        T bitval;
        uint8_t maxPixelVal = 0;  // min unsigned 8 bit value;
        in.setDeltaY(1);
        
        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->read(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
            for (int j = 0; j < nx; j++) {
                if (maxPixelVal < data[j])
                    maxPixelVal = data[j];
            }
            /*
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
             */
        }
        return maxPixelVal;
    }


    template uint8_t Raster::minPixelType<uint8_t>(Slice, bool);
    template uint8_t Raster::minPixelType<float>(Slice, bool);
    template uint8_t Raster::minPixelType<double>(Slice, bool);

    template uint8_t Raster::maxPixelType<uint8_t>(Slice, bool);
    template uint8_t Raster::maxPixelType<float>(Slice, bool);
    template uint8_t Raster::maxPixelType<double>(Slice, bool);

}// end namespace GeoStar
