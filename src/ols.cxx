#include "olsl.hxx"
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
    string inTrainingSites, inClassAttribute, inIdAttribute, inSatRaster, outObjStat, outClassStat, outTraingSites, outGroundTruth;

    // CLI options declaration
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")("its", po::value< string >(&inTrainingSites), "Input training sites vector.")(
        "ica", po::value< string >(&inClassAttribute), "Input training sites class attribute.")("iia", po::value< string >(&inIdAttribute),
                                                                                                "Input training sites id attribute.")(
        "isr", po::value< string >(&inSatRaster),
        "Input satellite data raster.")("oos", po::value< string >(&outObjStat),
                                        "Output object statistics.")("ocs", po::value< string >(&outClassStat), "Output class statistics.");
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

    // training sites reading
    GDALDataset* pVDs;
    pVDs = oll::openVectorDs(inTrainingSites.c_str());

    // satellite image reading
    GDALDataset* pRDs;
    pRDs = oll::openRasterDs(inSatRaster.c_str());

    // generate statistics
    oll::TrainingSitesContainer trainingSitesContainer = oll::readData(pVDs, inClassAttribute.c_str(), inIdAttribute.c_str(), pRDs);

    // writing statistics
    oll::writeObjStat(trainingSitesContainer, outObjStat.c_str());

    // write class statistics
    // oll::writeClassStat(trainingSitesContainer, outClassStat.c_str());

    return 0;
}
