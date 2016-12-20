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
}
