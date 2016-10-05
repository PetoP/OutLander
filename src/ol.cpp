#include "oll.hpp"
#include <iostream>

#include <ITK-4.10/itkIndent.h>

using std::string;
using std::cout;
using std::cerr;

// this will be set from arguments
const string sourceDirectory = "/run/media/peter/WD/ZIK/diplomovka/klasifikator/landsat_z_grassu/";
const string sourceImage = sourceDirectory + "L8.tif";

const string trainingSamples = sourceDirectory + "tren.shp";
const string classAtribure = "plod_id";

const string outputDirectory = "/home/peter/Plocha/";
const string outputImage = outputDirectory + "L8.tif";

int main()
{
    // input image reading
    oll::checkIfExists(sourceImage, oll::inputFilePath);

    typedef otb::ImageFileReader<oll::ImageType> LandsatReaderType;
    LandsatReaderType::Pointer reader = LandsatReaderType::New();
    reader->SetFileName(sourceImage);
    oll::ImageType::Pointer LandsatImage = reader->GetOutput();
    LandsatImage->UpdateOutputInformation();

    // input training samples reading
    oll::checkIfExists(trainingSamples, oll::inputFilePath);

    typedef otb::VectorDataFileReader<oll::VectorDataType> VectorReaderType;
    VectorReaderType::Pointer vectorReader = VectorReaderType::New();
    vectorReader->SetFileName(trainingSamples);
    vectorReader->Update();
    oll::VectorDataType::Pointer trainingSites = vectorReader->GetOutput();
    trainingSites->Update();

    // oll::train(LandsatImage, trainingSites, "/home/peter/modelDT.txt", classAtribure, oll::desicionTree);
    // oll::train(LandsatImage, trainingSites, "/home/peter/modelGBT.txt", classAtribure, oll::gradientBoostedTree);
    // oll::train(LandsatImage, trainingSites, "/home/peter/modelLibSVM.txt", classAtribure, oll::libSVM);

    oll::LabelImageType::Pointer trainingSitesRaster = oll::LabelImageType::New();

    oll::trainingSitesToRaster(trainingSites, trainingSitesRaster, LandsatImage, classAtribure);

    // oll::ulozRaster(trainingSitesRaster, "/home/peter/pokus.tif");

    return 1;
}
