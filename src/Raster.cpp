// Raster.cpp
//
// by Leland Pierce, Feb 18, 2017
// changes by Janice Richards, Jul/2017 to current
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "H5Cpp.h"

#include "File.hpp"
#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {

    // Opening an existing dataset/raster
  Raster::Raster(Image *image, const std::string &name){

      if(!image->datasetExists(name)) throw_RasterDoesNotExistError(image->getFullImagename()+"/"+name);

      // check if its a valid Raster:
      rasterobj = new H5::DataSet(image->openDataset(name));
      if(read_object_type() != "geostar::raster") {
          delete rasterobj;
          throw_RasterOpenError(image->getFullImagename()+"/"+name);
      }

      // finish setting object-specific data:
      rastername = name;
      fullRastername = image->getFullImagename()+"/"+name;
      rastertype = "geostar::raster";
      this->image = image;
      
      H5::DataType type = rasterobj->getDataType();
      raster_datatype = getRasterType(type);
  }// end-Raster-constructor


    
    // Creating a new dataset/raster, with CHANGEABLE (or "mutable") dimensions:
    Raster::Raster(Image *image, const std::string &name, const RasterType &type) {
        
        //if(image->datasetExists(name)) throw_RasterExistsError(image->imageobj->getFileName()+image->imageobj->getObjName()+"/"+name);
        if(image->datasetExists(name)) throw_RasterExistsError(image->getFullImagename()+"/"+name);

        // create a 2D dataset
        hsize_t dims[2];
        dims[0] = 3;
        dims[1] = 3;
        hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
        H5::DataSpace dataspace(2, dims, maxdims);

        //    Get the Hdf5 type corresponding to the given GeoStar type:
        H5::PredType h5Type = this->hdf5Type(type);
        
        //
        //Modify dataset creation properties, i.e. enable chunking. ... This is to allow the dataspace size to be changeable
        //  in the future ("mutable")!
        //
        H5::DSetCreatPropList cparms;
        hsize_t      chunk_dims[2] ={2, 5};
        cparms.setChunk( 2, chunk_dims );
        
        
        try {
            //rasterobj = new H5::DataSet(image->createDataset(name, h5Type, dataspace));
            rasterobj = new H5::DataSet(image->createDataset(name, h5Type, dataspace, cparms));
        } catch (H5::GroupIException e) {
            delete rasterobj;
            throw_RasterCreationMutableError(image->getFullImagename()+"/"+name+"  "+e.getDetailMsg());
        } catch (H5::DataTypeIException e) {
            delete rasterobj;
            throw_RasterCreationMutableError(image->getFullImagename()+"/"+name+"  "+e.getDetailMsg());
        }
        rastername = name;
        fullRastername = image->getFullImagename()+"/"+name;
        raster_datatype=type;
        rastertype = "geostar::raster";
        this->image = image;

        // set objtype attribute.
        write_object_type(rastertype);

    }// end-Raster-constructor



    // Creating a new dataset/raster, with IMMUTABLE dimensions:
  Raster::Raster(Image *image, const std::string &name, const RasterType &type,
           const int &nx, const int &ny) {

      if(image->datasetExists(name))  throw_RasterExistsError(image->getFullImagename()+"/"+name);

      // create a 2D dataset
      hsize_t dims[2];
      dims[0] = ny;
      dims[1] = nx;
      H5::DataSpace dataspace(2, dims);
     
      //    Get the Hdf5 type corresponding to the given GeoStar type:
      H5::PredType h5Type = this->hdf5Type(type);
      try {
          rasterobj = new H5::DataSet(image->createDataset(name, h5Type, dataspace));
      } catch (H5::GroupIException e) {
          delete rasterobj;
          throw_RasterCreationImmutableError(image->getFullImagename()+"/"+name+"  "+e.getDetailMsg());
      } catch (H5::DataTypeIException e) {
          delete rasterobj;
          throw_RasterCreationImmutableError(image->getFullImagename()+"/"+name+"  "+e.getDetailMsg());
      }

      rastername = name;
      fullRastername = image->getFullImagename()+"/"+name;
      raster_datatype=type;
      rastertype = "geostar::raster";
      this->image = image;
    
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
    
    
    void Raster::setSize(long int nx, long int ny) {
        hsize_t dims[2];
        dims[0] = ny;
        dims[1] = nx;
        try {
            rasterobj->extend(dims);
        } catch (H5::DataSetIException e) {
            //std::cerr << "Can't modify size for " << fullRastername << ": " << e.getDetailMsg() << std::endl;
            throw_RasterImmutableError(fullRastername);
        }
        return;
    }

    
    
  // in-place simple threshhold
  // < value : set to 0.
  void Raster::thresh(const double &value) {
      long int nx = get_nx();
      long int ny = get_ny();

      Slice slice(0,0,nx,1);

      std::vector<uint8_t> data(nx);

      for(int line=0;line<ny;++line) {
          slice.setY0(line);
          read(slice,data);
          for(int pixel=0; pixel<nx;++pixel) {
              if(data[pixel] < value) data[pixel]=0;
          }// endfor: pixel
          write(slice,data);
      }// endfor: line

  }// end: thresh
    
    
    template<typename T>
    void Raster::write(const Slice &outSlice, std::vector<T> buffer) {
        Slice slice = outSlice;
        
        // size of the slice of data is the SAME as the size of the slice in the file:
        hsize_t memdims[2];
        memdims[0]=slice.getDeltaY();
        memdims[1]=slice.getDeltaX();
        
        H5::DataSpace memspace(2,memdims);
        
        H5::DataSpace dataspace = rasterobj->getSpace();
        
        // set the slice within the file's dataset we want to write to:
        hsize_t count[2];
        hsize_t start[2];
        start[0]=slice.getY0();
        start[1]=slice.getX0();
        count[0]=slice.getDeltaY();
        count[1]=slice.getDeltaX();
        dataspace.selectHyperslab(H5S_SELECT_SET, count, start);
        
        //int totalSize = (count[0] - start[0]) * (count[1] - start[1]);
        int totalSize = count[0] * count[1];
        if (buffer.size() < totalSize) throw_SliceSizeError(fullRastername);
        
        H5::PredType h5Type = Raster::getHdf5Type<T>();
        
        try {
            rasterobj->write( (void *)&buffer[0], h5Type, memspace, dataspace );
        } catch (H5::DataSetIException e) {
            throw_RasterWriteError(fullRastername + " " + e.getDetailMsg());
        }
    }
    
    
    template<typename T>
    void Raster::read(const Slice &inSlice, std::vector<T> &buffer) const {
        Slice slice = inSlice;
        
        // size of the slice of data is the SAME as the size of the slice in the file:
        hsize_t memdims[2];
        memdims[0]=slice.getDeltaY();
        memdims[1]=slice.getDeltaX();
        H5::DataSpace memspace(2,memdims);
        
        H5::DataSpace dataspace = rasterobj->getSpace();
        
        // set the slice within the file's dataset we want to read from:
        // we want an offset of (50,50), and a size of 100X100
        hsize_t count[2];
        hsize_t start[2];
        start[0]=slice.getY0();
        start[1]=slice.getX0();
        count[0]=slice.getDeltaY();
        count[1]=slice.getDeltaX();
        dataspace.selectHyperslab(H5S_SELECT_SET, count, start);
        
        //int totalSize = (count[0] - start[0]) * (count[1] - start[1]);
        int totalSize = count[0] * count[1];
        if (buffer.size() < totalSize) buffer.resize(totalSize);
        
        H5::PredType h5Type = Raster::getHdf5Type<T>();
        
        try {
            rasterobj->read( (void *)&buffer[0], h5Type, memspace, dataspace );
        } catch (H5::DataSetIException e) {
            throw_RasterReadError(fullRastername + " " + e.getDetailMsg());
        }
    } // end: read


    
    void Raster::copy(Raster *rasNew) {
        switch(raster_datatype) {
            case INT8U:
                copyType<uint8_t>(rasNew);
                break;
            /*
             case INT8S:
             copyType<int8_t>(rasNew);
             case INT16U:
             copyType<uint16_t>(rasNew);
             case INT16S:
             copyType<int16_t>(rasNew);
             case INT32U:
             copyType<uint32_t>(rasNew);
             case INT32S:
             copyType<int32_t>(rasNew);
             case INT64U:
             copyType<uint64_t>(rasNew);
             case INT64S:
             copyType<int64_t>(rasNew);
             */
            case REAL32:
                copyType<float>(rasNew);
                break;
            case REAL64:
                copyType<double>(rasNew);
                break;
            //case COMPLEX_INT16:
            //    return
            default:
            throw_RasterUnsupportedTypeError(fullRastername);
        }
        return;
    }
    
    
    
    template <typename T>
    void Raster::copyType(Raster *outRaster) {
        
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
        
        for (long int i = yOut0; i < yOutMax; i++) {
            oldY = i;
            
            for (long int j = xOut0; j < xOutMax; j++) {
                oldX = j;
                pixelVal = getScaledPixelFromTile<T>(oldY, oldX, 0.0, 0.0, sliceInput, scalingSlice, reader);
                
                newData[j] = pixelVal;
            }
            sliceOut.setY0(i);
            rasNew->write(sliceOut,newData);
            
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "copy execution duration: " << duration << std::endl;
        
        return;
    }

    
    
    void Raster::verifySlice(Slice &slice) {
        if (slice.getX0() < 0) {
            slice.setX0(0);
        } else if (slice.getX0() > (get_nx() - 1)) {
            slice.setX0(get_nx() - 2);
        }
        
        if (slice.getY0() < 0) {
            slice.setY0(0);
        } else if (slice.getY0() > (get_ny() - 1)) {
            slice.setY0(get_ny() - 2);
        }
        
        if (slice.getDeltaX() < 0) {
            slice.setDeltaX(1);
        } else if (slice.getDeltaX() > (slice.getX0() + get_nx() - 1)) {
            slice.setDeltaX(slice.getX0() + get_nx() - 1);
        }
        
        if (slice.getDeltaY() < 0) {
            slice.setDeltaY(1);
        } else if (slice.getDeltaY() > (slice.getY0() + get_ny() - 1)) {
            slice.setDeltaY(slice.getY0() + get_ny() - 1);
        }
    }
    

    double Raster::translateXPoint(double origin, double val) {
        return origin + val;
    }

    double Raster::translateYPoint(double origin, double val) {
        return origin - val;
    }
        
    
    
    //
    //    m1,n .         o      .  m,n
    //                   |
    //                   X
    //                   |
    //                   |
    //                   |
    //                   |
    //   m1,n1 .         o      .  m,n1
    //
    
     template <typename T>
     T Raster::getScaledPixelFromTile(long int oldY, long int oldX, double xDiff, double yDiff, Slice &sliceInput, Slice &scalingSlice, TileIO<T> &reader) {
     
         long int m;
         long int n;
         long int m1, n1;
         T zeroPixel = 0;   // FOR NOW !!!!!!!!!!!!!
         
         /*
         if (oldY < sliceInput.getY0()) return zeroPixel;
         else if (oldY > (sliceInput.getY0()+sliceInput.getDeltaY()-1)) return zeroPixel;
         else if (oldX < sliceInput.getX0()) return zeroPixel;
         else if (oldX > (sliceInput.getX0()+sliceInput.getDeltaX()-1)) return zeroPixel;
         */
         
         
         if (xDiff < 0) {
             m1 = 0;
             m = 1;
             xDiff *= -1.0;
             if (oldY > 0) {
                 scalingSlice.setY0(oldY-1);  // read oldY-1 and oldY (for deltaY = 2)
                 scalingSlice.setDeltaY(2);
             } else if (oldY == 0) {
                 scalingSlice.setY0(0);
                 m = 0;
                 scalingSlice.setDeltaY(1);
             } else {
                 return zeroPixel;
             }
         } else {
             m1 = 1;
             m = 0;
             if ((oldY+1) < (sliceInput.getY0() + sliceInput.getDeltaY() - 1)) {
                 scalingSlice.setY0(oldY);   // read oldY and oldY+1 (for deltaY = 2)
                 scalingSlice.setDeltaY(2);
             } else if ((oldY+1) == (sliceInput.getY0() + sliceInput.getDeltaY() - 1)) {
                 scalingSlice.setY0(sliceInput.getY0() + sliceInput.getDeltaY() - 1);
                 m1 = 0;
                 scalingSlice.setDeltaY(1);
             } else {
                 return zeroPixel;
             }
         }
         if (yDiff < 0) {
             n1 = 0;
             n = 1;
             yDiff *= -1.0;
             if (oldX > 0) {
                 scalingSlice.setX0(oldX-1);  // read oldX-1 and oldX (for deltaX = 2)
                 scalingSlice.setDeltaX(2);
             } else if (oldX == 0) {
                 scalingSlice.setX0(0);
                 n = 0;
                 scalingSlice.setDeltaX(1);
             } else {
                 return zeroPixel;
             }
         } else {
             n1 = 1;
             n = 0;
             if ((oldX+1) < (sliceInput.getX0() + sliceInput.getDeltaX() - 1)) {
                 scalingSlice.setX0(oldX);   // read oldX and oldX+1 (for deltaX = 2)
                 scalingSlice.setDeltaX(2);
             } else if ((oldX+1) == (sliceInput.getX0() + sliceInput.getDeltaX() - 1)) {
                 scalingSlice.setX0(sliceInput.getX0() + sliceInput.getDeltaX() - 1);
                 n1 = 0;
                 scalingSlice.setDeltaX(1);
             } else {
                 return zeroPixel;
             }
         }
         std::vector<T> data = reader.tileRead(scalingSlice);
         int deltaX = scalingSlice.getDeltaX();
     
         T pixelValX1, pixelValX2, pixelVal;
         pixelValX1 = ((-data[m*deltaX + n] + data[m1*deltaX + n]) * xDiff) + data[m*deltaX + n];
         pixelValX2 = ((-data[m*deltaX + n1] + data[m1*deltaX + n1]) * xDiff) + data[m*deltaX + n1];
         pixelVal = ((-pixelValX1 + pixelValX2) * yDiff) + pixelValX1;
         return pixelVal;
     }
    
    
    
    
    void Raster::getNearest(long int& p, double& diff) {
        if (diff > 0.5) {
            p += 1;
            diff = diff - 1.0;
        }
    }
    
    //
    //    m1,n .         o      .  m,n
    //                   |
    //                   X
    //                   |
    //                   |
    //                   |
    //                   |
    //   m1,n1 .         o      .  m,n1
    //
    
    double Raster::getScaledPixel(long int oldY, long int oldX, double xDiff, double yDiff, Slice &sliceInput, double *data) {

        long int m = oldY - sliceInput.getY0();
        long int n = oldX - sliceInput.getX0();
        long int m1, n1;
        
        if (xDiff < 0) {
            m1 = m - 1;
            xDiff *= -1.0;
            if (m1 < 0) m1 = 0;
        } else {
            m1 = m + 1;
            if (m1 >= sliceInput.getDeltaY()) m1 = m;
        }
        if (yDiff < 0) {
            n1 = n - 1;
            yDiff *= -1.0;
            if (n1 < 0) n1 = 0;
        } else {
            n1 = n + 1;
            if (n1 >= sliceInput.getDeltaX()) n1 = n;
        }
        
        double pixelValX1, pixelValX2, pixelVal;
        pixelValX1 = ((-dataAt(m,n,data,sliceInput) + dataAt(m1,n,data,sliceInput)) * xDiff) + dataAt(m,n,data,sliceInput);
        pixelValX2 = ((-dataAt(m,n1,data,sliceInput) + dataAt(m1,n1,data,sliceInput)) * xDiff) + dataAt(m,n1,data,sliceInput);
        pixelVal = ((-pixelValX1 + pixelValX2) * yDiff) + pixelValX1;
        return pixelVal;
    }
    
    
    
    double Raster::dataAt(long int m, long int n, double *data, Slice &sliceInput) {
        //return *(data + m*(this->get_nx()) + n);
        return *(data + m*(sliceInput.getDeltaX()) + n);
    }
    
    
    void Raster::setWKT(std::string &wkt) {
        GeoStar::write_attribute(rasterobj,"wkt",wkt);
        this->wkt = wkt;
        return;
    }
    
    
    std::string Raster::getWKT() {
        std::string wktValue = GeoStar::read_attribute(rasterobj,"wkt");
        if (this->wkt != wktValue) this->wkt = wktValue;
        return wktValue;
    }
    
    
    void Raster::setLocation(std::string &location) {
        GeoStar::write_attribute(rasterobj,"location",location);
        //this->wkt = wkt;
        return;
    }
    
    
    std::string Raster::getLocation() {
        std::string locnValue = GeoStar::read_attribute(rasterobj,"location");
        return locnValue;
    }
    
    
    // Read the HDF5 'location' attribute for this object, and set the Raster's x0, y0, deltaX, and deltaY accordingly.
    //   Return true if the location attribute defines these 4 numbers, otherwise return false;
    bool Raster::getLocationAttributes() {
        std::string locn = getLocation();
        //double x0, y0, deltaX, deltaY;
        if (locn.substr(0,6) == "affine") {
            // parse the rest of the string to get the UL coordinates, and deltaX and deltaY:
            std::stringstream ss(locn.substr(6));
            if (!(ss >> x0))
                throw_AttributeParseError("location: x0");
            if (!(ss >> y0))
                throw_AttributeParseError("location: y0");
            if (!(ss >> deltaX))
                throw_AttributeParseError("location: deltaX");
            if (!(ss >> deltaY))
                throw_AttributeParseError("location: deltaY");
            //std::cerr << "x0=" << x0 << " y0=" << y0 << " deltaX=" << deltaX << " deltaY=" << deltaY << std::endl;
            // Otherwise, location attributes were set ok
            return true;
        }
        // Location attribute is NOT set:
        return false;
    }
    
    
    
    void Raster::setLocationAttributes(double x0, double y0, double deltaX, double deltaY) {
        std::string locn = "affine " + std::to_string(x0) + " " + std::to_string(y0) + " " +
            std::to_string(deltaX) + " " + std::to_string(deltaY);
        // Now set the HDF5 attribute "location" to this string:
        setLocation(locn);
        return;
    }

    
    
    double Raster::getMaxDiff(double v1, double v2, double v3, double v4) {
        double minval = v1;
        double maxval = v1;
        if (minval > v2) minval = v2;
        else if (maxval < v2) maxval = v2;
        if (minval > v3) minval = v3;
        else if (maxval < v3) maxval = v3;
        if (minval > v4) minval = v4;
        else if (maxval < v4) maxval = v4;
        return (maxval - minval);
    }
    
    
    H5::PredType Raster::hdf5Type(const RasterType &type) {
        switch(type) {
            case INT8U:
                return H5::PredType::NATIVE_UINT8;
            case INT8S:
                return H5::PredType::NATIVE_INT8;
            case INT16U:
                return H5::PredType::NATIVE_UINT16;
            case INT16S:
                return H5::PredType::NATIVE_INT16;
            case INT32U:
                return H5::PredType::NATIVE_UINT32;
            case INT32S:
                return H5::PredType::NATIVE_INT32;
            case INT64U:
                return H5::PredType::NATIVE_UINT64;
            case INT64S:
                return H5::PredType::NATIVE_INT64;
            case REAL32:
                return H5::PredType::NATIVE_FLOAT;
            case REAL64:
                return H5::PredType::NATIVE_DOUBLE;
            //case COMPLEX_INT16:
            //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
    }
    
    RasterType Raster::getRasterType(H5::DataType &type) {
        if (type == H5::PredType::NATIVE_UINT8) return INT8U;
        else if (type == H5::PredType::NATIVE_INT8) return INT8S;
        else if (type == H5::PredType::NATIVE_UINT16) return INT16U;
        else if (type == H5::PredType::NATIVE_INT16) return INT16S;
        else if (type == H5::PredType::NATIVE_UINT32) return INT32U;
        else if (type == H5::PredType::NATIVE_INT32) return INT32S;
        else if (type == H5::PredType::NATIVE_UINT64) return INT64U;
        else if (type == H5::PredType::NATIVE_INT64) return INT64S;
        else if (type == H5::PredType::NATIVE_FLOAT) return REAL32;
        else if (type == H5::PredType::NATIVE_DOUBLE) return REAL64;
        else throw_Hdf5UnsupportedTypeError(fullRastername);
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

    // NOTE: user does NOT need to call write<'type'>(...); merely call write(...) and type will be inferred
    //    from the vector parameter type:
    template void Raster::write<uint8_t>(const Slice &, std::vector<uint8_t>);
    template void Raster::write<float>(const Slice &, std::vector<float>);
    template void Raster::write<double>(const Slice &, std::vector<double>);

    template void Raster::read<uint8_t>(const Slice &, std::vector<uint8_t>&)const;
    template void Raster::read<float>(const Slice &, std::vector<float>&)const;
    template void Raster::read<double>(const Slice &, std::vector<double>&)const;
    
    template void Raster::copyType<uint8_t>(Raster*);
    template void Raster::copyType<float>(Raster*);
    template void Raster::copyType<double>(Raster*);
    
    template uint8_t Raster::getScaledPixelFromTile(long int, long int, double, double, Slice&, Slice&, TileIO<uint8_t>&);
    template float Raster::getScaledPixelFromTile(long int, long int, double, double, Slice&, Slice&, TileIO<float>&);
    template double Raster::getScaledPixelFromTile(long int, long int, double, double, Slice&, Slice&, TileIO<double>&);







    void Raster::scale_pixel_values(Raster *ras_out, const double &offset, const double &mult) const {
    long int nx = get_nx();
    long int ny = get_ny();
    int i;

    Slice slice(0,0,nx,1);

    std::vector<float>indata(nx);
    std::vector<float>outdata(nx);

    for(int line=0;line<ny;++line) {
      slice.setY0(line);
      read(slice,indata);
      for(int pixel=0; pixel<nx;++pixel) {
        i = mult*(indata[pixel]-offset);
        if (i<0)   i=0;
        outdata[pixel]= i;
      }// endfor: pixel
      ras_out->write(slice, outdata);
    }// endfor: line

  }// end: scale_pixel_values



  void Raster::copy(const Slice &inslice, Raster *ras_out) const {

	int i;
  //check and see if the input and output slices fit in their respective rasters
  	long int nx_in = get_nx();
	long int ny_in = get_ny();

	if (nx_in < (inslice.getX0() + inslice.getDeltaX()) ) throw_SliceSizeError("copy");
	if (ny_in < (inslice.getY0() + inslice.getDeltaY()) ) throw_SliceSizeError("copy");

	long int nx_out = ras_out->get_nx();
	long int ny_out = ras_out->get_ny();

	if (nx_out < (inslice.getDeltaX() )) throw_SliceSizeError("copy");
	if (ny_out < (inslice.getDeltaY() )) throw_SliceSizeError("copy");


        Slice islice(inslice.getX0(), 0, inslice.getDeltaX(), 1);
        Slice oslice(              0, 0, inslice.getDeltaX(), 1);

        std::vector<float>data(inslice.getDeltaX());

    for(int line=inslice.getY0(); line<inslice.getY0() + inslice.getDeltaY(); ++line) {
      islice.setY0(line);
      read(islice,data);

      oslice.setY0(line-inslice.getY0());
      ras_out->write(oslice, data);
    }// endfor: line


  }// end: copy



  void Raster::set(const Slice &slice, const int &value) {
    int i;

    long int nx = get_nx();
    long int ny = get_ny();
    
    //check if input slice fits in raster
    if (nx < (slice.getX0() + slice.getDeltaX()) ) throw_SliceSizeError("set");
    if (ny < (slice.getY0() + slice.getDeltaY()) ) throw_SliceSizeError("set");
    
    long int num = slice.getDeltaX()*slice.getDeltaY();
    std::vector<float>data(num);

    //assume slices are small, do all at one go:
    for(int i=0;i<num;i++){
      data[i]=value;
    }// endfor

    write(slice, data);

  }// end: set


 void Raster::drawPoint(long int x0, long int y0, const double radius, const double color) {

	long int nx = get_nx(), ny = get_ny();
        int delta=ceil(0.5*radius);

	if ((x0 - delta) < 0 || (x0 + delta) > nx) throw_SliceSizeError("drawPoint");
	if ((y0 - delta) < 0 || (y0 + delta) > ny) throw_SliceSizeError("drawPoint");

        Slice slice(x0-delta, y0-delta, x0+delta, y0+delta);

	long int size = 4.*delta*delta;
	std::vector<float>data(size);
	
	for(int i = 0; i < size; i++) {
	data[i] = color;
	}

	write(slice, data);

 }//end drawPoint

 void Raster::drawLine(Slice &slice, const double radius, const double color) {

	if (radius < 0) throw_RadiusSizeError("drawFilledCircle");

	long int nx = get_nx(), ny = get_ny();
        long int slice_x0 = slice.getX0();
        long int slice_y0 = slice.getY0();
        long int slice_dx = slice.getDeltaX();
        long int slice_dy = slice.getDeltaY();
	if ((slice_x0 - radius) < 0 || (slice_x0 + radius) > nx) throw_RasterSizeError("in drawLine");
	if ((slice_y0 - radius) < 0 || (slice_y0 + radius) > ny) throw_RasterSizeError("in drawLine");
	if ((slice_x0 + slice_dx - radius) < 0 || (slice_x0 + slice_dx + radius) > nx) throw_RasterSizeError("in drawLine");
	if ((slice_y0 + slice_dy - radius) < 0 || (slice_y0 + slice_dy + radius) > ny) throw_RasterSizeError("in drawLine");

	std::vector<float> data;
	read(slice, data);

	drawFilledCircle(slice_x0, slice_y0, radius, color);

	drawFilledCircle(slice_x0 + slice_dx, slice_y0 + slice_dy, radius, color);

	double slope = slice_dy / slice_dx;
	double y = 0;

	double distDivisor = sqrt((slice_dx * slice_dx) + (slice_dy * slice_dy));
	double lineDist = 0;

	//define equation of line as y-y1 = slope(x-x1)
	// -> y = slope*x - slope*x1 + y1
	// but within the slice, x1 and y1 are zero (initial points) so y = slope * x
	for (int i = 0; i < slice_dx; ++i) {
	  y = (slope * i);
	  data[y * slice_dy + i] = color;
	}

	//now sweep through slice and find any points with distance to line less than the radius
	//use (Ax + By + C) / sqrt(A^2 + B^2) for the distance calculation

	for (int i = 0; i < slice_dy; ++i) {
	  for (int j = 0; j < slice_dx; ++j) {
	    lineDist = fabs((slice_dx * i) - (slice_dy * j)) / distDivisor;
	    if (lineDist <= radius) {
		data[i * slice_dy + j] = color;
		}//endif
	  }//endfor
	}//endfor

	write(slice, data);
		
 }//end drawLine

  void Raster::drawRectangle(const Slice &slice, const int radius, const int color) {

	if (radius < 0) throw_RadiusSizeError("drawRectangle");

	long int nx = get_nx(), ny = get_ny();
	if (slice.getX0() < 0 || slice.getDeltaX() > nx) throw_RasterSizeError("in drawRectangle");
	if (slice.getY0() < 0 || slice.getDeltaY() > ny) throw_RasterSizeError("in drawRectangle");

	//draw edges of desired radius one at a time, setting each region to desired color

	//left edge
        Slice oslice=slice;
        oslice.setDeltaX(radius);
	set(oslice, color);

	//top edge
	oslice.setDeltaX(slice.getDeltaX());
	oslice.setDeltaY(radius);
	set(oslice, color);

	//bottom edge
        oslice=slice;
	oslice.setY0(slice.getY0() + slice.getDeltaY() -radius);
	oslice.setDeltaY(radius);
	set(oslice, color);

	//right edge
        oslice=slice;
	oslice.setX0(slice.getX0() + slice.getDeltaX() -radius);
	oslice.setDeltaY(radius);
	set(oslice, color);
		

 }//endRectangle


void Raster::drawFilledRectangle(const Slice &slice, const int radius, const int lineColor, const int fillColor) {

	if (radius < 0) throw_RadiusSizeError("drawFilledRectangle");

	long int nx = get_nx(), ny = get_ny();
	if (slice.getX0() < 0 || slice.getDeltaX() > nx) throw_RasterSizeError("in drawFilledRectangle");
	if (slice.getY0() < 0 || slice.getDeltaY() > ny) throw_RasterSizeError("in drawFilledRectangle");

	//set whole slice to filled color first
        Slice oslice = slice;
	set(oslice, fillColor);

	//draw edges of desired radius one at a time, setting each region to desired color
	//left edge
        oslice.setDeltaX(radius);
	set(oslice, lineColor);

	//top edge
	oslice.setDeltaX(slice.getDeltaX());
	oslice.setDeltaY(radius);
	set(oslice, lineColor);

	//bottom edge
        oslice=slice;
	oslice.setY0(slice.getY0() + slice.getDeltaY() -radius);
	oslice.setDeltaY(radius);
	set(oslice, lineColor);

	//right edge
        oslice=slice;
	oslice.setX0(slice.getX0() + slice.getDeltaX() -radius);
	oslice.setDeltaY(radius);
	set(oslice, lineColor);

 }//endFilledRectangle


 void Raster::drawFilledCircle(long int x0, long int y0, const double radius, const double color) {

   if (radius < 0) throw_RadiusSizeError("drawFilledCircle");

	//circle must not extend beyond raster
	long int nx = get_nx(), ny = get_ny();
	if ((x0 - radius) < 0 || (x0 + radius) > nx) throw_RasterSizeError("in drawFilledCircle");
	if ((y0 - radius) < 0 || (y0 + radius) > ny) throw_RasterSizeError("in drawFilledCircle");

	//slice enclosing circle
        Slice slice(x0-radius, y0-radius, 2*radius, 2*radius);

	long int size = 4.*radius*radius;
	std::vector<float>data(size);
	
	read(slice, data);

	//overwrite pixels within the circle
	double dx = 0, dy = 0;
	double distsq = 0;
	double rsq = radius * radius;
	for (int x = 0; x < 2*radius; ++x) {
	  for (int y = 0; y < 2*radius; ++y) {
	    dx = x - radius;
	    dy = y - radius;
	    distsq = dx * dx + dy * dy;
	     if (distsq <= rsq) {
               data[y * slice.getDeltaX() + x] = color;
	      }//endif
	   }//endfor
	}//endfor

	write(slice, data);

 }//end drawFilledCircle



  GeoStar::Raster * Raster::operator+(const GeoStar::Raster & r2)
  {
    GeoStar::Image * img = Raster::getParent();
    std::string str = rastername + "_PLUS_" + r2.rastername;
    GeoStar::Raster * result = new GeoStar::Raster(img, str, raster_datatype, get_nx(), get_ny());
    this->add(&r2, result);
    return result;
  }
  void Raster::add(const GeoStar::Raster * r2, GeoStar::Raster * ras_out)
  {
    long int nx = get_nx(), ny = get_ny();
    if(nx != r2->get_nx() || ny != r2->get_ny())throw_RasterSizeError("in add");
    Slice slice(0,0,nx,1);
    std::vector<float> bufferA(nx);
    std::vector<float> bufferB(nx);
    for(long int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, bufferA);
      r2->read(slice, bufferB);
      for(int j = 0; j < get_nx(); j++)bufferA[j] += bufferB[j];
      ras_out->write(slice, bufferA);
    }
  }

  GeoStar::Raster * Raster::operator-(const GeoStar::Raster & r2)
  {
    GeoStar::Image * img = Raster::getParent();
    std::string str = rastername + "_MINUS_" + r2.rastername;
    GeoStar::Raster * result= new GeoStar::Raster(img, str, raster_datatype, get_nx(), get_ny());
    this->subtract(&r2, result);
    return result;
  }
  void Raster::subtract(const GeoStar::Raster * r2, GeoStar::Raster * ras_out)
  {
    long int nx = get_nx(), ny = get_ny();
    if(nx != r2->get_nx() || ny != r2->get_ny())throw_RasterSizeError("in subtract");
    Slice slice(0,0,nx,1);
    std::vector<float> bufferA(nx);
    std::vector<float> bufferB(nx);
    for(long int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, bufferA);
      r2->read(slice, bufferB);
      for(int j = 0; j < get_nx(); j++)bufferA[j] -= bufferB[j];
      ras_out->write(slice, bufferA);
    }
  }

  GeoStar::Raster * Raster::operator*(const GeoStar::Raster & r2)
  {
    GeoStar::Image * img = Raster::getParent();
    std::string str = rastername + "_TIMES_" + r2.rastername;
    GeoStar::Raster * result= new GeoStar::Raster(img, str, raster_datatype, get_nx(), get_ny());
    this->multiply(&r2, result);
    return result;
  }
  void Raster::multiply(const GeoStar::Raster * r2, GeoStar::Raster * ras_out)
  {
    long int nx = get_nx(), ny = get_ny();
    if(nx != r2->get_nx() || ny != r2->get_ny())throw_RasterSizeError("in multiply");
    Slice slice(0,0,nx,1);
    std::vector<float> bufferA(nx);
    std::vector<float> bufferB(nx);
    for(long int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, bufferA);
      r2->read(slice, bufferB);
      for(int j = 0; j < get_nx(); j++)bufferA[j] *= bufferB[j];
      ras_out->write(slice, bufferA);
    }
  }

#ifdef YET
  GeoStar::Raster * Raster::operator/(const GeoStar::Raster & r2)
  {
    GeoStar::Image * img = Raster::getParent();
    std::string str = rastername + "_DIVIDEDBY_" + r2.rastername;
    GeoStar::Raster * result= new GeoStar::Raster(img, str, raster_datatype, get_nx(), get_ny());
    this->divide(r2, result);
    return result;
  }
#endif

  void Raster::divide(const GeoStar::Raster * r2, GeoStar::Raster * ras_out)
  {
    long int nx = get_nx(), ny = get_ny();
    if(nx != r2->get_nx() || ny != r2->get_ny())throw_RasterSizeError("in divide");
    Slice slice(0,0,nx,1);
    std::vector<float> bufferA(nx);
    std::vector<float> bufferB(nx);
    for(long int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, bufferA);
      r2->read(slice, bufferB);
      for(int j = 0; j < get_nx(); j++)
      {
        if(bufferB[j] == 0)bufferA[j] = 255; // divide by zero goes to max value
        else bufferA[j] /= bufferB[j];
      }
      ras_out->write(slice, bufferA);
    }
  }

  Raster* Raster::resize(Image *img, int resize_width, int resize_height){
    long int nx = get_nx(), ny = get_ny();

    if (resize_width < 0 || resize_height < 0) throw_RasterSizeError("in resize");

    if (resize_width == nx && resize_height == ny){
      return this;
    }

    Raster *ras_temp = img->create_raster("temp", GeoStar::INT8U, resize_width, ny);
    Raster *ras2 = img->create_raster("resized", GeoStar::INT8U, resize_width, resize_height);


    //first width, then height
    std::vector<float> src_width(nx);
    std::vector<float> src_height(ny);

    std::vector<float> destination_width(resize_width);
    std::vector<float> destination_height(resize_height);

    Slice slice(0,0,nx,1);
    Slice output_slice(0,0,resize_width,1);

    for (int i = 0; i < ny; ++i){
      //get new row
      slice.setY0(i);
      read(slice, src_width);
      //change each row in ras2

      // LEP: no: we need to do it ourselves, not with opencv!!! YETYET
      //cv::resize(cv::InputArray(src_width), 
      //           cv::OutputArray(destination_width), 
      //           cv::Size(resize_width, 1), resize_width/nx, 1);
      output_slice.setY0(i);
      ras_temp->write(output_slice, destination_width);
    }

    slice.setY0(0);
    slice.setDeltaX(1);
    slice.setDeltaY(ny);

    output_slice.setY0(0);
    output_slice.setDeltaX(1);
    output_slice.setDeltaY(resize_height);


    //now adjust the height
    for (int i = 0; i < resize_width; ++i){
      //get new column
      slice.setX0(i);
      ras_temp->read(slice, src_height);
      //change each col in ras2
      // LEP: no: we need to do it ourselves, not with opencv!!! YETYET
      //cv::resize(cv::InputArray(src_height), cv::OutputArray(destination_height), cv::Size(resize_height, 1), resize_height/ny, 1);

      output_slice.setX0(i);
      ras2->write(output_slice, destination_height);
    }
      delete ras2;
      return ras2;
  }

  GeoStar::Image * Raster::getParent()
  {
    size_t len = H5Iget_name(rasterobj->getId(),NULL,0);
    char buffer[len];
    H5Iget_name(rasterobj->getId(),buffer,len+1);
    std::string imgName = buffer;
    int a = imgName.find_first_of("/");
    imgName = imgName.substr(a,imgName.find_last_of("/")-a);

    std::string fileName = rasterobj->getFileName();
    GeoStar::File * f = new GeoStar::File(fileName, "existing");
    GeoStar::Image * img = new Image(f,imgName);
    delete f;
    return img;
  }


  GeoStar::Raster * GeoStar::Raster::operator+(const float & val)
  {
    GeoStar::Image * img = getParent();
    std::string str = rastername+"_PLUS_val";
    GeoStar::Raster * r2 = new GeoStar::Raster(img, str, raster_datatype, get_nx(),get_ny());
    long int nx = get_nx();
    long int ny = get_ny();
    Slice slice(0,0,nx,1);
    std::vector<float> buffer(nx);
    for(int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, buffer);
      for(int j = 0; j < nx; j++)buffer[j]+=val;
      r2->write(slice, buffer);
    }
    return r2;
  }
  GeoStar::Raster * GeoStar::Raster::operator-(const float & val)
  {
    GeoStar::Image * img = getParent();
    std::string str = rastername+"_MINUS_val";
    GeoStar::Raster * r2 = new GeoStar::Raster(img, str, raster_datatype, get_nx(),get_ny());
    long int nx = get_nx();
    long int ny = get_ny();
    Slice slice(0,0,nx,1);
    std::vector<float> buffer(nx);
    for(int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, buffer);
      for(int j = 0; j < nx; j++)buffer[j]-=val;
      r2->write(slice, buffer);
    }
    return r2;
  }
  GeoStar::Raster * GeoStar::Raster::operator*(const float & val)
  {
    GeoStar::Image * img = getParent();
    std::string str = rastername+"_TIMES_val";
    GeoStar::Raster * r2 = new GeoStar::Raster(img, str, raster_datatype, get_nx(),get_ny());
    long int nx = get_nx();
    long int ny = get_ny();
    Slice slice(0,0,nx,1);
    std::vector<float> buffer(nx);
    for(int i = 0; i < ny; i++)
    {
      slice.setY0(i);
      read(slice, buffer);
      for(int j = 0; j < nx; j++)buffer[j]*=val;
      r2->write(slice, buffer);
    }
    return r2;
  }
  GeoStar::Raster * GeoStar::Raster::operator/(const float & val)
  {
    GeoStar::Image * img = getParent();
    std::string str = rastername+"_DIVIDEDBY_val";
    GeoStar::Raster * r2 = new GeoStar::Raster(img, str, raster_datatype, get_nx(),get_ny());
    long int nx = get_nx();
    long int ny = get_ny();
    Slice slice(0,0,nx,1);
    std::vector<float> buffer(nx);
    if(val == 0) // can't divide by zero
    {
      for(int i = 0; i < nx; i++)buffer[i] = 255; // set to max value
      for(int i = 0; i < ny; i++)
      {
        slice.setY0(i);
        write(slice, buffer);
      }
    }
    else
    {
      for(int i = 0; i < ny; i++)
      {
        slice.setY0(i);
        read(slice, buffer);
        for(int j = 0; j < nx; j++)buffer[j]/=val;
        r2->write(slice, buffer);
      }
    }
    return r2;
  }


















  void Raster::addSaltPepper(Raster *rasterOut, const double low) {
	//check if probability parameters are right
	if(low < 0 || low > 0.5) throw_ProbablityError("addSaltPepper");

	const double high = 1 - low;

	long int nx = get_nx();
	long int ny = get_ny();
	long int nx_out = rasterOut->get_nx();
	long int ny_out = rasterOut->get_ny();
	//check raster bounds
	if (nx != nx_out) throw_RasterSizeError("in addSaltPepper");
	if (ny != ny_out) throw_RasterSizeError("in addSaltPepper");

	double temp = 0;
        Slice slice(0,0,nx,1);

	//init random seed, to limit pseudorandom results
	srand((unsigned)time(NULL));

	std::vector<double> data(nx);

	//loop through image line by line, calculating random value between 0 and 1		
	for (int i = 0; i < ny; ++i) {
          slice.setY0(i);
          read(slice, data);
          for (int j = 0; j < nx; ++j) {
            temp = ((double)rand() / (double)(RAND_MAX));
            //set values equal to 0 if below low thresh, or higher than high thresh
            if (temp <= low) data[j] = 0;
            else if (temp >= high) data[j] = 15000;
	  }//endfor
          rasterOut->write(slice, data);
	}//endfor
	
	
 }//end--addSaltPepper

 void Raster::bitShift(Raster *rasterOut, int bits, bool direction) {
   if (bits < 0) throw_BitError("bitShift");
	long int nx = get_nx();
	long int ny = get_ny();
	long int nx_out = rasterOut->get_nx();
	long int ny_out = rasterOut->get_ny();
	//check raster bounds
	if (nx != nx_out) throw_RasterSizeError("in bitShift");
	if (ny != ny_out) throw_RasterSizeError("in bitShift");

        Slice slice(0,0,nx,1);
	
	std::vector<float> data(nx);
	
	//loop through image and bitshift right or left, line by line
	if (direction) {
          double inverse = 1 / pow(2, bits);
	  for (int i = 0; i < ny; ++i) {
            slice.setY0(i);
            read(slice, data);
	    for (int j = 0; j < nx; ++j) {
              data[j] *= inverse;
	    }//endfor
	    rasterOut->write(slice, data);
	  }//endfor
	}//endif
	
	else {
          for (int i = 0; i < ny; ++i) {
            slice.setY0(i);
            read(slice, data);
            for (int j = 0; j < nx; ++j) {
              data[j] *= pow(2, bits);
            }//endfor
            rasterOut->write(slice, data);
          }//endfor
	}//endelse
        

 }//end--bitShift

  void Raster::autoLocalThresh(Raster *rasterOut, const int partitions) {

    long int nx = get_nx();
    long int ny = get_ny();
    long int nx_out = rasterOut->get_nx();
    long int ny_out = rasterOut->get_ny();
    //check raster bounds
    if (nx != nx_out) throw_RasterSizeError("in autoLocalThresh");
    if (ny != ny_out) throw_RasterSizeError("in autoLocalThresh");
    //check partition bounds
    if (partitions <= 0) throw_PartitionError("in autoLocalThresh");
    if (partitions > 150) throw_PartitionError("in autoLocalThresh");
    
    double partitionSizeX = nx / partitions;
    double partitionSizeY = ny / partitions;
    
    Slice slice(0,0,partitionSizeX,1);
      
    std::vector<double>data;
    double threshhold = 0;
    double max = 0;
    double min = 0;
    
    //first define the slice loops, x and y which go through the sectors.  reset thresh variables
    for (int y = 0; y < partitions; ++y) {
      for (int x = 0; x < partitions; ++x) {
        slice.setX0(x * partitionSizeX);
	max = 0;
	min = 100000; //assign large num to min to init - can't reassign variable to a data pt with every read
	threshhold = 0;
        //then the interior loops, which go into the actual sectors and do the reading and writing
        
        //first find max and min and compute average
        for (int i = 0; i < partitionSizeY; ++i) {
          slice.setY0(i + y * partitionSizeY);
          read(slice, data);
          for (int j = 0; j < partitionSizeX; ++j) {
            if (data[j] < min) min = data[j];
            if (data[j] > max) max = data[j];
          }//endfor - j
        }//endfor - i
        
	//not sure exactly what factor this should be divided by.  Gets better as partitions grow.
	threshhold = (max + min) / 3;
	
        
        //then perform threshholding operation
        for (int p = 0; p < partitionSizeY; ++p) {
          slice.setY0(p + y * partitionSizeY);
          read(slice, data);
          for (int q = 0; q < partitionSizeX; ++q) {
            if (data[q] < threshhold) data[q] = 0;
          }//endfor - q
          rasterOut->write(slice, data);
        }//endfor - p
        
      }//endfor - x
    }//endfor - y
    
  }//end - autoLocalThresh



  void Raster::FFT_2D(GeoStar::Image *img, Raster *rasOutReal, Raster *rasOutImg) {

	long int nx = 1024;
	long int ny = 1024;
	long int nx_outReal = rasOutReal->get_nx();
	long int ny_outReal = rasOutReal->get_ny();
	long int nx_outImg = rasOutImg->get_nx();
	long int ny_outImg = rasOutImg->get_ny();
	//check raster bounds
	if (nx != nx_outReal) throw_RasterSizeError("in FFT_2D");
	if (ny != ny_outReal) throw_RasterSizeError("in FFT_2D");
	if (nx != nx_outImg)  throw_RasterSizeError("in FFT_2D");
	if (ny != ny_outImg)  throw_RasterSizeError("in FFT_2D");

	fftw_complex *in, *out;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nx);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nx);
	fftw_plan plan;
	plan = fftw_plan_dft_1d(nx, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	Slice sliceFFTW(0,0,nx,1);
	std::vector<double> dataReal(nx);
	std::vector<double> dataImg(nx);
	
	GeoStar::Raster *rasBufferReal = img->create_raster("BufferReal", GeoStar::REAL32, nx, ny);
	GeoStar::Raster *rasBufferImg = img->create_raster("BufferImg", GeoStar::REAL32, nx, ny);

	//transform row by row
	for (int y = 0; y < ny; ++y) {	
          sliceFFTW.setY0(y);
          read(sliceFFTW, dataReal);
          for(int i=0;i<nx;i++) { 
            in[i][0]=dataReal[i]; 
            in[i][1]=0.0;
          }
          
	
          fftw_execute(plan);
          
          for (int i = 0; i < nx; i++) {
            dataReal[i] = out[i][0];
            dataImg[i] = out[i][1];
          }
          rasBufferReal->write(sliceFFTW, dataReal);
          rasBufferImg->write(sliceFFTW, dataImg);
          
	}//endfor - row-by-row
        
	//now delete objects and reinitialize for the ny size - cols transform
	fftw_destroy_plan(plan);
	fftw_free(in); fftw_free(out);
        
	fftw_complex *inCols, *outCols;
	inCols = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ny);
	outCols = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ny);
	fftw_plan planCols;
	planCols = fftw_plan_dft_1d(ny, inCols, outCols, FFTW_FORWARD, FFTW_ESTIMATE);

        sliceFFTW.setX0(0);
        sliceFFTW.setY0(0);
        sliceFFTW.setDeltaX(1);
        sliceFFTW.setDeltaY(ny);

	dataReal.resize(ny);
	dataImg.resize(ny);
	
	//transform col-by-col	
	for (int x = 0; x < nx; ++x) {
          sliceFFTW.setX0(x);
          rasBufferReal->read(sliceFFTW, dataReal);
          rasBufferImg->read(sliceFFTW, dataImg);
          for (int i = 0; i < ny; ++i) {
            inCols[i][0] = dataReal[i];
            inCols[i][1] = dataImg[i];
          }
          
          fftw_execute(planCols);
          
          for (int i = 0; i < ny; ++i) {
            dataReal[i] = outCols[i][0];
            dataImg[i] = outCols[i][1];
          }
          rasOutReal->write(sliceFFTW, dataReal);
          rasOutImg->write(sliceFFTW, dataImg);
          
	}//endfor - col-by-col
        
	delete rasBufferReal;
	delete rasBufferImg;
	fftw_destroy_plan(planCols);
	fftw_free(inCols); fftw_free(outCols);
        
   }//end - FFT_2D

  void Raster::FFT_2D_Inv(GeoStar::Image *img, Raster *rasOut, Raster *rasInImg) {

	long int nx = get_nx();
	long int ny = get_ny();
	long int nx_out = rasOut->get_nx();
	long int ny_out = rasOut->get_ny();
	long int nx_img = rasInImg->get_nx();
	long int ny_img = rasInImg->get_ny();
	//check raster bounds
	if (nx != nx_out) throw_RasterSizeError("in FFT_2D_Inv");
	if (ny != ny_out) throw_RasterSizeError("in FFT_2D_Inv");
	if (nx != nx_img) throw_RasterSizeError("in FFT_2D_Inv");
	if (ny != ny_img) throw_RasterSizeError("in FFT_2D_Inv");

	fftw_complex *in, *out;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ny);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ny);
	fftw_plan plan;
	plan = fftw_plan_dft_1d(ny, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

	Slice sliceFFTW(0,0,1,ny);
	std::vector<double> dataReal(ny);
	std::vector<double> dataImg(ny);
	
	GeoStar::Raster *rasBufferReal = img->create_raster("BufferRealInv", GeoStar::REAL32, nx, ny);
	GeoStar::Raster *rasBufferImg = img->create_raster("BufferImgInv", GeoStar::REAL32, nx, ny);

	//transform col by col
	for (int x = 0; x < nx; ++x) {	
          sliceFFTW.setX0(x);
          read(sliceFFTW, dataReal);
          rasInImg->read(sliceFFTW, dataImg);
          for(int i=0;i<ny;i++) { 
            in[i][0]=dataReal[i]; 
            in[i][1]=dataImg[i];
          }
          
          
          fftw_execute(plan);
          
          for (int i = 0; i < ny; i++) {
            dataReal[i] = out[i][0];
            dataImg[i] = out[i][1];
          }
          rasBufferReal->write(sliceFFTW, dataReal);
          rasBufferImg->write(sliceFFTW, dataImg);
          
	}//endfor - col-by-col
        
	//now delete objects and reinitialize for the nx size - rows transform
	fftw_destroy_plan(plan);
	fftw_free(in); fftw_free(out);
        
	fftw_complex *inRows, *outRows;
	inRows = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nx);
	outRows = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nx);
	fftw_plan planRows;
	planRows = fftw_plan_dft_1d(nx, inRows, outRows, FFTW_BACKWARD, FFTW_ESTIMATE);
        
        sliceFFTW.setX0(0);
        sliceFFTW.setY0(0);
        sliceFFTW.setDeltaX(nx);
        sliceFFTW.setDeltaY(1);
        
	dataReal.resize(nx);
	dataImg.resize(nx);
	
	//transform row-by-row	
	for (int y = 0; y < ny; ++y) {
          sliceFFTW.setY0(y);
          rasBufferReal->read(sliceFFTW, dataReal);
          rasBufferImg->read(sliceFFTW, dataImg);
          for (int i = 0; i < nx; ++i) {
            inRows[i][0] = dataReal[i];
            inRows[i][1] = dataImg[i];
          }
          
          fftw_execute(planRows);
          
          for (int i = 0; i < nx; ++i) {
            dataReal[i] = outRows[i][0];
          }
          rasOut->write(sliceFFTW, dataReal);
          
	}//endfor - row-by-row

	delete rasBufferReal;
	delete rasBufferImg;
	fftw_destroy_plan(planRows);
	fftw_free(inRows); fftw_free(outRows);
        
  }//end - FFT_2D_Inv

 void Raster::lowPassFilter(GeoStar::Image *img, Raster *rasInReal, Raster *rasInImg, Raster *rasOut) {
	//check the output raster only, all others are checked in the FFT_2D() function
	long int nx = 1024;
	long int ny = 1024;
	if (nx != rasOut->get_nx()) throw_RasterSizeError("in lowPassFilter");
	if (ny != rasOut->get_ny()) throw_RasterSizeError("in lowPassFilter");

	//transform
	FFT_2D(img, rasInReal, rasInImg);

	//set slice in output rasters to zero
	const long int width = nx / 3;
	Slice setSlice(width, width, width, width);
	rasInReal->set(setSlice, 0);
	rasInImg->set(setSlice, 0);

	//transform back
	rasInReal->FFT_2D_Inv(img, rasOut, rasInImg);

 } //end - lowPassFilter

 void Raster::contrastCorrection(Raster *rasOut, const double contrast) {

	long int nx = get_nx();
	long int ny = get_ny();
	long int nx_out = rasOut->get_nx();
	long int ny_out = rasOut->get_ny();
	//check raster bounds
	if (nx != nx_out) throw_RasterSizeError("in contrastCorrection");
	if (ny != ny_out) throw_RasterSizeError("in contrastCorrection");

	long int sum = 0;
	double average = 0;
	double newVal = 0;
	
        std::vector<double> data(nx);

        Slice slice(0,0,nx,1);

	for (int i = 0; i < ny; ++i) {
          slice.setY0(i);
          read(slice, data);
	  for (int j = 0; j < nx; ++j) {
            sum += data[j];
	  }//endfor - nx
	}//endfor - ny
        
	average = sum / (nx * ny);
        
	for (int i = 0; i < ny; ++i) {
          slice.setY0(i);
          read(slice, data);
	  for (int j = 0; j < nx; ++j) {
            newVal = contrast * (data[j] - average) + average;
            if (newVal < 0) newVal = 0;
            data[j] = newVal;
	  }//endfor - nx
          rasOut->write(slice, data);
	}//endfor - ny
        
 } //end - contrastCorrection













}// end namespace GeoStar
