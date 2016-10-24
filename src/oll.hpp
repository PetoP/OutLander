#ifndef OUTLANDERLIBRARY_HPP_
#define OUTLANDERLIBRARY_HPP_

#include <ITK-4.10/itkAtanImageFilter.h>
#include <ITK-4.10/itkGradientMagnitudeImageFilter.h>
#include <ITK-4.10/itkImageRegionIterator.h>
#include <ITK-4.10/itkLabelVotingImageFilter.h>
#include <ITK-4.10/itkVariableSizeMatrix.h>

#include <OTB-5.6/otbConfusionMatrixCalculator.h>
#include <OTB-5.6/otbConfusionMatrixMeasurements.h>
#include <OTB-5.6/otbConfusionMatrixToMassOfBelief.h>
#include <OTB-5.6/otbDEMCaracteristicsExtractor.h>
#include <OTB-5.6/otbDEMHandler.h>
#include <OTB-5.6/otbDSFusionOfClassifiersImageFilter.h>
#include <OTB-5.6/otbDecisionTreeMachineLearningModel.h>
#include <OTB-5.6/otbGradientBoostedTreeMachineLearningModel.h>
#include <OTB-5.6/otbImage.h>
#include <OTB-5.6/otbImageClassificationFilter.h>
#include <OTB-5.6/otbImageFileReader.h>
#include <OTB-5.6/otbImageFileWriter.h>
#include <OTB-5.6/otbImageListToVectorImageFilter.h>
#include <OTB-5.6/otbImageMetadataInterfaceBase.h>
#include <OTB-5.6/otbImageToLabelMapWithAttributesFilter.h>
#include <OTB-5.6/otbLabelImageToOGRDataSourceFilter.h>
#include <OTB-5.6/otbLabelImageToOGRDataSourceFilter.h>
#include <OTB-5.6/otbLabelImageToVectorDataFilter.h>
#include <OTB-5.6/otbLibSVMMachineLearningModel.h>
#include <OTB-5.6/otbListSampleGenerator.h>
#include <OTB-5.6/otbMachineLearningModelFactory.h>
#include <OTB-5.6/otbMultiplyByScalarImageFilter.h>
#include <OTB-5.6/otbOGRDataSourceToLabelImageFilter.h>
#include <OTB-5.6/otbOGRDataSourceWrapper.h>
#include <OTB-5.6/otbOGRFieldWrapper.h>
#include <OTB-5.6/otbVectorData.h>
#include <OTB-5.6/otbVectorDataFileReader.h>
#include <OTB-5.6/otbVectorDataIntoImageProjectionFilter.h>
#include <OTB-5.6/otbVectorDataToLabelImageFilter.h>
#include <OTB-5.6/otbVectorImage.h>

#include <fstream>
#include <iostream>
#include <limits>
#include <string.h>

#include <ogrsf_frmts.h>

#include <boost/filesystem.hpp>
#include <boost/math/constants/constants.hpp>

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
typedef otb::ConfusionMatrixMeasurements<ConfusionMatrixType, LabelPixelType> ConfusionMatrixMeasurementsType;
typedef itk::ImageRegionIterator<LabelImageType> LabelImageRegionIteratorType;
typedef otb::Image<PixelType, DIMENSION> DEMCharImageType;
typedef itk::ImageRegionConstIterator<DEMCharImageType> DEMCharImageRegionConstIteratorType;
typedef itk::GradientMagnitudeImageFilter<DEMCharImageType, DEMCharImageType> GradientMagnitudeImageFilterType;
typedef itk::AtanImageFilter<DEMCharImageType, DEMCharImageType> AtanImageFilterType;
typedef otb::MultiplyByScalarImageFilter<DEMCharImageType, DEMCharImageType> MultiplyByScalarImageFilterType;
typedef otb::ImageMetadataInterfaceBase::VectorType GeoTransformType;
// typedef otb::LabelImageToVectorDataFilter<LabelImageType, PixelType> LabelImageToVectorDataFilter;
// typedef otb::VectorDataToLabelImageFilter<VectorDataType, LabelImageType> VectorDataToLabelImageFilterType;
typedef otb::LabelImageToOGRDataSourceFilter<LabelImageType> LabelImageToOGRDataSourceFilterType;
typedef otb::OGRDataSourceToLabelImageFilter<LabelImageType> OGRDataSourceToLabelImageFilter;

// struct for recieving data from vypocitajChybovuMaticu
typedef struct
{
    ConfusionMatrixCalculatorType::ConfusionMatrixType confMat;
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

// typedef for storing reslassification rules
typedef std::vector<std::pair<LabelPixelType, LabelPixelType>> ReclassificationRulesType;

bool checkIfExists(const boost::filesystem::path path, const oll::existanceCheckType mode);
void loadRaster(oll::ImageType::Pointer raster, std::string path);
void loadRaster(oll::LabelImageType::Pointer raster, std::string path);
void loadRaster(oll::DEMCharImageType::Pointer raster, std::string path);
void loadVector(oll::VectorDataType::Pointer vector, std::string path);
void train(oll::ImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
           std::string classAttributeName, oll::trainingMethod trainingMethod);
void classify(oll::ImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster);
void ulozRaster(oll::LabelImageType::Pointer raster, std::string outputFile);
void ulozRaster(oll::ImageType::Pointer raster, std::string outputFile);
void ulozRaster(oll::DEMCharImageType::Pointer raster, std::string outputFile);
confMatData vypocitajChybovuMaticu(oll::LabelImageType::Pointer classifiedRaster, oll::VectorDataType::Pointer groundTruthVector,
                                   std::string classAttributeName);
void dsf(oll::LabelImageListType::Pointer classifiedImages, std::vector<oll::ConfusionMatrixType> &matrices,
         std::vector<oll::ConfusionMatrixCalculatorType::MapOfClassesType> &mapOfClasses, oll::LabelPixelType nodataLabel,
         oll::LabelPixelType undecidedLabel, oll::LabelImageType::Pointer outputRaster);
oll::ReclassificationRulesType readReclassificationRules(std::string pathToReclassificationRules);
void reclassifyRaster(const oll::LabelImageType::Pointer inputRaster, oll::LabelImageType::Pointer outputRaster,
                      const oll::ReclassificationRulesType &reclassificationRules);
void computeSlopeRaster(const oll::DEMCharImageType::Pointer demRaster, oll::DEMCharImageType::Pointer slopeRaster);
void podSklon(const oll::LabelImageType::Pointer podPlodRaster, const oll::DEMCharImageType::Pointer slopeRaster,
              oll::LabelImageType::Pointer outputRaster, oll::PixelType hranicnySklon);
void podRozloh(const oll::LabelImageType::Pointer podSklon, oll::LabelImageType::Pointer outputRaster, float hranicnaVelkost);
}

#endif
