#ifndef OUTLANDERLIBRARY_HPP_
#define OUTLANDERLIBRARY_HPP_

#include <ITK-4.10/itkLabelVotingImageFilter.h>
#include <ITK-4.10/itkVariableSizeMatrix.h>

#include <otbConfusionMatrixCalculator.h>
#include <otbConfusionMatrixToMassOfBelief.h>
#include <otbDSFusionOfClassifiersImageFilter.h>
#include <otbDecisionTreeMachineLearningModel.h>
#include <otbGradientBoostedTreeMachineLearningModel.h>
#include <otbImage.h>
#include <otbImageClassificationFilter.h>
#include <otbImageFileReader.h>
#include <otbImageFileWriter.h>
#include <otbImageListToVectorImageFilter.h>
#include <otbImageToLabelMapWithAttributesFilter.h>
#include <otbLibSVMMachineLearningModel.h>
#include <otbListSampleGenerator.h>
#include <otbMachineLearningModelFactory.h>
#include <otbVectorData.h>
#include <otbVectorDataFileReader.h>
#include <otbVectorDataIntoImageProjectionFilter.h>
#include <otbVectorDataToLabelImageFilter.h>
#include <otbVectorImage.h>

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
typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, ImageType> VectorDataReprojectionType;
typedef otb::ListSampleGenerator<VectorImageType, VectorDataType> ListSampleGeneratorType;
typedef otb::ConfusionMatrixCalculator<ListSampleGeneratorType::ListLabelType, ListSampleGeneratorType::ListSampleType>
        ConfusionMatrixCalculatorType;
// struct for recieving data from vypocitajChybovuMaticu
typedef struct {
    ConfusionMatrixType confMat;
    ConfusionMatrixCalculatorType::MapOfClassesType mapOfClasses;
} confMatData;

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
void loadRaster(oll::ImageType::Pointer raster, std::string path);
void loadRaster(oll::LabelImageType::Pointer raster, std::string path);
void loadVector(oll::VectorDataType::Pointer vector, std::string path);
void train(oll::ImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
           std::string classAttributeName, oll::trainingMethod trainingMethod);
void classify(oll::ImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster);
void ulozRaster(oll::LabelImageType::Pointer raster, std::string outputFile);
void ulozRaster(oll::ImageType::Pointer raster, std::string outputFile);
confMatData vypocitajChybovuMaticu(oll::LabelImageType::Pointer classifiedRaster, oll::VectorDataType::Pointer groundTruthVector,
                                           std::string classAttributeName);
}

#endif
