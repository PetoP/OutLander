#include "olsl.hxx"

namespace oll
{
    oll::ClassStatistics::ClassStatistics(const TrainingSitesContainer& trainingSitesContainer)
    {
        this->trainingSitesContainer = &trainingSitesContainer;
        PixelValuesType values;
        double avg, var, stdev, sum, numerator;
        long count;

        for (const int& spectralClass : trainingSitesContainer.getSpectralClasses())
        {
            spectralClasses.push_back(spectralClass);
            for (const int& band : trainingSitesContainer.getTrainingSites().begin()->getBands())
            {
                if (std::find(bands.begin(), bands.end(), band) == bands.end())
                {
                    bands.push_back(band);
                }
                values.clear();
                sum = numerator = 0;
                for (const TrainingSite& trSite : trainingSitesContainer.getTrainingSites())
                {
                    if (trSite.getSpectralClass() == spectralClass)
                    {
                        for (const double& value : trSite.getBandValues(band))
                        {
                            values.push_back(value);
                            sum += value;
                        }
                    }
                }

                count = values.size();
                avg = sum / count;

                for (const double& value : values)
                {
                    numerator += pow(value - avg, 2);
                }

                var = numerator / (count - 1);
                stdev = sqrt(var);

                classStatistics[spectralClass].pixelCount = count;
                classStatistics[spectralClass].bandStatistics[band].avg = avg;
                classStatistics[spectralClass].bandStatistics[band].stdev = stdev;
            }
        }

        std::sort(spectralClasses.begin(), spectralClasses.end());
    };

    oll::ClassStatistics::~ClassStatistics()
    {
    }

    double oll::ClassStatistics::getClassAvg(const int spectralClass, const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end() &&
            std::find(spectralClasses.begin(), spectralClasses.end(), spectralClass) != spectralClasses.end())
        {
            return classStatistics.at(spectralClass).bandStatistics.at(band).avg;
        }
        else
        {
            std::cerr << "Data for band " << band << " of spectral class " << spectralClass << " are not stored!" << std::endl;
            return -1;
        }
    };

    double oll::ClassStatistics::getClassStdev(const int spectralClass, const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end() &&
            std::find(spectralClasses.begin(), spectralClasses.end(), spectralClass) != spectralClasses.end())
        {
            return classStatistics.at(spectralClass).bandStatistics.at(band).stdev;
        }
        else
        {
            std::cerr << "Data for band " << band << " of spectral class " << spectralClass << " are not stored!" << std::endl;
            return -1;
        }
    };

    long oll::ClassStatistics::getPixelCount(const int spectralClass) const
    {
        if (std::find(spectralClasses.begin(), spectralClasses.end(), spectralClass) != spectralClasses.end())
        {
            return classStatistics.at(spectralClass).pixelCount;
        }
        else
        {
            std::cerr << "Data for spectral class " << spectralClass << " are not stored!" << std::endl;
            return -1;
        }
    };

    const oll::TrainingSitesContainer* oll::ClassStatistics::getTrainingSitesContainer() const
    {
        return trainingSitesContainer;
    };

    const oll::ClassStatistics::SpectralClassesVectorType& oll::ClassStatistics::getSpectralClasses() const
    {
        return spectralClasses;
    };

    const oll::BandsVectorType& oll::ClassStatistics::getBands() const
    {
        return bands;
    };

    void oll::ClassStatistics::writeStat(const char* filename) const
    {
        // output CSV preparation
        std::ofstream outputCSV;
        outputCSV.open(filename);
        outputCSV << "class,count";

        for (const int& band : getBands())
        {
            outputCSV << ",b" << band << "_avg"
                      << ",b" << band << "_stdev";
        }

        for (const int& specClass : getSpectralClasses())
        {
            outputCSV << std::endl << specClass << "," << getPixelCount(specClass);

            for (const int& band : getBands())
            {
                outputCSV << "," << getClassAvg(specClass, band) << "," << getClassStdev(specClass, band);
            }
        }

        outputCSV << std::endl;
        outputCSV.close();
    }
}
