#include <cpl_conv.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

namespace oll
{
GDALDataset *openVectorDs(const char *fileName);
GDALDataset *openRasterDs(const char *fileName);
void objectStatistics(GDALDataset *trainingSites, GDALDataset *satelliteImage, const char *outputFile);
}
