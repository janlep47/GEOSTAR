// Raster.cpp
//
// by Leland Pierce, Feb 18, 2017
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "RasterScaler.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {

  Raster::Raster(Image *image, const std::string &name){
    RasterOpenErrorException RasterOpenError;
    RasterDoesNotExistException RasterDoesNotExist;

    if(!image->datasetExists(name)) throw RasterDoesNotExist;

    // check if its a valid Raster:
    rasterobj = new H5::DataSet(image->openDataset(name));
    if(read_object_type() != "geostar::raster") {
      delete rasterobj;
      throw RasterOpenError;
    }//endif

    // finish setting object-specific data:
    rastername = name;
    rastertype = "geostar::raster";
    
  }// end-Raster-constructor




  Raster::Raster(Image *image, const std::string &name, const RasterType &type,
           const int &nx, const int &ny){

    RasterCreationErrorException RasterCreationError;
    RasterExistsException RasterExistsError;

    if(image->datasetExists(name)) throw RasterExistsError;

    // create a 2D dataset
    hsize_t dims[2];
    dims[0] = ny; 
    dims[1] = nx; 
    H5::DataSpace dataspace(2, dims);

    switch(type) {
    case INT8U:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_UINT8, dataspace));
      break;
    case INT16U:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_UINT16, dataspace));
      break;
    case REAL32:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_FLOAT, dataspace));
      break;
    case INT32S:
        rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_INT32, dataspace));
        break;
    default:
      throw RasterCreationError;
    }// end case

    rastername = name;
    raster_datatype=type;
    rastertype = "geostar::raster";
    
    // set objtype attribute.
    write_object_type(rastertype);

  }// end-Raster-constructor

  

  //returns the actual size of the raster in the x-direction
  long int Raster::get_nx() const {
    H5::DataSpace space=rasterobj->getSpace();
    hsize_t dims[2];
    space.getSimpleExtentDims(dims);
    return dims[1];
  }// end: get_nx

  //returns the actual size of the raster in the y-direction
  long int Raster::get_ny() const {
    H5::DataSpace space=rasterobj->getSpace();
    hsize_t dims[2];
    space.getSimpleExtentDims(dims);
    return dims[0];
  }// end: get_ny

    
    
    
  // in-place simple threshhold
  // < value : set to 0.
  void Raster::thresh(const double &value) {
    long int nx = get_nx();
    long int ny = get_ny();

    vector<long int>slice(4);
    slice[0]=0;
    slice[1]=0;
    slice[2]=nx;
    slice[3]=1;

    std::vector<uint8_t> data(nx);

    for(int line=0;line<ny;++line) {
      slice[1]=line;
      read(slice,data);
      for(int pixel=0; pixel<nx;++pixel) {
        if(data[pixel] < value) data[pixel]=0;
      }// endfor: pixel
      write(slice,data);
    }// endfor: line

  }// end: thresh
    

    
    void Raster::scale(GeoStar::Raster *rasNew, const std::string &name, const RasterType &type,
                          const int &nx, const int &ny) {
        
        RasterUnsupportedTypeException RasterUnsupportedTypeError;
        
        switch(type){
            case INT8U:
            {
                RasterScaler<uint8_t> *scalerUint8 = new RasterScaler<uint8_t>(this);
                scalerUint8->scale(rasNew, name, type, nx, ny);
            }
                return;
            case INT16U:
            {
                RasterScaler<uint16_t> *scalerUint16 = new RasterScaler<uint16_t>(this);
                scalerUint16->scale(rasNew, name, type, nx, ny);
            }
                return;
            case REAL32:
            {
                RasterScaler<float> *scalerReal32 = new RasterScaler<float>(this);
                scalerReal32->scale(rasNew, name, type, nx, ny);
            }
                return;
            default:
                throw RasterUnsupportedTypeError;
        }//end-switch
        
    }
    
    template <>  H5::PredType Raster::getHdf5Type<uint8_t>() {return H5::PredType::NATIVE_UINT8;}
    template <>  H5::PredType Raster::getHdf5Type<int8_t>() {return H5::PredType::NATIVE_INT8;}
    template <>  H5::PredType Raster::getHdf5Type<uint16_t>() {return H5::PredType::NATIVE_UINT16;}
    template <>  H5::PredType Raster::getHdf5Type<int16_t>() {return H5::PredType::NATIVE_INT16;}
    template <>  H5::PredType Raster::getHdf5Type<uint32_t>() {return H5::PredType::NATIVE_UINT32;}
    template <>  H5::PredType Raster::getHdf5Type<int32_t>() {return H5::PredType::NATIVE_INT32;}
    template <>  H5::PredType Raster::getHdf5Type<uint64_t>() {return H5::PredType::NATIVE_UINT64;}
    template <>  H5::PredType Raster::getHdf5Type<int64_t>() {return H5::PredType::NATIVE_INT64;}
    template <>  H5::PredType Raster::getHdf5Type<float>() {return H5::PredType::NATIVE_FLOAT;}
    template <>  H5::PredType Raster::getHdf5Type<double>() {return H5::PredType::NATIVE_DOUBLE;}
    
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_INT16>() {return H5::PredType::NATIVE_INT16;}
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_INT32>() {return H5::PredType::NATIVE_INT32;}
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_INT64>() {return H5::PredType::NATIVE_INT64;}
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_INT128>() {return H5::PredType::NATIVE_INT64;}
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_REAL64>() {return H5::PredType::NATIVE_DOUBLE;}
    //template <>  H5::PredType Raster::getHdf5Type<COMPLEX_REAL128>() {return H5::PredType::NATIVE_LDOUBLE;}


}// end namespace GeoStar
