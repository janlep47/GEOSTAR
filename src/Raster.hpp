// Raster.hpp
//
// by Leland Pierce, Feb 18, 2017
// changes by Janice Richards, Jul/2017 to current
//
//----------------------------------------
#ifndef RASTER_HPP_
#define RASTER_HPP_


#include <string>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <deque>
#include <chrono>

#include "H5Cpp.h"

#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "RasterType.hpp"
#include "attributes.hpp"
#include "Exceptions.hpp"
#include "RasterFunction.hpp"


#include <ogr_spatialref.h>
#include "gdal_priv.h"
#include <gdal_alg.h>    // GDALPolygonize()
#include "ogrsf_frmts.h"  // Vector/geometry types/processing

extern "C" {
#include "fftw3.h"
}

namespace GeoStar {
  class Image;
    class Slice;

/** \brief Raster -- Implementation of raster operations for HDF5-based GeoStar files.


This class is used as the standard interface for raster operations
 that are used for storing and processing of data in GeoStar using the HDF5 implementation.
In particular, first, data is imported into GeoStar from data provided by a remote sensing data
processing facility, or the output from another data processing system.
This data is stored in the GeoStar file format.

\see File, Image, Raster, RasterType, Slice, WarpParameters, TileIO

\par Usage Overview
The Raster class is meant to be used when dealing with GeoStar files.
Other classes are used to deal with external files in other formats.
This class provides methods for reading and writing rasters, as well as scaling and warping them.

Reading a raster requires a slice object, which describes the rectangular portion of the raster to read,
along with a templated vector buffer, to write the data into.

Writing a raster requires a slice object, which describes the rectangular portion of the raster to write
into, along with a templated vector buffer from which to write the data.

Scaling a raster is possible using several different approaches.  The scale functions return a new Raster
object, which contains the scaled data from the calling Raster.  See scale function documentation for
more details.

Warping a raster is possible using several different approaches.  The warp functions return a new Raster
object, which contains the warped data from the calling Raster.  See warp function documentation for
more details.
 
Rotating a raster is possible using two different methods, "rotate" and "rotateWithWarp".  Currently "rotate" is
preferred, as it is faster; "rotateWithWarp" is left in as a legacy function, for possible future testing.  For the
simple "rotate" function, there are two approaches.  See rotate function documentation for more details.

\par Details
This class also has functions to display the raster width and height, and get and set functions for the
raster type.

The class keeps track of the rastername, in case that is needed.

The constructors should only be called by the Image class: Image::createRaster.

*/
  class Raster {

  private:
      Image *image;
      std::string rastername;
      std::string fullRastername;
      std::string rastertype;
      RasterType  raster_datatype;
      
      // Attributes:
      // 'wkt'
      std::string wkt;  // the WKT associated with this raster, if any
      // 'location'
      double x0;       // actual wkt x-coordinate of the upper-left of the upper-left pixel
      double y0;       // actual wkt y-coordinate of the upper-left of the upper-left pixel
      double deltaX;   // the difference in wkt-coordinates, in the x-direction, for each pixel
      double deltaY;   // the difference in wkt-coordinates, in the y-direction, for each pixel
      
      void verifySlice(Slice &slice);
      void getNearest(long int& p, double& diff);

      double getScaledPixel(long int r, long int c, double xDiff, double yDiff, Slice &sliceInput, double *data);

      template <typename T>
      T getScaledPixelFromTile(long int oldY, long int oldX, double xDiff, double yDiff, Slice &sliceInput, Slice &scalingSlice,
                               TileIO<T> &reader);
      
      double dataAt(long int m, long int n, double *data, Slice &sliceInput);
      
      //template <typename T>
      //T getDataAt(long int m, long int n, std::vector<T> data, Slice &sliceInput);
      H5::PredType hdf5Type(const RasterType &type);
      RasterType getRasterType(H5::DataType &type);
      
      double translateXPoint(double origin, double val);
      double translateYPoint(double origin, double val);
      
      bool getLocationAttributes();
      void setLocationAttributes(double x0, double y0, double deltaX, double deltaY);
      
      double getMaxDiff(double v1, double v2, double v3, double v4);
      
      template <typename T>
      void copyType(Raster *rasNew);

  public:
    H5::DataSet *rasterobj;

    /** \brief Raster Constructor -- allows you to open an existing raster

    This constructor allows you to open an existing raster from within a preexisting HDF5-image object

    \see Read, Write

    \param[in] image
      This is the image object that holds the existing raster.  This is a string set by the user

    \param[in] name
      This is a string that holds the name of the Raster

    \returns
	A valid raster object upon success

    \Par Exceptions
	Exceptions that may be raised:

	RasterOpenError
	RasterDoesNotExist

    \Par Example
	Opening a raster named "test":
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "existing");

  	GeoStar::Image *img = file->open_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test");

	delete ras;
	delete img;
	delete file;
	}

	\endcode

    \Par Details
	The HDF5 Attribute named "object_type" must exist with the value "Geostar::HDF5"
	or it is not a Geostar file and an exception is thrown
    */
    Raster(Image *image, const std::string &name);

      // creating a new Raster
      // existence is an error
      Raster(Image *image, const std::string &name, const RasterType &type);

    /** \brief Raster Constructor -- allows you to create a new raster

    This constructor allows you to create a new raster from within a preexisting HDF5-image object

    \see Read, Write

    \param[in] image
      This is the image object that holds the raster.  This is a string set by the user

    \param[in] name
      This is a string that holds the name of the Raster

    \param[in] type
	Specifies, out of the existing Raster Types, what data type you want your new raster to be.
	Only accepts INT8U, INT16U, REAL32

    \param[in] nx
	specifies x-size (horizontal size) of new raster

    \param[in] ny
	specifies y-size (vertical size) of new raster

    \returns
	A valid raster object upon success

    \Par Exceptions
	Exceptions that may be raised:

	RasterCreationError
	RasterExistsError

    \Par Example
	creating a raster named "test":
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	delete ras;
	delete img;
	delete file;
	}

	\endcode

    \Par Details
	The HDF5 Attribute named "object_type" will be created with the value "Geostar::HDF5"
	so it is considered a Geostar file, else an exception is thrown
    */
      Raster(Image *image, const std::string &name, const RasterType &type,
             const int &nx, const int &ny);


      
      inline RasterType getRasterType() const {
          return raster_datatype;
      }
      
      inline std::string getRasterName() const {
          return rastername;
      }
      
      inline std::string getFullRasterName() const {
          return fullRastername;
      }

      

  /** \brief Raster::write_object_type allows one to change the value of the attribute string
      named "object_type" that is attached to this raster.

   Raster::write_object_type allows one to change the value of the attribute string
      named "object_type" that is attached to this raster.

   \see  Raster

   \param[in] name
       This is a string, set by the user, that holds the desired value for the attribute string.

   \returns
       nothing.

   \par Exceptions
       Exceptions that may be raised by this method:
       AttributeErrorException

   \par Example
       To identify this Raster as an unsigned 8-byte integer:
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           ras->write_object_type("GeoStar::INT8U");

       }//end-main
       \endcode

    \par Details
       Implementation in attributes.cpp for any object type.
       The implementation in this class makes it easier to use in the File context.
  
  */
    // write the "object_type" attribute
    inline void write_object_type(const std::string &value) {
        //GeoStar::write_object_type((H5::H5Location *)rasterobj,value);
        GeoStar::write_object_type(rasterobj,value);
    }

  /** \brief Raster::read_object_type allows one to read the string value of the attribute
      named "object_type" that is attached to this raster.

   Raster::read_object_type allows one to read the string value of the attribute
      named "object_type" that is attached to this raster.

   \see  Raster

   \returns
       the value of the attribute as a string.

   \par Example
       This is not for use by most users. An admin can use this to write a coded string
       to this attribute. It is meant for identifying this HDF5 file as a GeoStar file,
       using the string "geostar::hdf5", as in this example:
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           String rasterType = ras->read_object_type();

       }//end-main
       \endcode

    \par Details
       Implementation in attributes.cpp for any object type.
       The implementation in this class makes it easier to use in the File context.
  
  */
    // read the "object_type" attribute
    std::string read_object_type() const {
        //return GeoStar::read_object_type((H5::H5Location *)rasterobj);
        return GeoStar::read_object_type(rasterobj);
    }

  /** \brief Raster destructor allows one to delete a Raster object from memory.

   The Raster destructor is automatically called to clean up memory used by the Raster object.
   There is a single pointer to the HDF5 DataSet object that must be deleted.

   \see open, close

   \returns
       Nothing.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to open raster "ex1", from file "sirc_raco", and image "image1":
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
       }//end-main
       \endcode

       Before main ends, the c++ run-time system automatically destroys the raster variable
       using the Raster object destructor.

  */
    // cleans up the H5::DataSet object
    inline ~Raster() {
      delete rasterobj;
    }

      
      //H5::PredType getHdf5Type(const RasterType &type);
      //template <RasterType T> static H5::PredType getHdf5Type() {return H5::PredType::NATIVE_UINT8;}
      // just declare one ... others in Raster.cpp
      template <typename T> static H5::PredType getHdf5Type() {return H5::PredType::NATIVE_UINT8;}
      

  /** \brief Raster::write allows one to write a buffer of data to a rectangular region of this raster.
   
   Raster::write allows one to write a buffer of data to a rectangular region of this raster.  The
   rectangular region is described by a Slice object, and the data to be written is passed as a templated
   vector object.  Note that HDF5 write functions will automatically cast the buffer data to the appropriate
   type, for this raster object.

   \see  Raster

   \param[in] outSlice
       This is a Slice object, set by the user, that describes the rectangular region in this raster, into
       which to write the new data.
   
   \param[in] buffer
       This is the templated vector, which holds the new data to write into this raster.

   \returns
       nothing.

   \par Exceptions
       Exceptions that may be raised by this method:
       RasterWriteError
       SliceSizeError

   \par Example
       To write a buffer to a 100 X 50 upper left rectangle of the raster "ex1":
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           Slice outSlice(0,0,100,50);
           std::vector<uint_8u> buffer(5000);
           getNewRasterData(buffer);    // assign buffer contents ...
           ras->write(outSlice, buffer);

       }//end-main
       \endcode

    \par Details
       Implementation in attributes.cpp for any object type.
       The implementation in this class makes it easier to use in the File context.
  
  */
      // write data from a C-array to a slice in the DataSet in the file.
      // size of slice is the same for both C-array and DataSet
      // slice definition: x0, y0, dx, dy
      // pass in whatever type buffer you are using.
      // It should be consistent with the RasterType you pass in.
      // indexing in buffer: [iy*nx+ix]
      //
      // can call 'write(slice, bufr); don't need to say write<int>(slice, bufr)
      template<typename T>
      void write(const Slice &outSlice, std::vector<T> buffer);

      
      
  /** \brief Raster::read allows one to read a rectangular region of this raster into a buffer.
   
   Raster::read allows one to read a rectangular region of this raster into a buffer.  The
   rectangular region is described by a Slice object, and the data to be read is placed in a templated
   vector object.  Note that HDF5 read functions will automatically convert the raster data to the
   appropriate type (the templated vector parameter), for this raster object.

   \see  Raster

   \param[in] outSlice
       This is a Slice object, set by the user, that describes the rectangular region in this raster, from
       which to read the data.
   
   \param[in] buffer
       This is the templated vector, which will hold the read-in data from this raster.

   \returns
       nothing.

   \par Exceptions
       Exceptions that may be raised by this method:
       RasterReadError

   \par Example
       To read a buffer from the 100 X 50 upper left rectangle of the raster "ex1":
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           Slice inSlice(0,0,100,50);
           std::vector<uint_8u> buffer(5000);
           ras->read(inSlice, buffer);
           processData(buffer);      //  do something with the raster data ...

       }//end-main
       \endcode

    \par Details
       Implementation in attributes.cpp for any object type.
       The implementation in this class makes it easier to use in the File context.
  
  */
      // read data to a C-array from a slice in the DataSet in the file.
      // size of slice is the same for both C-array and DataSet
      // slice definition: x0, y0, dx, dy
      // pass in whatever type buffer you are using.
      // It should be consistent with the RasterType you pass in.
      // indexing in buffer: [iy*nx+ix]
      template<typename T>
      void read(const Slice &inSlice, std::vector<T> &buffer) const;

      


  /** \brief Raster:get_nx returns the number of pixels in the y-direction, for the given a raster.

   Raster::get_nx returns the number of pixels in the x-direction, for the given raster.

   \see Raster

   \returns
       the length of the raster, in the x-direction.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       First open the file and the raster, then use get_nx():
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           int rasterWidth = ras->get_nx();

       }//end-main
       \endcode

  */
    //returns the actual size of the image in the x-direction
    long int get_nx() const;

  /** \brief Raster:get_ny returns the number of pixels in the y-direction, for the given a raster.

   Raster::get_ny returns the number of pixels in the y-direction, for the given raster.

   \see Raster

   \returns
       the length of the raster, in the y-direction.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       First open the file and the raster, then use get_ny():
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           int rasterHeight = ras->get_ny();

       }//end-main
       \endcode

  */
    //returns the actual size of the image in the y-direction
    long int get_ny() const;


  /** \brief Raster:thresh threshholds a raster, in-place.

   Raster::thresh threshholds a raster, in-place.
   Sets every pixel to zero that has a value less than the threshhold.
   Does not change other pixels.

   \see Raster
   
   \param[in] name
       This is an integer, set by the user, equal to the desired threshhold value.

   \returns
       Nothing.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to set all pixels in a raster named "ex1"
       with values < 100 to zero. First open the file and the raster, then
       use thresh():
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
           ras->thresh(100.);

       }//end-main
       \endcode

  */
/**
       Here is an example of a particular raster before and
       after the threshhold operation:

       <table>
       <tr>
       <td>
       \imageSize{thresh1.jpg,height:200px;width:400px;}
       \image html thresh1.jpg "Before Threshholding"
       </td>
       <td>
       \imageSize{thresh2.jpg,height:200px;width:400px;}
       \image html thresh2.jpg "After Threshholding" width=30%
       </td>
       </tr>
       </table>


  */
    // in-place simple threshhold
    // < value : set to 0.
    void thresh(const double &value);

    // read a single-channel TIFF image
    // infile is the name of the input TIFF file
    // name is the string to identify the new raster that is created
    // if the image is more than 1 channel, it just uses the first.
    // for now, it creates a 8U channel.
    void read_tiff_1_8U(const std::string &infile, const std::string &name);   //UNUSED!!!!!!


      // NEED TO DOCUMENT (doxygen) !!!!!!!!!
      void setSize(long int nx, long int ny);
      
      void setWKT(std::string &wkt);
      std::string getWKT();
      void setLocation(std::string &location);
      std::string getLocation();







/** \brief scale -- scales image values within parameters

    Using the multiplier and the offset, this function corrects images so that their values are more easily manipulated.
	All values in the image are multiplied by the mult and shifted up or down by the offset.  The resulting data is
	written to a new raster.

    \see set, copy, thresh

    \param[out] ras_out
      raster that the data is being written to.  The raster must exist and must be the same size as the
	original raster.

    \param[in] mult
	multiplier that all values in the image are multiplied by.  Should be >0.

    \param[in] offset
	Value added to all points in the image.

    \returns
	nothing

    \Par Exceptions
	none

    \Par Example
	scaling a raster:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "existing");

  	GeoStar::Image *img = file->open_image("landsat");

  	GeoStar::Raster *ras;
	ras = img->read_file("LC08_L1TP_027033_20170506_20170515_01_T1_B7.TIF", "B07", 1);

	long int nx = ras->get_nx();
        long int ny = ras->get_ny();

        GeoStar:: Raster *ras2 = img->create_raster("test", GeoStar::REAL32, nx, ny);

	ras->scale(ras2, 0.06, 40);

	delete ras;
	delete ras2;
	delete img;
	delete file;

	}

	\endcode

    \Par Details
	Scale is a useful preprocessing tool to be able to more easily manipulate image values.
	Make sure the types of your rasters are consistient - output raster of type double is best.

	linear scaling: out = scale* (in -offset)
    */
    void scale_pixel_values(Raster *ras_out, const double &offset, const double &mult) const;

/** \brief copy -- copy a slice of an image

    copies a slice of an image to an existing/different channel.  This slice will be initialized to (0,0) in
    the new raster.

    \see set, scale, thresh

    \param[in] inslice
	This object is the area you define to copy data from and write to in new raster.  
        The dimensions are as follows:
	x-offset,
	y-offset,
	x-size of slice,
	y-size of slice.
	the slice will start at (0,0) in the new raster, i.e. x and y offsets will be zero.


    \param[out] ras_out
      raster that the data is being written to.  The raster must exist and must be the same size or bigger
	as the slice

    \returns
	nothing

    \Par Exceptions
	none

    \Par Example
	copying a slice of a raster:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "existing");

  	GeoStar::Image *img = file->open_image("landsat");

  	GeoStar::Raster *ras;
	ras = img->read_file("LC08_L1TP_027033_20170506_20170515_01_T1_B7.TIF", "B07", 1);

	long int nx = ras->get_nx();
        long int ny = ras->get_ny();

        GeoStar:: Raster *ras2 = img->create_raster("test", GeoStar::REAL32, nx, ny);

        Slice slice(1000,1000,4000,4000);

	ras->copy(slice, ras2);

	delete ras;
	delete ras2;
	delete img;
	delete file;

	}

	\endcode

    \Par Details
	none.
    */
    void copy(const Slice &inslice, Raster *ras_out) const;
 
      
      void copy(Raster *rasNew);



    /** \brief set -- set a slice of a raster to a value

    Sets a slice of a raster to a user defined value.

    \see copy, scale, thresh

    \param[in] slice
	This object is the area you define to copy data from and write to in new raster.  
        The dimensions are as follows:
	x-offset,
	y-offset,
	x-size of slice,
	y-size of slice.
	The slice should not be bigger than the raster.

    \param[in] value
      value to set the image data to within the slice.  Should be >0.

    \returns
	nothing

    \Par Exceptions
	none

    \Par Example
	set a slice of a raster to 900:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "existing");

  	GeoStar::Image *img = file->open_image("landsat");

  	GeoStar::Raster *ras;
	ras = img->read_file("LC08_L1TP_027033_20170506_20170515_01_T1_B7.TIF", "B07", 1);

	Slice slice(1000, 1500, 4000, 4000);

	ras->set(slice, 900);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

    \Par Details
	note the slice is a bare C-array - passing in a vector slice here will not work
    */
    void set(const Slice &slice, const int &value);



/** \brief drawPoint -- allows the user to draw a point in an image

    draws a point at a location in an image with a desired color and thickness
	Essentially just the set function, but implemented differently and meant to work with drawing.

    \see drawLine

    \param[in] x0
	the x-offset from the top left of the image you wish to draw your point at.
	should not be larger than image x size.

    \param[in] y0
	the y-offset from the top left of the image you wish to draw your point at.
	should not be larger than image y size.

    \param[in] radius
	the thickness of the point, i.e. how many pixels across the point should be.
	should not be larger than the image x or y size.

    \param[in] color
	the value of the point or all pixels within the point.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException

    \par Example
	drawing a point in a raster of radius 5 and color 100:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	ras->drawPoint(20, 20, 5, 100);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

	\par Details
	The point consists of a set of pixels with a given radius, so as the radius gets larger the point
	will appear as a square.

    */
 void drawPoint(long int x0, long int y0, const double radius, const double color);


/** \brief drawLine -- allows the user to draw a line in an image

    draws a line at a point in an image to another point with a desired color and thickness

    \see drawPoint

    \param[in] slice
	A SLice object with values as follows:
	x-offset from top left of image.  Point where you wish to start your line.
	y-offset from top left of image.  Point where you wish to start your line.
	dx, or x-offset from starting point.
	dy, or y-offset from starting point.

    \param[in] radius
	the thickness of the line, i.e. how many pixels across the line should be in the
	opposite direction of the line itself.
	should not be larger than the image x or y size.

    \param[in] color
	the value of the line.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	SliceSizeError
	RadiusSizeErrorException

    \par Example
	drawing a line in a raster of radius 2 and color 100:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

        SLice slice(20,20,20,20);

	ras->drawLine(slice, 2, 100);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{beforeImg.png,height:150px;width:150px;}
        \image html beforeImg.png "Before Line"
        </td>
        <td>
        \imageSize{afterLine.png,height:150px;width:150px;}
        \image html afterLine.png "After Line"
        </td>
        </tr>
        </table>

	\par Details
	The line connects two points, and will draw them for you.  No need to call drawPoint().
	Furthermore, the points will be of the same color and radius as the line.
	RasterSizeError will be thrown if slice exceeds raster dimensions.
	SliceSizeError will be thrown if slice.size() < 4.
	radiussizeerror exception will be thrown if the radius is less than zero.

	This function first calculates the slope of the desired line, then iterates through the slice and draws the bare
	line in y=mx format.  Then the slice is looped through again and the minimum perpendicular distance to the line is found 
	for each point in the slice using the formula (Ax + By + C) / sqrt(A^2 + B^2).  If distance is less than radius, the pixel
	is lit up.  Note that this distance calculation will not work for a vertical line.

    */

void drawLine(Slice &slice, const double radius, const double color);





/** \brief drawRectangle -- allows the user to draw a Rectangle in an image

    draws a Rectangle from the beginning slice xy pair to the final slice xy pair, diagonal to the initial.
	Draws with a desired edge color and edge thickness.

    \see drawPoint, drawLine, drawFilledRectangle

    \param[in] slice
	 Slice that defines a rectangular area in the Raster:
	x-offset from top left of image.  Point where you wish to start your rectangle
	y-offset from top left of image.  Point where you wish to start your rectangle
	dx, or x-offset from starting point.  Should end up mapping point coordinates
	    diagonal to the original point.
	dy, or y-offset from starting point.  Should end up mapping point coordinates
		diagonal to the original point.

    \param[in] radius
	the thickness of the rectangle edges, i.e. how far inward the rectangle edge should extend. radius should not be greater
	than distance between two edges, so for example slice[3] - slice[1] > radius or slice[2] - slice[0] > radius.

    \param[in] color
	the value of the lines connecting each point (edge color).  For a filled rectangle, see drawFilledRectangle.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	SliceSizeError

    \par Example
	drawing a rectangle in a raster of radius 8 and color 100:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	Slice slice(20, 20, 180, 180);

	ras->drawRectangle(slice, 8, 100);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

	\par Details
	The two points connected are diagonal from each other - the function fills in the corners itself.
	This function generates an empty rectangle - to draw a filled rectangle, use drawFilledRectangle.
	slizesizeerror exception will be thrown if slize.size() != 4
	rastersizeerror exception will be thrown if your slice is bigger than the raster or goes off the raster.

    */
  void drawRectangle(const Slice &slice, const int radius, const int color);

/** \brief drawFilledRectangle -- allows the user to draw a filled Rectangle in an image

    draws a Rectangle from the beginning slice xy pair to the final slice xy pair, diagonal to the initial.
	Draws with a desired edge color and edge thickness, filling after with desired color.

    \see drawPoint, drawLine, drawRectangle

    \param[in] slice
	object with values as follows:
	x-offset from top left of image.  Point where you wish to start your rectangle.
	y-offset from top left of image.  Point where you wish to start your rectangle.
	dx, or x-offset from starting point.  Should end up mapping point coordinates
	    diagonal to the original point.
	dy, or y-offset from starting point.  Should end up mapping point coordinates
		diagonal to the original point.

    \param[in] radius
	the thickness of the rectangle edges, i.e. how many pixels across the line should be in the
	opposite direction of the line itself.
	should not be larger than the image x or y size.

    \param[in] lineColor
	the value of the line connecting each point (edge color).

    \param[in] fillColor
	the value of all points inside the boundaries of the rectangle (within the lines), excluding
	the thickness and color of the lines themselves.

    \returns
	nothing

    \par Exceptions
	SliceSizeException
	RasterSizeErrorException

    \par Example
	drawing a rectangle in a raster of radius 8, lineColor 100, and fillColor 35:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	vector<long int>slice(4) = {20, 20, 180, 180};

	ras->drawFilledRectangle(slice, 8, 100, 35);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

	\par Details
	The two points connected are diagonal from each other - the function fills in the corners itself.
	This function generates a filled rectangle - to draw an empty rectangle, use drawRectangle.
	slizesizeerror exception will be thrown if slize.size() != 4
	rastersizeerror exception will be thrown if your slice is bigger than the raster or goes off the raster.

    */
void drawFilledRectangle(const Slice &slice, const int radius, 
                                 const int lineColor, const int fillColor);






/** \brief drawFilledCircle -- allows the user to draw a filled circle, or point,  in an image

    draws a point at a location in an image with a desired color and radius.
	Essentially just the set function, but implemented differently and meant to work with drawing.

    \see drawLine, drawRectangle

    \param[in] x0
	the center x-coordinate of the circle.  Should be able to accomodate the circle radius from this
	center point, so x0 - radius or x0 + radius should not be larger that the image x-size or less than zero.

    \param[in] y0
	the center y-coordinate of the circle.  Should be able to accomodate the circle radius from this
	center point, so y0 - radius or y0 + radius should not be larger that the image y-size or less than zero.

    \param[in] radius
	Represents the radius of the circle.  This radius will extend from x0, y0, and be used to make a circle.

    \param[in] color
	the value of the point or all pixels within the point.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	RadiusSizeErrorException

    \par Example
	drawing a point in a raster of radius 5 and color 100:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	ras->drawFilledCircle(20, 20, 5, 100);

	delete ras;
	delete img;
	delete file;

	}

	\endcode

	\par Details
	The radius will extend in all directions from the center x0 y0 pair, and a slice will be set using the size of that
	radius.  Pixel distance from circle is calculated using dx^2 + dy^2, where dx and dy are the difference from that
	pixel to the center of the circle.  If these distances are less than the radius, the pixel is lit up.

	radiussizeerror exception will be thrown if the radius is less than zero.
	rastersizeerror exception will be thrown if the circle is larger than the raster.

    */
 void drawFilledCircle(long int x0, long int y0, const double radius, const double color);













/** \brief add -- add two rasters

  Adds two rasters together.

  \see subtract, multiply, divide

  \param[in] r2
    The raster to add to this raster. Must have the same dimensions as this.

  \param[in] ras_out
    This raster will contain the sum of the two rasters.

  \returns
    None

  \par Exceptions
    RasterSizeErrorException -- raised when the two rasters have different sizes

  \par Example
    Stores the sum of ras1 and ras2 in the raster "sum"
  \code
    #include "geostar.hpp"
    #include <vector>
    int main() {
    GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
    GeoStar::Image *img = file->create_image("img");
    GeoStar::Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);
    GeoStar::Raster *ras2 = img->create_raster("ras2", GeoStar::INT8U, 100, 100);
    // fill ras1 and ras2 with data

    GeoStar::Raster * sum = img->create_raster("sum", GeoStar::INT8U, 100, 100);
    ras1->add(ras2, sum);
  }
  \endcode

  \par Details
  In order for add to work, this, r2, and ras_out must have the same dimensions. The function goes pixel by pixel and puts the value of this[i][j]+r2[i][j] in ras_out[i][j].
  */
  void add(const GeoStar::Raster * r2, GeoStar::Raster * ras_out);

  /** \brief subtract -- subtract a raster from this raster

    Subtracts a raster from this one.

    \see add, multiply, divide

    \param[in] r2
      The raster to subtract from this raster. Must have the same dimensions as this.

    \param[in] ras_out
      This raster will contain the difference between the two rasters.

    \returns
      None

    \Par Exceptions
      RasterSizeErrorException -- raised when the two rasters have different sizes

      \par Example
        Stores the difference of ras1 and ras2 in the raster "diff"
      \code
        #include "geostar.hpp"
        #include <vector>
        int main() {
        GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
        GeoStar::Image *img = file->create_image("img");
        GeoStar::Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);
        GeoStar::Raster *ras2 = img->create_raster("ras2", GeoStar::INT8U, 100, 100);
        // fill ras1 and ras2 with data

        GeoStar::Raster * diff = img->create_raster("diff", GeoStar::INT8U, 100, 100);
        ras1->subtract(ras2, diff);
      }
      \endcode

    \par Details
    In order for subtract to work, this, r2, and ras_out must have the same dimensions. The function goes pixel by pixel and puts the value of this[i][j]-r2[i][j] in ras_out[i][j].
    */
    void subtract(const GeoStar::Raster * r2, GeoStar::Raster * ras_out);

    /** \brief multiply -- multiply two rasters together pixel-by-pixel

      Multiplies two rasters together pixel-by-pixel.

      \see add, subtract, divide

      \param[in] r2
        The raster to multiply by. Must have the same dimensions as this.

      \param[in] ras_out
        This raster will contain the product of the two rasters.

      \returns
        None

      \Par Exceptions
        RasterSizeErrorException -- raised when the two rasters have different sizes

        \par Example
          Stores the product of ras1 and ras2 in the raster "prod"
        \code
          #include "geostar.hpp"
          #include <vector>
          int main() {
          GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
          GeoStar::Image *img = file->create_image("img");
          GeoStar::Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);
          GeoStar::Raster *ras2 = img->create_raster("ras2", GeoStar::INT8U, 100, 100);
          // fill ras1 and ras2 with data

          GeoStar::Raster * prod = img->create_raster("prod", GeoStar::INT8U, 100, 100);
          ras1->multiply(ras2, prod);
        }
        \endcode

      \par Details
      In order for multiply to work, this, r2, and ras_out must have the same dimensions. The function goes pixel by pixel and puts the value of this[i][j]*r2[i][j] in ras_out[i][j].
      */
      void multiply(const GeoStar::Raster * r2, GeoStar::Raster * ras_out);

      /** \brief divide -- divide this raster by another pixel-by-pixel

        Divide this rasters by another pixel-by-pixel.

        \see add, subtract, multiply

        \param[in] r2
          The raster to divide this by. Must have the same dimensions as this.

        \param[in] ras_out
          This raster will contain the quotient of the two rasters.

        \returns
          None

        \Par Exceptions
          RasterSizeErrorException -- raised when the two rasters have different sizes

          \par Example
            Stores the quotient of ras1 and ras2 in the raster "quot"
          \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar::Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);
            GeoStar::Raster *ras2 = img->create_raster("ras2", GeoStar::INT8U, 100, 100);
            // fill ras1 and ras2 with data

            GeoStar::Raster * quot = img->create_raster("quot", GeoStar::INT8U, 100, 100);
            ras1->divide(ras2, quot);
          }
          \endcode

        \par Details
        In order for divide to work, this, r2, and ras_out must have the same dimensions. The function goes pixel by pixel and puts the value of this[i][j]/r2[i][j] in ras_out[i][j].
        */
        void divide(const GeoStar::Raster * r2, GeoStar::Raster * ras_out);

        /** \brief resize -- resize this raster to desired dimensions

        This function resizes the given raster to the specified size

        \param[in] image
          The current GeoStar::Image must be passed to this function

        \param[in] resize_width
          The desired width of your raster

        \param[in] resize_height
          The desired height of your raster

        \returns
          The resized raster

        \Par Exceptions
          RasterSizeErrorException -- raised when the specified width or height are negative

        \Par Example

        \code
        #include <string>
#include <iostream>
#include <cstdint>
#include <vector>

#include "geostar.hpp"

#include "boost/filesystem.hpp"


int main() {

  std::vector<uint8_t> data(500*500);

  for(int j=0;j<500;++j) for(int i=0;i<500;++i) data[j*100+i]=(j*100+i)%255;

  // delete output file if already exists
  boost::filesystem::path p("resize.h5");
  boost::filesystem::remove(p);

  GeoStar::File *file = new GeoStar::File("resize.h5","new");

  GeoStar::Image *img = file->create_image("resize_1");

  img = file->open_image("resize_1");

  GeoStar::Raster *ras = img->create_raster("chan1", GeoStar::INT8U, 512, 1024);


  std::vector<long int> slice(4);
  slice[0]=10; slice [1]=15;
  slice[2]=500; slice[3]=500;
  ras->write(slice, data);

  ras->resize(img, 100, 200);

  delete ras;

  delete img;

  delete file;
        \endcode

        <table>
        <tr>
        <td>
        \imageSize{original_resized.png,height:300px;width:400px;}
        \image html original_resized.png "Before Resize"
        </td>
        <td>
        \imageSize{resized.png,height:60px;width:200px;}
        \image html resized.png "Resized Raster"
        </td>
        </tr>
        </table>

        */
        GeoStar::Raster* resize(GeoStar::Image *img, int resize_width, int resize_height);


        /** \brief getParent -- returns a pointer to this raster's parent Image

          This function returns a pointer to a this raster's parent Image.

          \see operator+, operator-, operator*, operator/

          \returns
            GeoStar::Image -- parent of this raster

          \par Exceptions
            None

          \par Example
          This function will make a new raster in the same image as the provided raster.
          \code
          #include "geostar.hpp"
          GeoStar::Raster * newChannel(GeoStar::Raster * ras1, std::string name) {
            GeoStar::Image * img = ras1->getParent();
            GeoStar::Raster * ras2 = new GeoStar::Raster(img,name,ras1->raster_datatype,ras1->get_nx(), ras1->get_ny());
            return ras2;
          }
          \endcode

          \par Details
          This function uses getFileName() to retrieve the raster's file, and gets the full file path in string form fo access the image name using substring operations. With both names, we can access an existing file and then access an existing image in that file.
          */
        GeoStar::Image * getParent();

        /** \brief operator+ -- add two rasters

          Returns a raster pointer containing the sum of two rasters.

          \see add

          \param[in] r2
            The raster to add to this raster. Must have the same dimensions as this.

          \returns
            A GeoStar::Raster pointer which contains the sum of the two rasters.

          \par Exceptions
            RasterSizeErrorException -- raised when the two rasters have different sizes

          \par Example
            Creates a 100x100 raster full of 1s, then makes ras2 point to the sum of that raster with itself.
          \code
          #include "geostar.hpp"
          #include <vector>
          int main() {
          GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
          GeoStar::Image *img = file->create_image("img");
          GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

          std::vector<uint8_t> data(100*100);
          for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=1;
          std::vector<long int> slice(4);
          slice[0]=0; slice [1]=0;
          slice[2]=100; slice[3]=100;
          ras1->write(slice, data);

          GeoStar::Raster * ras2 = *ras1 + *ras1;
          }
          \endcode

          \par Details
            This function uses getParent() to be able to make a new raster in "this" raster's Image. It then uses the add() function on that raster to store the summed rasters.
          */
        GeoStar::Raster * operator+(const GeoStar::Raster & r2);

        /** \brief operator- -- subtract a raster from another

          Returns a raster pointer containing the difference between this raster and r2.

          \see subtract

          \param[in] r2
            The raster to subtract from this raster. Must have the same dimensions as this.

          \returns
            A GeoStar::Raster pointer which contains the difference between the two rasters.

          \Par Exceptions
            RasterSizeErrorException -- raised when the two rasters have different sizes

            \Par Example
              Creates a 100x100 raster full of 1s, then makes ras2 point to a raster of zeroes.
            \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

            std::vector<uint8_t> data(100*100);
            for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=1;
            std::vector<long int> slice(4);
            slice[0]=0; slice [1]=0;
            slice[2]=100; slice[3]=100;
            ras1->write(slice, data);

            GeoStar::Raster * ras2 = *ras1 - *ras1;
            }
            \endcode

            \par Details
              This function uses getParent() to be able to make a new raster in "this" raster's Image. It then uses the subtract() function on that raster to store the difference between the rasters.
            */
        GeoStar::Raster * operator-(const GeoStar::Raster & r2);

        /** \brief operator* -- multiply a raster by another

          Returns a raster pointer containing the product of this raster and r2.

          \see multiply

          \param[in] r2
            The raster to multiply this raster with. Must have the same dimensions as this.

          \returns
            A GeoStar::Raster pointer which contains the product of the two rasters.

          \Par Exceptions
            RasterSizeErrorException -- raised when the two rasters have different sizes

            \Par Example
              Creates a 100x100 raster full of 2s, then makes ras2 point to a raster of 4s.
            \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

            std::vector<uint8_t> data(100*100);
            for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=2;
            std::vector<long int> slice(4);
            slice[0]=0; slice [1]=0;
            slice[2]=100; slice[3]=100;
            ras1->write(slice, data);

            GeoStar::Raster * ras2 = *ras1 * *ras1;
            }
            \endcode

            \par Details
              This function uses getParent() to be able to make a new raster in "this" raster's Image. It then uses the multiply() function on that raster to store the product of the rasters.
            */
        GeoStar::Raster * operator*(const GeoStar::Raster & r2);

        /** \brief operator/ -- divide a raster by another

          Returns a raster pointer containing the quotient of this raster and r2.

          \see divide

          \param[in] r2
            The raster to divide this raster by. Must have the same dimensions as this.

          \returns
            A GeoStar::Raster pointer which contains the quotient of the two rasters.

          \Par Exceptions
            RasterSizeErrorException -- raised when the two rasters have different sizes

            \Par Example
              Creates a 100x100 raster full of 2s, then makes ras2 point to a raster of 1s, a raster divided by itself.
            \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

            std::vector<uint8_t> data(100*100);
            for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=2;
            std::vector<long int> slice(4);
            slice[0]=0; slice [1]=0;
            slice[2]=100; slice[3]=100;
            ras1->write(slice, data);

            GeoStar::Raster * ras2 = *ras1 / *ras1;
            }
            \endcode

            \par Details
              This function uses getParent() to be able to make a new raster in "this" raster's Image. It then uses the divide() function on that raster to store the quotient of the rasters.
            */
        GeoStar::Raster * operator/(const GeoStar::Raster & r2);

        /** \brief operator+ -- add a constant to a raster

          Returns a raster pointer containing the raster with a constant value added to each pixel

          \see operator+

          \param[in] val
            The float value to add to each pixel in the raster.

          \returns
            A GeoStar::Raster pointer which contains the raster with a constant value added to each pixel

          \Par Exceptions
            None

            \Par Example
              This block of code intializes a raster where each pixel has a value of 2, then generates a new raster where each pixel of ras1 has 4 added to it.
            \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

            std::vector<uint8_t> data(100*100);
            for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=2;
            std::vector<long int> slice(4);
            slice[0]=0; slice [1]=0;
            slice[2]=100; slice[3]=100;
            ras1->write(slice, data);

            GeoStar::Raster * ras2 = *ras1 + 4;
            }
            \endcode

            \par Details
              This function uses getParent() to be able to make a new raster in "this" raster's Image. It then iterates through every pixel and adds the value to each.
            */
        GeoStar::Raster * operator+(const float & val);

        /** \brief operator- -- subtract a constant from a raster

          Returns a raster pointer containing the raster with a constant value subtracted from each pixel

          \see operator-

          \param[in] val
            The float value to subtract from each pixel in the raster.

          \returns
            A GeoStar::Raster pointer which contains the raster with a constant value subtracted from each pixel

          \Par Exceptions
            None

            \Par Example
              This block of code intializes a raster where each pixel has a value of 2, then generates a new raster where each pixel of ras1 has 1 subtracted from it.
            \code
            #include "geostar.hpp"
            #include <vector>
            int main() {
            GeoStar::File *file = new GeoStar::File("arithmetic.h5","new");
            GeoStar::Image *img = file->create_image("img");
            GeoStar:: Raster *ras1 = img->create_raster("ras1", GeoStar::INT8U, 100, 100);

            std::vector<uint8_t> data(100*100);
            for(int j=0;j<100;++j) for(int i=0;i<100;++i) data[j*100+i]=2;
            std::vector<long int> slice(4);
            slice[0]=0; slice [1]=0;
            slice[2]=100; slice[3]=100;
            ras1->write(slice, data);

            GeoStar::Raster * ras2 = *ras1 - 1;
            }
            \endcode

            \par Details
              This function uses getParent() to be able to make a new raster in "this" raster's Image. It then iterates through every pixel and adds the value to each.
            */
        GeoStar::Raster * operator-(const float & val);
        GeoStar::Raster * operator*(const float & val);
        GeoStar::Raster * operator/(const float & val);











/** \brief bitShift -- shifts the bits in an image right or left

    Writes to a new raster that has been bitshifted i bits to the right or left, in order to decrease or increase image contrast.

    \see read, write

    \param[out] rasterOut
	This is the raster object to which the bitshifted data will be written to.  The original image will remain unchanged.

    \param[in] bits
	The number of bits you wish to shift the image.

    \param[in] direction
	This is a string that denotes which direction you want to bitshift the image.  specify true to bitshift right,
	or false to bitshift left.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	BitException

    \par Example
	bitshifting the test raster 4 bits to the right:

	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	GeoStar::Raster *ras2 = new GeoStar::Raster(img, "testOut", GeoStar::REAL32, 200, 200);

	ras->bitshift(ras2, 4, 1);

	delete ras;
	delete ras2;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{beforeImg.png,height:150px;width:150px;}
        \image html beforeImg.png "Before Line"
        </td>
        <td>
        \imageSize{afterBitshiftRight.png,height:150px;width:150px;}
        \image html afterBitshiftRight.png "After bitshift right"
        </td>
	<td>
        \imageSize{afterBitshiftLeft.png,height:150px;width:150px;}
        \image html afterBitshiftLeft.png "After bitshift left"
        </td>
        </tr>
        </table>

	\par Details
	bitShifting to the right will create a raster with decreased contrast, shifting to the left increases contrast.

	rastersizeerror exception will be thrown if your rasterOut is not the same size as the original raster
	bitvalueerror exception will be thrown if you try to bitshift a negative number of bits.

    */
  void bitShift(Raster *rasterOut, int bits, bool direction);

/** \brief addSaltPepper -- adds salt and pepper noise to a raster

    writes to an output raster adding salt and pepper noise to a raster, corrupting it with a probability denoted by low.
	see parameters for more information.

    \see read, write

    \param[out] rasterOut
	This is the raster object to which the bitshifted data will be written to.  The original image will remain unchanged.

    \param[in] low
	this is the low probability threshhold for the raster, and it must be a value between 0 and 0.5.  Whatever the low
	threshhold is, the sum of the low and high threshholds must equal 1.  Values will be randomly selected between 0 and 1 for
	each pixel in the image, and if the random value is less than or equal to the low threshhold the pixel is set to 0.
	The high probability threshhold will be calculated from this value, and if the random value is greater than or equal to the high 	threshhold the pixel is set to 15000 by default.


    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	ProbabilityException

    \par Example
	adding salt and pepper noise to the test raster with a probability of 0.05:

	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	GeoStar::Raster *ras2 = new GeoStar::Raster(img, "testOut", GeoStar::REAL32, 200, 200);

	ras->addSaltPepper(ras2, 0.05);

	delete ras;
	delete ras2;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{beforeImg.png,height:150px;width:150px;}
        \image html beforeImg.png "Before adding Salt and Pepper"
        </td>
        <td>
        \imageSize{afterSaltPepper.png,height:150px;width:150px;}
        \image html afterSaltPepper.png "After adding salt and pepper"
        </td>
        </tr>
        </table>

	\par Details

	rastersizeerror exception will be thrown if your rasterOut is not the same size as the original raster.
	probability exception will be thrown if low is <0 or >0.5.
	High end value will be defaulted to 15000, if a higher value is needed some image preprocessing may be required.
	high end value will be between 0.5 and 1, and high + low must equal 1.
    */
  void addSaltPepper(Raster *rasterOut, const double low);

/** \brief autoLocalThresh -- threshholds a raster in chunks with an automatic threshhold

    writes to an output raster threshholding the input raster in sectors defined as squares of a user-set diameter.
	Automatically sets a threshhold by finding the average between local min and max in said sector, and progresses
	through image until all areas possible are threshholded.

    \see read, write

    \param[out] rasterOut
	This is the raster object to which the threshholded data will be written to.  The original image will remain unchanged.

    \param[in] partitions
	The image will be divided into partitions before threshholding, and this defines the number of partitions for each row and col, then 	     each distinct thresh.  So for example a 5 partition function call will have 25 total sectors, and in each of those a threshholding 		operation will occur.
	Partitions should not be less than or equal to zero or greater than 150 - as partitions grow the more computationally expensive this 		function becomes. As such, a hard cap of 150 is imposed.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException
	PartitionException

    \par Example
	threshholding into ten partitions across ras:

	\code
	#include "Geostar.hpp"
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 200, 200);

	GeoStar::Raster *ras2 = new GeoStar::Raster(img, "testOut", GeoStar::REAL32, 200, 200);

	ras->autoLocalThresh(ras2, 10);

	delete ras;
	delete ras2;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{beforeImg.png,height:150px;width:150px;}
        \image html beforeImg.png "Before local threshholding"
        </td>
        <td>
        \imageSize{afterlocalthresh5.png,height:150px;width:150px;}
        \image html afterlocalthresh5.png "After - 5 partitions"
        </td>
        <td>
        \imageSize{afterlocalthresh25.png,height:150px;width:150px;}
        \image html afterlocalthresh25.png "After - 25 partitions"
        </td>
        <td>
        \imageSize{afterlocalthresh50.png,height:150px;width:150px;}
        \image html afterlocalthresh50.png "After - 50 partitions"
        </td>
        <td>
        \imageSize{afterlocalthresh100.png,height:150px;width:150px;}
        \image html afterlocalthresh100.png "After - 100 partitions"
        </td>
        </tr>
        </table>

	\par Details

	rastersizeerror exception will be thrown if your rasterOut is not the same size as the original raster.
	partitionerror exception will be thrown if the partitions are less than or equal to zero or greater than 150.

	The threshholding formula is very flexible - at the moment it just computes the average between the max and min
	of a particular sector, but in reality the threshhold could be any characteristic of the sector data.
	The threshholding tends to get less strict as the number of partitions goes up as well - more partitions means the
	threshhold of each sector tends to be lower and more accurate for that small area.  A good way to view this function is that
	partitions is just a threshhold parameter, and the higher the partitions the lower the threshhold.
    */
  void autoLocalThresh(Raster *rasterOut, const int partitions);



/** \brief FFT_2D -- Performs a two-dimensional Fast Fourier Transform

    writes to two output rasters, one for the real part of the FFT output and one for the imaginary part.  Takes in real data
	from the raster this is called on, transforms across rows to two buffer rasters, and then transforms across cols to
	the final output rasters.  The output rasters will be one dimensional, starting at (0,0) in the output rasters and
	stretching horizontally across in the x-direction. 

    \see read, write, FFT_2D_Inv

    \param[in] img
	The image object you wish to use to create your buffer rasters in.  Typically should be the same image that your input/output
	rasters are stored in.

    \param[out] rasOutReal
	This is the raster object to which the real part of the FFT data will be written to.  The original image will remain unchanged.

    \param[out] rasOutImg
	This is the raster object to which the img part of the FFT data will be written to.  The original image will remain unchanged.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException

    \par Example
	Performing a FFT on ras, outputting to two rasters:

	\code
	#include "Geostar.hpp"
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasReal = new GeoStar::Raster(img, "Real", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasImg = new GeoStar::Raster(img, "Imaginary", GeoStar::REAL32, 1024, 1024);

	ras->FFT_2D(img, rasReal, rasImg);

	delete ras;
	delete rasReal;
	delete rasImg;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{OutputRealFFT.png,height:150px;width:150px;}
        \image html OutputRealFFT.png "After FFT with a sinusoidal signal - real part"
        </td>
        <td>
        \imageSize{OutputImaginaryFFT.png,height:150px;width:150px;}
        \image html OutputImaginaryFFT.png "After FFT with a sinusoidal signal - imaginary part"
        </td>
        </tr>
        </table>

	\par Details

	rastersizeerror exception will be thrown if your rasOutReal and rasOutImg are not the same size as your input raster.

	For now, the raster size is hard set at 1024x1024.  This is because when doing the col by col transform, the function can run
	extremely slow as image x size grows.  Thus, ensure your output rasters are also size 1024x1024.

	The FFT will perform a complex to complex transfer - the real part of the complex numbers will be data from the raster this 		function is called on, but the imaginary part will be set to all zeros.  Data is read line by line and 1D transforms are performed
	row by row to real and imaginary buffer rasters, then column by column from the buffers to the output rasters.
    */
  void FFT_2D(GeoStar::Image *img, Raster *rasOutReal, Raster *rasOutImg);

/** \brief FFT_2D_Inv -- Performs a two-dimensional Inverse Fast Fourier Transform

    writes to one output raster for the real output.  Takes in real data from the raster this is called on, and imaginary data 
	from the rasInImg parameter, transforms across cols to two buffer rasters, and then transforms across rows to the final output raster.

    \see read, write, FFT_2D

    \param[in] img
	The image object you wish to use to create your buffer rasters in.  Typically should be the same image that your input/output
	rasters are stored in.

    \param[out] rasOut
	This is the raster object to which the real part of the InvFFT data will be written to.  The original image will remain unchanged.

    \param[in] rasInImg
	This is the raster object from which the img part of the FFT data will be read.  Real data comes from the raster this function is
	called on.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException

    \par Example
	Performing an InvFFT on ras, outputting to a raster:

	\code
	#include "Geostar.hpp"
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *rasOut = new GeoStar::Raster(img, "out", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasReal = new GeoStar::Raster(img, "Real", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasImg = new GeoStar::Raster(img, "Imaginary", GeoStar::REAL32, 1024, 1024);

	rasReal->FFT_2D_Inv(img, rasOut, rasImg);

	delete rasOut;
	delete rasReal;
	delete rasImg;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{OutputRealFFT.png,height:150px;width:150px;}
        \image html OutputRealFFT.png "Before InvFFT with a sinusoidal signal - real part"
        </td>
        <td>
        \imageSize{OutputImaginaryFFT.png,height:150px;width:150px;}
        \image html OutputImaginaryFFT.png "Before InvFFT with a sinusoidal signal - imaginary part"
        </td>
        <td>
        \imageSize{afterFFTInv.png,height:150px;width:150px;}
        \image html afterFFTInv.png "After InvFFT with a sinusoidal signal - real data"
        </td>
        </tr>
        </table>


	\par Details

	rastersizeerror exception will be thrown if your rasOut and rasImg are not the same size as your input raster.

	For now, the raster size is hard set at 1024x1024.  This is because when doing the col by col transform, the function can run
	extremely slow as image x size grows.  Thus, ensure your output rasters are also size 1024x1024.

	The FFT will perform a complex to complex transfer - the real part of the complex numbers will be data from the raster this 		function is called on, and imaginary from the rasImg parameter.  Data is read line by line and 1D transforms are performed
	col by col to real and imaginary buffer rasters, then row by row from the buffers to the output raster.
    */
  void FFT_2D_Inv(GeoStar::Image *img, Raster *rasOut, Raster *rasInImg);

/** \brief LowPassFilter -- Performs a low-pass filter on an image

    writes to an output raster, performing a set of Fourier transforms by row and col on the input raster, setting a square of all zeros
	in the FFT output rasters, and transforming back to real data.  

    \see read, write, FFT_2D, set

    \param[in] img
	The image object you wish to use to create your buffer rasters in.  Typically should be the same image that your input/output
	rasters are stored in.

    \param[in] rasInReal
	This is the raster object to which the real part of the FFT data will be written to.  The original image will remain unchanged.

    \param[in] rasInImg
	This is the raster object to which the img part of the FFT data will be written to.  The original image will remain unchanged.

    \param[out] rasOutReal
	This is the raster object the final inverse FFT transform data will be written to.  The original image will remain unchanged.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException

    \par Example
	using a low-pass filter on ras, outputting to a single real raster:

	\code
	#include "Geostar.hpp"
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasReal = new GeoStar::Raster(img, "Real", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasImg = new GeoStar::Raster(img, "Imaginary", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasOut = new GeoStar::Raster(img, "Output", GeoStar::REAL32, 1024, 1024);

	ras->lowPassFilter(img, rasReal, rasImg, rasOut);

	delete ras;
	delete rasReal;
	delete rasImg;
	delete rasOut;
	delete img;
	delete file;

	}

	\endcode

	\par Details

	rastersizeerror exception will be thrown if your rasinReal, rasinImg, and rasOut are not the same size as your input raster.

	For now, the raster size is hard set at 1024x1024.  This is because when doing the col by col transform, the function can run
	extremely slow as image x size grows.  Thus, ensure your output rasters are also size 1024x1024.

	The FFT will perform a complex to complex transfer - the real part of the complex numbers will be data from the raster this 		function is called on, but the imaginary part will be set to all zeros.  Data is read line by line and 1D transforms are performed
	row by row to real and imaginary buffer rasters, then column by column from the buffers to the output rasters.
	A square with width hard set to 1/3 the input raster's size is created in the center of the real and imaginary rasters using
	set(), and then this data is inversely transformed using the FFT once more.
    */
  void lowPassFilter(GeoStar::Image *img, Raster *rasInReal, Raster *rasInImg, Raster *rasOut);


/** \brief contrastCorrection - modifies the contrast of an image

	Used to enhance the visual appearance of an image.
    Writes to an output raster, using the formula pixelOut = contrast * (pixelIn - average) + average, where average is the mean value
	of the pixels in the original image.  Must be used with graylevel images, and does not modify the original raster.

    \see read, write

    \param[out] rasOut
	This is the raster object to which the contrast corrected data will be written to.  Must be same size as original raster.

    \param[in] contrast
	The scalar by which you multiply your pixels to get the final contrast corrected pixel.

    \returns
	nothing

    \par Exceptions
	RasterSizeErrorException

    \par Example
	correcting the contrast on ras by a factor of 2, outputting to a single real raster:

	\code
	#include "Geostar.hpp"
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras = new GeoStar::Raster(img, "test", GeoStar::REAL32, 1024, 1024);

	GeoStar::Raster *rasOut = new GeoStar::Raster(img, "Output", GeoStar::REAL32, 1024, 1024);

	ras->contrastCorrection(rasOut, 2);

	delete ras;
	delete rasOut;
	delete img;
	delete file;

	}

	\endcode

	<table>
        <tr>
        <td>
        \imageSize{BeforeImg.png,height:125px;width:125px;}
        \image html BeforeImg.png "Before Contrast Correction"
        </td>
        <td>
        <td>
        \imageSize{aftercontrastcorrectionOG_table.png,height:125px;width:125px;}
        \image html aftercontrastcorrectionOF_table.png "Before Contrast Correction - table version"
        </td>
        <td>
        \imageSize{aftercontrastcorrection25.png,height:125px;width:125px;}
        \image html aftercontrastcorrection25.png "After Contrast Correction with contrast = 0.25"
        </td>
        <td>
        \imageSize{aftercontrastcorrection25_table.png,height:125px;width:125px;}
        \image html aftercontrastcorrection25_table.png "After Contrast Correction with contrast = 0.25 - table version"
        </td>
        </tr>
        </table>

	\par Details

	rastersizeerror exception will be thrown if your rasOut is not the same size as your input raster.
	Any values that are below zero after being corrected will be set to zero, then written to the output raster.

	The program reads line by line into a vector, adding all values to one sum.  It then divides the sum by total
	number of pixels to get the average.  Then the image is looped through again and read line by line, applying
	the formula described in the brief to each pixel.  At the end of this loop, the new pixel data is written line by line
	into the output raster. 
    */
  void contrastCorrection(Raster *rasOut, const double contrast);



















       
#include "Raster_bitmap.hpp"
#include "Raster_flip.hpp"
#include "Raster_histogram.hpp"
#include "Raster_minmax.hpp"
#include "Raster_polygon.hpp"
#include "Raster_reproject.hpp"
#include "Raster_rotate.hpp"
#include "Raster_scale.hpp"
#include "Raster_set.hpp"
#include "Raster_warp.hpp"

  }; // end class: Raster
  
}// end namespace GeoStar


#endif //RASTER_HPP_
