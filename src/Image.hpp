// Image.hpp
//
// by Leland Pierce, Feb 18, 2017
// 
// revised March 11, 2017: added: read_tiff_1_8U()
//
//-------------------------------------
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include <string>

#include "H5Cpp.h"
#include "Raster.hpp"
#include "attributes.hpp"

#include "gdal_priv.h"

namespace GeoStar {
  class File;
  
/** \brief Image -- Contains Image based operations for HDF5-Image objects

The Image class is used to access and manipulate the second level of an HDF5 file, the image.
The image contains all the seperate channels, or rasters, that will be manipulated in the raster class.

\see File, Raster


\Par Usage Overview

This class is meant to be used in conjunction with the File and Raster classes.
In order to create or open an image, one must first create or open an HDF5 File object.

Creation of an Image requires an image name.

Deleting an image is only possible if it is not "open".

Any open raster objects in the image must first be closed before closing the image.

This class also enables one to create and open rasters.

\Par Details
The class keeps track of the image name and the image type.

There are tools to check if a dataset exists before creating or opening one.

*/
  class Image {
    
  private:
    std::string imagename;
      std::string fullImagename;
    std::string imagetype;
    RasterType getGeoStarType(const GDALDataType &type);
    GDALDataType getGDALType(const Raster *ras);
      File *ownerFile;
      
      // For now, this is a private function.  May make sense to later allow public access (?)
      //void moveRaster(Image *otherImg, Raster *raster);

  public:
    H5::Group *imageobj;

    /** \brief Image Constructor -- allows you to open an existing image or create a new one

    This constructor allows you to open an existing image or create a new one
    from within a preexisting HDF5-image object.  The new image is created empty, and upon
	success a valid image object is returned.

    \see Read, Write

    \param[in] file 
      This is the file object that either holds the existing image, or that is desired to hold
	the new image.  This is a string set by the user

    \param[in] name
      This is a string that holds the name of the existing Image, or the name of the new image

    \returns
	A valid image object upon success

    \Par Exceptions
	Exceptions that may be raised:

	ImageOpenError

    \Par Example
	Opening an existing Image named "landsat":
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "existing");

  	GeoStar::Image *img = new GeoStar::Image(file, "landsat");

	delete img;
	delete file;
	}

	\endcode

	note that this code is also completely valid for creating a new image named landsat
	in the File object.

    \Par Details
	The HDF5 Attribute named "object_type" must exist with the value "Geostar::HDF5"
	or it is not a Geostar Image and an exception is thrown.

	Even if the image you try to open does not exist, the function will still create a 
	valid image object with the desired name.  The procedure to create or open a image with this
	constructor is the same.
    */
    Image(File *file, const std::string &name);

    /** \brief write_object_type -- allows you to write the type attribute of the image

    Can change or implement the attribute of a image, type needs to be "Geostar::HDF5"

    \see read_object_type

    \param[in] type 
      This string is the name of the attribute you want to write to the image.

    \returns
	Nothing

    \Par Exceptions
	None

    \Par Example
	Adding an attribute to an image named "Test":
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

	img->write_object_type("Test");
  	
	delete img;
	delete file;

	}

	\endcode

    \Par Details
	Most of the time, this function will not even need to be called - the image
	recieves the "Geostar::HDF5" attribute during creation.

	Implementation in attributes.cpp for any object type. 
       The implementation in this class makes it easier to use in the Image context.
    */
    inline void write_object_type(const std::string &value) {
        //GeoStar::write_object_type((H5::H5Location *)imageobj,value);
        GeoStar::write_object_type((H5::H5Object *)imageobj,value);
    }


    /** \brief read_object_type -- allows you to read the type attribute of the image

    Can see what attribute the image has, usually to check if type is "Geostar::HDF5"

    \see write_object_type

    \returns
	A string containing the Object attribute.

    \Par Exceptions
	None

    \Par Example
	Reading the attribute of an image:
	\code
	#include "Geostar.hpp"
	#include <string>
	using namespace std;

	int main() {
	GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

	string attribute = img->read_object_type();

	delete img;
	delete file;

	}

	\endcode

    \Par Details
	Almost all the time this function should return "Geostar::HDF5".
	Implementation in attributes.cpp for any object type. 
       The implementation in this class makes it easier to use in the Image context.
    */
    inline std::string read_object_type() const {
        //return GeoStar::read_object_type((H5::H5Location *)imageobj);
        return GeoStar::read_object_type((H5::H5Object *)imageobj);
    }
      
      inline std::string getImagename() const {
          return imagename;
      }
      
      inline std::string getFullImagename() const {
          return fullImagename;
      }

    /** \brief Image destructor allows one to delete a Raster object from memory.

   The Image destructor is automatically called to clean up memory used by the Image object.
   There is a single pointer to the HDF5 Image object that must be deleted.

   \see open, close

   \returns
       Nothing.

   \par Exceptions
       None.

   \par Example
       creating an image named "landsat":
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
         GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");


       }//end-main
       \endcode

       Before main ends, the c++ run-time system automatically destroys the raster variable 
       using the Image object destructor.

  */
    inline ~Image() {
      delete imageobj;
    }

    /** \brief create_raster allows one to create a new GeoStar raster.

   Image::create_raster is used to make a new raster in a GeoStar Image.
   The new raster is created empty, with the specified size, 
	except for the attribute needed to define it as a GeoStar Raster.
   On success, a valid Raster object is returned.

   \see  open_raster

   \param[in] name
       This is a string, set by the user, that holds the desired name of the new raster.

   \param[in] type
	This is one of the valid rastertypes defined in rastertype.hpp.  Note that the
	rastertype must be included in the GeoStar namespace.

   \param[in] nx
	This is the size of the raster in the x-direction.

   \param[in] ny
	this is the size of the raster in the y-direction.

   \returns
       A valid Raster object on success.

   \par Exceptions
       None

   \par Example
       creating a raster named "test":

       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras;
	ras = img->create_raster("test", GeoStar::REAL32, 400, 400);

	delete ras;
	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
       The HDF5 raster attribute named "object_type" is created for the new image and filled 
       with value "geostar::image".
  
  */
    inline Raster *create_raster(const std::string &name, 
                                 const RasterType &type,
                                 const int &nx, const int &ny) {
      return new Raster(this,name,type,nx,ny);
    }

      inline Raster *create_raster(const std::string &name,
                                   const RasterType &type) {
          return new Raster(this,name,type);
      }


    /** \brief open_raster allows one to open an existing GeoStar raster.

   Image::open_raster is used to open an existing raster in a GeoStar Image.

   On success, a valid Raster object is returned.

   \see  create_raster

   \param[in] name
       This is a string, set by the user, that holds the name of the existing raster.

   \returns
       A valid Raster object on success.

   \par Exceptions
       None

   \par Example
       opening a raster named "test":

       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

  	GeoStar::Raster *ras;
	ras = img->open_raster("test");

	delete ras;
	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
       The HDF5 raster attribute named "object_type" must exist and be and filled 
       with value "geostar::image, otherwise the raster is not a GeoStar object.
  
  */
    Raster *open_raster(const std::string &name) {
      return new Raster(this,name);
    }




    /** \brief delete_raster attempts to delete a raster with the given name from the image.

    \see create_raster

    \param[in] name
        A string that holds the Raster name

    \returns
        Nothing

    \par Exceptions
       Exceptions that may be raised by this method:
       none

    \par Example
      \code{.cpp}
      #include "geostar.hpp"
      #include <string>

      int main(int argc, char **argv)
      {
        GeoStar::File *file = new GeoStar::File("a_delete.h5","new");

        GeoStar::Image *img = file->create_image("ers1");
        GeoStar:: Raster *ras = img->create_raster("chan1", GeoStar::INT8U, 512, 1024);
        delete ras;

        file->delete_raster("chan1");

        delete img;
        delete file;
      }
      \endcode

    \par Details
        The HDF5 dataset associated with the raster is deleted by removing the
        link from the Image group. If the raster is not found, the function
        simply does nothing.
    */
     void delete_raster(const std::string &name) {
      H5Ldelete(this->imageobj->getLocId(), name.c_str(), H5P_DEFAULT);
      return;
    }










    /** \brief datasetExists returns true if the named dataset exists, false otherwise.

   datasetExists is used to check on the existence of a dataset, before trying to create
   a new one. This is to avoid name collisions. Returns true of the dataset exists, false otherwise.

   \see  open_dataset

   \param[in] name
       This is a string, set by the user, that holds the name of the dataset.

   \returns
       true if dataset exists,
       false if dataset does not exist.

   \par Exceptions
 	none

   \par Example
       This function is not for use by most users. An admin user could use this
       function to check for the existence of a group, and then either create it or open it.
       Example checking for existence of a dataset "test"
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

           if(img->datasetExists("test")){
           // open the dataset...
           } else {
           // create the dataset.
           } //endif

	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
       This routine is not for the use of most users.
       This rouine merely tries to open the dataset, and on success it closes the dataset and returns true.
       On failure, it returns false.
  */
    bool datasetExists(const std::string &name);

    /** \brief createDaataset creates the named dataset and returns it.

   datasetExists  creates the named dataset and returns it.
   
   \see  openDataset

   \param[in] name
       This is a string, set by the user, that holds the name of the dataset.

   \param[in] data_type
	

   \param[in] data_space
	

   \returns
       The dataset on success.

   \par Exceptions
       none

   \par Example
       This function is not for use by most users. 
       Example creating a dataset named "test"

       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
            GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

           H5::Dataset *rasterobj;  

	H5::PredType h5Type = H5T_NATIVE_INT;
	hsize_t dims[2];
    	dims[0] = 40; 
    	dims[1] = 40; 
    	H5::DataSpace dataspace(2, dims);
	      
              rasterobj = new H5::DataSet(img->createDataset("test", h5Type, dataspace));

	delete rasterobj;
	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
       This routine is not for the use of most users.
       It is basically a stub that calls the HDF5 function of the same name.
  */
      inline H5::DataSet createDataset(const std::string &name,
                                       const H5::DataType &data_type,
                                       const H5::DataSpace &data_space) {
          return imageobj->createDataSet(name,data_type,data_space);
      }

      // create a new dataset in this Image (group)
      // existence is an error
      inline H5::DataSet createDataset(const std::string &name,
                                       const H5::DataType &data_type,
                                       const H5::DataSpace &data_space,
                                       const H5::DSetCreatPropList &prop_list) {
          return imageobj->createDataSet(name,data_type,data_space,prop_list);
      }

    /** \brief openDataset opens the named dataset and returns it.

   File::openDataset  opens the named dataset and returns it.
   
   \see  createDataset

   \param[in] name
       This is a string, set by the user, that holds the name of the Dataset.

   \returns
       The dataset on success.

   \par Exceptions
       none

   \par Example
       This function is not for use by most users. 
	Example opening dataset named "test"
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

           H5::Dataset *rasterobj;
              rasterobj = new H5::Dataset(  img->openDataset("test"));

	delete rasterobj;
	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
       This routine is not for the use of most users.
       It is basically a stub that calls the HDF5 function of the same name.
  */
    inline H5::DataSet openDataset(const std::string &name) {
      return imageobj->openDataSet(name);
    }


/** \brief read_file reads a file and returns a new raster

   read_file reads a single channel image file.  If the image is more than 1 channel, it reads the channel
	number specified in nChannels parameter.  For now, a float channel is created.
   
   \see  Read, Write

   \param[in] infile
	This is a string that holds the name of the file you are reading from

   \param[in] name
       This is a string, set by the user, that holds the name of the raster you are creating.

   \param[in] nChannels
	This is an integer that specifies what channel of the image you want to read from.

   \returns
       A new raster with the channel data inside on success.

   \par Exceptions
	Exceptions that may be raised by this method:
       FileOpenError

   \par Example
       Example creating a raster named "test" from 1 channel of a file named "data.tif"

       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file = new GeoStar::File("a1.h5", "new");

  	GeoStar::Image *img = file->create_image("landsat");

	GeoStar::Raster *ras;
	ras = img->read_file("data.tif", "test", 1);

	delete ras;
	delete img;
	delete file;

       }//end-main
       \endcode

    \par Details
	The raster will be created to be the same size and have the same values as the channel
	you are reading from.  The infile must exist, otherwise an exception is thrown.
 
  */
    // read a multi-channel image file
    // infile is the name of the input image file
    // names is the vector of strings to identify the new raster channels that are created
    // returns a vector of the newly-created Rasters.
      std::vector<Raster *> read_file(const std::string &infile);
      std::vector<Raster *> read_file(const std::string &infile, std::vector<std::string> &names);

    template <typename T>
    void fill_raster(Raster *ras, GDALRasterBand *poBand, const int nx,
                     const int ny, const GDALDataType typ);
      
      template <typename T>
      void copy_raster(Raster *ras, GDALRasterBand *poBand, const int nx,
                              const int ny, const GDALDataType typ);
      

      // write a multi-channel image file
      // outfile is the name of the output image file
      // channels is the vector of strings to identify the which raster channels are to be written
      //  to the output image file
      void write_file(const std::string &outfile, const std::string &format, std::vector<std::string> &channels);

      int getNumberOfChannels(const std::string &infile);
      std::vector<std::string> getChannels();
      
      Image* scale(const double &xratio, const double &yratio);
      //Raster* scale(const long int &nx, const long int &ny);
      //Raster* scale(const double &xratio, const double &yratio, const RasterType &type, const std::string &name);
      //Raster* scale(const long int &nx, const long int &ny, const RasterType &type, const std::string &name);
      //Raster* scale(GeoStar::Raster *rasNew);  (doesn't make sense for Image(?))
      //Raster* scale(const Slice &in, const double &xratio, const double &yratio);
      //Raster* scale(const Slice &in, const long int &nx, const long int &ny);
      //Raster* scale(const Slice &in, const double &xratio, const double &yratio, const RasterType &type, const std::string &name);
      //Raster* scale(const Slice &in, const long int &nx, const long int &ny, const RasterType &type, const std::string &name);
      //Raster* scale(const Slice &in, Raster *outRaster);
      //Raster* scale(const Slice &in, const Slice &out, Raster *outRaster);
      //Raster* warp(const WarpParameters warpData, const Slice &in, const Slice &out, Raster *outRaster);
      //Raster* transform(std::string &newWKT, std::string &newRasterName, double newDeltaX, double newDeltaY);
      
      Image* rotate(const float angle);  // done
      Image* rotate(const float angle, const Slice &in);  // done
      Image* warp(const WarpParameters warpData, const Slice &in, const Slice &out, Image *outImage); // or outName?
      Image* flip(const std::string &newImageName, short flipAxis); // done
      Image* transform(std::string &newWKT, std::string &newImageName, double newDeltaX, double newDeltaY); //done
      void setWKT(std::string &wkt);  //done
      void setLocation(std::string &location);  //done
      
      // Copy this Image to a new Image, under the same file, with the given name.
      // All channels (Rasters) will be copied.  The reference to the new copied Image is returned.
      Image* copy_to(const std::string &newImageName);
      
      // Copy this Image to a new Image, under the same file, with the given name.  The vector of
      // strings given, is the names of the channels of this Image, which should be copied under the
      // new Image.  The reference to the new copied Image is returned.
      Image* copy_to(const std::string &newImageName, std::vector<std::string> &copyChannels);


  }; // end class: Image
  
}// end namespace GeoStar


#endif
