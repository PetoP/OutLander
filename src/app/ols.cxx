#include "../oll/olsl.hxx"
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
    string inTrainingSitesDs, inTrainingSitesLyr, inClassAttribute, inIdAttribute, inSatRaster, outObjStat, outClassStat, outTraingSites,
        outGroundTruth;

    // CLI options declaration
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("itd", po::value< string >(&inTrainingSitesDs), "Input training sites vector data source.")
        ("itl", po::value< string >(&inTrainingSitesLyr), "Input training sites vector layer name.")
        ("ica", po::value< string >(&inClassAttribute), "Input training sites class attribute.")
        ("iia", po::value< string >(&inIdAttribute), "Input training sites id attribute.")
        ("isr", po::value< string >(&inSatRaster), "Input satellite data raster.")
        ("oos", po::value< string >(&outObjStat), "Output object statistics.")
        ("ocs", po::value< string >(&outClassStat), "Output class statistics.");

    po::variables_map vm;

    // CLI options handling
    // po::command_line_parser parser(argc, argv);
    // parser.style(
    //     po::command_line_style::unix_style |
    //     po::command_line_style::allow_long_disguise
    //     );
    // po::parsed_options parsed_options = parser.run();
    // po::store(parsed_options, vm);

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    bool parametersOk = true;

    if (vm.count("help"))
    {
        cout << "Usage: " << argv[0] /*<< usage*/ << endl << endl << desc << endl;
        return 1;
    }

    GDALAllRegister();

    // satellite image reading
    GDALDataset* pRDs;
    pRDs = oll::openRasterDs(inSatRaster.c_str());

    oll::TrainingSitesContainer trainingSitesContainer;
    if (vm.count("itl"))
    {
        // training sites reading
        OGRLayer* pLyr;
        pLyr = oll::openVectorLyr(inTrainingSitesDs.c_str(), inTrainingSitesLyr.c_str());
        // generate statistics
        trainingSitesContainer = oll::readData(pLyr, inClassAttribute.c_str(), inIdAttribute.c_str(), pRDs);
    }
    else
    {
        GDALDataset* pVDs;
        pVDs = oll::openVectorDs(inTrainingSitesDs.c_str());
        trainingSitesContainer = oll::readData(pVDs, inClassAttribute.c_str(), inIdAttribute.c_str(), pRDs);
    }

    // writing statistics
    trainingSitesContainer.writeStat(outObjStat.c_str());

    // compute class statistics
    oll::ClassStatistics classStatistics(trainingSitesContainer);

    // write class statistics
    classStatistics.writeStat(outClassStat.c_str());

    return 0;
}
