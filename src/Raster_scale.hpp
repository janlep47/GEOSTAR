// Raster_scale.hpp
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

  public:

  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the scale
   factor for both the x and y directions, for the new scaled raster.

   \see Raster, TileIO
   
   \param[in] x-ratio
       The scale factor, in the x-direction.
   \param[in] y-ratio
       The scale factor, in the y-direction.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale a raster named "ex1" to half it's size.
       First open the file and the raster, then use scale():
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
   
           GeoStar::Raster *newRas = ras->scale(0.5, 0.5);

       }//end-main
       \endcode

  */
      Raster* scale(const double &xratio, const double &yratio);

  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the number
   of pixels for both the x and y directions, for the new scaled raster.

   \see Raster, TileIO
   
   \param[in] nx
       The number of pixels in the x-direction, of the new scaled raster.
   \param[in] ny
       The number of pixels in the y-direction, of the new scaled raster.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale a raster named "ex1" to 100 X 200 pixels.
       First open the file and the raster, then use scale():
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
   
           GeoStar::Raster *newRas = ras->scale(100, 200);

       }//end-main
       \endcode

  */
      Raster* scale(const long int &nx, const long int &ny);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the scale
   factor for both the x and y directions, in addition to the type of the scaled raster, and the name to use,
   for the scaled raster.

   \see Raster, RasterType, TileIO
   
   \param[in] x-ratio
       The scale factor, in the x-direction.
   \param[in] y-ratio
       The scale factor, in the y-direction.
   \param[in] type
       The raster type of the new scaled raster
   \param[in] name
       The name of the new scaled raster

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale a raster named "ex1" to half its width and three fourths of its
       height, creating the new raster with an unsigned int 8-bit type, and a name of "ex1Scaled".
       First open the file and the raster, then use scale():
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
   
           GeoStar::Raster *newRas = ras->scale(0.5, 0.75, GeoStar::INT8U, "ex1Scaled");

       }//end-main
       \endcode

  */
      Raster* scale(const double &xratio, const double &yratio, const RasterType &type, const std::string &name);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the
   number of pixels in the x and y directions, in addition to the type of the scaled raster, and the name
   to use for the scaled raster.

   \see Raster, RasterType, TileIO
   
   \param[in] nx
       The number of pixels in the x-direction, of the new scaled raster.
   \param[in] ny
       The number of pixels in the y-direction, of the new scaled raster.
   \param[in] type
       The raster type of the new scaled raster
   \param[in] name
       The name of the new scaled raster

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale a raster named "ex1" to 100 X 200 pixels, creating the new raster
       with an unsigned int 8-bit type, and a name of "ex1Scaled".
       First open the file and the raster, then use scale():
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
   
           GeoStar::Raster *newRas = ras->scale(100, 200, GeoStar::INT8U, "ex1Scaled");

       }//end-main
       \endcode

  */
      Raster* scale(const long int &nx, const long int &ny, const RasterType &type, const std::string &name);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to scale this
   raster to fit into the previously created new Raster.  It is assumed that the new Raster has its width
   and height already set to the desired dimensions.

   \see Raster, RasterType, TileIO
   
   \param[in] rasNew
       The new raster to scale this raster into.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale a raster named "ex1" to 100 X 200 pixels, creating the new raster
       with an unsigned int 8-bit type, and a name of "ex1Scaled".
       First open the file and the raster, then use scale():
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
               newRas = img->create_raster("ex1Scaled", GeoStar::INT8U, 100, 200)
       }//end-try
       catch (...) {
           cerr << "GeoStar::create_raster failure"<<endl;
       }//end-catch
   
           newRas = ras->scale(newRas);

       }//end-main
       \endcode

  */
      Raster* scale(GeoStar::Raster *rasNew);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the scale
   factor for both the x and y directions for the new scaled raster, along with information of the
   rectangular portion of this raster to scale.

   \see Raster, Slice, TileIO
   
   \param[in] inSlice
       The slice information: specifies the upper left position of this raster, along with the width
       and height of this slice, to use as the data to scale.
   \param[in] x-ratio
       The scale factor, in the x-direction.
   \param[in] y-ratio
       The scale factor, in the y-direction.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1" to
       half it's size.  First open the file and the raster, then use scale():
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
           Slice inSlice(0,0,40,50);
           GeoStar::Raster *newRas = ras->scale(inSlice, 0.5, 0.5);

       }//end-main
       \endcode

  */
      Raster* scale(const Slice &in, const double &xratio, const double &yratio);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the number
   of pixels for both the x and y directions for the new scaled raster, along with information of the
   rectangular portion of this raster to scale.

   \see Raster, Slice, TileIO
   
   \param[in] inSlice
       The slice information: specifies the upper left position of this raster, along with the width
       and height of this slice, to use as the data to scale.
   \param[in] nx
       The number of pixels in the x-direction, of the new scaled raster.
   \param[in] ny
       The number of pixels in the y-direction, of the new scaled raster.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1" to
       half it's size.  First open the file and the raster, then use scale():
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
           Slice inSlice(0,0,40,50);
           GeoStar::Raster *newRas = ras->scale(inSlice, 0.5, 0.5);

       }//end-main
       \endcode

  */
      Raster* scale(const Slice &in, const long int &nx, const long int &ny);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the scale
   factor for both the x and y directions for the new scaled raster, along with information of the
   rectangular portion of this raster to scale.  In addition, the type of the scaled raster, and the name
   to use, is also specified for the scaled raster.

   \see Raster, Slice, TileIO
   
   \param[in] inSlice
       The slice information: specifies the upper left position of this raster, along with the width
       and height of this slice, to use as the data to scale.
   \param[in] x-ratio
       The scale factor, in the x-direction.
   \param[in] y-ratio
       The scale factor, in the y-direction.
   \param[in] type
       The raster type of the new scaled raster
   \param[in] name
       The name of the new scaled raster

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1" to
       half it's size, creating the new raster with an unsigned int 8-bit type, and a name of "ex1Scaled".
       First open the file and the raster, then use scale():
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

           Slice inSlice(0,0,40,50);
           GeoStar::Raster *newRas = ras->scale(inSlice, 0.5, 0.5, GeoStar::INT8U, "ex1Scaled");

       }//end-main
       \endcode

  */
      Raster* scale(const Slice &in, const double &xratio, const double &yratio, const RasterType &type, const std::string &name);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to give the number
   of pixels for both the x and y directions for the new scaled raster, along with information of the
   rectangular portion of this raster to scale.  In addition, the type of the scaled raster, and the name
   to use, is also specified for the scaled raster.

   \see Raster, Slice, TileIO
   
   \param[in] inSlice
       The slice information: specifies the upper left position of this raster, along with the width
       and height of this slice, to use as the data to scale.
   \param[in] nx
       The number of pixels in the x-direction, of the new scaled raster.
   \param[in] ny
       The number of pixels in the y-direction, of the new scaled raster.
   \param[in] type
       The raster type of the new scaled raster
   \param[in] name
       The name of the new scaled raster

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1" to
       100 X 200 pixels, creating the new raster with an unsigned int 8-bit type, and a name of "ex1Scaled".
       First open the file and the raster, then use scale():
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

           Slice inSlice(0,0,40,50);
           GeoStar::Raster *newRas = ras->scale(inSlice, 100, 200, GeoStar::INT8U, "ex1Scaled");

       }//end-main
       \endcode

  */
      Raster* scale(const Slice &in, const long int &nx, const long int &ny, const RasterType &type, const std::string &name);
  /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to scale this
   raster to fit into the previously created new Raster.  It is assumed that the new Raster has its width
   and height already set to the desired dimensions.  In addition, information describing the rectangular
   portion of this raster to scale, is also specified.

   \see Raster, RasterType, Slice, TileIO
   
   \param[in] inSlice
       The slice information: specifies the upper left position of this raster, along with the width
       and height of this slice, to use as the data to scale.
   \param[in] rasNew
       The new raster to scale this raster into.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1"
       to 100 X 200 pixels, creating the new raster with an unsigned int 8-bit type, and a name of
       "ex1Scaled". First open the file and the raster, then use scale():
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
               newRas = img->create_raster("ex1Scaled", GeoStar::INT8U, 100, 200)
       }//end-try
       catch (...) {
           cerr << "GeoStar::create_raster failure"<<endl;
       }//end-catch
   
           Slice inSlice(0,0,40,50);
           newRas = ras->scale(inSlice, newRas);

       }//end-main
       \endcode

  */
      Raster* scale(const Slice &in, Raster *outRaster);
      
   /** \brief Raster:scale create a new raster, that is a scaled version of this raster.

   Raster::scale creates a scaled version of this raster as output.  This method is used to scale this
   raster to fit into the previously created new Raster.  It is assumed that the new Raster has its width
   and height already set to the desired dimensions.  In addition, information describing the rectangular
   portion of this raster to scale, is also specified.

   \see Raster, RasterType, Slice, TileIO
   
   \param[in] inSlice
       The slice information of the input raster: specifies the upper left position of this raster, along
       with the width and height of this slice, to use as the data to scale.
   \param[in] outSlice
       The slice information of the output raster: specifies the upper left position of the output raster,
       along with the width and height to scale the input to, in the output raster.
   \param[in] rasNew
       The new raster to scale this raster into.

   \returns
       new scaled Raster.

   \par Exceptions
       Exceptions that may be raised by this method:
       None.

   \par Example
       Let's say a user wants to scale an upper left portion of a raster (40 X 50 pixels) named "ex1"
       to 100 X 200 pixels, creating the new raster with an unsigned int 8-bit type, and a name of
       "ex1Scaled". The scaled output should be written to the rectangular portion of its raster, where
       the upper left corner is at (10,20) with a width = 80, and height = 100.  First open the file
       and the raster, then use scale():
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
               newRas = img->create_raster("ex1Scaled", GeoStar::INT8U, 100, 200)
       }//end-try
       catch (...) {
           cerr << "GeoStar::create_raster failure"<<endl;
       }//end-catch
   
           Slice inSlice(0,0,40,50);
           Slice outSlice(10,20,80,100);
           newRas = ras->scale(inSlice, outSlice, newRas);

       }//end-main
       \endcode

  */
     Raster* scale(const Slice &in, const Slice &out, Raster *outRaster);
      


