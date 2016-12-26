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
    typedef std::vector< int > BandsVectorType;

    // class stores data for individual training sites
    class TrainingSite
    {
      public:
        typedef std::map< int, PixelValuesType > ValuesPerBandType;

      private:
        int id;                          // training site id
        int spectralClass;               // spectral class of training site
        ValuesPerBandType valuesPerBand; // pixel values per band
        BandsVectorType bands;           // bads with values
        long pixelCount;

      public:
        TrainingSite();
        TrainingSite(const int id, const int coverClass);
        ~TrainingSite();
        void addBandValues(const int band, const PixelValuesType pixelValues); // add pixel values for band
        const PixelValuesType& getBandValues(const int band) const;            // get pixel values for band
        double getBandAvg(const int band) const;                               // get average of pixel values for band
        double getBandVariance(const int band) const;                          // get variace of pixel values for band
        double getBandStdev(const int band) const;                             // get standard deviation of pixel values for band
        long getPixelCount() const;                                            // get number of pixels in training site
        int getId() const;                                                     // get training site ID
        int getSpectralClass() const;                                          // get spectral class of training site
        const BandsVectorType& getBands() const;
    };

    // class for storing all training sites
    class TrainingSitesContainer
    {
      public:
        typedef std::vector< TrainingSite > TrainingSitesType;
        typedef std::vector< int > TrainingSitesIdsType;
        typedef std::vector< int > TrainingSitesSpecClassType;

      private:
        TrainingSitesType trainingSites;            // training sites
        TrainingSitesIdsType trainingSitesIds;      // ids of training sites
        TrainingSitesSpecClassType spectralClasses; // unique spectral classes ids
        BandsVectorType bands;                      // bands

      public:
        void addTrainingSite(TrainingSite trSite); // add new training site to container
        void removeTrainingSite(const int id);     // remove training site from container
        const TrainingSite& getTrainingSite(const int id) const;
        int getTrainingSitesCount() const;
        const TrainingSitesIdsType& getTrainingSitesIds() const;
        const TrainingSitesType& getTrainingSites() const;
        const TrainingSitesSpecClassType& getSpectralClasses() const;
        void writeStat(const char* filename) const;
        const BandsVectorType& getBands() const;
    };

    // class for calculating and storing class statistics values
    class ClassStatistics
    {
      public:
        typedef std::vector< int > SpectralClassesVectorType;

      private:
        // stores statistic values for single band
        typedef struct
        {
            double avg;
            double stdev;
        } BandStatisticsType;

        // stores all information about signe spectral class
        typedef struct
        {
            long pixelCount;
            std::map< int, BandStatisticsType > bandStatistics;
        } BandDataType;

        std::map< int, BandDataType > classStatistics;

        BandsVectorType bands;
        SpectralClassesVectorType spectralClasses;
        const TrainingSitesContainer* trainingSitesContainer;

      public:
        ClassStatistics(const TrainingSitesContainer& trainingSitesContainer);
        ~ClassStatistics();
        double getClassAvg(const int spectralClass, const int band) const;
        double getClassStdev(const int spectralClass, const int band) const;
        long getPixelCount(const int spectralClass) const;
        const TrainingSitesContainer* getTrainingSitesContainer() const;
        const SpectralClassesVectorType& getSpectralClasses() const;
        const BandsVectorType& getBands() const;
        void writeStat(const char* filename) const;
    };

    GDALDataset* openVectorDs(const char* fileName);
    OGRLayer* openVectorLyr(const char* fileName, const char* lyrName);
    GDALDataset* openRasterDs(const char* fileName);
    TrainingSitesContainer readData(GDALDataset* trainingSites, const char* classAttribute, const char* idAttribute,
                                    GDALDataset* satelliteImage);
    TrainingSitesContainer readData(OGRLayer* pLyr, const char* classAttribute, const char* idAttribute, GDALDataset* satelliteImage);
    bool pixelPolyGeomIntersection(OGRPolygon* polygonGeom, OGREnvelope* polygonEnvelope, double* xres, double* yres, int* width,
                                   int* pixel);
}

#endif
