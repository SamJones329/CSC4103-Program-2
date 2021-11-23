# CSC4103-Program-2
 CSC 4103 Operating Systems Program 2 Assignment: Implement a simulated RAID-5 array
 
 Name: Samuel Jones<br>
 Date: 11/22/2021

 I have provided a sample input text file to easily see and understand the results of the program.
 
 ## Compilation
 This code can be compiled with GCC using the following command in a bash terminal:<br>
 `g++ -o raid raid.cpp`

 The file must be given execute permission in linux with the following command:<br>
 `chmod +x raid`

 Then, the file can be run in the following configuration:<br>
 `raid 4 16 write input.txt`

 As you can see there are four command line arguments, which will be explained in the next section.

 ## Command Line Arguments
 The executable requires 4 command line arguments
 1. Integer >= 3: Number of disks to be simulated in the RAID-5 array
 2. Integer > 0: The block size of the disks in bytes
 3. String (read, write, rebuild): The command for the program to execute
 4. String: A path to a file
 
 ### Command Explanations
 The commands have the following functions
 - Read: Takes input from the file specified in command line argument 4 and stores it in disk files.
 - Write: Takes the information stored in the simulated disk array and writes it to the file path specified in command line argument 4
 - Rebuild: Rebuilds the disk specified by command line argument 4 (where arg4 = disk.i, i in (0..arg1-1)), simulating a lost drive being rebuilt using parity information.