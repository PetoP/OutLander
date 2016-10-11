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

int main()
{
    // input image reading
    oll::ImageType::Pointer landsatImage = oll::ImageType::New();
    oll::loadRaster(landsatImage, sourceImage);

    // input training samples reading
    oll::VectorDataType::Pointer trainingSites = oll::VectorDataType::New();
    oll::loadVector(trainingSites, trainingSamples);

    // input groundTruth reading
    oll::VectorDataType::Pointer groundTruthVector = oll::VectorDataType::New();
    oll::loadVector(groundTruthVector, groundTruth);
    // image training
    // oll::train(landsatImage, trainingSites, "/home/peter/modelDT.txt", classAtribure, oll::desicionTree);
    // oll::train(landsatImage, trainingSites, "/home/peter/modelGBT.txt", classAtribure, oll::gradientBoostedTree);
    // oll::train(landsatImage, trainingSites, "/home/peter/modelLibSVM.txt", classAtribure, oll::libSVM);

    // image classification
    oll::LabelImageType::Pointer DTClassified = oll::LabelImageType::New();
    oll::LabelImageType::Pointer GBTClassified = oll::LabelImageType::New();
    oll::LabelImageType::Pointer LibSVMClassified = oll::LabelImageType::New();
    // oll::classify(landsatImage, "/home/peter/modelDT.txt", DTClassified);
    // oll::classify(landsatImage, "/home/peter/modelGBT.txt", GBTClassified);
    // oll::classify(landsatImage, "/home/peter/modelLibSVM.txt", LibSVMClassified);

    // oll::ulozRaster(DTClassified, "/home/peter/classified_DT.tif");
    // oll::ulozRaster(GBTClassified, "/home/peter/classified_GBT.tif");
    // oll::ulozRaster(LibSVMClassified, "/home/peter/classified_SVM.tif");

    oll::loadRaster(DTClassified, "/home/peter/classified_DT.tif");
    oll::loadRaster(GBTClassified, "/home/peter/classified_GBT.tif");
    oll::loadRaster(LibSVMClassified, "/home/peter/classified_SVM.tif");

    // confusion matrices computation
    oll::ConfusionMatrixType DTcm = oll::vypocitajChybovuMaticu(DTClassified, groundTruthVector, classAtribure);
    oll::ConfusionMatrixType GBTcm = oll::vypocitajChybovuMaticu(GBTClassified, groundTruthVector, classAtribure);
    oll::ConfusionMatrixType LibSVMcm = oll::vypocitajChybovuMaticu(LibSVMClassified, groundTruthVector, classAtribure);

    // for (unsigned int i = 0; i < cm.Rows(); ++i)
    // {
    //     for (unsigned int j = 0; j < cm.Cols(); ++j)
    //     {
    //         std::cout << cm.operator()(i, j) << " ";
    //     }
    //     std::cout << std::endl;
    // }

    std::vector<oll::ConfusionMatrixType> matrices;
    oll::LabelImageListType::Pointer classifiedImages = oll::LabelImageListType::New();
    matrices.push_back(DTcm);
    classifiedImages->PushBack(DTClassified);
    matrices.push_back(GBTcm);
    classifiedImages->PushBack(GBTClassified);
    matrices.push_back(LibSVMcm);
    classifiedImages->PushBack(LibSVMClassified);

    return 1;
}
