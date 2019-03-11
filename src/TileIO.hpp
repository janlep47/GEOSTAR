// TileIO.hpp
//
// by Janice Richards, Oct 23, 2017
//
//----------------------------------------
#ifndef TILEIO_HPP_
#define TILEIO_HPP_


#include <string>
#include <vector>
#include <map>


#include "H5Cpp.h"

#include "Slice.hpp"
#include "WarpParameters.hpp"
#include "TileIO.hpp"
#include "RasterType.hpp"
#include "attributes.hpp"
#include "Exceptions.hpp"


namespace GeoStar {
    class Raster;
    class Slice;

    template <class T>
    class Tile {
    public:
        Slice slice;
        int sliceWidth;
        std::vector<T> data;
        Tile(Slice &slice) {
            this->slice = slice;
            sliceWidth = slice.getDeltaX();
            if ((sliceWidth * slice.getDeltaY()) < 0) {
                std::cerr << " OOPS ";
                if (sliceWidth < 0) std::cerr << " deltaX < 0 ";
                else std::cerr << " deltaY < 0 ";
            }
            data.resize(sliceWidth * slice.getDeltaY());
        }
        
        Tile() {
            sliceWidth = 0;
        }
        
    };

  template <class T>
  class TileIO {

  private:
      const Raster *raster;
      std::map<int, Tile<T> > tiles;
      int maxNumberTiles;
      Slice tileDescriptor;
      // Key is the tilenumber, and value is the last time accessed (timer)
      std::map<int, long int> lastAccessTimes;
      long int timer;
      
      int rasterWidth;
      int rasterHeight;
      
      int numberXTiles;
      int numberYTiles;
      double oneOverNumberXTiles;
      
      double xScale;
      double yScale;
      
      Tile<T> verifyNeededTileIsAvailable(const int &tileNeeded);
      void readInNeededTile(int tileNumber);
      void throwOutLeastUsedTile();

  public:
      // create a TileIO with strideX and strideY (defaulted to 1), to be able to skip over pixels,
      //  i.e. skip every strideX pixels (in x-direction), etc.
      TileIO(Raster *raster, const Slice &tileDescriptor, const int &maxNumberTiles = 1);
      
      // return maxNumberTiles
      inline long int getMaxNumberTiles() {
        return maxNumberTiles;
      }
      
      // set maxNumberTiles
      inline void setMaxNumberTiles(const int &maxNumberTiles) {
          this->maxNumberTiles = maxNumberTiles;
      }
      
      std::vector<T> tileRead(const Slice &pixelsToRead);

  }; // end class: TileIO
  
}// end namespace GeoStar


#endif //TILEIO_HPP_
