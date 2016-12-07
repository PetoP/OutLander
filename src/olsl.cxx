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

void objectStatistics(GDALDataset *trainingSites, const char *classAttribute, const char *idAttribute, GDALDataset *satelliteImage,
                      const char *outputFile)
{
    // layer reading and testing
    OGRLayer *pLyr;
    pLyr = trainingSites->GetLayer(0);

    if (pLyr == NULL)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Can't open training sites layer !" << std::endl;
        exit(-1);
    }

    // attributes testing
    OGRFeatureDefn *pFeatDef = pLyr->GetLayerDefn();
    int idFieldIndex, classFieldIndex;
    idFieldIndex = pFeatDef->GetFieldIndex(idAttribute);
    classFieldIndex = pFeatDef->GetFieldIndex(classAttribute);

    if (idFieldIndex < 0)
    {
        // TODO toto vyrieš vínikou
        std::cerr << "Training sites vector does not have attribute \"" << idAttribute << "\"!" << std::endl;
        exit(-1);
    }

    if (classFieldIndex < 0)
    {
        // TODO toto vyrieš vínikou
        std::cerr << "Training sites vector does not have attribute \"" << classAttribute << "\"!" << std::endl;
        exit(-1);
    }

    if (pFeatDef->GetFieldDefn(idFieldIndex)->GetType() != OFTInteger)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Id field of training sites should be of type integer!" << std::endl;
        exit(-1);
    }

    if (pFeatDef->GetFieldDefn(classFieldIndex)->GetType() != OFTInteger)
    {
        // TODO toto vyrieš vínimkou
        std::cerr << "Class field of training sites should be of type integer!" << std::endl;
        exit(-1);
    }

    // extracting ifo from raster layer
    OGRSpatialReference projection = satelliteImage->GetProjectionRef();
    int bandCount = satelliteImage->GetRasterCount();
    double geoTransform[6];
    satelliteImage->GetGeoTransform(geoTransform);
    double *ulx = geoTransform;
    double *uly = geoTransform + 3;
    double *xres = geoTransform + 1;
    double *yres = geoTransform + 5;
    GDALDataType rasterDataType = satelliteImage->GetRasterBand(1)->GetRasterDataType();

    // feature iterating
    OGRFeature *pFeat;
    pLyr->ResetReading();

    bool featureOutsiteRaster;

    if (pLyr->GetGeomType() != OGRwkbGeometryType::wkbPolygon)
    {
        // TODO toto vyrieš výnimkou
        std::cerr << "Training sites vector geometry is not of type POLYGON!" << std::endl;
        exit(-1);
    }

    while ((pFeat = pLyr->GetNextFeature()) != NULL)
    {
        // feature reprojection and metadata reading
        OGRPolygon *pFeatGeom = (OGRPolygon *)pFeat->GetGeometryRef();
        pFeatGeom->transformTo(&projection);

        OGREnvelope *pFeatEnv = new OGREnvelope();
        pFeatGeom->getEnvelope(pFeatEnv);

        // info about clipped raster
        int xofset = (pFeatEnv->MinX - *ulx) / *xres;
        int yofset = (*uly - pFeatEnv->MaxY) / *yres;

        // check, if feature is within raster
        if (((pFeatEnv->MaxX - *ulx) / *xres > satelliteImage->GetRasterXSize()) || xofset < 0 ||
            ((*uly - pFeatEnv->MinY) / *yres < -satelliteImage->GetRasterYSize()) || yofset > 0)
        {
            featureOutsiteRaster = true;
        }
        else
        {
            featureOutsiteRaster = false;
        }

        if (!featureOutsiteRaster)
        {
            int width = ((pFeatEnv->MaxX - pFeatEnv->MinX) / *xres) + 1;
            int height = -((pFeatEnv->MaxY - pFeatEnv->MinY) / *yres) + 1;

            // reading raster data overlayd by feature bbox
            double *pRastData = (double *)CPLMalloc(sizeof(double) * width * height);

            for (int band = 1; band <= bandCount; ++band)
            {
                GDALRasterBand *pRastBand = satelliteImage->GetRasterBand(band);
                GDALRasterIO(pRastBand, GF_Read, xofset, -yofset, width, height, pRastData, width, height, GDT_Float64, 0, 0);

                //
                std::vector<double> values;
                for (int pixel = 0; pixel < (width * height); pixel++)
                {
                    if (pixelPolyGeomIntersection(pFeatGeom, pFeatEnv, xres, yres, &width, &height, &pixel))
                    {
                        values.push_back(*(pRastData + pixel));
                    }
                }

                double sum = 0;
                for (double value : values)
                {
                    sum += value;
                }

                float priemer = sum / values.size();

                std::cout << pFeat->GetFieldAsInteger(idFieldIndex) << ":" << band << " - " << priemer << std::endl;
            }
        }
    }
};

bool pixelPolyGeomIntersection(OGRPolygon *polygonGeom, OGREnvelope *polygonEnvelope, double *xres, double *yres, int *width, int *height,
                               int *pixel)
{
    int row, column;
    float x, y;

    row = *pixel / *width;
    column = *pixel - (row * *width);

    x = polygonEnvelope->MinX + (column * *xres) + *xres * 0.5;
    y = polygonEnvelope->MaxY - (row * -(*yres)) - *yres * 0.5;

    return polygonGeom->Intersects(new OGRPoint(x, y));
}
}
