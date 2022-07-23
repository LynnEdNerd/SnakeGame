#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include "disk.h"
    
// This method will read the high score from Disk::_highScoreFileLocation.  If the file does not exist, it will
// create the file and write "0" to it and return 0 to the caller.  If it does exist, it will read the score that is encoded as a string.  It will 
// convert the string to an integer and return the integer.
int Disk::readHighScore() {
    std::ifstream highScoreFile;
    // Attempt to open the high score file.  If it does not exist, create it and write "0" to it.  Otherwise,
    // read the value from the file and return it to the user.
    highScoreFile.open(*Disk::_highScoreFileLocation);
    if (!highScoreFile) {
        // The high score file does not exist.  Create one, write "0" to it, close the file, and return 0.
        std::ofstream highScoreWriteFile;
        highScoreWriteFile.open(*Disk::_highScoreFileLocation);
        highScoreWriteFile << "0";
        highScoreWriteFile.close();
        return 0;
    }

    std::string highScoreBuff;
    
    // Read the high score, close the file, and return the high score to the host.
    int retValue = 0;
    if (std::getline(highScoreFile, highScoreBuff)) {
        retValue = std::stoi(highScoreBuff);
    }
    highScoreFile.close();
    return (retValue);
}

// This method opens the file at _highScoreFileLocation and writes the highScore to it.
void Disk::writeHighScore ( int highScore ) {
    std::ofstream highScoreFile;

    highScoreFile.open(*Disk::_highScoreFileLocation);
    highScoreFile << highScore;
    highScoreFile.close();
}