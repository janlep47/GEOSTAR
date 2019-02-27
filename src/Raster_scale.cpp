// Raster_scale.cpp
//
// by Janice Richards, Feb 20, 2018
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

//#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {
    
    
    Raster* Raster::scale(const double &xratio, const double &yratio) {
        long int newNx, newNy;
        newNx = (long int) (xratio*get_nx());
        newNy = (long int) (yratio*get_ny());
        //std::cout << "newNx=" << newNx << "   newNy=" << newNy << std::endl;
        return scale(newNx, newNy);
    }
    

    Raster* Raster::scale(const long int &nx, const long int &ny) {
        // create a new raster with name 'chan_<nx>_<ny>', with same type as this (source) raster,
        //  under the same image that this raster is part of:
        std::string rasName = "chan_"+std::to_string(nx)+"_"+std::to_string(ny);
        return scale(nx, ny, raster_datatype, rasName);
    }
    
    
    Raster* Raster::scale(const double &xratio, const double &yratio, const RasterType &type, const std::string &name) {
        long int newNx, newNy;
        newNx = (long int) (xratio*get_nx());
        newNy = (long int) (yratio*get_ny());
        //std::cout << "newNx=" << newNx << "   newNy=" << newNy << std::endl;
        return scale(newNx, newNy, type, name);
    }

    
    Raster* Raster::scale(const long int &nx, const long int &ny, const RasterType &type, const std::string &name) {
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(name, type, nx, ny);
        } catch (RasterExistsException e) {
            //std::cerr << "*** Raster: " << name << " already exists!" << std::endl;
            rasNew = image->open_raster(name);
            return 0;
        }
        return scale(rasNew);
    }
    
    Raster* Raster::scale(GeoStar::Raster *rasNew) {
        Slice in(0,0,get_nx(),get_ny());
        return scale(in, rasNew);
    }
    
    
    Raster* Raster::scale(const Slice &in, const double &xratio, const double &yratio) {
        long int newNx, newNy;
        Slice inputSlice = in;
        newNx = (long int) (xratio*inputSlice.getDeltaX());
        newNy = (long int) (yratio*inputSlice.getDeltaY());
        //std::cout << "newNx=" << newNx << "   newNy=" << newNy << std::endl;
        return scale(in, newNx, newNy);
    }
    
    
    Raster* Raster::scale(const Slice &in, const long int &nx, const long int &ny) {
        std::string rasName = "chan_"+std::to_string(nx)+"_"+std::to_string(ny);
        return scale(in, nx, ny, raster_datatype, rasName);
    }
    
    Raster* Raster::scale(const Slice &in, const double &xratio, const double &yratio, const RasterType &type, const std::string &name) {
        long int newNx, newNy;
        Slice inputSlice = in;
        newNx = (long int) (xratio*inputSlice.getDeltaX());
        newNy = (long int) (yratio*inputSlice.getDeltaY());
        //std::cout << "newNx=" << newNx << "   newNy=" << newNy << std::endl;
        return scale(in, newNx, newNy, type, name);
    }
    
    Raster* Raster::scale(const Slice &in, const long int &nx, const long int &ny, const RasterType &type, const std::string &name) {
        GeoStar::Raster *rasNew;
        try {
            rasNew = image->create_raster(name, type, nx, ny);
        } catch (RasterExistsException e) {
            rasNew = image->open_raster(name);
            return 0;
        }
        return scale(in,rasNew);
    }
    
    Raster* Raster::scale(const Slice &in, GeoStar::Raster *outRaster) {
        Slice sliceOut(0,0,outRaster->get_nx(),outRaster->get_ny());
        return scale(in, sliceOut, outRaster);
    }
    
    
    Raster* Raster::scale(const Slice &inSlice, const Slice &outSlice, GeoStar::Raster *outRaster) {
        Slice out = outSlice;
        Slice in = inSlice;
        verifySlice(in);

        // NOW, create 3 "ground control points", that correspond to points in the output slice,
        //  which will map to the input slice
        int ngcp = 3;
        int order = 0;
        //double rix[3], riy[3], rox[3], roy[3];
        //double rmsx[3], rmsy[3];
        std::vector<double> rix(ngcp), riy(ngcp), rox(ngcp), roy(ngcp);
        double rmsx, rmsy;
        
        
        // Compare input slice points (x0,y0), (x0+deltaX, y0), (x0+deltaX, y0+deltaY) with
        //  the corresponding 3 points in the output slice, to get the polynomial coefficients,
        //  for the warp function that will scale the output raster based on these input and output
        //   slices:
        rix[0] = (double)in.getX0(); rix[1] = rix[0] + in.getDeltaX(); rix[2] = rix[1];
        riy[0] = (double)in.getY0(); riy[1] = riy[0]; riy[2] = riy[0] + in.getDeltaY();
        
        rox[0] = (double)out.getX0(); rox[1] = rox[0] + out.getDeltaX(); rox[2] = rox[1];
        roy[0] = (double)out.getY0(); roy[1] = roy[0]; roy[2] = roy[0] + out.getDeltaY();
        
        GeoStar::WarpParameters warpData;
        // go backwards, because we will START with the new output coordinates, and map them to the
        //  old input coordinates:
        warpData.GCPRegression2D(rox, roy, rix, riy, &order, &rmsx, &rmsy);
        return warp(warpData, in, out, outRaster);
    }
    

}// end namespace GeoStar
