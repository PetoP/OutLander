#ifndef OUTLANDERLIBRARY_HPP_
#define OUTLANDERLIBRARY_HPP_

#include <OTB-5.6/otbDecisionTreeMachineLearningModel.h>
#include <OTB-5.6/otbGradientBoostedTreeMachineLearningModel.h>
#include <OTB-5.6/otbImage.h>
#include <OTB-5.6/otbImageClassificationFilter.h>
#include <OTB-5.6/otbImageFileReader.h>
#include <OTB-5.6/otbImageFileWriter.h>
#include <OTB-5.6/otbLibSVMMachineLearningModel.h>
#include <OTB-5.6/otbListSampleGenerator.h>
#include <OTB-5.6/otbMachineLearningModelFactory.h>
#include <OTB-5.6/otbVectorData.h>
#include <OTB-5.6/otbVectorDataFileReader.h>
#include <OTB-5.6/otbVectorDataIntoImageProjectionFilter.h>
#include <OTB-5.6/otbVectorImage.h>

#include <boost/filesystem.hpp>

#define EXIT_MESSAGE "EXITING!"
#define FILE_DOES_NOT_EXISTS 1
#define PATH_DOES_NOT_EXISTS 2
#define PATH_IS_NOT_FILE 3
#define PATH_IS_NOT_DIR 4
#define PARAMETER_OUT_OF_RANGE 5

namespace oll
{
enum existanceCheckType
{
    inputFilePath,
    folderPath,
    outputFilePath
};

enum trainingMethod
{
    libSVM,
    gradientBoostedTree,
    desicionTree
};

bool checkIfExists(const boost::filesystem::path path, const oll::existanceCheckType mode);
}

#endif
