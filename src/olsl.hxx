#ifndef OLSL_HXX_
#define OLSL_HXX_

#include <cpl_conv.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogrsf_frmts.h>
#include <vector>

#include <fstream>

namespace oll
{
    typedef std::vector< double > objectValuesType;
    typedef std::map< int, std::map< int, objectValuesType > > allValuesType;
    typedef struct
    {
        double avg;
        double var;
        double stdev;
    } statValStructType;
    typedef std::map< int, std::map< int, statValStructType > > statValuesType;

    GDALDataset* openVectorDs(const char* fileName);
    GDALDataset* openRasterDs(const char* fileName);
    allValuesType readData(GDALDataset* trainingSites, const char* classAttribute, const char* idAttribute, GDALDataset* satelliteImage);
    bool pixelPolyGeomIntersection(OGRPolygon* polygonGeom, OGREnvelope* polygonEnvelope, double* xres, double* yres, int* width,
                                   int* pixel);
    void writeObjStat(const allValuesType values, const char* filename);
    double avg(const oll::objectValuesType& values);
}

#endif
