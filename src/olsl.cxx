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
        oll::TrainingSitesContainer trainingSitesContainer;

        // layer reading and testing
        OGRLayer* pLyr;
        pLyr = trainingSites->GetLayer(0);

        if (pLyr == NULL)
        {
            // TODO toto vyrieš vínimkou
            std::cerr << "Can't open training sites layer !" << std::endl;
            exit(-1);
        }

        // attributes testing
        OGRFeatureDefn* pFeatDef = pLyr->GetLayerDefn();
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

        // feature iteration
        pLyr->ResetReading();
        while ((pFeat = pLyr->GetNextFeature()) != NULL)
        {
            // feature reprojection and metadata reading
            pFeatGeom = (OGRPolygon*)pFeat->GetGeometryRef();
            pFeatGeom->transformTo(&projection);
            pFeatGeom->getEnvelope(pFeatEnv);

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
                oll::TrainingSite trainingSite(pFeat->GetFieldAsInteger(idFieldIndex), pFeat->GetFieldAsInteger(classFieldIndex));

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

    void writeObjStat(const TrainingSitesContainer& trainingSitesContainer, const char* filename)
    {
        // output CSV preparation
        std::ofstream outputCSV;
        outputCSV.open(filename);
        outputCSV << "id,count";

        // print headers
        const int firstId = trainingSitesContainer.getTrainingSitesIds()[0];
        const oll::TrainingSite& firstTrainingSite = trainingSitesContainer.getTrainingSite(firstId);
        const oll::TrainingSite::BandsVectorType bands = firstTrainingSite.getBands();
        for (const int& band : bands)
        {
            outputCSV << ",b" << band << "_avg"
                      << ",b" << band << "_stdev";
        }

        double avg, stdev;

        // iterate over training sites
        const oll::TrainingSitesContainer::TrainingSitesType trainingSites = trainingSitesContainer.getTrainingSites();
        for (const oll::TrainingSite& trainingSite : trainingSites)
        {
            outputCSV << std::endl << trainingSite.getId();
            outputCSV << "," << trainingSite.getCoverClass();

            for (const int& band : bands)
            {
                outputCSV << "," << trainingSite.getBandAvg(band) << "," << trainingSite.getBandStdev(band);
            }
        }

        outputCSV << std::endl;
        outputCSV.close();
    }

    void writeClassStat(const char* filename)
    {
        // output CSV preparation
        std::ofstream outputCSV;
        outputCSV.open(filename);
        outputCSV << "class,count";
        // for (int band = 1; band <= (int)classStat.begin()->second.size(); ++band)
        // {
        //     outputCSV << ",b" << band << "_avg"
        //               << ",b" << band << "_stdev";
        // }

        double avg, stdev;
        // for (auto const& record : classStat)
        // {
        //     outputCSV << std::endl << record.first;
        //     outputCSV << "," << record.second.begin()->second.count;

        //     for (auto const& band : record.second)
        //     {
        //         avg = band.second.avg;
        //         stdev = band.second.stdev;
        //         outputCSV << "," << avg << "," << stdev;
        //     }
        // }

        outputCSV << std::endl;
        outputCSV.close();
    }
}
