// Raster_warp.hpp
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

      // oldwarp reads entire image, does not use tiles ....  Save for comparison with regular warp (warpType()).
      Raster* oldwarp(const WarpParameters warpData, const Slice &in, const Slice &out, Raster *outRaster);

      template <typename T>
      Raster* warpType(const WarpParameters warpInfo, const Slice &in, const Slice &out, Raster *outRaster);
      

  public:


      // Save this, even though "rotate(angle,in)" is faster ... for possible future testing
      /** \brief Raster:rotateWithWarp create a new raster, that is a rotated version of this raster.
       
       Raster::rotateWithWarp creates a rotated version of this raster as output.  This method is used to rotate a portion
       of this raster to fit into a new Raster.  Information describing both the angle and the rectangular
       portion of this raster to rotate, is specified.  NOTE that this is similar to Raster:rotate, but is slower.
       This function is not recommended for general use, but has been left available for further testing.
       
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
       GeoStar::Raster *rotatedRaster = ras->rotateWithWarp(angleInRadians, inSlice);
       
       }//end-main
       \endcode
       
       */
      
      // Save this for testing later .... make sure still slower than specialized rotate()
      Raster* rotateWithWarp(const float angle, const Slice &in);

      // warp #1:  calls warp #6
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs);
      // warp #2:  calls warp #6
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inslice);
      // warp #3:  calls warp #6
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const std::string &name);
      // warp #4:  calls warp #6
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inslice, const std::string &name);
      // warp #5:  calls warp #6
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, Raster *rasNew);
      // warp #6:  calls warp(warpInfo, inslice, outslice, rasNew)
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice inSlice, Raster *rasNew);
      // warp #7:  calls warp #8
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice, const Slice &outSlice, const std::string &name);
      // warp #8:  calls (warpInfo, inslice, outslice, rasNew)
      Raster* warp(std::vector<Point> inputGCPs, std::vector<Point> outputGCPs, const Slice &inSlice, const Slice &outSlice, Raster *rasNew);



  /** \brief Raster:warp create a new raster, that is a warped version of this raster.

   Raster::warp creates a warped raster as output.
   The warp method has multiple function signatures.

   \see Raster, Slice, WarpParameters, TileIO
   
   \param[in] warp-data
       The warp-related data - class type WarpParameters.
   \param[in] inSlice
       The input slice information - class type Slice.
   \param[in] outSlice
       The output slice information - class type Slice.
   \param[in] newRaster
       The new Raster to write this warped raster to

   \returns
       new warped Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to warp a raster named "ex1" using a set of 3 ground control points.
       First open the file and the raster.  Then create a WarpParameters object, using the set of
       known ground control points.  Finally, call warp with the WarpParameters object, the input
       slice of the current raster to warp, and the output slice (portion) of the new raster to
       write the warped raster data to.
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
   
       GeoStar::Raster *newRas;
       try {
               newRas = img->create_raster("ex1Warped", GeoStar::INT8U, 100, 200)
       }//end-try
       catch (...) {
           cerr << "GeoStar::create_raster failure"<<endl;
       }//end-catch
   
           Slice inSlice(0,0,40,50);
           Slice outSlice(0,0,100,200);
           GeoStar::WarpParameters warpData;
           std::vector<double> rix, riy, rox, roy;
           rix[0] = 5; rix[1] = 30; rix[2] = 16;
           riy[0] = 7; riy[1] = 22; riy[2] = 9;
           rox[0] = 12; rox[1] = 51; rox[2] = 68;
           roy[0] = 22; roy[1] = 35; roy[2] = 53;
           int order = 1;
           double rmsx = 0.0, rmsy = 0.0;
           warpData.GCPregression(rix, riy, rox, roy, &order, &rmsx, &rmsy);
           newRas = ras->warp(warpData, inSlice, outSlice, newRas);

       }//end-main
       \endcode

  */
      Raster* warp(const WarpParameters warpData, const Slice &in, const Slice &out, Raster *outRaster);



