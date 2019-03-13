// polygonize.hpp
//
// by Janice Richards, Mar 14, 2018
//
//----------------------------------------


namespace GeoStar {
    
    /*
    class Raster {

        private:

            //template <typename T>
            //Raster* flipType(Raster *outRaster, const int flipAxis);
      

        public:
            //static const short FLIP_HORIZONTALLY = 1;
            //static const short FLIP_VERTICALLY = 2;
            //static const short FLIP_BOTH = 3;

            //Raster* flip(const std::string &newRasterName, short flipAxis);
            //Raster* flip(Raster *outRaster, short flipAxis);
    }
     */
    
    CPLErr CPL_DLL CPL_STDCALL
    Polygonize( GDALRasterBandH hSrcBand,
                   GDALRasterBandH hMaskBand,
                   OGRLayerH hOutLayer, int iPixValField,
                   char **papszOptions,
                   GDALProgressFunc pfnProgress,
                   void * pProgressArg );
    
    CPLErr CPL_DLL CPL_STDCALL
    FPolygonize( GDALRasterBandH hSrcBand,
                    GDALRasterBandH hMaskBand,
                    OGRLayerH hOutLayer, int iPixValField,
                    char **papszOptions,
                    GDALProgressFunc pfnProgress,
                    void * pProgressArg );

    
    
    
    /************************************************************************/
    /*                          Polygon Enumerator                          */
    /************************************************************************/
    
#define GP_NODATA_MARKER -51502112
    
    template<class DataType, class EqualityTest> class RasterPolygonEnumeratorT
    
    {
    private:
        void     MergePolygon( int nSrcId, int nDstId );
        int      NewPolygon( DataType nValue );
        
    public:  // these are intended to be readonly.
        
        GInt32   *panPolyIdMap;
        DataType   *panPolyValue;
        
        int      nNextPolygonId;
        int      nPolyAlloc;
        
        int      nConnectedness;
        
    public:
        RasterPolygonEnumeratorT( int nConnectedness=4 );
        ~RasterPolygonEnumeratorT();
        
        void     ProcessLine( DataType *panLastLineVal, DataType *panThisLineVal,
                             GInt32 *panLastLineId,  GInt32 *panThisLineId,
                             int nXSize );
        
        void     CompleteMerges();
        
        void     Clear();
    };
    
    struct IntEqualityTest
    {
        bool operator()(GInt32 a, GInt32 b) { return a == b; }
    };
    
    typedef RasterPolygonEnumeratorT<GInt32, IntEqualityTest> RasterPolygonEnumerator;
    
    /*
    typedef void* (*GDALTransformDeserializeFunc)( CPLXMLNode *psTree );
    
    void* GDALRegisterTransformDeserializer(const char* pszTransformName,
                                            GDALTransformerFunc pfnTransformerFunc,
                                            GDALTransformDeserializeFunc pfnDeserializeFunc);
    void GDALUnregisterTransformDeserializer(void* pData);
    
    void GDALCleanupTransformDeserializerMutex();
    */
    
    /* Transformer cloning */
    
    /*
    void* GDALCreateTPSTransformerInt( int nGCPCount, const GDAL_GCP *pasGCPList,
                                      int bReversed, char** papszOptions );
    
    void CPL_DLL * GDALCloneTransformer( void *pTransformerArg );
    */

    
}

