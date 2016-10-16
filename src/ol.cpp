#include "oll.hpp"
#include <iostream>

using std::string;
using std::cout;
using std::cerr;

// this will be set from arguments
const string sourceDirectory = "/run/media/peter/WD/ZIK/diplomovka/klasifikator/landsat_z_grassu/";
const string sourceImage = sourceDirectory + "L8.tif";

const string trainingSamples = sourceDirectory + "tren.shp";
const string groundTruth = sourceDirectory + "kappa.shp";
const string classAtribure = "plod_id";

const string outputDirectory = "/home/peter/Plocha/";
const string outputImage = outputDirectory + "L8.tif";

const string demDir = "/run/media/peter/WD/mapy/SRTM/";

int main()
{
    // DEM registration
    otb::DEMHandler::Pointer demHandler = otb::DEMHandler::Instance();
    if (! demHandler->IsValidDEMDirectory(demDir.c_str()))
    {
        std::cerr << "Zlý dem dir\n";
    }
    demHandler->OpenDEMDirectory(demDir);

    // input image reading
    oll::ImageType::Pointer landsatImage = oll::ImageType::New();
    oll::loadRaster(landsatImage, sourceImage);

    // input training samples reading
    oll::VectorDataType::Pointer trainingSites = oll::VectorDataType::New();
    oll::loadVector(trainingSites, trainingSamples);

    // input ground truth vector reading
    oll::VectorDataType::Pointer groundTruthVector = oll::VectorDataType::New();
    oll::loadVector(groundTruthVector, groundTruth);

    // image training
    oll::train(landsatImage, trainingSites, "/home/peter/modelDT.txt", classAtribure, oll::desicionTree);
    oll::train(landsatImage, trainingSites, "/home/peter/modelGBT.txt", classAtribure, oll::gradientBoostedTree);
    oll::train(landsatImage, trainingSites, "/home/peter/modelLibSVM.txt", classAtribure, oll::libSVM);

    // image classification
    oll::LabelImageType::Pointer DTClassified = oll::LabelImageType::New();
    oll::LabelImageType::Pointer GBTClassified = oll::LabelImageType::New();
    oll::LabelImageType::Pointer LibSVMClassified = oll::LabelImageType::New();
    oll::classify(landsatImage, "/home/peter/modelDT.txt", DTClassified);
    oll::classify(landsatImage, "/home/peter/modelGBT.txt", GBTClassified);
    oll::classify(landsatImage, "/home/peter/modelLibSVM.txt", LibSVMClassified);

    oll::ulozRaster(DTClassified, "/home/peter/classified_DT.tif");
    oll::ulozRaster(GBTClassified, "/home/peter/classified_GBT.tif");
    oll::ulozRaster(LibSVMClassified, "/home/peter/classified_SVM.tif");

    // oll::loadRaster(DTClassified, "/home/peter/classified_DT.tif");
    // oll::loadRaster(GBTClassified, "/home/peter/classified_GBT.tif");
    // oll::loadRaster(LibSVMClassified, "/home/peter/classified_SVM.tif");

    // confusion matrices computation
    oll::confMatData DTcm = oll::vypocitajChybovuMaticu(DTClassified, groundTruthVector, classAtribure);
    oll::confMatData GBTcm = oll::vypocitajChybovuMaticu(GBTClassified, groundTruthVector, classAtribure);
    oll::confMatData LibSVMcm = oll::vypocitajChybovuMaticu(LibSVMClassified, groundTruthVector, classAtribure);

    std::vector<oll::ConfusionMatrixType> matrices;
    std::vector<oll::ConfusionMatrixCalculatorType::MapOfClassesType> maps;
    oll::LabelImageListType::Pointer classifiedImages = oll::LabelImageListType::New();
    matrices.push_back(DTcm.confMat);
    maps.push_back(DTcm.mapOfClasses);
    classifiedImages->PushBack(DTClassified);
    matrices.push_back(GBTcm.confMat);
    maps.push_back(GBTcm.mapOfClasses);
    classifiedImages->PushBack(GBTClassified);
    matrices.push_back(LibSVMcm.confMat);
    maps.push_back(LibSVMcm.mapOfClasses);
    classifiedImages->PushBack(LibSVMClassified);

    oll::LabelImageType::Pointer fusedImage = oll::LabelImageType::New();
    oll::dsf(classifiedImages, matrices, maps, 0, 255, fusedImage);

    oll::ulozRaster(fusedImage, "/home/peter/fused.tif");

    return 1;
}
