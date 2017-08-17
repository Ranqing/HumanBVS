#include "../../Qing/qing_common.h"

#include "option.h"
#include "findmatch.h"

int main(int argc, char *argv[]) {

    cout << "usage: " << argv[0] << "\tFRM_0245\tstereo_A01A02.info\n" << endl;
    if (3 != argc) {
        cout << "invalid arguments..." << endl;
        return -1;
    }

    string input_folder = "/Users/Qing/Data/20161224";
    string frame_name = argv[1];
    string file_name = argv[2];
    string option_file = input_folder + "/Humans_stereo/" + frame_name + "/" + file_name;
    cout << "option file: " << option_file << endl;

    COption option(input_folder);
    option.init(option_file);

    CFindMatch findMatch;
    findMatch.init(option);
    findMatch.run();

    return 1;

}