#ifndef OUTLANDERLIBRARY_HPP_
#define OUTLANDERLIBRARY_HPP_

#include <ITK-4.10/itkLabelVotingImageFilter.h>
#include <OTB-5.6/otbConfusionMatrixToMassOfBelief.h>
#include <OTB-5.6/otbDSFusionOfClassifiersImageFilter.h>
#include <OTB-5.6/otbDecisionTreeMachineLearningModel.h>
#include <OTB-5.6/otbGradientBoostedTreeMachineLearningModel.h>
#include <OTB-5.6/otbImage.h>
#include <OTB-5.6/otbImageClassificationFilter.h>
#include <OTB-5.6/otbImageFileReader.h>
#include <OTB-5.6/otbImageFileWriter.h>
#include <OTB-5.6/otbImageListToVectorImageFilter.h>
#include <OTB-5.6/otbImageToLabelMapWithAttributesFilter.h>
#include <OTB-5.6/otbLibSVMMachineLearningModel.h>
#include <OTB-5.6/otbListSampleGenerator.h>
#include <OTB-5.6/otbMachineLearningModelFactory.h>
#include <OTB-5.6/otbVectorData.h>
#include <OTB-5.6/otbVectorDataFileReader.h>
#include <OTB-5.6/otbVectorDataIntoImageProjectionFilter.h>
#include <OTB-5.6/otbVectorDataToLabelImageFilter.h>
#include <OTB-5.6/otbVectorImage.h>

#include <fstream>

#include <boost/filesystem.hpp>

#define EXIT_MESSAGE "EXITING!"
#define FILE_DOES_NOT_EXISTS 1
#define PATH_DOES_NOT_EXISTS 2
#define PATH_IS_NOT_FILE 3
#define PATH_IS_NOT_DIR 4
#define PARAMETER_OUT_OF_RANGE 5

namespace oll
{
// definície typov
const unsigned int DIMENSION = 2;
typedef double PixelType;
typedef unsigned short LabelPixelType;
typedef otb::VectorImage<PixelType, DIMENSION> ImageType;
typedef otb::Image<LabelPixelType, DIMENSION> LabelImageType;
typedef otb::VectorData<PixelType, DIMENSION> VectorDataType;
typedef unsigned long ConfusionMatrixEltType;
typedef itk::VariableSizeMatrix<ConfusionMatrixEltType> ConfusionMatrixType;
typedef otb::ConfusionMatrixToMassOfBelief<ConfusionMatrixType, LabelPixelType> ConfusionMatrixToMassOfBeliefType;
typedef ConfusionMatrixToMassOfBeliefType::MapOfClassesType MapOfClassesType;
typedef otb::ImageList<LabelImageType> LabelImageListType;
typedef otb::VectorImage<LabelPixelType, DIMENSION> VectorImageType;
typedef otb::ImageListToVectorImageFilter<LabelImageListType, VectorImageType> ImageListToVectorImageFilterType;
typedef otb::DSFusionOfClassifiersImageFilter<VectorImageType, LabelImageType> DSFusionOfClassifiersImageFilterType;
typedef otb::VectorDataToLabelImageFilter<VectorDataType, LabelImageType> VectorDataToLabelImageFilterType;

// druh testovania existenice priečinku, alebo súboru
enum existanceCheckType
{
    inputFilePath,
    folderPath,
    outputFilePath
};

// druhy klasifikoačných metód
enum trainingMethod
{
    libSVM,
    gradientBoostedTree,
    desicionTree
};

bool checkIfExists(const boost::filesystem::path path, const oll::existanceCheckType mode);
void train(oll::ImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
           std::string classAttributeName, oll::trainingMethod trainingMethod);
void classify(oll::ImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster);
void trainingSitesToRaster(oll::VectorDataType::Pointer trainingSites, oll::LabelImageType::Pointer outputRaster,
                           oll::ImageType::Pointer referenceRaster, std::string attribute);
void ulozRaster(oll::LabelImageType::Pointer raster, std::string outputFile);
void vypocitajChybovuMaticu(oll::LabelImageType::Pointer classifiedRaster, oll::LabelImageType::Pointer groundTruth);
}

#endif
