// TileIO.cpp
//
// by Janice Richards, Oct 23, 2017
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <map>

#include "geostar.hpp"
#include "Slice.hpp"
#include "TileIO.hpp"

namespace GeoStar {
    
    template <class T>
    TileIO<T>::TileIO(Raster *raster, const Slice &tileDescriptor, const int &maxNumberTiles) {
        this->raster = raster;
        this->tileDescriptor = tileDescriptor;
        this->maxNumberTiles = maxNumberTiles;
        rasterWidth = raster->get_nx();
        rasterHeight = raster->get_ny();
        xScale = 1.0 / tileDescriptor.getDeltaX();
        yScale = 1.0 / tileDescriptor.getDeltaY();
        
        numberXTiles = (int) floor(rasterWidth * xScale);
        if ((numberXTiles*tileDescriptor.getDeltaX()) != rasterWidth) numberXTiles++;
        numberYTiles = (int) floor(rasterHeight * yScale);
        if ((numberYTiles*tileDescriptor.getDeltaY()) != rasterHeight) numberYTiles++;
        oneOverNumberXTiles = 1.0 / numberXTiles;
        timer = 0;
    }// end-TileIO-constructor
    
    
    
    //change Slice to have TOTAL#PIXELS (set to deltaX*deltaY) ... so don't have to continually multiply here
    template <class T>
    std::vector<T> TileIO<T>::tileRead(const Slice &pixelsToReadIn) {
        Slice pixelsToRead = pixelsToReadIn;
        int minX = pixelsToRead.getX0();
        int maxX = minX + pixelsToRead.getDeltaX() - 1;
        int minY = pixelsToRead.getY0();
        int maxY = minY + pixelsToRead.getDeltaY() - 1;

        std::vector<T> data(pixelsToRead.getNumberPixels());
        
        // Check for illegal access:
        if (minX < 0) {
            throw_SliceDataError("negative x0 Slice value");
            //return data;
        } else if (maxX > rasterWidth) {
            throw_SliceDataError("delta-x Slice value beyond raster width");
            //return data;
        } else if (minY < 0) {
            throw_SliceDataError("negative y0 Slice value");
            //return data;
        } else if (maxY > rasterHeight) {
            throw_SliceDataError("delta-y Slice value beyond raster height");
            //return data;
        }
        std::map<int, long int>::iterator it;     // <tile#,timer>  (timer = last accessed time)
        
        int yTile, xTile;
        xTile = minX * xScale;
        yTile = minY * yScale;
        int tileNeeded = xTile + (yTile * numberXTiles);
        
        int previousTileNumber = tileNeeded;
        
        Tile<T> currentTile = verifyNeededTileIsAvailable(tileNeeded);
        Tile<T> originalTile = currentTile;
        int originalTileNeeded = tileNeeded;
        
        int x0 = currentTile.slice.getX0();
        int y0 = currentTile.slice.getY0();
        int tileWidth = currentTile.slice.getDeltaX();  // can be smaller than normal slice width, if @ end
        int tileHeight = currentTile.slice.getDeltaY();  //   "   slice height "
        
        int x,y,n,timer;
        n = 0;
        
        bool xTilec, yTilec;
        xTilec = false;

        for (int j = minY; j <= maxY; j++) {
            yTilec = false;
            // If xtile needed was changed, (at most ONCE for now, since only reading at most 2X2 pixels!!)
            //   during reading x values, reset the tile to the previous one, to continue reading
            //   the y values:
            if (xTilec) {
                currentTile = originalTile;
                tileNeeded = originalTileNeeded;
                x0 = currentTile.slice.getX0();
                y0 = currentTile.slice.getY0();
                tileWidth = currentTile.slice.getDeltaX();
                tileHeight = currentTile.slice.getDeltaY();
            }
            xTilec = false;
            // "while" for possible future cases where many pixels are to be read, possibly spanning
            //  more tiles than 2X2 (Max for reading up to 4 pixels):
            while (j > (y0 + tileHeight - 1)) {
                yTilec = true;
                tileNeeded += numberXTiles;
                // set the last accessed time for the previous tile
                it = lastAccessTimes.find(previousTileNumber);
                if (it != lastAccessTimes.end())
                    lastAccessTimes[previousTileNumber] = timer;
                previousTileNumber = tileNeeded;
                // Read in next needed tile, if NOT already available in Tile heap (map)
                currentTile = verifyNeededTileIsAvailable(tileNeeded);
                x0 = currentTile.slice.getX0();
                y0 = currentTile.slice.getY0();
                tileWidth = currentTile.slice.getDeltaX();
                tileHeight = currentTile.slice.getDeltaY();
            }
            y = j - y0;
            for (int i = minX; i <= maxX; i++) {
                // "while" for possible future cases where many pixels are to be read, possibly spanning
                //  more tiles than 2X2 (Max for reading up to 4 pixels):
                while (i > (x0 + tileWidth - 1)) {
                    xTilec = true;
                    tileNeeded++;
                    // set the last accessed time for the previous tile
                    it = lastAccessTimes.find(previousTileNumber);
                    if (it != lastAccessTimes.end())
                        lastAccessTimes[previousTileNumber] = timer;
                    previousTileNumber = tileNeeded;
                    // Read in next needed tile, if NOT already available in Tile heap (map)
                    currentTile = verifyNeededTileIsAvailable(tileNeeded);
                    x0 = currentTile.slice.getX0();
                    y0 = currentTile.slice.getY0();
                    tileWidth = currentTile.slice.getDeltaX();
                    tileHeight = currentTile.slice.getDeltaY();
                }
                x = i - x0;
                data[n] = currentTile.data[x+(y*tileWidth)];
                n++;
                // Update access info:
                timer++;
            }
        }
        // Update most recent tile used (last-accessed-time)
        it = lastAccessTimes.find(tileNeeded);
        if (it != lastAccessTimes.end())
            lastAccessTimes[tileNeeded] = timer;

        return data;
    }

    
    
    template <class T>
    Tile<T> TileIO<T>::verifyNeededTileIsAvailable(const int &tileNeeded) {
        typename std::map<int,Tile<T> >::iterator it;
        it = tiles.find(tileNeeded);
        if (it == tiles.end()) {
            readInNeededTile(tileNeeded);
            it = tiles.find(tileNeeded);
        }
        return it->second;
    }
     
    
    
    template <class T>
    void TileIO<T>::readInNeededTile(int tileNumber) {
        int x0, y0, deltaX, deltaY;

        int yTileNumber = (int) (tileNumber * oneOverNumberXTiles);
        int xTileNumber = tileNumber - (yTileNumber * numberXTiles);
        deltaX = tileDescriptor.getDeltaX();
        deltaY = tileDescriptor.getDeltaY();
        x0 = xTileNumber * deltaX;
        y0 = yTileNumber * deltaY;
        
        // For possible fractional-sized tiles at right / bottom of raster:
        if ((x0 + deltaX - 1) >= raster->get_nx())
            deltaX = raster->get_nx() - x0;
        if ((y0 + deltaY - 1) >= raster->get_ny())
            deltaY = raster->get_ny() - y0;
        
        Slice tileSlice(x0,y0,deltaX,deltaY);
        
        // DIES at this call below (Tile constructor ...)
        Tile<T> newTile(tileSlice);

        // Throw out the least recently accessed tile, if we're at the maximum number of tiles:
        if (tiles.size() >= maxNumberTiles)
            throwOutLeastUsedTile();
        raster->read(tileSlice, newTile.data);
        tiles[tileNumber] = newTile;
        return;
    }
    
    
    
    template <class T>
    void TileIO<T>::throwOutLeastUsedTile() {
        typename std::map<int, Tile<T> >::iterator itTiles = tiles.begin();
        int tileNumber = itTiles->first;
        Tile<T> tile = itTiles->second;
        
        std::map<int, long int>::iterator itAccessTimes;

        long int timestamp = 0;
        itAccessTimes = lastAccessTimes.find(tileNumber);
        if (itAccessTimes != lastAccessTimes.end())
            timestamp = itAccessTimes->second;
        
        long int timestampMin = timestamp;
        int tileMinNumber = tileNumber;
        Tile<T> tileMin;
        itTiles++;
        while (itTiles != tiles.end()) {
            tileNumber = itTiles->first;
            tile = itTiles->second;
            
            timestamp = 0;
            itAccessTimes = lastAccessTimes.find(tileNumber);
            if (itAccessTimes != lastAccessTimes.end())
                timestamp = itAccessTimes->second;
            
            if (timestamp < timestampMin) {
                timestampMin = timestamp;
                tileMinNumber = tileNumber;
            }
            itTiles++;
        }
        
        tiles.erase(tileMinNumber);
        lastAccessTimes.erase(tileMinNumber);
        return;
    }
    
    // explicit instantiations
    template class TileIO<uint8_t>;
    template class TileIO<float>;
    template class TileIO<double>;

    
}// end namespace GeoStar
