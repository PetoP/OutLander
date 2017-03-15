#include "../oll/olcl.hxx"
#include <boost/program_options.hpp>
#include <ctime>
#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{
    // CLI creation
    namespace po = boost::program_options;

    // variables to hold CLI options
    string sourceImage, trainingSamples, groundTruth, classAtribure, demDir, reclasRulesFile, outRecl, outPodPlod, outPodSklon, outAll,
        outAlbedo;
    bool svm = false;
    bool svmlin = false;
    bool gbt = false;
    bool dt = false;
    int numClassifiers = 0;
    bool classify = false;
    bool reclassify = false;
    bool optimize = false;

    // CLI options declaration
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("svm,s", "Use SVM classifier.")
        ("linsvm,l", "Use linear SVM classifier.")
        ("opt,o", "Optimize classifier parmetters.")
        ("gbt,g", "Use GBT classifier.")
        ("dt,d", "Use DT classifier.")
        ("l8", "Satellite is L8")
        ("s2", "Satellite is S2")
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

    string usage = " [-hslgd] [--l8] [--s2] --isr --its --igt --ica --demdir [--irr] [--olr] [--occ] [--ocs] [--oa]";

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
        cout << "Option isr is missing!" << endl;
        parametersOk = false;
    }

    if (!vm.count(("oa")) || (vm.count("its")) || vm.count("igt") || vm.count("ica") || vm.count("demdir") || vm.count("olr") ||
        vm.count("occ") || vm.count("ocs") || vm.count("oa") || vm.count("out"))
    {
        classify = true;
        if (!vm.count("svm") && !vm.count("gbt") && !vm.count("dt"))
        {
            svm = gbt = dt = true;
            numClassifiers = 3;
        }
        else
        {
            if (vm.count("linsvm"))
            {
                svmlin = true;
                svm = true;
                numClassifiers++;
            }

            if (vm.count("svm"))
            {
                svm = true;
                numClassifiers++;
            }

            if (vm.count("gbt"))
            {
                gbt = true;
                numClassifiers++;
            }

            if (vm.count("dt"))
            {
                dt = true;
                numClassifiers++;
            }
        }

        if (vm.count("opt"))
        {
            optimize = true;
        }

        if (!vm.count("its"))
        {
            cout << "Option its is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("igt"))
        {
            cout << "Option igt is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("ica"))
        {
            cout << "Option ica is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("demdir"))
        {
            cout << "Option demdir is missing!" << endl;
            parametersOk = false;
        }

        if (!vm.count("olr") && !vm.count("occ") && !vm.count("ocs") && !vm.count("out"))
        {
            cout << "No output specified!" << endl;
            parametersOk = false;
        }

        if (vm.count("occ") || vm.count("ocs") || vm.count("out"))
        {
            reclassify = true;
        }

        if (vm.count("oa")) {
            if ((!vm.count("l8") && !vm.count("s2")) || (vm.count("l8") && vm.count("s2")))
            {
                cout << "You need to specify satellite for albedo calculation!" << endl;
                parametersOk = false;
            }
        }
    }

    if (!parametersOk)
    {
        cout << endl << "Use option -h [--help] for help." << endl << "Usage: " << argv[0] << usage << endl;
        return -1;
    }

    time_t timer1;
    time(&timer1);
    string sufix = std::to_string(timer1);
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
            cerr << "Input dem directory is not usable!\n";
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

        oll::ReclassificationRulesType reclassificationRules;
        if (reclassify)
        {
            if (!oll::checkIfExists(reclasRulesFile, oll::inputFilePath))
            {
                cerr << "Can't read input reclassification rules file!" << endl;
                return -1;
            }

            // reading and validation of input reclassification rules
            reclassificationRules = oll::readReclassificationRules(reclasRulesFile);
        }

        // image training and classification
        string modelDT = "/tmp/modelDT" + sufix;
        string modelGBT = "/tmp/modelGBT.txt" + sufix;
        string modelSVM = "/tmp/modelSVM.txt" + sufix;
        oll::LabelImageType::Pointer DTClassified = oll::LabelImageType::New();
        oll::LabelImageType::Pointer GBTClassified = oll::LabelImageType::New();
        oll::LabelImageType::Pointer SVMClassified = oll::LabelImageType::New();
        oll::confMatData DTcm, GBTcm, SVMcm;
        std::vector< oll::ConfusionMatrixType > matrices;
        std::vector< oll::ConfusionMatrixCalculatorType::MapOfClassesType > maps;
        oll::LabelImageListType::Pointer classifiedImages = oll::LabelImageListType::New();
        oll::LabelImageType::Pointer fusedImage;
        if (dt)
        {
            cout << "Decision tree classification ";

            //TODO tu si skončil, výpis časom trvania jednotlivých operácií

            std::time(&timer1);
            oll::train(inputImage, trainingSites, modelDT, classAtribure, oll::desicionTree, false, optimize);
            oll::classify(inputImage, modelDT, DTClassified);
            DTcm = oll::vypocitajChybovuMaticu(DTClassified, groundTruthVector, classAtribure);
            matrices.push_back(DTcm.confMat);
            maps.push_back(DTcm.mapOfClasses);
            classifiedImages->PushBack(DTClassified);
            fusedImage = DTClassified;
        }
        if (gbt)
        {
            oll::train(inputImage, trainingSites, modelGBT, classAtribure, oll::gradientBoostedTree, false, optimize);
            oll::classify(inputImage, modelGBT, GBTClassified);
            GBTcm = oll::vypocitajChybovuMaticu(GBTClassified, groundTruthVector, classAtribure);
            matrices.push_back(GBTcm.confMat);
            maps.push_back(GBTcm.mapOfClasses);
            classifiedImages->PushBack(GBTClassified);
            fusedImage = GBTClassified;
        }
        if (svm)
        {
            oll::train(inputImage, trainingSites, modelSVM, classAtribure, oll::libSVM, svmlin, optimize);
            oll::classify(inputImage, modelSVM, SVMClassified);
            SVMcm = oll::vypocitajChybovuMaticu(SVMClassified, groundTruthVector, classAtribure);
            matrices.push_back(SVMcm.confMat);
            maps.push_back(SVMcm.mapOfClasses);
            classifiedImages->PushBack(SVMClassified);
            fusedImage = SVMClassified;

            oll::printConfMat(SVMcm, cout);
        }

        // oll::ulozRaster(DTClassified, "/home/peter/classified_DT.tif");
        // oll::ulozRaster(GBTClassified, "/home/peter/classified_GBT.tif");
        // oll::ulozRaster(SVMClassified, "/home/peter/classified_SVM.tif");

        if (numClassifiers >= 1)
        {
            // fusion of classified images
            oll::LabelImageType::Pointer fusedImage = oll::LabelImageType::New();
            oll::dsf(classifiedImages, matrices, maps, 0, 255, fusedImage);
        };

        if (vm.count("olr"))
        {
            oll::ulozRaster(fusedImage, outRecl);
        }

        if (reclassify)
        {
            // condition of crop applicaiton
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
                oll::podSklon(podPlod, slope, podSklon, 8.13);

                if (vm.count("ocs"))
                {
                    oll::ulozRaster(podSklon, outPodSklon);
                }

                // condition of area application
                if (vm.count("out"))
                {
                    oll::LabelImageType::Pointer podRozloh = oll::LabelImageType::New();
                    oll::podRozloh(podSklon, podRozloh, 6000);
                    oll::ulozRaster(podRozloh, outAll);
                }
            }
        }
    }
    if (vm.count("oa"))
    {
        oll::satellites satellite;
        if (vm.count("l8"))
        {
            satellite = oll::Landsat8;
        }
        else if (vm.count("s2"))
        {
            satellite = oll::Sentinel2;
        }

        oll::DoubleImageType::Pointer albedo = oll::DoubleImageType::New();
        oll::albedo(inputImage, satellite, albedo);
        oll::ulozRaster(albedo, outAlbedo);
    }

    return 0;
}
