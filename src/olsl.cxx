#include "olsl.hxx"
#include <iostream>

namespace oll
{
GDALDataset *openVectorDs(const char *fileName)
{
    GDALDataset *pDs;
    pDs = (GDALDataset *)GDALOpenEx(fileName, GDAL_OF_VECTOR, NULL, NULL, NULL);

    if (pDs == NULL)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Can't open dataset " << fileName << "!" << std::endl;
        exit(-1);
    }

    return pDs;
};

GDALDataset *openRasterDs(const char *fileName)
{
    GDALDataset *pDs;
    pDs = (GDALDataset *)GDALOpen(fileName, GA_ReadOnly);

    if (pDs == NULL)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Can't open dataset " << fileName << "!" << std::endl;
        exit(-1);
    }

    return pDs;
};

void objectStatistics(GDALDataset *trainingSites, GDALDataset *satelliteImage, const char *outputFile)
{
    OGRLayer *pLyr;
    pLyr = trainingSites->GetLayer(0);

    if (pLyr == NULL)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Can't open training sites layer !" << std::endl;
        exit(-1);
    }

    OGRFeature *pFeat;
    pLyr->ResetReading();

    while((pFeat = pLyr->GetNextFeature()) != NULL)
    {
        std::cout << pFeat->GetFID() << std::endl;
    }

};
}
