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

    template <typename T>
    void Raster::setPixels(T pixelVal) {
        Slice in(0,0,get_nx(),get_ny());
        setPixels(in,pixelVal);
    }
    

    template <typename T>
    void Raster::setPixels(Slice in, T pixelVal) {
        
        // First verify that 'pixelVal' is the correct type for this raster:
        std::string givenType = typeid(pixelVal).name();
        std::string correctType;
        switch(raster_datatype) {
            case INT8U:
                uint8_t uint8_tVar;
                correctType = typeid(uint8_tVar).name();
                break;
                /*
                 case INT8S:
                 case INT16U:
                 case INT16S:
                 case INT32U:
                 case INT32S:
                 case INT64U:
                 case INT64S:
                 */
            case REAL32:
                float floatVar;
                correctType = typeid(floatVar).name();
                break;

            case REAL64:
                double doubleVar;
                correctType = typeid(doubleVar).name();
                break;

                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
        if (givenType.compare(correctType) != 0)
            throw_InvalidPixelValueType(fullRastername+"type given: "+givenType+
                                        "required type: "+correctType);

        
        long int nx = in.getDeltaX();
        long int ny = in.getDeltaY();
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + nx;
        long int yInMax = yIn0 + ny;
        
        std::vector<T> data(nx);             // new data;  this will hold the output slice
        in.setDeltaY(1);
        for (int j = 0; j < nx; j++)
            data[j] = pixelVal;

        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->write(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
        }
        return;
    }
    
    

    template void Raster::setPixels<uint8_t>(uint8_t);
    template void Raster::setPixels<float>(float);
    template void Raster::setPixels<double>(double);

    template void Raster::setPixels<uint8_t>(Slice, uint8_t);
    template void Raster::setPixels<float>(Slice, float);
    template void Raster::setPixels<double>(Slice, double);

}// end namespace GeoStar
