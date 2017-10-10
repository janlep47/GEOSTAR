// Image.cpp
//
// by Leland Pierce, Feb 18, 2017
// 
// revised March 11, 2017: added: read_tiff_1_8U()
//
//--------------------------------------------


#include <string>
#include <iostream>
#include "H5Cpp.h"

#include "File.hpp"
#include "Image.hpp"
#include "Raster.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

//extern "C" {
//#include "tiff.h"
//#include "tiff_io.h"
//#include "string.h"
//}
#include <cstdlib>


#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()


namespace GeoStar {

  Image::Image(File *file, const std::string &name){
    ImageOpenErrorException ImageOpenError;

    // opens Image if it exists, creates if doesn't

    if(file->groupExists(name)){
      // check validity of exisiting object:
      imageobj = new H5::Group( file->openGroup( name ));
      // check for valid objtype attribute.
      if(read_object_type() != "geostar::image") {
        delete imageobj;
        throw ImageOpenError;
      }// endif
      // finish setting object-specific data:
      imagename = name;
      imagetype = "geostar::image";

    } else {
      // create a new Image group:
      imageobj = new H5::Group( file->createGroup( name ));
      imagename = name;
      imagetype = "geostar::image";
      // set objtype attribute.
      write_object_type(imagetype);
    }// endif

  }// end-Image-constructor





  bool Image::datasetExists(const std::string &name) {
    try{
      H5::Exception::dontPrint();
      H5::DataSet obj = imageobj->openDataSet( name );
      return true;
    }
    catch (...) {
      return false;
    }
  }// end: datasetExists


  Raster *Image::read_file(const std::string &infile, const std::string &name){
    // 1. open the data file
    // 2. figure out data characteristics
    // 3. create empty raster
    // 4. fill raster.
    FileOpenErrorException FileOpenError;

    // 1. open the data file
    GDALDataset *poDataset;
    GDALAllRegister();
    char cname[infile.length()+10]; // need a copy because it's declared const
    strcpy(cname, infile.c_str());
    poDataset = (GDALDataset *) GDALOpen(cname, GA_ReadOnly );
    if( poDataset == NULL ) throw FileOpenError;

    // 2. figure out data characteristics
    GDALRasterBand *poBand;
    poBand = poDataset->GetRasterBand( 1 ); // the first channel.
    int nx = poBand->GetXSize();
    int ny = poBand->GetYSize();

    // 3. create empty raster
    Raster *ras = create_raster(name,GeoStar::REAL32,nx,ny);

    // 4. fill raster:
    std::vector<long int> slice(4);
    slice[0]=0;  slice[1]=0;
    slice[2]=nx; slice[3]=1;

    vector<float> buf(nx);

    for(int iy=0;iy<ny;++iy){
      CPLErr err = poBand->RasterIO( GF_Read, 
                    0, iy, nx, 1,
                    &buf[0], nx, 1, GDT_Float32,   // &buf[0] for parameter void *
                    0, 0 );
      slice[1]=iy;
      ras->write(slice,buf);
    } // endfor

    //CPLFree(buf);
    GDALClose(poDataset);

    // 5. return the new raster object
    return ras;

  }// end-function: read_file





}// end namespace GeoStar
