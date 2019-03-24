// Raster_polygon.cpp
//
// by Janice Richards, Mar 3, 2018
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <array>

#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "Polygon.hpp"
#include "TileIO.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"
#include "RasterFunction.hpp"

#include "polygonize.hpp"      // REMOVE after removing the OLD stuff (used for comparison of ours w/ GDAL)


#define NO_DATA_VAL 0;

namespace GeoStar {

    /*
    
    // write internal (HDF5) raster data to gdal raster band, for gdal/ogr processing:
    template <typename T>
    void Raster::copy_raster(GDALRasterBand *poBand, const int nx,
                            const int ny, const GDALDataType typ) {
        Slice slice(0,0,nx,1);
        std::vector<T> buf(nx);
        
        for(int iy=0;iy<ny;++iy){
            slice.setY0(iy);
            read(slice, buf);
            // write internal raster buffer to the gdal raster band.
            CPLErr err = poBand->RasterIO( GF_Write,
                                          0, iy, nx, 1,
                                          &buf[0], nx, 1, typ,   // &buf[0] for parameter void *
                                          0, 0 );
        } // endfor
        return;
    }
    
    // write (external image file, which was written to gdal raster band),
    //   to internal (hdf5 file) raster data
    //
    template <typename T>
    void Raster::fill_raster(GDALRasterBand *poBand, const int nx,
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
            write(slice,buf);
        } // endfor
        return;
    }
*/

    
    //Vector* Raster::rasterToPolygon(const std::string &newVectorName) {
    void Raster::rasterToPolygon(const std::string &newVectorName) {
        // Create the new Vector, to hold the transformed raster, using the same type and size as this raster:
        /*
        RasterType type = raster_datatype;
        GeoStar::Vector *vecNew;
        try {
            vecNew = image->create_vector(newVectorName, type, get_nx(), get_ny());
        } catch (VectorExistsException e) {
            rasNew = image->open_raster(newVectorName);
        }
        return rasterToPolygon(rasNew);
         */
        // TEMP:
        rasterToPolygon();
        return;
    }
    

    //Vector* Raster::rasterToPolygon(Vector *vecNew) {
    void Raster::rasterToPolygon() {
        switch(raster_datatype) {
            case INT8U:
                //return rasterToPolygonType<uint8_t>(vecNew);
                rasterToPolygonType<uint8_t>();
                break;
                /*
                 case INT8S:
                 return rasterToPolygonType<int8_t>(vecNew,);
                 case INT16U:
                 return rasterToPolygonType<uint16_t>(vecNew);
                 case INT16S:
                 return rasterToPolygonType<int16_t>(vecNew);
                 case INT32U:
                 return rasterToPolygonType<uint32_t>(vecNew);
                 case INT32S:
                 return rasterToPolygonType<int32_t>(vecNew);
                 case INT64U:
                 return rasterToPolygonType<uint64_t>(vecNew);
                 case INT64S:
                 return rasterToPolygonType<int64_t>(vecNew);
                 */
            case REAL32:
                //return rasterToPolygonType<float>(vecNew);
                rasterToPolygonType<float>();
                break;

            case REAL64:
                //return rasterToPolygonType<double>(vecNew);
                rasterToPolygonType<double>();
                break;
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
    }
    
    
/*
    template <typename T>
    //Vector*  Raster::rasterToPolygonType(Vector *vecNew) {
    void  Raster::rasterToPolygonType_SAVEIT() {

        //GDALDataset *poDataset;
        GDALAllRegister();
        GDALDriver *poDriverMem;
        
        // NOTE: when getting a driver, to then create the (output) vector dataset, when you use
        //  any format other than "Memory", you must specify an output file name (in the create vector
        //  dataset call).  This file is then created and written to, when running GDALPolygonize.
        //
        // See  http://www.gdal.org/ogr_formats.html  for list of formats allowed:
        //
        //GDALDriver * pDriver = GetGDALDriverManager()->GetDriverByName( "ESRI Shapefile" );
        //X GDALDriver * pDriver = GetGDALDriverManager()->GetDriverByName( "OpenStreetMap XML and PBF" );
        //X GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "OpenStreetMap XML" );
        //GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "GeoJSON" );
        //X GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "XML" );
        
        // NOTE!!! GML is the (default) output format of gdal_polygonize.py utility program!!!
        //  (see my files generated: polygonCrap.tif     and   polygonCrap.xsd)
        //GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "GML" );
        GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "Memory" );
        
        if (pDriver == NULL) {
            std::cerr << " driver is NULL!" << std::endl;
            return;
        }
        char **papszMetadata;
        
        poDriverMem = GetGDALDriverManager()->GetDriverByName("MEM");
        
        papszMetadata = poDriverMem->GetMetadata();
        
        GDALDataset *poDstMem;
        char **papszOptions = NULL;
        GDALRasterBand *pMaskBand = NULL;
        
        int nx = get_nx();
        int ny = get_ny();
        
        //GDALDataType typ = getGDALType(ras);
        GDALDataType typ = GDT_Byte;
        
        //poDstMem = poDriverMem->Create( "", nx, ny, channels.size(), typ, papszOptions );
        poDstMem = poDriverMem->Create( "", nx, ny, 1, typ, papszOptions );
        std::cerr << " got here a" << std::endl;
        
        
        //char *tgtPath = "polygonProgJunk.shp";
        //GDALDataset * pTgtDataset = pDriver->Create( tgtPath, 0, 0, 0, GDT_Unknown, NULL );
        
        // UNCOMMENT LATER >>>>>
        //GDALDataset * pTgtDataset = pDriver->Create( "polygonMoreProgJunk2", 0, 0, 0, GDT_Unknown, NULL );
        
        //try ... (above WORKS)
        //GDALDataset * pTgtDataset = poDriverMem->Create( "x", 0, 0, 0, GDT_Unknown, NULL );  X
        //GDALDataset * pTgtDataset = poDriverMem->Create( "x", nx, ny, 1, GDT_Unknown, NULL );   X
        GDALDataset * pTgtDataset = pDriver->Create( "", 0, 0, 0, GDT_Unknown, NULL );
        
        
        
        std::cerr << "got here b" << std::endl;
        
        
        
        GDALRasterBand *poBand;
        
        // loop for all the rasters in the image, actually ....
        poBand = poDstMem->GetRasterBand(1);     //(i+1);
        
        GeoStar::Slice slice(0,0,nx,1);
        std::vector<uint8_t> buf(nx);      //really .... templated
        
        for(int iy=0;iy<ny;++iy){
            slice.setY0(iy);
            read(slice, buf);
            // write internal raster buffer to the gdal raster band.
            CPLErr err = poBand->RasterIO( GF_Write,
                                          0, iy, nx, 1,
                                          &buf[0], nx, 1, typ,   // &buf[0] for parameter void *
                                          0, 0 );
        } // endfor
        
        std::cerr << "OK ... got here ..." << std::endl;
        
        // Create a layer.
        //OGRLayer *outLayer = poDstMem->CreateLayer( "layer", NULL, wkbPolygon, NULL );
        OGRLayer *outLayer = pTgtDataset->CreateLayer( "layer", NULL, wkbPolygon, NULL );
        int valueFieldIdx = 0;
        
        //GDALPolygonize(poBand, pMaskBand, outLayer, valueFieldIdx, papszOptions, GDALTermProgress, NULL);
        Polygonize(poBand, pMaskBand, outLayer, valueFieldIdx, papszOptions, GDALTermProgress, NULL);
        std::cerr << " here 1" << std::endl;
        
        
        int junkCounter = 0;
        OGRFeature *poFeature;
        outLayer->ResetReading();
        while( (poFeature = outLayer->GetNextFeature()) != NULL  && junkCounter <= 10)
        {
            OGRFeatureDefn *poFDefn = outLayer->GetLayerDefn();
            int iField;
            for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
            {
                OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
                if( poFieldDefn->GetType() == OFTInteger )
                    printf( "%d,", poFeature->GetFieldAsInteger( iField ) );
                else if( poFieldDefn->GetType() == OFTInteger64 )
                    printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
                else if( poFieldDefn->GetType() == OFTReal )
                    printf( "%.3f,", poFeature->GetFieldAsDouble(iField) );
                else if( poFieldDefn->GetType() == OFTString )
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
                else
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
            }
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();
            if (poGeometry == NULL) {
                printf( "geometry is null" );
            } else if( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
            {
                OGRPoint *poPoint = (OGRPoint *) poGeometry;
                printf( "%.3f,%3.f\n", poPoint->getX(), poPoint->getY() );
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ) {
                OGRPolygon *polygon = (OGRPolygon *) poGeometry;
                
 
                 //char *wktStrSrc;
                 //char *wktStrDst;
                 //oSourceSRS.exportToWkt(&wktStrSrc);
                 //oTargetSRS.exportToWkt(&wktStrDst);
                 //GeoStar::Point ptToTransform(10.5, 20.5);
                 // Convert the char* to c++ string type
                 //std::string wktSource(wktStrSrc);
                 //std::string wktTarget(wktStrDst);
 
                char *wktChars;
                polygon->exportToWkt(&wktChars);
                //std::string wktStr(wktChars);
                printf( "a polygon: %s\n", wktChars);
                
                
                //printf( "a polygon: %s\n", polygon->getGeometryName() );
                //if (polygon->hasCurveGeometry()) {
                //    printf( " -> is curved! \n" );
                //} else {
                //    printf( " -> is NOT curved! \n" );
                //}
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbLineString ) {
                printf( "a LineString!\n" );
            } else {
                printf( "no tested geometry\n" );
            }
            OGRFeature::DestroyFeature( poFeature );
            
            junkCounter++;
        }
        GDALClose( pTgtDataset );
        GDALClose( poDstMem );
    
        
        return;
    }
    
    */
    
    template <typename T>
    //Vector*  Raster::rasterToPolygonType(Vector *vecNew) {
    void  Raster::rasterToPolygonType() {
        
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        GeoStar::Slice tileDescriptor(0,0,10,10);
        int maxNumberSlicesInRam = 70;
        GeoStar::TileIO<T> reader(this, tileDescriptor, maxNumberSlicesInRam);
        
        GeoStar::Slice scalingSlice(0,0,2,2,4);

        //GDALDataset *poDataset;
        GDALAllRegister();
        //GDALDriver *poDriverMem;
        
        int nx = get_nx();
        int ny = get_ny();
        Slice sliceInput(0,0,nx,ny);
        
        // FOR NOW, set NO_DATA_VAL to 0
        T pixelVal = NO_DATA_VAL;
        
        // the key is the pixel-value (color)
        //std::map<int, std::vector<std::map<int,std::vector<Point>>> > polygonInfo;
        std::map<int,std::vector<Polygon> > polygonInfo;
        int polygonNumber = 0;
        bool contiguousPolygonFound = false;

        long int lx, ly;
        
        // Start reading pixels
        for (int x = 0; x < nx; ++x) {
            lx = x;
            for (int y = 0; y < ny; ++y) {
                contiguousPolygonFound = false;
                ly = y;
                //pixelVal = getTiledPixelVal(x, y);
                pixelVal = getScaledPixelFromTile<T>(ly, lx, 0.0, 0.0, sliceInput, scalingSlice, reader);
                //if (pixelVal == NO_DATA_VAL) continue;
                if (pixelVal == 0) continue;   // TEMPORARY !!!!!!!!!!!!!!!!!!!

                typename std::map<int, std::vector<Polygon> >::iterator it;
                it = polygonInfo.find(pixelVal);
                // IF no polygons yet defined with this pixel value:
                if (it == polygonInfo.end()) {
                    //std::cerr << "pixelVal: " << pixelVal
                    //    << "  first found, for (" << x << "," << y << ")" << std::endl;
                    // NO polygon information YET for this pixel value ... create it
                    Polygon polygon(polygonNumber, x, y);
                    std::vector<Polygon> polygons(1);
                    polygons[0] = polygon;
                    polygonInfo[pixelVal] = polygons;
                    polygonNumber++;
                    
                // Otherwise, there is at least one other polygon already defined, having this pixel value:
                } else {
                    std::vector<Polygon> polygons = it->second;
                    //std::cerr << "polygon found for: " << pixelVal
                    //    << ",  for (" << x << "," << y << ")";
                    // For each polygon with this pixelVal, find the first, IF ANY,
                    //  polygon which has a 4-connected contiguous same-value pixel,
                    //  meaning either directly above (y-1) or directly left (x-1)
                    //
                    // Start with the most-recently created Polygon in the list
                    //  (bottom up)
                    int polyIndex;
                    
                    //std::cerr << " ...polygons.size()=" << polygons.size() << std::endl;
                    
                    for (int i = polygons.size()-1; i >= 0; i--) {
                        Polygon polygon = polygons[i];
                        std::vector<Point> points = polygon.getPoints(x);
                        //std::cerr << " polygon points for x=0 size=" << points.size() << std::endl;
                        if (points.size() != 0) {
                            // First see if the MOST RECENTLY ADDED point has the y-1 address value
                            //  (so pixel is JUST ABOVE):

                            //std::cerr << " here 1" << std::endl;
                            //for (int j = points.size()-1; j >= 0; j--) {
                                Point pt = points[points.size()-1];
                                if (pt.y == y - 1) {
                                    polygon.addPoint(x,y);
                                    polygons[i] = polygon;
                                    polygonInfo[pixelVal] = polygons;
                                    contiguousPolygonFound = true;
                                    //std::cerr << " found point just above" << std::endl;
                                }
                            //}
                            if (contiguousPolygonFound) break; // OUT OF THE ABOVE FOR LOOP, done with this pixel
                        } // endif any other points in this column

                        // Otherwise, if no above pixel, look for pixels JUST LEFT, which will have y'=y and x'=x-1
                        //  ... will need to search through entire list:
                        
                        std::cerr << "looking for LEFT point ... " << std::endl;
                        std::vector<Point> leftPoints = polygon.getPoints(x-1);
                        //std::cerr << " here 2.0" << std::endl;
                        if (leftPoints.size() != 0) {
                            // for each of this same-colored pixels with the left-x address,
                            //  see if any of the pixels have the SAME y value.
                            //std::cerr << " here 2.5" << std::endl;
                            for (int j = 0; j < leftPoints.size(); j++) {
                                Point pt = leftPoints[j];
                                if (pt.y == y) {
                                    std::cerr << "  ... FOUND LEFT point !!! " << std::endl;
                                    polygon.addPoint(x,y);
                                    polygons[i] = polygon;
                                    polygonInfo[pixelVal] = polygons;
                                    contiguousPolygonFound = true;
                                    //std::cerr << " found point just left" << std::endl;
                                    break;
                                }
                            }
                            if (contiguousPolygonFound) break; // OUT OF THE ABOVE FOR LOOP, done with this pixel
                        } // endif any left-points in this polygon
                    }  // end-for each polygon w/ = pixelValue
                    
                    if (!contiguousPolygonFound) {
                        // No polygons with the same pixelvalue that are also CONTIGUOUS
                        // Create a new polygon:
                        std::cerr << "Creating a new polygon ... x=" << x << " y=" << y << std::endl;
                        Polygon polygon(polygonNumber, x, y);
                        polygons.push_back(polygon);
                        polygonInfo[pixelVal] = polygons;
                        polygonNumber++;
                    }
                }  // end-if any polygons with this pixelValue
            } // end-for (y = 0, to ny)
            // Now that we're done with processing ONE COLUMN, edgify EACH polygon
            
            typename std::map<int, std::vector<Polygon> >::iterator it = polygonInfo.begin();
            while (it != polygonInfo.end()) {
                int pixelValue = it->first;
                std::vector<Polygon> polygons = it->second;
                for (int i = 0; i < polygons.size(); i++) {
                    Polygon polygon = polygons[i];
                    polygon.edgify(x);
                    polygons[i] = polygon;  //new
                }
                polygonInfo[pixelVal] = polygons; // new
                
                it++;
            }

            
        }  // end-for (x = 0, to nx)
    
        
        
        
        
        
        
        
        

        
        // Test first 10 polygons....
        /*
        int junkCounter = 0;
        OGRFeature *poFeature;
        outLayer->ResetReading();
        while( (poFeature = outLayer->GetNextFeature()) != NULL  && junkCounter <= 10)
        {
            OGRFeatureDefn *poFDefn = outLayer->GetLayerDefn();
            int iField;
            for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
            {
                OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
                if( poFieldDefn->GetType() == OFTInteger )
                    printf( "%d,", poFeature->GetFieldAsInteger( iField ) );
                else if( poFieldDefn->GetType() == OFTInteger64 )
                    printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
                else if( poFieldDefn->GetType() == OFTReal )
                    printf( "%.3f,", poFeature->GetFieldAsDouble(iField) );
                else if( poFieldDefn->GetType() == OFTString )
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
                else
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
            }
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();
            if (poGeometry == NULL) {
                printf( "geometry is null" );
            } else if( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint ) {
                OGRPoint *poPoint = (OGRPoint *) poGeometry;
                printf( "%.3f,%3.f\n", poPoint->getX(), poPoint->getY() );
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ) {
                OGRPolygon *polygon = (OGRPolygon *) poGeometry;
                char *wktChars;
                polygon->exportToWkt(&wktChars);
                printf( "a polygon: %s\n", wktChars);
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbLineString ) {
                printf( "a LineString!\n" );
            } else {
                printf( "no tested geometry\n" );
            }
            OGRFeature::DestroyFeature( poFeature );
            
            junkCounter++;
        }
         */
         
         
        
        //GDALClose( pTgtDataset );
        //GDALClose( poDstMem );
        return;
    }


    
    /*
    Raster* Raster::polygonToRaster(const std::string &newRasterName) {
        // Create the new Raster, to hold the transformed raster, using the same type and size as this raster:
        RasterType type = raster_datatype;
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(newRasterName, type, get_nx(), get_ny());
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(newRasterName);
        }
        return polygonToRaster(rasNew);
    }
    

    Raster* Raster::polygonToRaster(Raster *rasNew) {
        switch(raster_datatype) {
            case INT8U:
                return polygonToRasterType<uint8_t>(rasNew);
                
                 case INT8S:
                 return polygonToRasterType<int8_t>(rasNew,);
                 case INT16U:
                 return polygonToRasterType<uint16_t>(rasNew);
                 case INT16S:
                 return polygonToRasterType<int16_t>(rasNew);
                 case INT32U:
                 return polygonToRasterType<uint32_t>(rasNew);
                 case INT32S:
                 return polygonToRasterType<int32_t>(rasNew);
                 case INT64U:
                 return polygonToRasterType<uint64_t>(rasNew);
                 case INT64S:
                 return polygonToRasterType<int64_t>(rasNew);
                
            case REAL32:
                return polygonToRasterType<float>(rasNew);
                
            case REAL64:
                return polygonToRasterType<double>(rasNew);
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
    }
    
    

    template <typename T>
    Raster*  Raster::polygonToRasterType(Raster *rasNew) {
     
        long int nx = in.getDeltaX();
        long int ny = in.getDeltaY();
        long int xIn0 = in.getX0();
        long int yIn0 = in.getY0();
        long int xInMax = xIn0 + nx;
        long int yInMax = yIn0 + ny;
        
        std::vector<T> data(nx);             // new data;  this will hold the output slice
        in.setDeltaY(1);
        for (int j = 0; j < nx; j++)
            data[j] = pixelVal;

        for (int i = yIn0; i < yInMax; i++) {
            in.setY0(i);
            try {
                this->write(in,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << "H5::DataSetIException  for '" << fullRastername << "'  " << e.getCDetailMsg() << std::endl;
            }
        }
     
        return rasNew;
    }
    */
    

    
    //Vector* Raster::rasterToPolygon(const std::string &newVectorName) {
    void Raster::OLDrasterToPolygon(const std::string &newVectorName) {
        // Create the new Vector, to hold the transformed raster, using the same type and size as this raster:
        /*
         RasterType type = raster_datatype;
         GeoStar::Vector *vecNew;
         try {
         vecNew = image->create_vector(newVectorName, type, get_nx(), get_ny());
         } catch (VectorExistsException e) {
         rasNew = image->open_raster(newVectorName);
         }
         return rasterToPolygon(rasNew);
         */
        // TEMP:
        OLDrasterToPolygon();
        return;
    }
    
    
    //Vector* Raster::rasterToPolygon(Vector *vecNew) {
    void Raster::OLDrasterToPolygon() {
        switch(raster_datatype) {
            case INT8U:
                //return OLDrasterToPolygonType<uint8_t>(vecNew);
                OLDrasterToPolygonType<uint8_t>();
                break;
                /*
                 case INT8S:
                 return OLDrasterToPolygonType<int8_t>(vecNew,);
                 case INT16U:
                 return OLDrasterToPolygonType<uint16_t>(vecNew);
                 case INT16S:
                 return OLDrasterToPolygonType<int16_t>(vecNew);
                 case INT32U:
                 return OLDrasterToPolygonType<uint32_t>(vecNew);
                 case INT32S:
                 return OLDrasterToPolygonType<int32_t>(vecNew);
                 case INT64U:
                 return OLDrasterToPolygonType<uint64_t>(vecNew);
                 case INT64S:
                 return OLDrasterToPolygonType<int64_t>(vecNew);
                 */
            case REAL32:
                //return OLDrasterToPolygonType<float>(vecNew);
                OLDrasterToPolygonType<float>();
                break;
                
            case REAL64:
                //return OLDrasterToPolygonType<double>(vecNew);
                OLDrasterToPolygonType<double>();
                break;
                //case COMPLEX_INT16:
                //    return
            default:
                throw_RasterUnsupportedTypeError(fullRastername);
        }
    }
    
    
    
    template <typename T>
    //Vector*  Raster::rasterToPolygonType(Vector *vecNew) {
    void  Raster::OLDrasterToPolygonType() {
        
        //GDALDataset *poDataset;
        GDALAllRegister();
        GDALDriver *poDriverMem;
        
        // NOTE: when getting a driver, to then create the (output) vector dataset, when you use
        //  any format other than "Memory", you must specify an output file name (in the create vector
        //  dataset call).  This file is then created and written to, when running GDALPolygonize.
        //
        // See  http://www.gdal.org/ogr_formats.html  for list of formats allowed:
        //
        //GDALDriver * pDriver = GetGDALDriverManager()->GetDriverByName( "ESRI Shapefile" );
        //X GDALDriver * pDriver = GetGDALDriverManager()->GetDriverByName( "OpenStreetMap XML and PBF" );
        //X GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "OpenStreetMap XML" );
        //GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "GeoJSON" );
        //X GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "XML" );
        
        // NOTE!!! GML is the (default) output format of gdal_polygonize.py utility program!!!
        //  (see my files generated: polygonCrap.tif     and   polygonCrap.xsd)
        //GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "GML" );
        GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName( "Memory" );
        
        if (pDriver == NULL) {
            std::cerr << " driver is NULL!" << std::endl;
            return;
        }
        char **papszMetadata;
        
        poDriverMem = GetGDALDriverManager()->GetDriverByName("MEM");
        
        papszMetadata = poDriverMem->GetMetadata();
        
        GDALDataset *poDstMem;
        char **papszOptions = NULL;
        GDALRasterBand *pMaskBand = NULL;
        
        int nx = get_nx();
        int ny = get_ny();
        
        //GDALDataType typ = getGDALType(ras);
        GDALDataType typ = GDT_Byte;
        
        //poDstMem = poDriverMem->Create( "", nx, ny, channels.size(), typ, papszOptions );
        poDstMem = poDriverMem->Create( "", nx, ny, 1, typ, papszOptions );
        std::cerr << " got here a" << std::endl;
        
        
        //char *tgtPath = "polygonProgJunk.shp";
        //GDALDataset * pTgtDataset = pDriver->Create( tgtPath, 0, 0, 0, GDT_Unknown, NULL );
        
        // UNCOMMENT LATER >>>>>
        //GDALDataset * pTgtDataset = pDriver->Create( "polygonMoreProgJunk2", 0, 0, 0, GDT_Unknown, NULL );
        
        //try ... (above WORKS)
        //GDALDataset * pTgtDataset = poDriverMem->Create( "x", 0, 0, 0, GDT_Unknown, NULL );  X
        //GDALDataset * pTgtDataset = poDriverMem->Create( "x", nx, ny, 1, GDT_Unknown, NULL );   X
        GDALDataset * pTgtDataset = pDriver->Create( "", 0, 0, 0, GDT_Unknown, NULL );
        
        
        
        std::cerr << "got here b" << std::endl;
        
        
        
        GDALRasterBand *poBand;
        
        // loop for all the rasters in the image, actually ....
        poBand = poDstMem->GetRasterBand(1);     //(i+1);
        
        GeoStar::Slice slice(0,0,nx,1);
        std::vector<uint8_t> buf(nx);      //really .... templated
        
        for(int iy=0;iy<ny;++iy){
            slice.setY0(iy);
            read(slice, buf);
            // write internal raster buffer to the gdal raster band.
            CPLErr err = poBand->RasterIO( GF_Write,
                                          0, iy, nx, 1,
                                          &buf[0], nx, 1, typ,   // &buf[0] for parameter void *
                                          0, 0 );
        } // endfor
        
        //std::cerr << "OK ... got here ..." << std::endl;
        
        // Create a layer.
        //OGRLayer *outLayer = poDstMem->CreateLayer( "layer", NULL, wkbPolygon, NULL );
        OGRLayer *outLayer = pTgtDataset->CreateLayer( "layer", NULL, wkbPolygon, NULL );
        int valueFieldIdx = 0;
        
        //GDALPolygonize(poBand, pMaskBand, outLayer, valueFieldIdx, papszOptions, GDALTermProgress, NULL);
        Polygonize(poBand, pMaskBand, outLayer, valueFieldIdx, papszOptions, GDALTermProgress, NULL);
        //std::cerr << " here 1" << std::endl;
        
        
        int junkCounter = 0;
        OGRFeature *poFeature;
        outLayer->ResetReading();
        while( (poFeature = outLayer->GetNextFeature()) != NULL)    //  && junkCounter <= 10)
        {
            OGRFeatureDefn *poFDefn = outLayer->GetLayerDefn();
            int iField;
            for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
            {
                OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
                if( poFieldDefn->GetType() == OFTInteger )
                    printf( "%d,", poFeature->GetFieldAsInteger( iField ) );
                else if( poFieldDefn->GetType() == OFTInteger64 )
                    printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
                else if( poFieldDefn->GetType() == OFTReal )
                    printf( "%.3f,", poFeature->GetFieldAsDouble(iField) );
                else if( poFieldDefn->GetType() == OFTString )
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
                else
                    printf( "%s,", poFeature->GetFieldAsString(iField) );
            }
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();
            if (poGeometry == NULL) {
                printf( "geometry is null" );
            } else if( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
            {
                OGRPoint *poPoint = (OGRPoint *) poGeometry;
                printf( "%.3f,%3.f\n", poPoint->getX(), poPoint->getY() );
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ) {
                OGRPolygon *polygon = (OGRPolygon *) poGeometry;
                
                // Convert the char* to c++ string type
                //std::string wktSource(wktStrSrc);
                char *wktChars;
                polygon->exportToWkt(&wktChars);
                //std::string wktStr(wktChars);
                printf( "a polygon: %s\n", wktChars);
            } else if ( wkbFlatten(poGeometry->getGeometryType()) == wkbLineString ) {
                printf( "a LineString!\n" );
            } else {
                printf( "no tested geometry\n" );
            }
            OGRFeature::DestroyFeature( poFeature );
            
            junkCounter++;
        }
        GDALClose( pTgtDataset );
        GDALClose( poDstMem );
        
        
        return;
    }

    

    //template Vector* Raster::rasterToPolygonType<uint8_t>(Vector*);
    //template Vector* Raster::rasterToPolygonType<float>(Vector*);
    //template Vector* Raster::rasterToPolygonType<double>(Vector*);
    template void Raster::rasterToPolygonType<uint8_t>();
    template void Raster::rasterToPolygonType<float>();
    template void Raster::rasterToPolygonType<double>();

    template void Raster::OLDrasterToPolygonType<uint8_t>();
    template void Raster::OLDrasterToPolygonType<float>();
    template void Raster::OLDrasterToPolygonType<double>();

    //template Raster* Raster::polygonToRasterType<uint8_t>(Raster*);
    //template Raster* Raster::polygonToRasterType<float>(Raster*);
    //template Raster* Raster::polygonToRasterType<double>(Raster*);

}// end namespace GeoStar
