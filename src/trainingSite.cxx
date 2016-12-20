#include "olsl.hxx"

namespace oll
{
    oll::TrainingSite::TrainingSite(const int id, const int coverClass)
    {
        this->id = id;
        this->spectralClass = coverClass;
    };

    oll::TrainingSite::~TrainingSite()
    {
    }

    void oll::TrainingSite::addBandValues(const int band, PixelValuesType pixelValues)
    {
        if (std::find(bands.begin(), bands.end(), band) == bands.end())
        {
            bands.push_back(band);
            pixelCount = pixelValues.size();
            valuesPerBand[band] = pixelValues;
        }
        else
        {
            std::cerr << "Data for band " << band << " are already stored in training site " << id << "!" << std::endl;
        }
    };

    const oll::PixelValuesType& oll::TrainingSite::getBandValues(const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            return valuesPerBand.at(band);
        }
        else
        {
            std::cerr << "Data for band " << band << " are not stored in training site " << id << "!" << std::endl;
            return PixelValuesType();
        }
    };

    double oll::TrainingSite::getBandAvg(const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            double sum = 0;
            for (double value : valuesPerBand.at(band))
            {
                sum += value;
            }

            return sum / pixelCount;
        }
        else
        {
            std::cerr << "Data for band " << band << " are not stored in training site " << id << "!" << std::endl;
            return -1;
        }
    };

    double oll::TrainingSite::getBandVariance(const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            const double avg = getBandAvg(band);
            double numerator = 0;

            for (double x : valuesPerBand.at(band))
            {
                numerator += pow(x - avg, 2);
            }

            return numerator / (pixelCount - 1);
        }
        else
        {
            std::cerr << "Data for band " << band << " are not stored in training site " << id << "!" << std::endl;
            return -1;
        }
    };

    double oll::TrainingSite::getBandStdev(const int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            return sqrt(getBandVariance(band));
        }
        else
        {
            std::cerr << "Data for band " << band << " are not stored in training site " << id << "!" << std::endl;
            return -1;
        }
    };

    long oll::TrainingSite::getPixelCount() const
    {
        return pixelCount;
    };

    int oll::TrainingSite::getId() const
    {
        return id;
    };

    int oll::TrainingSite::getSpectralClass() const
    {
        return spectralClass;
    };

    const oll::BandsVectorType& oll::TrainingSite::getBands() const
    {
        return bands;
    }
}
