#ifndef OLSL_HXX_
#define OLSL_HXX_

#include <cpl_conv.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogrsf_frmts.h>
#include <vector>

namespace oll
{
GDALDataset *openVectorDs(const char *fileName);
GDALDataset *openRasterDs(const char *fileName);
void objectStatistics(GDALDataset *trainingSites, const char *classAttribute, const char *idAttribute, GDALDataset *satelliteImage,
                      const char *outputFile);
bool pixelPolyGeomIntersection(OGRPolygon *polygonGeom, OGREnvelope *polygonEnvelope, double *xres, double *yres, int *width, int *height,
                               int *pixel);
}

#endif
