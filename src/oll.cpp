#include "oll.hpp"

namespace oll
{
bool checkIfExists(const boost::filesystem::path path, const oll::existanceCheckType mode)
{
    switch (mode)
    {
    // input file
    case oll::inputFilePath:
        if (!boost::filesystem::exists(path))
        {
            std::cerr << "File " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(FILE_DOES_NOT_EXISTS);
        }
        if (!boost::filesystem::is_regular_file(path))
        {
            std::cerr << path << " is not a valid file!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(PATH_IS_NOT_FILE);
        }
        return true;
        break;

    // path
    case oll::folderPath:
        if (!boost::filesystem::exists(path))
        {
            std::cerr << "Directory " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(FILE_DOES_NOT_EXISTS);
        }
        if (!boost::filesystem::is_directory(path))
        {
            std::cerr << path << " is not a valid directory!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(PATH_IS_NOT_DIR);
        }
        return true;
        break;

    // output path (input is output file name)
    case oll::outputFilePath:
        if (!boost::filesystem::exists(path.parent_path()))
        {
            std::cerr << "Path " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(PATH_DOES_NOT_EXISTS);
        }
        if (!boost::filesystem::is_directory(path.parent_path()))
        {
            std::cerr << path << " is not a valid directory!" << std::endl << EXIT_MESSAGE << std::endl;
            exit(PATH_IS_NOT_DIR);
        }
        return true;
        break;
    default:
        std::cerr << "Parameter mode value " << mode << " out of range!" << std::endl << EXIT_MESSAGE << std::endl;
    }
}

void loadRaster(oll::ImageType::Pointer raster, std::string path)
{
    oll::checkIfExists(path, oll::inputFilePath);

    typedef otb::ImageFileReader<oll::ImageType> ImageFileReaderType;
    ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
    reader->SetFileName(path);
    reader->Update();
    raster->Graft(reader->GetOutput());
    raster->Update();
}

void loadRaster(oll::LabelImageType::Pointer raster, std::string path)
{
    oll::checkIfExists(path, oll::inputFilePath);

    typedef otb::ImageFileReader<oll::LabelImageType> ImageFileReaderType;
    ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
    reader->SetFileName(path);
    reader->Update();
    raster->Graft(reader->GetOutput());
    raster->Update();
}

void loadVector(oll::VectorDataType::Pointer vector, std::string path)
{
    oll::checkIfExists(path, oll::inputFilePath);

    typedef otb::VectorDataFileReader<oll::VectorDataType> VectorReaderType;
    VectorReaderType::Pointer vectorReader = VectorReaderType::New();
    vectorReader->SetFileName(path);
    vectorReader->Update();

    vector->Graft(vectorReader->GetOutput());
    vector->Update();
}

void train(oll::ImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
           std::string classAttributeName, oll::trainingMethod trainingMethod)
{
    // vector data reprojection to source image projection
    typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, ImageType> VectorDataReprojectionType;
    VectorDataReprojectionType::Pointer vdReproj = VectorDataReprojectionType::New();

    vdReproj->SetInputImage(image);
    vdReproj->SetInput(trainingSites);
    vdReproj->SetUseOutputSpacingAndOriginFromImage(false);
    vdReproj->Update();

    // transforming training samples into samples
    typedef otb::ListSampleGenerator<ImageType, VectorDataType> ListSampleGeneratorType;
    ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();

    sampleGenerator->SetInput(image);
    sampleGenerator->SetInputVectorData(trainingSites);
    sampleGenerator->SetClassKey(classAttributeName);
    sampleGenerator->SetBoundByMin(false);

    sampleGenerator->Update();

    // training itself
    switch (trainingMethod)
    {
    case oll::libSVM:
    {
        // TODO: redirect svm output
        typedef otb::LibSVMMachineLearningModel<ImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType> SVMType;
        SVMType::Pointer SVMClassifier = SVMType::New();
        SVMClassifier->SetKernelType(CvSVM::LINEAR);
        SVMClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
        SVMClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());

        SVMClassifier->Train();
        SVMClassifier->Save(outputModel);

        break;
    }
    case oll::gradientBoostedTree:
    {
        typedef otb::GradientBoostedTreeMachineLearningModel<ImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType> GBTType;
        GBTType::Pointer GBTClassifier = GBTType::New();
        GBTClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
        GBTClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());

        GBTClassifier->Train();
        GBTClassifier->Save(outputModel);
        break;
    }
    case oll::desicionTree:
    {
        typedef otb::DecisionTreeMachineLearningModel<ImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType> DTType;
        DTType::Pointer DTClassifier = DTType::New();
        DTClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
        DTClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());

        DTClassifier->Train();
        DTClassifier->Save(outputModel);
        break;
    }
    }
}

void classify(oll::ImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster)
{
    oll::checkIfExists(inputModel, oll::inputFilePath);

    typedef otb::ImageClassificationFilter<oll::ImageType, oll::LabelImageType> ClassificationFilterType;
    typedef ClassificationFilterType::ModelType ModelType;
    typedef otb::MachineLearningModelFactory<oll::PixelType, oll::LabelPixelType> MachineLearningModelFactoryType;
    typedef otb::ImageFileReader<oll::ImageType> ReaderType;
    // typedef otb::ImageFileWriter<oll::LabelImageType> WriterType;

    ClassificationFilterType::Pointer classifier = ClassificationFilterType::New();

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputModel);

    ModelType::Pointer model =
        MachineLearningModelFactoryType::CreateMachineLearningModel(inputModel, MachineLearningModelFactoryType::ReadMode);

    model->Load(inputModel);
    classifier->SetModel(model);

    classifier->SetInput(image);

    classifier->Update();
    outputRaster->Graft(classifier->GetOutput());
}

oll::confMatData vypocitajChybovuMaticu(oll::LabelImageType::Pointer classifiedRaster, oll::VectorDataType::Pointer groundTruthVector,
                                        std::string classAttributeName)
{
    // vector data reprojection to source image projection
    typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, LabelImageType> VectorDataReprojectionType;
    VectorDataReprojectionType::Pointer vdReproj = VectorDataReprojectionType::New();

    vdReproj->SetInputImage(classifiedRaster);
    vdReproj->SetInput(groundTruthVector);
    vdReproj->SetUseOutputSpacingAndOriginFromImage(false);
    vdReproj->Update();

    // LabelImageType to VectorImageType
    // TODO: zmeň na Cast
    oll::LabelImageListType::Pointer imageList = oll::LabelImageListType::New();
    imageList->PushBack(classifiedRaster);
    oll::ImageListToVectorImageFilterType::Pointer imageListToVectorImage = oll::ImageListToVectorImageFilterType::New();
    imageListToVectorImage->SetInput(imageList);
    oll::VectorImageType::Pointer classifiedVectorRaster = imageListToVectorImage->GetOutput();
    classifiedVectorRaster->Update();

    // transforming training samples into samples
    oll::ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();
    sampleGenerator->SetInput(classifiedVectorRaster);
    sampleGenerator->SetInputVectorData(groundTruthVector);
    sampleGenerator->SetClassKey(classAttributeName);
    sampleGenerator->SetValidationTrainingProportion(1);
    sampleGenerator->SetBoundByMin(false);
    sampleGenerator->Update();

    // confusion matrix computation
    oll::ConfusionMatrixCalculatorType::Pointer cm = oll::ConfusionMatrixCalculatorType::New();
    cm->SetReferenceLabels(sampleGenerator->GetValidationListLabel());
    cm->SetProducedLabels(sampleGenerator->GetValidationListSample());
    cm->Compute();

    oll::confMatData output;
    output.mapOfClasses = cm->GetMapOfClasses();
    output.confMat = cm->GetConfusionMatrix();
    return output;
}

void dsf(oll::LabelImageListType::Pointer classifiedImages, std::vector<oll::ConfusionMatrixType> &matrices,
         std::vector<oll::ConfusionMatrixCalculatorType::MapOfClassesType> &mapOfClasses, oll::LabelPixelType nodataLabel,
         oll::LabelPixelType undecidedLabel, oll::LabelImageType::Pointer outputRaster)
{
    // confusion matrices to masses of belief
    oll::ConfusionMatrixToMassOfBeliefType::Pointer cm2mb = oll::ConfusionMatrixToMassOfBeliefType::New();
    oll::ConfusionMatrixToMassOfBeliefType::MassOfBeliefDefinitionMethod mbDef = oll::ConfusionMatrixToMassOfBeliefType::PRECISION;
    oll::DSFusionOfClassifiersImageFilterType::VectorOfMapOfMassesOfBeliefType massesOfBelief;

    for (unsigned int i = 0; i < matrices.size(); ++i)
    {
        oll::ConfusionMatrixToMassOfBeliefType::MapOfClassesType moc;
        for (ConfusionMatrixCalculatorType::MapOfClassesType::const_iterator mpt = mapOfClasses[i].begin(); mpt != mapOfClasses[i].end();
             ++mpt)
        {
            moc.insert(std::pair<oll::LabelPixelType const, int>((LabelPixelType)mpt->first, (LabelPixelType)mpt->second));
        }

        cm2mb->SetConfusionMatrix(matrices[i]);
        cm2mb->SetMapOfClasses(moc);
        cm2mb->SetDefinitionMethod(mbDef);
        cm2mb->Update();
        massesOfBelief.push_back(cm2mb->GetMapMassOfBelief());
    }

    // input images to vectorImage
    ImageListToVectorImageFilterType::Pointer il2vi = ImageListToVectorImageFilterType::New();
    il2vi->SetInput(classifiedImages);

    // fusion
    oll::DSFusionOfClassifiersImageFilterType::Pointer dsfusion = oll::DSFusionOfClassifiersImageFilterType::New();
    dsfusion->SetInput(il2vi->GetOutput());
    dsfusion->SetInputMapsOfMassesOfBelief(&massesOfBelief);
    dsfusion->SetLabelForNoDataPixels(nodataLabel);
    dsfusion->SetLabelForUndecidedPixels(undecidedLabel);
    dsfusion->Update();
    outputRaster->Graft(dsfusion->GetOutput());
}

void ulozRaster(oll::LabelImageType::Pointer raster, std::string outputFile)
{
    oll::checkIfExists(outputFile, oll::outputFilePath);

    typedef otb::ImageFileWriter<LabelImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputFile);
    writer->SetInput(raster);
    writer->Update();
}

void ulozRaster(oll::ImageType::Pointer raster, std::string outputFile)
{
    oll::checkIfExists(outputFile, oll::outputFilePath);

    typedef otb::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputFile);
    writer->SetInput(raster);
    writer->Update();
}

}
