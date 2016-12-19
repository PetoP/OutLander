#include "olsl.hxx"

namespace oll
{
    oll::TrainingSite::TrainingSite(const int id, const int coverClass)
    {
        this->id = id;
        this->coverClass = coverClass;
    };

    oll::TrainingSite::~TrainingSite()
    {
    }

    void oll::TrainingSite::addBandValues(int band, PixelValuesType pixelValues)
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

    oll::PixelValuesType oll::TrainingSite::getBandValues(int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            return valuesPerBand.at(band);
        }
        else
        {
            std::cerr << "Data for band " << band << " ate not stored in training site " << id << "!" << std::endl;
            return PixelValuesType();
        }
    };

    double oll::TrainingSite::getBandAvg(int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            long sum = 0;
            for (double value : valuesPerBand.at(band))
            {
                sum += value;
            }

            return sum / pixelCount;
        }
        else
        {
            std::cerr << "Data for band " << band << " ate not stored in training site " << id << "!" << std::endl;
            return -1;
        }
    };

    double oll::TrainingSite::getBandVariance(int band) const
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
            std::cerr << "Data for band " << band << " ate not stored in training site " << id << "!" << std::endl;
            return -1;
        }
    };

    double oll::TrainingSite::getBandStdev(int band) const
    {
        if (std::find(bands.begin(), bands.end(), band) != bands.end())
        {
            return sqrt(getBandVariance(band));
        }
        else
        {
            std::cerr << "Data for band " << band << " ate not stored in training site " << id << "!" << std::endl;
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

    int oll::TrainingSite::getCoverClass() const
    {
        return coverClass;
    };

    const oll::TrainingSite::BandsVectorType& oll::TrainingSite::getBands() const
    {
        return bands;
    }
}
