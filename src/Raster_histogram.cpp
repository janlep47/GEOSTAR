// Raster_histogram.cpp
//
// by Janice Richards, Mar 2, 2018
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
    std::vector<int> Raster::histogram(std::vector<T> binValues) {
        Slice in(0,0,get_nx(),get_ny());
        return histogram(in,binValues);
    }
    
    
    template <typename T>
    std::vector<int> Raster::histogram(Slice in, std::vector<T> binValues) {
        // verify binValues not empty:
        
        
        // First verify that 'pixelVal' is the correct type for this raster:
        std::string givenType = typeid(binValues[0]).name();
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
        T pixelVal;
        bool binFound;
        
        std::vector<int> bins(binValues.size() + 1);
        int maxBinIndex = binValues.size();
        // initial output bins:
        for (int i = 0; i < bins.size(); i++)
            bins[i] = 0;
        
        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->read(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
            for (int j = 0; j < nx; j++) {
                pixelVal = data[j];
                for (int k = 0; k < binValues.size(); k++) {
                    binFound = false;
                    if (pixelVal <= binValues[k]) {
                        bins[k]++;
                        binFound = true;
                        break;
                    }
                }
                if (!binFound) bins[maxBinIndex]++;
            }
        }
        return bins;
    }
    
    
    
    template std::vector<int> Raster::histogram<uint8_t>(std::vector<uint8_t>);
    template std::vector<int> Raster::histogram<float>(std::vector<float>);
    template std::vector<int> Raster::histogram<double>(std::vector<double>);
    
    template std::vector<int> Raster::histogram<uint8_t>(Slice, std::vector<uint8_t>);
    template std::vector<int> Raster::histogram<float>(Slice, std::vector<float>);
    template std::vector<int> Raster::histogram<double>(Slice, std::vector<double>);

}// end namespace GeoStar
