/**
 * Name: Jones, Samuel
 * Email: sjon228@lsu.edu
 * Project: PA-2 (RAID)
 * Instructor: Feng Chen
 * Class: cs4103-au21
 * Login ID: cs410348
 * Date: 11/22/2021
 * 
 * @brief Simulates a RAID-5 Array
 * 
 */


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


/**
 * @brief Obtains null padded block of data from input filestream.
 * 
 * @param blockSize Bytes per block
 * @param file Input file stream to pull data from
 * @param block Byte array to deposit data into
 * 
 * @return int Index of the last byte of data written to block array
 */
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


/**
 * @brief Write from file to RAID-5 Array
 * 
 * @param numDisks Number of disks in the array
 * @param blockSize Size of data block in bytes
 * @param inputFilePath Path to file to read from
 */
void writeToDisksFromFile(int numDisks, int blockSize, string inputFilePath) {

    //get disk file streams
    ofstream disks[numDisks];
    for(int i = 0; i < numDisks; i++) {
        disks[i] = ofstream("disk." + to_string(i));
    }
    
    //get input file stream
    ifstream in(inputFilePath);
    
    //keep track of which disk is the parity disk for the current stripe
    int parityDisk = numDisks-1;

    //loop until finished inputting data
    while(!in.eof()) {
        
        //get blocks to write
        vector<char*> blocks;
        vector<int> bytesToWrite;
        for(int i = 0; i < numDisks-1; i++) {
            char* newBlock = (char*) malloc(blockSize + 1);
            int lastDataIndex = getBlock(blockSize, &in, newBlock);
            blocks.push_back(newBlock);
            bytesToWrite.push_back(lastDataIndex+1);
        }

        //calculate parity block
        char* parityBlock = (char*) malloc(blockSize+1);
        for(int i = 0; i < blockSize; i++) {
            char parityByte = 0;
            for(char* block : blocks) {
                parityByte ^= block[i];
            }
            parityBlock[i] = parityByte;
        }
        parityBlock[blockSize] = '\0';

        //write blocks
        int index = 0;
        for(int i = 0; i < numDisks; i++) {
            if(i == parityDisk) {
                disks[i].write(parityBlock, blockSize);
            } else {
                disks[i].write(blocks.at(index), bytesToWrite.at(index));
                index++;
            }
        }

        //free allocated block arrays
        free(parityBlock);
        for(char* block : blocks) {
            free(block);
        }

        //calculate next parity disk
        parityDisk--;
        if(parityDisk == -1) parityDisk = numDisks-1;
    }   

    //close file streams
    for(int i = 0; i < numDisks; i++) {
        disks[i].close();
    }
    in.close();
}


/**
 * @brief Read data from RAID-5 array into file
 * 
 * @param numDisks Number of disks in RAID-5 array
 * @param blockSize Size of data blocks in bytes
 * @param outputFilePath Path to file to write to
 */
void readFromDiskToFile(int numDisks, int blockSize, string outputFilePath) {
    
    //get disk file streams
    ifstream disks[numDisks];
    for(int i = 0; i < numDisks; i++) {
        disks[i] = ifstream("disk." + to_string(i));
    }

    //get output file stream1
    ofstream out(outputFilePath);

    //vector to hold blocks to write to output
    vector<char*> blocks;
    
    //vector to keep track of number of bytes to write to 
    //output file when end of file is encountered so to 
    //not write trailing null characters
    vector<int> bytesToWrite;
    
    //keep track of parity disk for current stripe
    int parityDisk = numDisks - 1;

    //loop until finished getting data blocks to write to file
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

    //write data blocks to file
    int numBlocks = blocks.size();
    for(int i = 0; i < numBlocks; i++) {
        char* block = blocks.at(i);
        out.write(block, bytesToWrite.at(i));
        free(block);
    }

    //close file streams
    for(int i = 0; i < numDisks; i++) {
        disks[i].close();
    }
    out.close();
}


/**
 * @brief Rebuild a disk of the RAID-5 array
 * 
 * @param numDisks Number of disks in the RAID-5 array
 * @param blockSize Size of data block in bytes
 * @param brokenDiskPath The file name of the broken disk file
 * @param brokenDisk The integer value of the file extension of the broken disk file that indicates its index
 */
void rebuildDisk(int numDisks, int blockSize, string brokenDiskPath, int brokenDisk) {
    
    //get input streams for good disks
    ifstream goodDisks[numDisks-1];
    int index = 0;
    for(int i = 0; i < numDisks; i++) {
        if(i == brokenDisk) continue;
        goodDisks[index] = ifstream("disk." + to_string(i));
        index++;
    }

    //get output stream for disk to rebuild
    ofstream broken(brokenDiskPath);

    //vectors to hold blocks to write to bad disk and 
    //to keep track of how many bytes to write from each block
    vector<char*> writeBlocks;
    vector<int> bytesToWrite;

    //loop until all data read from other disks
    while(!goodDisks[0].eof()) {

        //get blocks from good disks
        vector<char*> blocks;
        for(int i = 0; i < numDisks-1; i++) {
            char* block = (char*) malloc(blockSize + 1);
            int lastDataIndex = getBlock(blockSize, &goodDisks[i], block);
            blocks.push_back(block);
        }

        //rebuild missing block (block from bad disk) of current stripe
        char* rebuiltBlock = (char*) malloc(blockSize+1);
        for(int i = 0; i < blockSize; i++) {
            char parityByte = 0;
            for(char* block : blocks) {
                parityByte ^= block[i];
            }
            rebuiltBlock[i] = parityByte;
        }
        rebuiltBlock[blockSize] = '\0';

        //add rebuilt blocks to blocks to write to bad disk
        writeBlocks.push_back(rebuiltBlock);

        //check if at end of file to make sure don't need to only write part of current blocks
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

        //free allocated blocks
        for(char* block : blocks) {
            free(block);
        }
    }

    //write blocks to broken disk file and free
    int numBlocks = writeBlocks.size();
    for(int i = 0; i < numBlocks; i++) {
        char* block = writeBlocks.at(i);
        broken.write(block, bytesToWrite.at(i));
        free(block);
    }
}


/**
 * @brief Pulls data from command line arguments and determines 
 * which operation to run, then calls the respective function 
 * for that operation
 * 
 * @param argc Number of command line arguments, should be 4
 * @param argv Command line arguments, should be of format 
 * [Number of disks, Number of bytes per data block, command, File path]
 * 
 * @return int 0 if successful, -1 if encoutered issue with arguments.
 */
int main(int argc, char** argv) {
    
    if(argc != 5) {
        cerr << "Wrong number of command-line args" << endl;
        return -1;
    }

    //parse command line arguments
    int numDisks = stoi(argv[1]), 
        blockSize = stoi(argv[2]);
    string cmd = argv[3], 
        filePath = argv[4];

    if(!numDisks || !blockSize || cmd.empty() || filePath.empty()) {
        cerr << "Invalid command line arguments" << endl;
        return -1;
    }

    //call corresponding function for given command
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

    //successful exit
    return 0;
}