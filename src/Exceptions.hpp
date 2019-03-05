#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <exception>


#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Example, to throw a GeoStar exception, which is a subclass of std::exception
//    throw RasterOpenErrorException();



namespace GeoStar {
    
    class geoException : public std::runtime_error {
    protected:
        std::string msg;
    public:
        geoException(const std::string &arg, const char *file, int line) :
        std::runtime_error(arg) {
            std::ostringstream o;
            o << file << ":" << line << "\n";
            msg = o.str();
        }
        ~geoException() throw() {}
        const char *what() const throw() {
            return msg.c_str();
        }
        std::string printError() const {
            return msg;
        }
    };
    
    class SliceSizeException: public geoException {
    public:
        SliceSizeException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Slice Size Error: too large for raster '" << arg << "'\n";
            msg += o.str();
        }
    };

    #define throw_SliceSizeError(arg) throw SliceSizeException(arg,__FILE__, __LINE__);
    
    class SliceDataException: public geoException {
    public:
        SliceDataException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Slice Data Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_SliceDataError(arg) throw SliceDataException(arg,__FILE__, __LINE__);
    
    

  class DataTypeException: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "DataTypeError";
    }
  };
    
    
    
    class Hdf5UnsupportedTypeException: public geoException {
    public:
        Hdf5UnsupportedTypeException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Hdf5 Error: raster '" << arg << "' has an unsupported HDF5 type!\n";
            msg += o.str();
        }
    };

    #define throw_Hdf5UnsupportedTypeError(arg) throw Hdf5UnsupportedTypeException(arg,__FILE__, __LINE__);
    
    

    class FileExistsException: public geoException {
    public:
        FileExistsException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "File Creation Error: file '" << arg << "' already exists\n";
            msg += o.str();
        }
    };
    
    #define throw_FileExistsError(arg) throw FileExistsException(arg,__FILE__, __LINE__);

    class FileAccessException: public geoException {
    public:
        FileAccessException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "File Access Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_FileAccessError(arg) throw FileAccessException(arg,__FILE__, __LINE__);

    class FileDoesNotExistException: public geoException {
    public:
        FileDoesNotExistException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "File Open Error: file '" << arg << "' does not exist\n";
            msg += o.str();
        }
    };

    #define throw_FileDoesNotExistError(arg) throw FileDoesNotExistException(arg,__FILE__, __LINE__);

    class FileOpenErrorException: public geoException {
    public:
        FileOpenErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "File Open Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_FileOpenError(arg) throw FileOpenErrorException(arg,__FILE__, __LINE__);
    
    /*
    class FileCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "FileCreationError";
            }
    };
    class FileDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "FileDestroyError";
            }
    };
     */

    class ImageOpenErrorException: public geoException {
    public:
        ImageOpenErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Image Open Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_ImageOpenError(arg) throw ImageOpenErrorException(arg,__FILE__, __LINE__);

    class ImageExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ImageExists";
            }
    };
    class ImageDoesNotExistException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ImageDoesNotExist";
            }
    };
    class ImageCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ImageCreationError";
            }
    };
    class ImageDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ImageDestroyError";
            }
    };





    
  // exceptions:
  //struct FileExists{};
  //struct FileDoesNotExist{};
  //struct FileCreationError{};
  //struct FileOpenError{};
  //struct FileDestroyError{};

  //struct ImageExists{};
  //struct ImageDoesNotExist{};
  //struct ImageCreationError{};
  //struct ImageOpenError{};
  //struct ImageDestroyError{};

    
    class RasterOpenErrorException: public geoException {
    public:
        RasterOpenErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Open Error: wrong type for raster '" << arg << "'\n";
            msg += o.str();
        }
    };

    #define throw_RasterOpenError(arg) throw RasterOpenErrorException(arg,__FILE__, __LINE__);
    
    class RasterDoesNotExistException: public geoException {
    public:
        RasterDoesNotExistException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Open Error: raster '" << arg << "' does not exist\n";
            msg += o.str();
        }
    };

    #define throw_RasterDoesNotExistError(arg) throw RasterDoesNotExistException(arg,__FILE__, __LINE__);
    
    class RasterExistsException: public geoException {
    public:
        RasterExistsException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Creation Error: raster '" << arg << "' already exists\n";
            msg += o.str();
        }
    };

    #define throw_RasterExistsError(arg) throw RasterExistsException(arg,__FILE__, __LINE__);
    
    class RasterUnsupportedTypeException: public geoException {
    public:
        RasterUnsupportedTypeException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Error: raster '" << arg << "' has an unsupported type!\n";
            msg += o.str();
        }
    };

    #define throw_RasterUnsupportedTypeError(arg) throw RasterUnsupportedTypeException(arg,__FILE__, __LINE__);

    class RasterWriteErrorException: public geoException {
    public:
        RasterWriteErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Write Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_RasterWriteError(arg) throw RasterWriteErrorException(arg,__FILE__, __LINE__);
    
    class RasterReadErrorException: public geoException {
    public:
        RasterReadErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Read Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_RasterReadError(arg) throw RasterReadErrorException(arg,__FILE__, __LINE__);
    
    class RasterCreationMutableException: public geoException {
    public:
        RasterCreationMutableException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster (mutable) Creation Error: " << arg << "\n";
            msg += o.str();
        }
    };
    
    #define throw_RasterCreationMutableError(arg) throw RasterCreationMutableException(arg,__FILE__, __LINE__);
    
    class RasterCreationImmutableException: public geoException {
    public:
        RasterCreationImmutableException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster (mutable) Creation Error: " << arg << "\n";
            msg += o.str();
        }
    };
    
    #define throw_RasterCreationImmutableError(arg) throw RasterCreationImmutableException(arg,__FILE__, __LINE__);
    
    class RasterImmutableException: public geoException {
    public:
        RasterImmutableException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster dimension re-assignment Error: '" << arg << "' was created as immutable.\n";
            msg += o.str();
        }
    };
    
    #define throw_RasterImmutableError(arg) throw RasterImmutableException(arg,__FILE__, __LINE__);
    
    class RasterNotABitmapException: public geoException {
    public:
        RasterNotABitmapException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster '" << arg << "' is NOT a bitmap!\n";
            msg += o.str();
        }
    };
    
    #define throw_RasterNotABitmapError(arg) throw RasterNotABitmapException(arg,__FILE__, __LINE__);
    
    class InvalidPixelValueTypeException: public geoException {
    public:
        InvalidPixelValueTypeException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Invalid Pixel Value Type Exception: " << arg <<"\n";
            msg += o.str();
        }
    };
    
    #define throw_InvalidPixelValueType(arg) throw InvalidPixelValueTypeException(arg,__FILE__, __LINE__);

    





    class RasterSizeException: public geoException {
    public:
        RasterSizeException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Raster Size Error: too large for raster '" << arg << "'\n";
            msg += o.str();
        }
    };

    #define throw_RasterSizeError(arg) throw RasterSizeException(arg,__FILE__, __LINE__);
 

/*
    class RasterOpenErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "RasterOpenError";
            }
    };
 */
    class RasterDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "RasterDestroyError";
            }
    };


    //struct RasterExists{};
    //struct RasterDoesNotExist{};
    //struct RasterCreationError{};
    //struct RasterOpenError{};
    //struct RasterDestroyError{};

    class MetadatExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "MetadataExists";
            }
    };
    class MetadataDoesNotExistException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "MetadataDoesNotExist";
            }
    };
    class MetadatCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "MetadataCreationError";
            }
    };
    class MetadatOpenErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "MetadatapenError";
            }
    };
    class MetadatDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "MetadataDestroyError";
            }
    };

    //struct MetadataExists{};
    //struct MetadataDoesNotExist{};
    //struct MetadataCreationError{};
    //struct MetadataOpenError{};
    //struct MetadataDestroyError{};
    
    class AttributeParseException: public geoException {
    public:
        AttributeParseException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Attribute Parse Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_AttributeParseError(arg) throw AttributeParseException(arg,__FILE__, __LINE__);
    
    class AttributeDoesNotExistException: public geoException {
    public:
        AttributeDoesNotExistException(const std::string &src, const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "For '" << src << "',  " <<   "Attribute Undefined: '" << arg << "'\n";
            msg += o.str();
        }
    };

    #define throw_AttributeDoesNotExistError(src,arg) throw AttributeDoesNotExistException(src,arg,__FILE__, __LINE__);
    
    class AttributeErrorException: public geoException {
    public:
        AttributeErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Attribute Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_AttributeError(arg) throw AttributeErrorException(arg,__FILE__, __LINE__);


    class AttributeExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "AttributeExists";
            }
    };
    class InvalidAttributeFormatException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "InvalidAttributeFormat";
            }
    };
    
    
    class CoordinateTransformException: public geoException {
    public:
        CoordinateTransformException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Coordinate Transformation Error: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_CoordinateTransformError(arg) throw CoordinateTransformException(arg,__FILE__, __LINE__);
    
    class FlipOptionException: public geoException {
    public:
        FlipOptionException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Invalid Flip Option: " << arg << "\n";
            msg += o.str();
        }
    };

    #define throw_FlipOptionError(arg) throw FlipOptionException(arg,__FILE__, __LINE__);
    

    //struct AttributeExists{};
    //struct AttributeDoesNotExist{};
    //struct InvalidAttributeFormat{};
    //struct AttributeError{};

    class DirectoryExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "DirectoryExists";
            }
    };
    class DirectoryDoesNotExistException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "DirectoryDoesNotExist";
            }
    };
    class DirectoryCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "DirectoryCreationError";
            }
    };
    class DirectoryDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "DirectoryDestroyError";
            }
    };

    //struct DirectoryExists{};
    //struct DirectoryDoesNotExist{};
    //struct DirectoryCreationError{};
    //struct DirectoryDestroyError{};
    
    class NotGeoSciFileException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "NotGeoSciFile";
            }
    };
    class NotGeoSciImageException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "NotGeoSciImage";
            }
    };
    class NotGeoSciRasterException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "NotGeoSciRaster";
            }
    };

    //struct NotGeoSciFile{};
    //struct NotGeoSciImage{};
    //struct NotGeoSciRaster{};


  class UnknownThreshMethodException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "UnknownThreshMethod";
            }
    };

  class InputOutOfRangeException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "InputOutOfRange";
            }
    };
  class RasterTypeErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "RasterTypeError";
            }
    };
  class InvalidKernelSizeException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "InvalidKernelSize";
            }
    };
  class InvalidRasterSizeException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "InvalidRasterSize";
            }
    };
    /** \brief Excpetion thrown when two or more rasters that need to be the same size are not.

    */









  class DifferentSizedRastersException : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "DifferentSizedRasters";
        }
    };


    /** \brief Excpetion thrown when the same number of rasters are not being compared

    */
  class DifferentNumRastersException : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "DifferentNumRasters";
        }
    };

    /** \brief Excpetion thrown when invalid string input is given
    */
  class BadStringException : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "BadString";
        }
    };
    /** \brief Excpetion thrown when invalid integer is given
    */
  class BadIntegerException : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "BadInteger";
        }
    };


    //struct UnknownThreshMethod{};
    //struct InputOutOfRange{};
    //struct RasterTypeError{};
    //struct InvalidKernelSize{};
    //struct InvalidRasterSize{};



  class VectorExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "VectorExists";
            }
    };
  class VectorDoesNotExistException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "VectorDoesNotExist";
            }
    };
  class VectorCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "VectorCreationError";
            }
    };
  class VectorOpenErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "VectorOpenError";
            }
    };
  class VectorDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "VectorDestroyError";
            }
    };

  //struct VectorExists{};
  //struct VectorDoesNotExist{};
  //struct VectorCreationError{};
  //struct VectorOpenError{};
  //struct VectorDestroyError{};

  class ShapeExistsException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeExists";
            }
    };
  class ShapeDoesNotExistException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeDoesNotExist";
            }
    };
  class ShapeCreationErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeCreationError";
            }
    };

  class ShapeReadErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeReadError";
            }
    };
  class ShapeWriteErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeWriteError";
            }
    };


  class ShapeOpenErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeOpenError";
            }
    };
  class ShapeDestroyErrorException: public std::exception
    {
        virtual const char* what() const throw()
            {
                return "ShapeDestroyError";
            }
    };


    //struct ShapeExists{};
    //struct ShapeDoesNotExist{};
    //struct ShapeCreationError{};
    //struct ShapeOpenError{};
    //struct ShapeDestroyError{};








    class IfileOpenErrorException: public geoException {
    public:
        IfileOpenErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Ifile Open Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_IfileOpenError(arg) throw IfileOpenErrorException(arg,__FILE__, __LINE__);

   class IfileExistsException: public geoException {
    public:
        IfileExistsException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Ifile Exists Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_IfileExistsError(arg) throw IfileExistsException(arg,__FILE__, __LINE__);

   class IfileDoesNotExistException: public geoException {
    public:
        IfileDoesNotExistException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Ifile Does not Exist Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_IfileDoesNotExistError(arg) throw IfileDoesNotExistException(arg,__FILE__, __LINE__);

   class IfileCreationErrorException: public geoException {
    public:
        IfileCreationErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Ifile Creation Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_IfileCreationError(arg) throw IfileCreationErrorException(arg,__FILE__, __LINE__);


   class IfileDestroyErrorException: public geoException {
    public:
        IfileDestroyErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Ifile Destroy Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_IfileDestroyError(arg) throw IfileDestroyErrorException(arg,__FILE__, __LINE__);



   class RadiusSizeErrorException: public geoException {
    public:
        RadiusSizeErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Radius Size Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_RadiusSizeError(arg) throw RadiusSizeErrorException(arg,__FILE__, __LINE__);



   class ProbablityErrorException: public geoException {
    public:
        ProbablityErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Probablity Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_ProbablityError(arg) throw ProbablityErrorException(arg,__FILE__, __LINE__);



   class BitErrorException: public geoException {
    public:
        BitErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Bit Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_BitError(arg) throw BitErrorException(arg,__FILE__, __LINE__);


   class PartitionErrorException: public geoException {
    public:
        PartitionErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "Partition Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_PartitionError(arg) throw PartitionErrorException(arg,__FILE__, __LINE__);








   class SQLDatabaseCreationErrorException: public geoException {
    public:
        SQLDatabaseCreationErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "SQLDatabase Creation Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_SQLDatabaseCreationError(arg) throw SQLDatabaseCreationErrorException(arg,__FILE__, __LINE__);


   class SQLDatabaseOpenErrorException: public geoException {
    public:
        SQLDatabaseOpenErrorException(const std::string &arg, const char *file, int line) :
        geoException(arg, file, line) {
            std::ostringstream o;
            o << "SQLDatabase Open Error: '" << arg << "' \n";
            msg += o.str();
        }
    };

    #define throw_SQLDatabaseOpenError(arg) throw SQLDatabaseOpenErrorException(arg,__FILE__, __LINE__);





}// end namespace GeoStar


#endif
//end EXCEPTIONS_HPP_
