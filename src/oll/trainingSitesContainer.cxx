#include "olsl.hxx"

namespace oll
{
    void oll::TrainingSitesContainer::addTrainingSite(oll::TrainingSite trSite)
    {
        if (std::find(trainingSitesIds.begin(), trainingSitesIds.end(), trSite.getId()) == trainingSitesIds.end())
        {
            trainingSites.push_back(trSite);
            trainingSitesIds.push_back(trSite.getId());
            if (std::find(spectralClasses.begin(), spectralClasses.end(), trSite.getSpectralClass()) == spectralClasses.end())
            {
                spectralClasses.push_back(trSite.getSpectralClass());
            }
        }
        else
        {
            std::cerr << "Training site id " << trSite.getId() << " allready stored!" << std::endl;
        }

        for (int band : trSite.getBands())
        {
            if (std::find(bands.begin(), bands.end(), band) == bands.end())
            {
                bands.push_back(band);
            }
        }
    };

    void oll::TrainingSitesContainer::removeTrainingSite(const int id)
    {
        TrainingSitesIdsType::iterator idPos = std::find(trainingSitesIds.begin(), trainingSitesIds.end(), id);
        if (idPos != trainingSitesIds.end())
        {
            trainingSitesIds.erase(idPos);
            for (TrainingSitesType::iterator trainingSite = trainingSites.begin(); trainingSite < trainingSites.end(); ++trainingSite)
            {
                if (trainingSite->getId() == id)
                {
                    trainingSites.erase(trainingSite);
                    return;
                }
            }
        }
        else
        {
            std::cerr << "Training site id " << id << "does not exist!" << std::endl;
        }
    };

    const oll::TrainingSite& oll::TrainingSitesContainer::getTrainingSite(const int id) const
    {
        if (std::find(trainingSitesIds.begin(), trainingSitesIds.end(), id) != trainingSitesIds.end())
        {
            for (const oll::TrainingSite& trainingSite : trainingSites)
            {
                if (trainingSite.getId() == id)
                {
                    return trainingSite;
                }
            }
        }
        else
        {
            std::cerr << "Training site id " << id << "does not exist!" << std::endl;
        }
    };

    int oll::TrainingSitesContainer::getTrainingSitesCount() const
    {
        return trainingSites.size();
    };

    const oll::TrainingSitesContainer::TrainingSitesIdsType& oll::TrainingSitesContainer::getTrainingSitesIds() const
    {
        return trainingSitesIds;
    };

    const oll::TrainingSitesContainer::TrainingSitesType& oll::TrainingSitesContainer::getTrainingSites() const
    {
        return trainingSites;
    };

    const oll::TrainingSitesContainer::TrainingSitesSpecClassType& oll::TrainingSitesContainer::getSpectralClasses() const
    {
        return spectralClasses;
    };

    void oll::TrainingSitesContainer::writeStat(const char* filename) const
    {
        // output CSV preparation
        std::ofstream outputCSV;
        outputCSV.open(filename);
        outputCSV << "id,class,count";

        // print headers
        for (const int& band : bands)
        {
            outputCSV << ",b" << band << "_avg"
                      << ",b" << band << "_stdev";
        }

        // iterate over training sites
        const oll::TrainingSitesContainer::TrainingSitesType trainingSites = getTrainingSites();
        for (const oll::TrainingSite& trainingSite : trainingSites)
        {
            outputCSV << std::endl << trainingSite.getId()
                      << "," << trainingSite.getSpectralClass()
                      << "," << trainingSite.getPixelCount();

            for (const int& band : bands)
            {
                outputCSV << "," << trainingSite.getBandAvg(band) << "," << trainingSite.getBandStdev(band);
            }
        }

        outputCSV << std::endl;
        outputCSV.close();

        // outputCSVT
        std::ofstream outputCSVT;
        std::string filenameCSVT;
        filenameCSVT = filename;
        filenameCSVT += "t";
        outputCSVT.open(filenameCSVT.c_str());
        outputCSVT << "\"Integer\",\"Integer\",\"Integer\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\"" << std::endl;

    };

    const oll::BandsVectorType& oll::TrainingSitesContainer::getBands() const
    {
        return bands;
    };
}
