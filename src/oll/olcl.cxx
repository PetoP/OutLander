 #include "olcl.hxx"

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
                // std::cerr << "File " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(FILE_DOES_NOT_EXISTS);
                return false;
            }
            if (!boost::filesystem::is_regular_file(path))
            {
                // std::cerr << path << " is not a valid file!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(PATH_IS_NOT_FILE);
                return false;
            }
            return true;

        // path
        case oll::folderPath:
            if (!boost::filesystem::exists(path))
            {
                // std::cerr << "Directory " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(FILE_DOES_NOT_EXISTS);
                return false;
            }
            if (!boost::filesystem::is_directory(path))
            {
                // std::cerr << path << " is not a valid directory!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(PATH_IS_NOT_DIR);
                return false;
            }
            return true;

        // output path (input is output file name)
        case oll::outputFilePath:
            if (!boost::filesystem::exists(path.parent_path()))
            {
                // std::cerr << "Path " << path << " does not exists!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(PATH_DOES_NOT_EXISTS);
                return false;
            }
            if (!boost::filesystem::is_directory(path.parent_path()))
            {
                // std::cerr << path << " is not a valid directory!" << std::endl << EXIT_MESSAGE << std::endl;
                // exit(PATH_IS_NOT_DIR);
                return false;
            }
            return true;
        }
    }

    void loadRaster(oll::VectorImageType::Pointer raster, std::string path)
    {
        // oll::checkIfExists(path, oll::inputFilePath);

        typedef otb::ImageFileReader< oll::VectorImageType > ImageFileReaderType;
        ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
        reader->SetFileName(path);
        reader->Update();
        raster->Graft(reader->GetOutput());
        raster->Update();
    }

    void loadRaster(oll::LabelImageType::Pointer raster, std::string path)
    {
        // oll::checkIfExists(path, oll::inputFilePath);

        typedef otb::ImageFileReader< oll::LabelImageType > ImageFileReaderType;
        ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
        reader->SetFileName(path);
        reader->Update();
        raster->Graft(reader->GetOutput());
        raster->Update();
    }

    void loadRaster(oll::DoubleImageType::Pointer raster, std::string path)
    {
        // oll::checkIfExists(path, oll::inputFilePath);

        typedef otb::ImageFileReader< oll::DoubleImageType > ImageFileReaderType;
        ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
        reader->SetFileName(path);
        reader->Update();
        raster->Graft(reader->GetOutput());
        raster->Update();
    }

    void loadVector(oll::VectorDataType::Pointer vector, std::string path)
    {
        // oll::checkIfExists(path, oll::inputFilePath);

        typedef otb::VectorDataFileReader< oll::VectorDataType > VectorReaderType;
        VectorReaderType::Pointer vectorReader = VectorReaderType::New();
        vectorReader->SetFileName(path);
        vectorReader->Update();

        vector->Graft(vectorReader->GetOutput());
        vector->Update();
    }

    void train(oll::VectorImageType::Pointer image, oll::VectorDataType::Pointer trainingSites, std::string outputModel,
               std::string classAttributeName, oll::trainingMethod trainingMethod, bool linear, bool optimize)
    {
        // vector data reprojection to source image projection
        typedef otb::VectorDataIntoImageProjectionFilter< VectorDataType, VectorImageType > VectorDataReprojectionType;
        VectorDataReprojectionType::Pointer vdReproj = VectorDataReprojectionType::New();

        vdReproj->SetInputImage(image);
        vdReproj->SetInput(trainingSites);
        vdReproj->SetUseOutputSpacingAndOriginFromImage(false);
        vdReproj->Update();

        // transforming training samples into samples
        typedef otb::ListSampleGenerator< VectorImageType, VectorDataType > ListSampleGeneratorType;
        ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();

        sampleGenerator->SetInput(image);
        sampleGenerator->SetInputVectorData(trainingSites);
        sampleGenerator->SetClassKey(classAttributeName);
        // sampleGenerator->SetBoundByMin(false);

        sampleGenerator->Update();

        // training itself
        switch (trainingMethod)
        {
        case oll::libSVM:
        {
            if (!linear)
            {
                typedef otb::SVMMachineLearningModel< VectorImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType > SVMType;
                SVMType::Pointer SVMClassifier = SVMType::New();
                SVMClassifier->SetKernelType(CvSVM::RBF);
                SVMClassifier->SetSVMType(CvSVM::NU_SVC);
                SVMClassifier->SetNu(0.5);
                SVMClassifier->SetGamma(1.0e-07);
                SVMClassifier->SetC(10000000000);
                SVMClassifier->SetParameterOptimization(optimize);
                SVMClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
                SVMClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());
                SVMClassifier->Train();
                SVMClassifier->Save(outputModel);
            }
            else
            {
                // TODO: redirect svm output
                typedef otb::LibSVMMachineLearningModel< VectorImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType > SVMType;
                SVMType::Pointer SVMClassifier = SVMType::New();
                SVMClassifier->SetKernelType(LINEAR);
                SVMClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
                SVMClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());
                SVMClassifier->Train();
                SVMClassifier->Save(outputModel);
            }

            break;
        }
        case oll::gradientBoostedTree:
        {
            typedef otb::GradientBoostedTreeMachineLearningModel< VectorImageType::InternalPixelType,
                                                                  ListSampleGeneratorType::ClassLabelType >
                GBTType;
            GBTType::Pointer GBTClassifier = GBTType::New();
            GBTClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
            GBTClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());

            GBTClassifier->Train();
            GBTClassifier->Save(outputModel);
            break;
        }
        case oll::desicionTree:
        {
            typedef otb::DecisionTreeMachineLearningModel< VectorImageType::InternalPixelType, ListSampleGeneratorType::ClassLabelType >
                DTType;
            DTType::Pointer DTClassifier = DTType::New();
            DTClassifier->SetInputListSample(sampleGenerator->GetTrainingListSample());
            DTClassifier->SetTargetListSample(sampleGenerator->GetTrainingListLabel());

            DTClassifier->Train();
            DTClassifier->Save(outputModel);
            break;
        }
        }
    }

    void classify(oll::VectorImageType::Pointer image, std::string inputModel, oll::LabelImageType::Pointer outputRaster)
    {
        // oll::checkIfExists(inputModel, oll::inputFilePath);

        typedef otb::ImageClassificationFilter< oll::VectorImageType, oll::LabelImageType > ClassificationFilterType;
        typedef ClassificationFilterType::ModelType ModelType;
        typedef otb::MachineLearningModelFactory< oll::PixelType, oll::LabelPixelType > MachineLearningModelFactoryType;
        typedef otb::ImageFileReader< oll::VectorImageType > ReaderType;
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
        typedef otb::VectorDataIntoImageProjectionFilter< VectorDataType, LabelImageType > VectorDataReprojectionType;
        VectorDataReprojectionType::Pointer vdReproj = VectorDataReprojectionType::New();

        vdReproj->SetInputImage(classifiedRaster);
        vdReproj->SetInput(groundTruthVector);
        vdReproj->SetUseOutputSpacingAndOriginFromImage(false);
        vdReproj->Update();

        // LabelImageType to VectorImageType
        oll::ImageToVectorImageCastFilterType::Pointer li2vi = oll::ImageToVectorImageCastFilterType::New();
        li2vi->SetInput(classifiedRaster);
        oll::LabelVectorImageType::Pointer classifiedVectorRaster = li2vi->GetOutput();
        classifiedVectorRaster->Update();

        // transforming training sites into samples
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

    void dsf(oll::LabelImageListType::Pointer classifiedImages, std::vector< oll::ConfusionMatrixType >& matrices,
             std::vector< oll::ConfusionMatrixCalculatorType::MapOfClassesType >& mapOfClasses, oll::LabelPixelType nodataLabel,
             oll::LabelPixelType undecidedLabel, oll::LabelImageType::Pointer outputRaster)
    {
        // confusion matrices to masses of belief
        oll::ConfusionMatrixToMassOfBeliefType::Pointer cm2mb = oll::ConfusionMatrixToMassOfBeliefType::New();
        oll::ConfusionMatrixToMassOfBeliefType::MassOfBeliefDefinitionMethod mbDef = oll::ConfusionMatrixToMassOfBeliefType::PRECISION;
        oll::DSFusionOfClassifiersImageFilterType::VectorOfMapOfMassesOfBeliefType massesOfBelief;

        for (unsigned int i = 0; i < matrices.size(); ++i)
        {
            oll::ConfusionMatrixToMassOfBeliefType::MapOfClassesType moc;
            for (ConfusionMatrixCalculatorType::MapOfClassesType::const_iterator mpt = mapOfClasses[i].begin();
                 mpt != mapOfClasses[i].end(); ++mpt)
            {
                moc.insert(std::pair< oll::LabelPixelType const, int >((LabelPixelType)mpt->first, (LabelPixelType)mpt->second));
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
        // oll::checkIfExists(outputFile, oll::outputFilePath);

        typedef otb::ImageFileWriter< LabelImageType > WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetGlobalWarningDisplay(false);
        writer->SetFileName(outputFile);
        writer->SetInput(raster);
        writer->Update();
    }

    void ulozRaster(oll::DoubleImageType::Pointer raster, std::string outputFile)
    {
        // oll::checkIfExists(outputFile, oll::outputFilePath);

        typedef otb::ImageFileWriter< DoubleImageType > WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetGlobalWarningDisplay(false);
        writer->SetFileName(outputFile);
        writer->SetInput(raster);
        writer->Update();
    }

    void ulozRaster(oll::VectorImageType::Pointer raster, std::string outputFile)
    {
        // oll::checkIfExists(outputFile, oll::outputFilePath);

        typedef otb::ImageFileWriter< VectorImageType > WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetGlobalWarningDisplay(false);
        writer->SetFileName(outputFile);
        writer->SetInput(raster);
        writer->Update();
    }

    oll::ReclassificationRulesType readReclassificationRules(std::string pathToReclassificationRules)
    {
        std::ifstream rulesFile(pathToReclassificationRules.c_str(), std::ios::in);
        oll::ReclassificationRulesType reclassificationRules;

        unsigned long a, b;
        std::string line, first, second;

        bool badData = false;

        if (!rulesFile.is_open())
        {
            // TODO lepšie sformuluj chybovú hlášku a vyhoď výnimku
            std::cerr << "Unable to open file" << std::endl;
        }
        else
        {
            // iterating lines
            int lineNumber = 0;
            while (getline(rulesFile, line))
            {
                ++lineNumber;

                // searching for delimiter
                char delimiter = ' ';

                // is there any delimiter?
                if (line.find(delimiter) == std::string::npos)
                {
                    // TODO vyhoď výnimku
                    std::cerr << "It seems there is only one number at line " << lineNumber << "!" << std::endl;
                    badData = true;
                }
                else
                {
                    // spliting by first occurence of delimiter
                    first = line.substr(0, line.find(" "));
                    second = line.substr(line.find(" ") + 1, line.size());

                    // testing if there are only digits
                    for (std::string::const_iterator it = first.begin(); it != first.end(); ++it)
                    {
                        if (!isdigit(*it))
                        {
                            std::cerr << "Non-digit character in first value of input reclassification rules file at line " << lineNumber
                                      << "." << std::endl;
                        }
                    }

                    for (std::string::const_iterator it = second.begin(); it != second.end(); ++it)
                    {
                        if (!isdigit(*it))
                        {
                            // TODO vyhoď výnimku
                            std::cerr << "Non-digit character in second value of input reclassification rules file at line " << lineNumber
                                      << "." << std::endl;
                            badData = true;
                        }
                    }
                }

                // conversion to long to check if in range of oll::LabelPixelType
                a = atoll(first.c_str());
                b = atoll(second.c_str());

                if (a > std::numeric_limits< oll::LabelPixelType >::max())
                {
                    // TODO vyhoď výnimku
                    std::cerr << "First value at line " << lineNumber << " is higher than maximum label value "
                              << std::numeric_limits< oll::LabelPixelType >::max() << "!" << std::endl;
                    badData = true;
                }
                else if (b > std::numeric_limits< oll::LabelPixelType >::max())
                {
                    // TODO vyhoď výnimku
                    std::cerr << "Second value at line " << lineNumber << " is higher than maximum label value "
                              << std::numeric_limits< oll::LabelPixelType >::max() << "!" << std::endl;
                    badData = true;
                }
                else
                {
                    reclassificationRules.push_back(std::pair< LabelPixelType, LabelPixelType >((LabelPixelType)a, (LabelPixelType)b));
                }
            }
        }

        rulesFile.close();

        // TODO toto by mal riešiť hlavný program
        if (badData)
        {
            exit(-1);
        }

        return reclassificationRules;
    }

    void reclassifyRaster(const oll::LabelImageType::Pointer inputRaster, oll::LabelImageType::Pointer outputRaster,
                          const oll::ReclassificationRulesType& reclassificationRules)
    {
        outputRaster->Graft(inputRaster);
        outputRaster->Update();

        oll::LabelImageRegionIteratorType lii(outputRaster, outputRaster->GetRequestedRegion());
        oll::ReclassificationRulesType::const_iterator rri;

        for (lii.GoToBegin(); !lii.IsAtEnd(); ++lii)
        {
            for (rri = reclassificationRules.begin(); rri != reclassificationRules.end(); ++rri)
            {
                if (lii.Value() == rri->first)
                {
                    lii.Set(rri->second);
                }
            }
        }
    }

    void computeSlopeRaster(const oll::DoubleImageType::Pointer demRaster, oll::DoubleImageType::Pointer slopeRaster)
    {
        slopeRaster->CopyInformation(demRaster);
        slopeRaster->Update();

        // code borrowed and modified from OTB (otbDEMCaracteristicsExtractor.txx)
        oll::GradientMagnitudeImageFilterType::Pointer gmif = oll::GradientMagnitudeImageFilterType::New();
        oll::AtanImageFilterType::Pointer aif = oll::AtanImageFilterType::New();
        oll::MultiplyByScalarImageFilterType::Pointer mbsif = oll::MultiplyByScalarImageFilterType::New();

        double r2dc = 180 / boost::math::constants::pi< double >();

        gmif->SetInput(demRaster);
        aif->SetInput(gmif->GetOutput());
        aif->Update();
        mbsif->SetInput(aif->GetOutput());
        mbsif->SetCoef(r2dc);
        mbsif->Update();

        slopeRaster->Graft(mbsif->GetOutput());
        slopeRaster->SetProjectionRef(demRaster->GetProjectionRef());
    }

    void podSklon(const oll::LabelImageType::Pointer podPlodRaster, const oll::DoubleImageType::Pointer slopeRaster,
                  oll::LabelImageType::Pointer outputRaster, oll::PixelType hranicnySklon)
    {
        outputRaster->Graft(podPlodRaster);
        outputRaster->Update();

        oll::LabelImageRegionIteratorType lii(outputRaster, outputRaster->GetRequestedRegion());
        oll::DoubleImageRegionConstIteratorType dii(slopeRaster, slopeRaster->GetRequestedRegion());

        for (lii.GoToBegin(), dii.GoToBegin(); !lii.IsAtEnd(); ++lii, ++dii)
        {
            if (dii.Value() >= hranicnySklon && lii.Value() == 1)
            {
                lii.Set(2);
            }
        }
    }

    void podRozloh(const oll::LabelImageType::Pointer podSklon, oll::LabelImageType::Pointer outputRaster, float hranicnaVelkost)
    {
        // preparing stuff
        std::string attributeName = "val";
        OGRSpatialReference* pSRS = new OGRSpatialReference(podSklon->GetProjectionRef().c_str());

        // new vector dataset
        GDALDriver* pDriver = (GDALDriver*)GDALGetDriverByName("Memory");
        GDALDataset* newPolygons = pDriver->Create("vymazma", 0, 0, 0, GDT_Unknown, NULL);
        OGRLayer* newLyr = newPolygons->CreateLayer("vymazma", pSRS, wkbPolygon, NULL);
        OGRFieldDefn field("val", OFTInteger);
        newLyr->CreateField(&field);

        // filters creation
        oll::LabelImageToOGRDataSourceFilterType::Pointer li2vd = oll::LabelImageToOGRDataSourceFilterType::New();
        oll::OGRDataSourceToLabelImageFilter::Pointer vd2li = oll::OGRDataSourceToLabelImageFilter::New();

        // vectorization
        li2vd->SetInput(podSklon);
        li2vd->SetFieldName(attributeName);
        li2vd->Update();

        // getting output of vectorization
        GDALDataset* polygons = &(const_cast< otb::ogr::DataSource* >(li2vd->GetOutput())->ogr());
        OGRLayer* lyr = polygons->GetLayer(0);

        // copying and modifying features
        OGRFeature* pFeature;
        lyr->ResetReading();
        while ((pFeature = lyr->GetNextFeature()) != NULL)
        {
            if (pFeature->GetFieldAsInteger(attributeName.c_str()) == 1 &&
                ((OGRPolygon*)pFeature->GetGeometryRef())->get_Area() < hranicnaVelkost)
            {
                pFeature->SetField(attributeName.c_str(), 2);
            }
            if (newLyr->CreateFeature(pFeature) != OGRERR_NONE)
            {
                std::cerr << "Failed to create feature FID: " << pFeature->GetFID() << std::endl;
            }
        }

        // OTB OGRFeature creation and rasterization
        otb::ogr::DataSource::Pointer newPolygonsOtb = otb::ogr::DataSource::New(newPolygons, otb::ogr::DataSource::Modes::Read);
        vd2li->AddOGRDataSource(newPolygonsOtb);
        vd2li->SetBurnAttribute(attributeName);
        vd2li->SetOutputParametersFromImage(podSklon);
        vd2li->Update();

        outputRaster->Graft(vd2li->GetOutput());
    }

    void alignDEM(const oll::VectorImageType::Pointer sourceImage, oll::DoubleImageType::Pointer alignedDem)
    {
        std::string dstProj = sourceImage->GetProjectionRef();

        // getting DEM Raster
        oll::DEMToImageGenerator::Pointer dig = oll::DEMToImageGenerator::New();
        dig->SetOutputOrigin(sourceImage->GetOrigin());
        dig->SetOutputProjectionRef(dstProj);
        dig->SetOutputSize(sourceImage->GetRequestedRegion().GetSize());
        dig->SetOutputSpacing(sourceImage->GetSpacing());
        dig->Update();

        alignedDem->Graft(dig->GetOutput());
    }

    void albedo(const oll::VectorImageType::Pointer satelliteImage, const oll::satellites satelliteType,
                oll::DoubleImageType::Pointer albedoRaster)
    {
        // creating new raster
        albedoRaster->SetProjectionRef(satelliteImage->GetProjectionRef());
        albedoRaster->SetOrigin(satelliteImage->GetOrigin());
        albedoRaster->SetSpacing(satelliteImage->GetSpacing());
        albedoRaster->SetRegions(satelliteImage->GetLargestPossibleRegion());
        albedoRaster->Allocate();

        // setting bands indexies
        int blue, red, nir1, nir2, swir;
        if (satelliteType == oll::Landsat8)
        {
            blue = 2 - 1;
            red = 4 - 1;
            nir1 = 5 - 1;
            nir2 = 6 - 1;
            swir = 7 - 1;
        }
        else if (satelliteType == oll::Sentinel2)
        {
            double xSpacing = round(std::abs(satelliteImage->GetSpacing()[0]));
            if (xSpacing == 20)
            {
                blue = 1 - 1;
                red = 3 - 1;
                nir1 = 7 - 1;
                nir1 = 8 - 1;
                swir = 9 - 1;
            }
            else if (xSpacing == 60)
            {
                blue = 2 - 1;
                red = 4 - 1;
                nir1 = 8 - 1;
                nir2 = 8 - 1;
                nir2 = 10 - 1;
                swir = 11 - 1;
            }
            else
            {
                std::cerr << "Albedo can be calculated from 20 and 60 m resolution Sentinel2 images only!" << std::endl;
                exit(1);
            }
        }

        oll::ImageRegionConstIteratorType sii(satelliteImage, satelliteImage->GetLargestPossibleRegion());
        oll::DoubleImageRegionIteratorType aii(albedoRaster, albedoRaster->GetLargestPossibleRegion());

        for (sii.GoToBegin(), aii.GoToBegin(); !sii.IsAtEnd(); ++sii, ++aii)
        {
            aii.Set((0.356 * sii.Get()[blue] + 0.130 * sii.Get()[red] + 0.0373 * sii.Get()[nir1] + 0.085 * sii.Get()[nir2] + 0.072 * sii.Get()[swir] - 0.0018) / 101.6);
        }
    }

    void printConfMat(confMatData& cmd, std::ostream& outStream, bool perClass)
    {
        oll::ConfusionMatrixMeasurementsType::Pointer cmm = oll::ConfusionMatrixMeasurementsType::New();
        cmm->SetMapOfClasses(cmd.mapOfClasses);
        cmm->SetConfusionMatrix(cmd.confMat);
        cmm->Compute();

        if (perClass)
        {
            outStream << "class,kappa" << std::endl;
            double a, b, c, d;
            for (auto& cl : cmm->GetMapOfClasses())
            {
                a = b = c = d = 0;
                a = cmm->GetTruePositiveValues()[cl.second];
                b = cmm->GetFalseNegativeValues()[cl.second];
                c = cmm->GetFalsePositiveValues()[cl.second];
                d = cmm->GetTrueNegativeValues()[cl.second];

                outStream << +cl.first << "," << oll::kappa(a, b, c, d) << std::endl;
            }
            outStream << ",";
        }

        outStream << cmm->GetKappaIndex() << std::endl;
    }

    double kappa(double a, double b, double c, double d)
    {
        double dev, po, ma, mb, pe;

        dev = a + b + c + d;

        po = (a + d) / dev;

        ma = ((a + b) * (a + c)) / dev;
        mb = ((c + d) * (b + d)) / dev;
        pe = (ma + mb) / dev;

        return (po - pe) / (1 - pe);
    }
}
