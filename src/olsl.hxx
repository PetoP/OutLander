#ifndef OLSL_HXX_
#define OLSL_HXX_

#include <cpl_conv.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogrsf_frmts.h>
#include <vector>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

namespace oll
{
    // vector for storing pixel values
    typedef std::vector< double > PixelValuesType;

    // // struct for storing data per training site
    // typedef struct
    // {
    //     int classId;                                    // stores training site classId
    //     std::map< int, PixelValuesType > valuesPerBand; // stores band number and pixel values in that band
    // } objectValuesStructType;

    // // map that stores data about all training sites
    // typedef std::map< int,                     // training site ID
    //                   objectValuesStructType > // informations abou training site
    //     allValuesType;

    // typedef struct
    // {
    //     long count;
    //     double avg;
    //     double stdev;
    // } statValStructType;

    // typedef std::map< int, std::map< int, statValStructType > > classStatType;

    // class stores data for individual training sites
    class TrainingSite
    {
      public:
        typedef std::map< int, PixelValuesType > ValuesPerBandType;
        typedef std::vector< int > BandsVectorType;

      private:
        int id;                          // training site id
        int coverClass;                  // landcover class of training site
        ValuesPerBandType valuesPerBand; // pixel values per band
        BandsVectorType bands;           // bads with values
        long pixelCount;

      public:
        TrainingSite();
        TrainingSite(const int id, const int coverClass);
        ~TrainingSite();
        void addBandValues(const int band, const PixelValuesType pixelValues); // add pixel values for band
        PixelValuesType getBandValues(int band) const;                         // get pixel values for band
        double getBandAvg(int band) const;                                     // get average of pixel values for band
        double getBandVariance(int band) const;                                // get variace of pixel values for band
        double getBandStdev(int band) const;                                   // get standard deviation of pixel values for band
        long getPixelCount() const;                                            // get number of pixels in training site
        int getId() const;                                                     // get training site ID
        int getCoverClass() const;                                             // get landcover class of training site
        const BandsVectorType& getBands() const;
    };

    // class for storing all training sites
    class TrainingSitesContainer
    {
      public:
        typedef std::vector< TrainingSite > TrainingSitesType;
        typedef std::vector< int > TrainingSitesIdsType;

      private:
        TrainingSitesType trainingSites;       // stores all training sites
        TrainingSitesIdsType trainingSitesIds; // stores all ids of training sites

      public:
        void addTrainingSite(TrainingSite trSite); // add new training site to container
        void removeTrainingSite(const int id);     // remove training site from container
        const TrainingSite& getTrainingSite(const int id) const;
        int getTrainingSitesCount() const;
        const TrainingSitesIdsType& getTrainingSitesIds() const;
        const TrainingSitesType& getTrainingSites() const;
    };

    GDALDataset* openVectorDs(const char* fileName);
    GDALDataset* openRasterDs(const char* fileName);
    TrainingSitesContainer readData(GDALDataset* trainingSites, const char* classAttribute, const char* idAttribute,
                                    GDALDataset* satelliteImage);
    bool pixelPolyGeomIntersection(OGRPolygon* polygonGeom, OGREnvelope* polygonEnvelope, double* xres, double* yres, int* width,
                                   int* pixel);
    void writeObjStat(const TrainingSitesContainer& trainingSitesContainer, const char* filename);
    void writeClassStat(const TrainingSitesContainer& trainingSitesContainer, const char* filename);
}

#endif
