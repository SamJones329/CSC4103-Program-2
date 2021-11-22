#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

//returns -1 if whole block is null else returns index of last byte of data
int getBlock(int blockSize, ifstream *file, char* block) {
    for(int i = 0; i < blockSize; i++) block[i] = '\0';
    file->read(block, blockSize);
    block[blockSize] = '\0';
    if(file->eof()) {
        int i;
        for(i = blockSize-1; i >= 0; i--) {
            if(block[i] != '\0') {
                break;
            }
        } 
        return i;
    } else {
        return blockSize-1;
    }
}

void getTrimIndices(vector<char*> blocks, int blockSize, int* trimIndices) {
    for(int i = blocks.size()-1; i >= 0; i--) {
        char* block = blocks.at(i);
        int trimIndex = -1;
        for(int j = blockSize-1; j >= 0; j--) {
            if(block[j] == '\0') trimIndex = j;
            else break;
        }
        trimIndices[i] = trimIndex;
    }
}


void writeToDisksFromFile(int numDisks, int blockSize, string inputFilePath) {

    ofstream disks[numDisks];
    for(int i = 0; i < numDisks; i++) {
        disks[i] = ofstream("disk." + to_string(i));
    }
    
    ifstream in(inputFilePath);
    
    int parityDisk = numDisks-1;
    while(!in.eof()) {
        
        vector<char*> blocks;
        vector<int> bytesToWrite;
        for(int i = 0; i < numDisks-1; i++) {
            char* newBlock = (char*) malloc(blockSize + 1);
            int lastDataIndex = getBlock(blockSize, &in, newBlock);
            blocks.push_back(newBlock);
            bytesToWrite.push_back(lastDataIndex+1);
        }

        char* parityBlock = (char*) malloc(blockSize+1);
        for(int i = 0; i < blockSize; i++) {
            char parityByte = 0;
            for(char* block : blocks) {
                parityByte ^= block[i];
            }
            parityBlock[i] = parityByte;
        }
        parityBlock[blockSize] = '\0';

        int index = 0;
        for(int i = 0; i < numDisks; i++) {
            if(i == parityDisk) {
                disks[i].write(parityBlock, blockSize);
            } else {
                disks[i].write(blocks.at(index), bytesToWrite.at(index));
                index++;
            }
        }

        free(parityBlock);
        for(char* block : blocks) {
            free(block);
        }

        parityDisk--;
        if(parityDisk == -1) parityDisk = numDisks-1;
    }   

    for(int i = 0; i < numDisks; i++) {
        disks[i].close();
    }
    in.close();
}

void readFromDiskToFile(int numDisks, int blockSize, string outputFilePath) {
    
    ifstream disks[numDisks];
    for(int i = 0; i < numDisks; i++) {
        disks[i] = ifstream("disk." + to_string(i));
    }

    ofstream out(outputFilePath);

    vector<char*> blocks;
    vector<int> bytesToWrite;
    int parityDisk = numDisks - 1;
    while(!disks[0].eof()) {
        for(int i = 0; i < numDisks; i++) {
            char* block = (char*) malloc(blockSize + 1);
            int lastDataIndex = getBlock(blockSize, &disks[i], block);
            if(i == parityDisk){
                free(block);
                continue;
            }   
            blocks.push_back(block);
            bytesToWrite.push_back(lastDataIndex+1);
        }

        parityDisk--;
        if(parityDisk == -1) parityDisk = numDisks-1;
    }

    int numBlocks = blocks.size();
    for(int i = 0; i < numBlocks; i++) {
        char* block = blocks.at(i);
        out.write(block, bytesToWrite.at(i));
        free(block);
    }

    for(int i = 0; i < numDisks; i++) {
        disks[i].close();
    }
    out.close();
}

void rebuildDisk(int numDisks, int blockSize, string brokenDiskPath, int brokenDisk) {
    
    ifstream goodDisks[numDisks-1];
    int index = 0;
    for(int i = 0; i < numDisks; i++) {
        if(i == brokenDisk) continue;
        goodDisks[index] = ifstream("disk." + to_string(i));
        index++;
    }

    ofstream broken(brokenDiskPath);

    vector<char*> writeBlocks;
    vector<int> bytesToWrite;
    while(!goodDisks[0].eof()) {

        vector<char*> blocks;
        for(int i = 0; i < numDisks-1; i++) {
            char* block = (char*) malloc(blockSize + 1);
            int lastDataIndex = getBlock(blockSize, &goodDisks[i], block);
            blocks.push_back(block);
        }

        // bool skip = false;
        // if(goodDisks[0].eof()) skip = true;

        // if(!skip) {
        char* rebuiltBlock = (char*) malloc(blockSize+1);
        for(int i = 0; i < blockSize; i++) {
            char parityByte = 0;
            for(char* block : blocks) {
                parityByte ^= block[i];
            }
            rebuiltBlock[i] = parityByte;
        }
        rebuiltBlock[blockSize] = '\0';

        writeBlocks.push_back(rebuiltBlock);

        if(goodDisks[0].eof()) {
            int i;
            for(i = blockSize-1; i >= 0; i--) {
                if(rebuiltBlock[i] != '\0') {
                    break;
                }
            } 
            bytesToWrite.push_back(i+1);
        } else {
            bytesToWrite.push_back(blockSize);
        }

        for(char* block : blocks) {
            free(block);
        }
    }

    int numBlocks = writeBlocks.size();
    for(int i = 0; i < numBlocks; i++) {
        char* block = writeBlocks.at(i);
        broken.write(block, bytesToWrite.at(i));
        free(block);
    }
}


int main(int argc, char** argv) {
    
    if(argc != 5) {
        cerr << "Wrong number of command-line args" << endl;
        return -1;
    }

    int numDisks = stoi(argv[1]), 
        blockSize = stoi(argv[2]);
    string cmd = argv[3], 
        filePath = argv[4];

    if(!numDisks || !blockSize || cmd.empty() || filePath.empty()) {
        cerr << "Invalid command line arguments" << endl;
        return -1;
    }

    if(cmd == "write") {
        writeToDisksFromFile(numDisks, blockSize, filePath);
    } else if(cmd == "read") {
        readFromDiskToFile(numDisks, blockSize, filePath);
    } else if(cmd == "rebuild") {
        int brokenDisk = filePath[filePath.length()-1] - '0';
        if(brokenDisk < 0 || brokenDisk > numDisks-1) {
            cerr << "Invalid disk number/disk file path: " << brokenDisk << endl;
            return -1;
        }
        rebuildDisk(numDisks, blockSize, filePath, brokenDisk);
    } else {
        cerr << "Invalid command (arg3)" << endl;
        return -1;
    }

    return 0;
}