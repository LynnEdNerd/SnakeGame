#ifndef DISK_H
#define DISK_H
#include <string>

class Disk {
  public:
    // Constructor
    Disk(std::string highScoreFileLocation) {
      _highScoreFileLocation = new std::string(highScoreFileLocation);
    }

    // Destructor
    ~Disk() { 
      delete _highScoreFileLocation;
    }

    // Copy constructor
    Disk(const Disk &source) {
     _highScoreFileLocation = new std::string(*source._highScoreFileLocation);
    }

    // Move constructor
    Disk(Disk &&source) {
      delete _highScoreFileLocation;
      _highScoreFileLocation = source._highScoreFileLocation;
      source._highScoreFileLocation = nullptr;
    }

    // Copy assignment operator
    Disk &operator=(const Disk &source) {
        if (this == &source)
          return *this;
        delete _highScoreFileLocation;

        _highScoreFileLocation = new std::string(*source._highScoreFileLocation);
    }

    // Move assignment operator
    Disk &operator=(Disk &&source) {
      if (this == &source)
        return *this;
      delete _highScoreFileLocation;
      _highScoreFileLocation = source._highScoreFileLocation;
    }

    int readHighScore();
    void writeHighScore(int newHighScore);
    std::string GetHighScoreFileLocation() {return *_highScoreFileLocation;}
  private:
    std::string *_highScoreFileLocation;
};
#endif
