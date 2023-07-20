#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>

namespace fs = std::filesystem;
using std::string;
using std::cout;
using std::endl;

struct keysList
{
    string keyB = "-b"; // For output size in bytes
    string keyA = "-a"; // For output size for every file
    string keyC = "-c"; // For output total size
    string keyS = "-s"; // For output only total size
};

long int sum_size(const fs::path& p, fs::file_status s,
                  keysList &key, string &inputKey, int sizeBlock=512)
{
    if (fs::is_directory(s))
    {
        long int sumSize = 0;

        // If folder is empty and '-b' key is NOT entered, output size in blocks
        if (fs::is_empty(p) && inputKey != key.keyB) sumSize = 1;

        for (const auto &entry : fs::directory_iterator(p))
        {
            try
            {
                sumSize += sum_size(entry.path(), fs::status(entry.path()),
                                    key, inputKey);
            }
            catch (const fs::filesystem_error& ex)
            {
                std::cerr << "ERROR! " << ex.what() << std::endl;
            }
        }

        // If '-s' key entered, output only total size
        if (inputKey != key.keyS) cout << sumSize << "\t" << string(p) << endl;
        return sumSize;
    }

    else
    {
        std::fstream file(p);
        long int sizeInBlocks = 0;

        file.seekg(0, std::ios::end);
        long int size = file.tellg();
        file.close();

        // If '-b' key entered, return size in bytes
        if (inputKey == key.keyB) return size;

        if (size % sizeBlock != 0) sizeInBlocks = size / sizeBlock + 1;
        else sizeInBlocks = size / sizeBlock;

        // Min size in blocks = 1
        if (sizeInBlocks == 0) sizeInBlocks = 1;

        // If '-a' key entered output size for every file
        if (inputKey == key.keyA)
            cout << sizeInBlocks << "\t" << string(p) << endl;

        return sizeInBlocks;
    }
}


int main(int argc, char *argv[])
{
    keysList keys;

    string inputKey = "";
    string fileName = "";
    int numPath = 0;
    int minNumOfArg = 2; // Minimum number of arguments
    char keyFile[] = "--files";
    std::vector<string> listOfPathes;

    inputKey += *argv[1]; inputKey += *(argv[1]+1);

    if (inputKey != keys.keyB && inputKey != keys.keyA &&
        inputKey != keys.keyC && inputKey != keys.keyS)
        numPath = 1;
    else numPath = minNumOfArg;

    if (argc > minNumOfArg &&
        (strncmp(argv[1], keyFile, 6) == 0 || strncmp(argv[2], keyFile, 6) == 0))
    {
        fileName = argv[numPath+1];
        fileName.erase(0, 5); // Delete 'from='

        std::ifstream source(fileName);
        std::string linePathString;
        while (std::getline(source, linePathString))
            listOfPathes.push_back(linePathString.data());
        source.close();

        argc = listOfPathes.size();
        for (int i = 0; i < argc; i++)
            argv[i] = const_cast<char*>(listOfPathes[i].c_str());
        numPath = 0;
    }

    for (; numPath < argc; numPath++) {
        if (inputKey == keys.keyC)
            cout << sum_size(argv[numPath], fs::status(argv[numPath]), keys, inputKey)
                 << "\ttotal" << endl;
        else if (inputKey == keys.keyS)
            cout << sum_size(argv[numPath], fs::status(argv[numPath]), keys, inputKey)
                 << "\t" << string(argv[numPath]) << endl;
        else sum_size(argv[numPath], fs::status(argv[numPath]), keys, inputKey);
    }

    return 0;
}
