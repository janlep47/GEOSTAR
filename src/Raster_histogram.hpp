// Raster_histogram.hpp
//
// by Janice Richards, Mar 2, 2018
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

      template <typename T>
      std::vector<int> histogram(std::vector<T> binValues);

      template <typename T>
      std::vector<int> histogram(Slice in, std::vector<T> binValues);


