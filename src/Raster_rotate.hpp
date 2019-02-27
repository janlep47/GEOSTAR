// Raster_rotate.hpp
//
// by Janice Richards, Feb 20, 2018
//
//----------------------------------------

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


  private:

      template <typename T>
      Raster* rotateType(const float angle, const Slice &in, const Slice &out, Raster *outRaster);
      

  public:

      
      /** \brief Raster:rotate create a new raster, that is a rotated version of this raster.
       
       Raster::rotate creates a rotated version of this raster as output.  This method is used to rotate this
       raster to fit into the previously created new Raster.  It is assumed that the new Raster has its width
       and height already set to the desired dimensions.  In addition, information describing the rectangular
       portion of this raster to scale, is also specified.
       
       \see Raster, RasterType, Slice, TileIO
       
       \param[in] angle
       The angle to rotate the (entire) input raster, in radians.
       
       \returns
       new rotated Raster.
       
       \par Exceptions
       Exceptions that may be raised by this method:
       None.
       
       \par Example
       Let's say a user wants to rotate an a raster named "ex1"
       by 30 degrees.  First open the file and the raster, then use rotate():
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
       
       GeoStar::Raster *ras;
       try {
       ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
       cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
       
       double angle = 30.0;
       double angleInRadians = angle*PI/180;
       GeoStar::Raster *rotatedRaster = ras->rotate(angleInRadians);
       
       }//end-main
       \endcode
       
       */
      Raster* rotate(const float angle);
      
      /** \brief Raster:rotate create a new raster, that is a rotated version of this raster.
       
       Raster::rotate creates a rotated version of this raster as output.  This method is used to rotate a portion
       of this raster to fit into a new Raster.  Information describing both the angle and the rectangular
       portion of this raster to rotate, is specified.
       
       \see Raster, RasterType, Slice, TileIO
       
       \param[in] angle
       The angle to rotate the (entire) input raster, in radians.
       \param[in] inSlice
       The input slice information - class type Slice.

       \returns
       new rotated Raster.
       
       \par Exceptions
       Exceptions that may be raised by this method:
       None.
       
       \par Example
       Let's say a user wants to rotate the upper left portion (50 X 50 pixels) of a raster named "ex1"
       by 30 degrees.  First open the file and the raster, then use rotate():
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
       
       GeoStar::Raster *ras;
       try {
       ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
       cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
       
       double angle = 30.0;
       double angleInRadians = angle*PI/180;
       Slice inSlice(0,0,50,50);
       GeoStar::Raster *rotatedRaster = ras->rotate(angleInRadians, inSlice);
       
       }//end-main
       \endcode
       
       */
      Raster* rotate(const float angle, const Slice &in);
      Raster* rotate(const float angle, Raster *outRaster);
      Raster* rotate(const float angle, const Slice &in, Raster *outRaster);
      Raster* rotate(const float angle, const Slice &in, const Slice &out, Raster *outRaster);
      //Raster* rotateType(const float angle, const Slice &in, const Slice &out, Raster *outRaster);


