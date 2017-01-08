#ifndef OUTLANDERLIBRARY_HXX_
#define OUTLANDERLIBRARY_HXX_

#include "itkAtanImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkLabelVotingImageFilter.h"
#include "itkVariableSizeMatrix.h"

#include "otbConfusionMatrixCalculator.h"
#include "otbConfusionMatrixMeasurements.h"
#include "otbConfusionMatrixToMassOfBelief.h"
#include "otbDEMCaracteristicsExtractor.h"
#include "otbDEMHandler.h"
#include "otbDEMToImageGenerator.h"
#include "otbDSFusionOfClassifiersImageFilter.h"
#include "otbDecisionTreeMachineLearningModel.h"
#include "otbGenericMapProjection.h"
#include "otbGenericRSResampleImageFilter.h"
#include "otbGradientBoostedTreeMachineLearningModel.h"
#include "otbImage.h"
#include "otbImageClassificationFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbImageMetadataInterfaceBase.h"
#include "otbImageToLabelMapWithAttributesFilter.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbLabelImageToOGRDataSourceFilter.h"
#include "otbLabelImageToOGRDataSourceFilter.h"
#include "otbLabelImageToVectorDataFilter.h"
#include "otbLibSVMMachineLearningModel.h"
#include "otbListSampleGenerator.h"
#include "otbMachineLearningModelFactory.h"
#include "otbMapProjections.h"
#include "otbMultiplyByScalarImageFilter.h"
#include "otbOGRDataSourceToLabelImageFilter.h"
#include "otbOGRDataSourceWrapper.h"
#include "otbOGRFieldWrapper.h"
#include "otbVectorData.h"
#include "otbVectorDataFileReader.h"
#include "otbVectorDataIntoImageProjectionFilter.h"
#include "otbVectorDataToLabelImageFilter.h"
#include "otbVectorImage.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string.h>
#include <cmath>

#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal_priv.h>
#include <gdalwarper.h>
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
    // definície typov pixlov
    typedef double PixelType;
    typedef unsigned short LabelPixelType;

    // definícia typov rastrov
    const unsigned int DIMENSION = 2;
    typedef otb::Image< LabelPixelType, DIMENSION > LabelImageType;
    typedef otb::Image< PixelType, DIMENSION > DoubleImageType;
    typedef otb::VectorImage< PixelType, DIMENSION > VectorImageType;
    typedef otb::VectorImage< LabelPixelType, DIMENSION > LabelVectorImageType;

    // definícia vektorových dát
    typedef otb::VectorData< PixelType, DIMENSION > VectorDataType;

    typedef unsigned long ConfusionMatrixEltType;
    typedef itk::VariableSizeMatrix< ConfusionMatrixEltType > ConfusionMatrixType;
    typedef otb::ConfusionMatrixToMassOfBelief< ConfusionMatrixType, LabelPixelType > ConfusionMatrixToMassOfBeliefType;
    typedef ConfusionMatrixToMassOfBeliefType::MapOfClassesType MapOfClassesType;
    typedef otb::ImageList< LabelImageType > LabelImageListType;
    typedef otb::ImageListToVectorImageFilter< LabelImageListType, LabelVectorImageType > ImageListToVectorImageFilterType;
    typedef otb::DSFusionOfClassifiersImageFilter< LabelVectorImageType, LabelImageType > DSFusionOfClassifiersImageFilterType;
    typedef otb::VectorDataToLabelImageFilter< VectorDataType, LabelImageType > VectorDataToLabelImageFilterType;
    typedef otb::VectorDataIntoImageProjectionFilter< VectorDataType, VectorImageType > VectorDataReprojectionType;
    typedef otb::ListSampleGenerator< LabelVectorImageType, VectorDataType > ListSampleGeneratorType;
    typedef otb::ConfusionMatrixCalculator< ListSampleGeneratorType::ListLabelType, ListSampleGeneratorType::ListSampleType >
        ConfusionMatrixCalculatorType;
    typedef otb::ConfusionMatrixMeasurements< ConfusionMatrixType, int > ConfusionMatrixMeasurementsType;
    typedef itk::ImageRegionIterator< LabelImageType > LabelImageRegionIteratorType;
    typedef itk::ImageRegionIterator< DoubleImageType > DoubleImageRegionIteratorType;
    typedef itk::ImageRegionConstIterator< VectorImageType > ImageRegionConstIteratorType;

    typedef itk::ImageRegionConstIterator< DoubleImageType > DoubleImageRegionConstIteratorType;
    typedef itk::GradientMagnitudeImageFilter< DoubleImageType, DoubleImageType > GradientMagnitudeImageFilterType;
    typedef itk::AtanImageFilter< DoubleImageType, DoubleImageType > AtanImageFilterType;
    typedef otb::MultiplyByScalarImageFilter< DoubleImageType, DoubleImageType > MultiplyByScalarImageFilterType;
    typedef otb::ImageMetadataInterfaceBase::VectorType GeoTransformType;
    typedef otb::LabelImageToOGRDataSourceFilter< LabelImageType > LabelImageToOGRDataSourceFilterType;
    typedef otb::OGRDataSourceToLabelImageFilter< LabelImageType > OGRDataSourceToLabelImageFilter;
    typedef otb::ImageToVectorImageCastFilter< LabelImageType, LabelVectorImageType > ImageToVectorImageCastFilterType;

    // na transformáciu DEM DIR do rastra (OTB špecifický prístup)
    typedef otb::DEMToImageGenerator< DoubleImageType > DEMToImageGenerator;

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

    // satellite types
    enum satellites
    {
        Landsat8,
        Sentinel2
    };

    // typedef for storing reslassification rules
    typedef std::vector< std::pair< LabelPixelType, LabelPixelType > > ReclassificationRulesType;

    bool checkIfExists(const boost::filesystem::path path, const oll::existanceCheckType mode);
    void loadRaster(oll::VectorImageType::Pointer raster, std::string path);
    void loadRaster(oll::LabelImageType::Pointer raster, std::string path);
    void loadRaster(oll::DoubleImageType::Pointer raster, std::string path);
    void loadVector(oll::VectorDataType::Pointer vector, std::string path);
    void train(oll::VectorImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
               std::string classAttributeName, oll::trainingMethod trainingMethod);
    void classify(oll::VectorImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster);
    void ulozRaster(oll::LabelImageType::Pointer raster, std::string outputFile);
    void ulozRaster(oll::VectorImageType::Pointer raster, std::string outputFile);
    void ulozRaster(oll::DoubleImageType::Pointer raster, std::string outputFile);
    confMatData vypocitajChybovuMaticu(oll::LabelImageType::Pointer classifiedRaster, oll::VectorDataType::Pointer groundTruthVector,
                                       std::string classAttributeName);
    void dsf(oll::LabelImageListType::Pointer classifiedImages, std::vector< oll::ConfusionMatrixType >& matrices,
             std::vector< oll::ConfusionMatrixCalculatorType::MapOfClassesType >& mapOfClasses, oll::LabelPixelType nodataLabel,
             oll::LabelPixelType undecidedLabel, oll::LabelImageType::Pointer outputRaster);
    oll::ReclassificationRulesType readReclassificationRules(std::string pathToReclassificationRules);
    void reclassifyRaster(const oll::LabelImageType::Pointer inputRaster, oll::LabelImageType::Pointer outputRaster,
                          const oll::ReclassificationRulesType& reclassificationRules);
    void computeSlopeRaster(const oll::DoubleImageType::Pointer demRaster, oll::DoubleImageType::Pointer slopeRaster);
    void podSklon(const oll::LabelImageType::Pointer podPlodRaster, const oll::DoubleImageType::Pointer slopeRaster,
                  oll::LabelImageType::Pointer outputRaster, oll::PixelType hranicnySklon);
    void podRozloh(const oll::LabelImageType::Pointer podSklon, oll::LabelImageType::Pointer outputRaster, float hranicnaVelkost);
    void alignDEM(const oll::VectorImageType::Pointer sourceImage, oll::DoubleImageType::Pointer alignedDem);
    void albedo(const oll::VectorImageType::Pointer satelliteImage, const oll::satellites satelliteType,
                oll::DoubleImageType::Pointer albedoRaster);
    void printConfMat(confMatData& cmd, std::ostream& outStream, bool perClass = true);
    double kappa(double a, double b, double c, double d);
}

#endif
