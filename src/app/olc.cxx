#include "../oll/olcl.hxx"
#include <boost/program_options.hpp>
#include <iostream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{
    // CLI creation
    namespace po = boost::program_options;

    // variables to hold CLI options
    string sourceImage, trainingSamples, groundTruth, classAtribure, demDir, reclasRulesFile, outRecl, outPodPlod, outPodSklon, outAll, outAlbedo;
    bool classify = false;

    // CLI options declaration
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("isr", po::value< string >(&sourceImage), "Input multiband satellite raster data.")
        ("its", po::value< string >(&trainingSamples), "Input training samples vector.")
        ("igt", po::value< string >(&groundTruth), "Input ground truth vector.")
        ("ica", po::value< string >(&classAtribure), "Class attribute name in input training samples and ground truth vectors.")
        ("irr", po::value< string >(&reclasRulesFile), "Text file containing reclassification rules (1)")
        ("demdir", po::value< string >(&demDir), "Directory containing SRTM hgt files.")
        ("olr", po::value< string >(&outRecl), "Output landcover raster.")
        ("occ", po::value< string >(&outPodPlod), "Output condition of crop raster.")
        ("ocs", po::value< string >(&outPodSklon), "Output condition of slope raster.")
        ("out", po::value< string >(&outAll), "Output raster with all condition applied.")
        ("oa", po::value< string >(&outAlbedo), "Output albedo raster.");

    std::string usage = " [-h] --isr --its --igt --ica --demdir [--irr] [--olr] [--occ] [--ocs] [--oa]";

    // CLI options handling
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    bool parametersOk = true;

    if (vm.count("help"))
    {
        cout << "Usage: " << argv[0] << usage << endl << endl << desc << endl;
        return 1;
    }

    if (!vm.count("isr"))
    {
        std::cout << "Option isr is missing!" << endl;
        parametersOk = false;
    }

    if (!vm.count(("oa")) || (vm.count("its")) || vm.count("igt") || vm.count("ica") || vm.count("demdir") || vm.count("olr"))
    {
        classify = true;
        if (!vm.count("its"))
        {
            std::cout << "Option its is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("igt"))
        {
            std::cout << "Option igt is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("ica"))
        {
            std::cout << "Option ica is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("demdir"))
        {
            std::cout << "Option demdir is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("olr") && !vm.count("occ") && !vm.count("ocs") && !vm.count("out"))
        {
            std::cout << "No output specified!" << endl;
            parametersOk = false;
        }
    }

    if (!parametersOk)
    {
        std::cout << endl << "Use option -h [--help] for help." << endl << "Usage: " << argv[0] << usage << endl;
        return -1;
    }

    GDALAllRegister();

    // input image reading
    if (!oll::checkIfExists(sourceImage, oll::inputFilePath))
    {
        cerr << "Can't read input satellite image!" << endl;
        return -1;
    }
    oll::VectorImageType::Pointer inputImage = oll::VectorImageType::New();
    oll::loadRaster(inputImage, sourceImage);

    if (classify)
    {
        // DEM registration
        otb::DEMHandler::Pointer demHandler = otb::DEMHandler::Instance();
        if (!demHandler->IsValidDEMDirectory(demDir.c_str()))
        {
            std::cerr << "Input dem directory is not usable!\n";
            return -1;
        }
        demHandler->OpenDEMDirectory(demDir);

        // input training samples reading
        if (!oll::checkIfExists(trainingSamples, oll::inputFilePath))
        {
            cerr << "Can't read input training samples!" << endl;
            return -1;
        }
        oll::VectorDataType::Pointer trainingSites = oll::VectorDataType::New();
        oll::loadVector(trainingSites, trainingSamples);

        // input ground truth vector reading
        if (!oll::checkIfExists(groundTruth, oll::inputFilePath))
        {
            cerr << "Can't read input ground truth!" << endl;
            return -1;
        }
        oll::VectorDataType::Pointer groundTruthVector = oll::VectorDataType::New();
        oll::loadVector(groundTruthVector, groundTruth);

        if (!oll::checkIfExists(reclasRulesFile, oll::inputFilePath))
        {
            cerr << "Can't read input reclassification rules file!" << endl;
            return -1;
        }

        // reading and validation of input reclassification rules
        oll::ReclassificationRulesType reclassificationRules = oll::readReclassificationRules(reclasRulesFile);

        // image training
        string modelDT = "/tmp/modelDT.txt";
        string modelGBT = "/tmp/modelGBT.txt";
        string modelLibSVM = "/tmp/modelLibSVM.txt";
        oll::train(inputImage, trainingSites, modelDT, classAtribure, oll::desicionTree);
        oll::train(inputImage, trainingSites, modelGBT, classAtribure, oll::gradientBoostedTree);
        oll::train(inputImage, trainingSites, modelLibSVM, classAtribure, oll::libSVM);

        // image classification
        oll::LabelImageType::Pointer DTClassified = oll::LabelImageType::New();
        oll::LabelImageType::Pointer GBTClassified = oll::LabelImageType::New();
        oll::LabelImageType::Pointer LibSVMClassified = oll::LabelImageType::New();
        oll::classify(inputImage, modelDT, DTClassified);
        oll::classify(inputImage, modelGBT, GBTClassified);
        oll::classify(inputImage, modelLibSVM, LibSVMClassified);

        // oll::ulozRaster(DTClassified, "/home/peter/classified_DT.tif");
        // oll::ulozRaster(GBTClassified, "/home/peter/classified_GBT.tif");
        // oll::ulozRaster(LibSVMClassified, "/home/peter/classified_SVM.tif");

        // confusion matrices computation
        oll::confMatData DTcm = oll::vypocitajChybovuMaticu(DTClassified, groundTruthVector, classAtribure);
        oll::confMatData GBTcm = oll::vypocitajChybovuMaticu(GBTClassified, groundTruthVector, classAtribure);
        oll::confMatData LibSVMcm = oll::vypocitajChybovuMaticu(LibSVMClassified, groundTruthVector, classAtribure);

        std::vector< oll::ConfusionMatrixType > matrices;
        std::vector< oll::ConfusionMatrixCalculatorType::MapOfClassesType > maps;
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

        // fusion of classified images
        oll::LabelImageType::Pointer fusedImage = oll::LabelImageType::New();
        oll::dsf(classifiedImages, matrices, maps, 0, 255, fusedImage);

        if (vm.count("olr"))
        {
            oll::ulozRaster(fusedImage, outRecl);
        }

        // condition od crop applicaiton
        oll::LabelImageType::Pointer podPlod = oll::LabelImageType::New();
        oll::reclassifyRaster(fusedImage, podPlod, reclassificationRules);

        if (vm.count("occ"))
        {
            oll::ulozRaster(podPlod, outPodPlod);
        }

        // condition of slope application
        if (vm.count("ocs") || vm.count("out"))
        {
            oll::DoubleImageType::Pointer alignedDem = oll::DoubleImageType::New();
            oll::alignDEM(inputImage, alignedDem);

            oll::DoubleImageType::Pointer slope = oll::DoubleImageType::New();
            oll::computeSlopeRaster(alignedDem, slope);

            oll::LabelImageType::Pointer podSklon = oll::LabelImageType::New();
            oll::podSklon(podPlod, slope, podSklon, 5);

            if (vm.count("ocs"))
            {
                oll::ulozRaster(podSklon, outPodSklon);
            }

            // condition of area application
            if (vm.count("out"))
            {
                oll::LabelImageType::Pointer podRozloh = oll::LabelImageType::New();
                oll::podRozloh(podSklon, podRozloh, 150);
                oll::ulozRaster(podRozloh, outAll);
            }
        }
    }
    if (vm.count("oa"))
    {
        oll::DoubleImageType::Pointer albedo = oll::DoubleImageType::New();
        oll::albedo(inputImage, oll::Sentinel2, albedo);
        oll::ulozRaster(albedo, outAlbedo);
    }

    return 0;
}
