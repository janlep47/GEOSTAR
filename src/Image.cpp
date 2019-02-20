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

    Image::Image(File *file, const std::string &name) {
        //ImageOpenErrorException ImageOpenError;

    // opens Image if it exists, creates if doesn't

        if(file->groupExists(name)){
            // check validity of exisiting object:
            imageobj = new H5::Group( file->openGroup( name ));
            // check for valid objtype attribute.
            if(read_object_type() != "geostar::image") {
                delete imageobj;
                throw_ImageOpenError(name);
            }// endif
            // finish setting object-specific data:
            imagetype = "geostar::image";

        } else {
            // create a new Image group:
            imageobj = new H5::Group( file->createGroup( name ));
            imagetype = "geostar::image";
            // set objtype attribute.
            write_object_type(imagetype);
        }// endif
        imagename = name;
        fullImagename = imageobj->getFileName()+"/"+name;
        ownerFile = file;

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
    
    
    int Image::getNumberOfChannels(const std::string &infile) {
        //FileOpenErrorException FileOpenError;
        
        GDALDataset *poDataset;
        GDALAllRegister();
        char cname[infile.length()+10]; // need a copy because it's declared const
        strcpy(cname, infile.c_str());
        poDataset = (GDALDataset *) GDALOpen(cname, GA_ReadOnly );
        if( poDataset == NULL ) throw_FileOpenError(infile);
        
        int numBands = poDataset->GetRasterCount();
        GDALClose(poDataset);
        return numBands;
    }
    
    
    std::vector<Raster *> Image::read_file(const std::string &infile) {
        //FileOpenErrorException FileOpenError;
        
        GDALDataset *poDataset;
        GDALAllRegister();
        char cname[infile.length()+10]; // need a copy because it's declared const
        strcpy(cname, infile.c_str());
        poDataset = (GDALDataset *) GDALOpen(cname, GA_ReadOnly );
        if( poDataset == NULL ) throw_FileOpenError(infile);
        
        int numBands = poDataset->GetRasterCount();
        GDALClose(poDataset);
        
        std::vector<std::string> rasterNames(numBands);
        for (int i = 0; i < numBands; i++)
            rasterNames[i] = "channel"+std::to_string(i+1);
        return read_file(infile, rasterNames);
    }
        
        
    std::vector<Raster *> Image::read_file(const std::string &infile, std::vector<std::string> &names) {
        // 1. open the data file
        // 2. figure out data characteristics
        // 3. create empty raster
        // 4. fill raster.
        //FileOpenErrorException FileOpenError;

        // 1. open the data file
        GDALDataset *poDataset;
        GDALAllRegister();
        char cname[infile.length()+10]; // need a copy because it's declared const
        strcpy(cname, infile.c_str());
        poDataset = (GDALDataset *) GDALOpen(cname, GA_ReadOnly );
        if( poDataset == NULL ) throw_FileOpenError(infile);
      
      
        int numBands = poDataset->GetRasterCount();
        std::vector<std::string> rasterNames(numBands);
        std::vector<Raster *> rasters(numBands);
      
        if (names.empty()) {
            for (int i = 0; i < numBands; i++)
                rasterNames[i] = "channel"+std::to_string(i+1);
        } else {
            int namesSize = names.size();
            for (int i = 0; i < numBands; i++) {
                if (i < namesSize) {
                    rasterNames[i] = names[i];
                } else {
                rasterNames[i] = "channel" + std::to_string(i+1);
                }
            }
        }
      
        GDALRasterBand *poBand;
        for (int i = 0; i < numBands; i++) {
        
            // 2. figure out data characteristics
            poBand = poDataset->GetRasterBand( i+1 ); // the ith channel.
            int nx = poBand->GetXSize();
            int ny = poBand->GetYSize();

            // Get raster type:
            GDALDataType typ = poBand->GetRasterDataType();
            GeoStar::RasterType rasterTyp = getGeoStarType(typ);
      
            // 3. create empty raster
            Raster *ras = create_raster(rasterNames[i],rasterTyp,nx,ny);


            // 4. fill raster:
            switch(rasterTyp) {
                case INT8U:
                    fill_raster<uint8_t>(ras, poBand, nx, ny, typ);
                    break;
                    /*
                     case INT8S:
                     fill_raster<int8_t>(ras, poBand, nx, ny, typ);
                     case INT16U:
                     fill_raster<uint16_t>(ras, poBand, nx, ny, typ);
                     case INT16S:
                     fill_raster<int16_t>(ras, poBand, nx, ny, typ);
                     case INT32U:
                     fill_raster<uint32_t>(ras, poBand, nx, ny, typ);
                     case INT32S:
                     fill_raster<int32_t>(ras, poBand, nx, ny, typ);
                     case INT64U:
                     fill_raster<uint64_t>(ras, poBand, nx, ny, typ);
                     case INT64S:
                     fill_raster<int64_t>(ras, poBand, nx, ny, typ);
                     */
                case REAL32:
                    fill_raster<float>(ras, poBand, nx, ny, typ);
                    break;
                case REAL64:
                    fill_raster<double>(ras, poBand, nx, ny, typ);
                    break;
                    //case COMPLEX_INT16:
                    //    return ...
                default:
                    //throw RasterUnsupportedTypeError;
                    fill_raster<double>(ras, poBand, nx, ny, typ);
            }
            rasters[i] = ras;
        }  // end for
          
        GDALClose(poDataset);

        // 5. return the new raster objects
        return rasters;

    }// end-function: read_file


    // write file data to internal raster data
    template <typename T>
    void Image::fill_raster(Raster *ras, GDALRasterBand *poBand, const int nx,
                            const int ny, const GDALDataType typ) {
        Slice slice(0,0,nx,1);
        std::vector<T> buf(nx);

        for(int iy=0;iy<ny;++iy){
            // read gdal raster band data for writing to internal raster data
            CPLErr err = poBand->RasterIO( GF_Read,
                                          0, iy, nx, 1,
                                          &buf[0], nx, 1, typ,   // &buf[0] for parameter void *
                                          0, 0 );
            slice.setY0(iy);
            ras->write(slice,buf);
        } // endfor
        return;
    }

    
    
    
    // write internal raster data to gdal raster band, for output to file.
    template <typename T>
    void Image::copy_raster(Raster *ras, GDALRasterBand *poBand, const int nx,
                            const int ny, const GDALDataType typ) {
        Slice slice(0,0,nx,1);
        std::vector<T> buf(nx);
        
        for(int iy=0;iy<ny;++iy){
            slice.setY0(iy);
            ras->read(slice, buf);
            // write internal raster buffer to the gdal raster band.
            CPLErr err = poBand->RasterIO( GF_Write,
                                          0, iy, nx, 1,
                                          &buf[0], nx, 1, typ,   // &buf[0] for parameter void *
                                          0, 0 );
        } // endfor
        return;
    }
    
    
    
    
    void Image::write_file(const std::string &outfile, const std::string &format, std::vector<std::string> &channels) {
        //FileOpenErrorException FileOpenError;
        GDALAllRegister();
     
        // 1.
        GDALDriver *poDriver, *poDriverMem;
        char **papszMetadata;
     
        char cformat[format.length()+10];
        strcpy(cformat, format.c_str());
        
        poDriverMem = GetGDALDriverManager()->GetDriverByName("MEM");
     
     
        papszMetadata = poDriverMem->GetMetadata();
     
        GDALDataset *poDstMem, *poDstDS;
        char **papszOptions = NULL;
     
        // if channels vector is empty, assume all this image's rasters will be written
        if (channels.empty()) {
        }
     
        // Get the raster specified by the (1st) channel name:
        Raster *ras = open_raster(channels[0]);
        int nx = ras->get_nx();
        int ny = ras->get_ny();
     
        // Get the type of raster data (of the 1st raster channel specified ...) THIS IS A PROBLEM, b/c
        //  we're creating the driver, based on this raster type, BUT WHAT IF the other rasters being written
        //  ARE DIFFERENT TYPES?!   ... may have to check later, and either CONVERT or THROW AN EXCEPTION
        GDALDataType typ = getGDALType(ras);
     
        poDstMem = poDriverMem->Create( "", nx, ny, channels.size(), typ, papszOptions );

        GDALRasterBand *poBand;
     
        for (int i = 0; i < channels.size(); i++) {
     
            // test for first raster which was already opened, for checking raster type to create output driver earlier
            // NOTE that the raster channels are all assumed to be the SAME TYPE!
            if (i != 0) ras = open_raster(channels[i]);
     
            poBand = poDstMem->GetRasterBand(i+1);
     
            switch(typ) {
                case GDT_Byte:
                    copy_raster<uint8_t>(ras, poBand, nx, ny, typ);
                    break;
     
                    //case GDT_UInt16:
                    //copy_raster<uint16_t>(ras, poBand, nx, ny, typ);
                    //case GDT_Int16:
                    //copy_raster<int16_t>(ras, poBand, nx, ny, typ);
                    //case GDT_UInt32:
                    //copy_raster<uint32_t>(ras, poBand, nx, ny, typ);
                    //case GDT_Int32:
                    //copy_raster<int32_t>(ras, poBand, nx, ny, typ);
     
                case GDT_Float32:
                    copy_raster<float>(ras, poBand, nx, ny, typ);
                    break;
                case GDT_Float64:
                    copy_raster<double>(ras, poBand, nx, ny, typ);
                    break;
                    //case GDT_CInt16:
                    //    return ...COMPLEX_INT32
                    //case GDT_CInt32:
                    //    return ...COMPLEX_INT64
                    //case GDT_CFloat32:
                    //    return ...COMPLEX_REAL64
                    //case GDT_CFloat64:
                    //    return ...COMPLEX_REAL128
                default:
                    //throw RasterUnsupportedTypeError;
                    copy_raster<double>(ras, poBand, nx, ny, typ);
            }
        } // endfor
        poDriver = GetGDALDriverManager()->GetDriverByName(cformat);
        if( poDriver == NULL ) throw std::runtime_error("invalid GDAL driver");
        
        // write the previously generated gdal memory dataset to this output file
        poDstDS = poDriver->CreateCopy(outfile.c_str(), poDstMem, FALSE, papszOptions, NULL, NULL);
        
        // Once we're done, properly close the dataset
        GDALClose(poDstMem);
        GDALClose( (GDALDatasetH) poDstDS );
        return;
     }
     

    
    std::vector<std::string> Image::getChannels() {
        int numChannels = imageobj->getNumObjs();
        std::vector<std::string> channels(numChannels);
        
        int chIdx = 0;
        for (int i = 0; i < numChannels; i++) {
            std::string obj = imageobj->getObjnameByIdx(i);
            H5G_obj_t objType = imageobj->getObjTypeByIdx(i);

            if (objType == H5G_DATASET) {
                channels[chIdx] = obj;
                chIdx++;
            }
        }
        channels.resize(chIdx);
        return channels;
    }
    
    
    GDALDataType Image::getGDALType(const Raster *ras) {
        switch (ras->getRasterType()) {
            case GeoStar::INT8U:
                return GDT_Byte;
            case GeoStar::INT16U:
                return GDT_UInt16;
            case GeoStar::INT16S:
                return GDT_Int16;
            case GeoStar::INT32U:
                return GDT_UInt32;
            case GeoStar::INT32S:
                return GDT_Int32;
            case GeoStar::REAL32:
                return GDT_Float32;
            case GeoStar::REAL64:
                return GDT_Float64;
                
            // NOTE: Gdal complex size is for EACH component, GeoStar size is for COMBINED components
            case GeoStar::COMPLEX_INT32:
                return GDT_CInt16;
            case GeoStar::COMPLEX_INT64:
                return GDT_CInt32;
            case GeoStar::COMPLEX_REAL64:
                return GDT_CFloat32;
            case GeoStar::COMPLEX_REAL128:
                return GDT_CFloat64;
            default:
                return GDT_Float64;
        }
        
    }
    
    RasterType Image::getGeoStarType(const GDALDataType &type) {
        switch(type) {
            case GDT_Byte:
                return (GeoStar::INT8U);
            case GDT_UInt16:
                return (GeoStar::INT16U);
            case GDT_Int16:
                return (GeoStar::INT16S);
            case GDT_UInt32:
                return (GeoStar::INT32U);
            case GDT_Int32:
                return (GeoStar::INT32S);
            case GDT_Float32:
                return (GeoStar::REAL32);
            case GDT_Float64:
                return (GeoStar::REAL64);
                
            // NOTE: Gdal complex size is for EACH component, GeoStar size is for COMBINED components
            case GDT_CInt16:
                return (GeoStar::COMPLEX_INT32);
            case GDT_CInt32:
                return (GeoStar::COMPLEX_INT64);
            case GDT_CFloat32:
                return (GeoStar::COMPLEX_REAL64);
            case GDT_CFloat64:
                return (GeoStar::COMPLEX_REAL128);
            case GDT_Unknown:
                return (GeoStar::REAL64);   // For now ....
            default:
                return (GeoStar::REAL64);   // For now ....
                
            // GeoStar types NOT supported by gdal types:
            //  INT8S, INT64U, INT64S,
            //  COMPLEX_INT128 (would correspond to GDT_CInt64)
            //  COMPLEX_INT16 (would correspond to GDT_CInt8)
            //
        }
    }
    
    Image* Image::rotate(const float angle) {
        std::string newImgName = imagename+"ROTATE_"+std::to_string(angle);
        Image *imgNew = new Image(ownerFile, newImgName);
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            Raster *newRas = imgNew->create_raster(ras->getRasterName(), ras->getRasterType());
            ras->rotate(angle, newRas);
        }
        return imgNew;
    }
    
    Image* Image::rotate(const float angle, const Slice &inslice) {
        std::string newImageName = imagename+"ROTATE_"+std::to_string(angle);
        Image *imgNew = new Image(ownerFile, newImageName);
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            Raster *newRas = imgNew->create_raster(ras->getRasterName(), ras->getRasterType());
            ras->rotate(angle, inslice, newRas);
        }
        return imgNew;
    }
    
    Image* Image::flip(const std::string &newImageName, short flipAxis) {
        Image *imgNew = new Image(ownerFile, newImageName);
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            Raster *newRas = imgNew->create_raster(ras->getRasterName(), ras->getRasterType(), ras->get_nx(), ras->get_ny());
            ras->flip(newRas, flipAxis);
        }
        return imgNew;
    }
    
    Image* Image::transform(std::string &newWKT, std::string &newImageName, double newDeltaX, double newDeltaY) {
        Image *imgNew = new Image(ownerFile, newImageName);
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            Raster *newRas = imgNew->create_raster(ras->getRasterName(), ras->getRasterType());
            ras->transform(newWKT, newDeltaX, newDeltaY, newRas);
        }
        return imgNew;
    }
    
    
    // Copy this Image to a new Image, under the same file, with the given name.
    // All channels (Rasters) will be copied.  The reference to the new copied Image is returned.
    Image* Image::copy_to(const std::string &newImageName) {
        // Get the list of all this Image's channels (Rasters):
        std::vector<std::string> channels = getChannels();
        return copy_to(newImageName, channels);
    }
    
    
    // Copy this Image to a new Image, under the same file, with the given name.  The vector of
    // strings given, is the names of the channels of this Image, which should be copied under the
    // new Image.  The reference to the new copied Image is returned.
    Image* Image::copy_to(const std::string &newImageName, std::vector<std::string> &copyChannels) {
        // First get a list of actual channels, to verify against given list of 'copy-from' channels:
        //std::vector<std::string> channels = getChannels();  // MAY NOT NEED TO DO
        Image *imgNew = new Image(ownerFile, newImageName);
        for (int i = 0; i < copyChannels.size(); i++) {
            Raster *ras = open_raster(copyChannels[i]);
            Raster *newRas = imgNew->create_raster(ras->getRasterName(), ras->getRasterType(),
                                                   ras->get_nx(), ras->get_ny());
            ras->copy(newRas);
        }
        return imgNew;
    }

    
    
    
    
    
    
    
    
    
    void Image::setWKT(std::string &wkt) {
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            ras->setWKT(wkt);
        }
        return;
    }
    
    void Image::setLocation(std::string &location) {
        std::vector<std::string> channels = getChannels();
        for (int i = 0; i < channels.size(); i++) {
            Raster *ras = open_raster(channels[i]);
            ras->setLocation(location);
        }
        return;
    }

    /*  DOESN'T WORK ....  may add this capability later as a public function ????
    void Image::moveRaster(Image *otherImg, Raster *raster) {
        // H5P_DEFAULT
        try {
            imageobj->dereference(*raster->rasterobj,H5P_DEFAULT,H5R_DATASET_REGION);
        } catch (H5::ReferenceException e) {
            std::cerr << "dereference: "+e.getDetailMsg() << std::endl;
        }
        try {
            otherImg->imageobj->reference(raster->rasterobj, raster->getRasterName(),H5R_DATASET_REGION);
        } catch (H5::ReferenceException e) {
            std::cerr << "reference: "+e.getDetailMsg() << std::endl;
        }
    }
     */
    template void Image::fill_raster<uint8_t>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);
    template void Image::fill_raster<float>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);
    template void Image::fill_raster<double>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);

    template void Image::copy_raster<uint8_t>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);
    template void Image::copy_raster<float>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);
    template void Image::copy_raster<double>(Raster*, GDALRasterBand*, const int, const int, const GDALDataType);


}// end namespace GeoStar
