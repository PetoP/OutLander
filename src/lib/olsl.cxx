#include "olsl.hxx"

namespace oll
{
    GDALDataset* openVectorDs(const char* fileName)
    {
        GDALDataset* pDs;
        pDs = (GDALDataset*)GDALOpenEx(fileName, GDAL_OF_VECTOR, NULL, NULL, NULL);

        if (pDs == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open dataset " << fileName << "!" << std::endl;
            exit(-1);
        }

        return pDs;
    };

    OGRLayer* openVectorLyr(const char* fileName, const char* lyrName)
    {
        GDALDataset* pDs;
        pDs = (GDALDataset*)GDALOpenEx(fileName, GDAL_OF_VECTOR, NULL, NULL, NULL);

        if (pDs == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open dataset " << fileName << "!" << std::endl;
            exit(-1);
        }

        OGRLayer* pLyr = pDs->GetLayerByName(lyrName);

        if (pLyr == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open training sites layer \"" << lyrName << "\"!" << std::endl;
            exit(-1);
        }

        return pLyr;
    };

    GDALDataset* openRasterDs(const char* fileName)
    {
        GDALDataset* pDs;
        pDs = (GDALDataset*)GDALOpen(fileName, GA_ReadOnly);

        if (pDs == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open dataset " << fileName << "!" << std::endl;
            exit(-1);
        }

        return pDs;
    };

    oll::TrainingSitesContainer readData(GDALDataset* trainingSites, const char* classAttribute, const char* idAttribute,
                                         GDALDataset* satelliteImage)
    {
        // layer reading and testing
        OGRLayer* pLyr;
        pLyr = trainingSites->GetLayer(0);

        if (pLyr == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open training sites layer!" << std::endl;
            exit(-1);
        }

        return oll::readData(pLyr, classAttribute, idAttribute, satelliteImage);
    };

    oll::TrainingSitesContainer readData(OGRLayer* pLyr, const char* classAttribute, const char* idAttribute, GDALDataset* satelliteImage)
    {
        oll::TrainingSitesContainer trainingSitesContainer;
        // attributes testing
        OGRFeatureDefn* pFeatDef = pLyr->GetLayerDefn();
        int idFieldIndex, classFieldIndex;
        idFieldIndex = pFeatDef->GetFieldIndex(idAttribute);
        classFieldIndex = pFeatDef->GetFieldIndex(classAttribute);
        bool useFID = false;

        if (idFieldIndex < 0)
        {
            // TODO toto vyrieš vínikou
            std::cerr << "Id field not found, using geometry FID!" << std::endl;
            useFID = true;
        }

        if (classFieldIndex < 0)
        {
            // TODO toto vyrieš vínikou
            std::cerr << "Training sites vector does not have attribute \"" << classAttribute << "\"!" << std::endl;
            exit(-1);
        }

        if (!useFID)
        {
            if (pFeatDef->GetFieldDefn(idFieldIndex)->GetType() != OFTInteger)
            {
                // TODO toto vyrieš vínimkou
                std::cerr << "Id field of training sites should be of type integer!" << std::endl;
                exit(-1);
            }
        }

        if (pFeatDef->GetFieldDefn(classFieldIndex)->GetType() != OFTInteger)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Class field of training sites should be of type integer!" << std::endl;
            exit(-1);
        }

        if (pLyr->GetGeomType() != OGRwkbGeometryType::wkbPolygon)
        {
            // TODO toto vyrieš výnimkou
            std::cerr << "Training sites vector geometry is not of type POLYGON!" << std::endl;
            exit(-1);
        }

        // extracting ifo from raster layer
        OGRSpatialReference projection = satelliteImage->GetProjectionRef();
        int bandCount = satelliteImage->GetRasterCount();
        double geoTransform[6];
        satelliteImage->GetGeoTransform(geoTransform);
        double* ulx = geoTransform;
        double* uly = geoTransform + 3;
        double* xres = geoTransform + 1;
        double* yres = geoTransform + 5;

        OGRFeature* pFeat;
        OGRPolygon* pFeatGeom;
        OGREnvelope* pFeatEnv = new OGREnvelope();
        GDALRasterBand* pRastBand;
        bool featureOutsiteRaster;
        int xofset, yofset, width, height;
        double* pRastData;
        oll::PixelValuesType values;
        int id;

        // feature iteration
        pLyr->ResetReading();
        while ((pFeat = pLyr->GetNextFeature()) != NULL)
        {
            // feature reprojection and metadata reading
            pFeatGeom = (OGRPolygon*)pFeat->GetGeometryRef();
            pFeatGeom->transformTo(&projection);
            pFeatGeom->getEnvelope(pFeatEnv);
            if (useFID)
            {
                id = pFeat->GetFID();
            }
            else
            {
                id = pFeat->GetFieldAsInteger(idFieldIndex);
            }

            // info about clipped raster
            xofset = (pFeatEnv->MinX - *ulx) / *xres;
            yofset = (*uly - pFeatEnv->MaxY) / *yres;

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
                oll::TrainingSite trainingSite(id, pFeat->GetFieldAsInteger(classFieldIndex));

                // croped raster dimensions
                width = ((pFeatEnv->MaxX - pFeatEnv->MinX) / *xres) + 1;
                height = -((pFeatEnv->MaxY - pFeatEnv->MinY) / *yres) + 1;

                // band iteration
                for (int band = 1; band <= bandCount; ++band)
                {
                    // reading raster data overlayed by feature bbox
                    pRastData = (double*)CPLMalloc(sizeof(double) * width * height);
                    pRastBand = satelliteImage->GetRasterBand(band);
                    GDALRasterIO(pRastBand, GF_Read, xofset, -yofset, width, height, pRastData, width, height, GDT_Float64, 0, 0);

                    // reading intersecting pixels from cropped raster
                    values.clear();
                    for (int pixel = 0; pixel < (width * height); pixel++)
                    {
                        if (pixelPolyGeomIntersection(pFeatGeom, pFeatEnv, xres, yres, &width, &pixel))
                        {
                            values.push_back(*(pRastData + pixel));
                        }
                    }

                    trainingSite.addBandValues(band, values);
                }
                trainingSitesContainer.addTrainingSite(trainingSite);
            }
        }
        return trainingSitesContainer;
    };

    bool pixelPolyGeomIntersection(OGRPolygon* polygonGeom, OGREnvelope* polygonEnvelope, double* xres, double* yres, int* width,
                                   int* pixel)
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
