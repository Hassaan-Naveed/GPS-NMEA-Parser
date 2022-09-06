#include <cassert>
#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <vector>
#include <map>

#include "earth.h"
#include "parseNMEA.h"

namespace NMEA
{
  bool isSupportedSentenceFormat(std::string format)
  {
      const std::string supportedFormats[3] = {"GLL", "GGA" ,"RMC"};

      // Checks that the input string is present within the supportedFormats array.
      bool isSupported = std::find(std::begin(supportedFormats), std::end(supportedFormats), format) != std::end(supportedFormats);
      return isSupported;
  }

  bool isWellFormedSentence(std::string candidateSentence)
  {
      //Format that the candidateSentence should be
      //"(\\$GP)([A-Z]{3})," Begins with '$GP' followed by 3 uppercase characters between A-Z (inclusive) followed by a ','
      //"[,\\w.]*\\*" Next any number or letter, or '.', or nothing, followed by a '*'
      //"([0-9A-Fa-f]{2})" Finally 2 character between either 0-9, A-F, or a-f (inclusive)
      const std::regex correctFormat ("(\\$GP)([A-Z]{3}),[\\w.,-]*\\*([0-9A-Fa-f]{2})");

      // Compare the candidateSentence to the format, and return a boolean value
      return std::regex_match(candidateSentence, correctFormat);
  }

  std::string splitSentence(std::string sentence, std::string delimiter, bool returnAfter)
  {
    std::string newSentence;

    if (returnAfter) {
      //Substring should be after delimiter
      newSentence = sentence.substr(sentence.find(delimiter) + 1);
    }
    else {
      //Substring should be before delimiter
      newSentence = sentence.substr(0, sentence.find(delimiter));
    }

    return newSentence;
  }

  int hexToDec(std::string hexValue)
  {
    std::stringstream stream;
    int decimalValue;

    stream << std::hex << hexValue;
    stream >> decimalValue;

    return decimalValue;
  }

  bool hasCorrectChecksum(std::string sentence)
  {
    if (isWellFormedSentence(sentence)) {

      const std::string delimiter = "*";
      //Splits the sentence into two strings before and after the '*'
      std::string splitString = splitSentence(sentence, delimiter, false);
      std::string checksum = splitSentence(sentence, delimiter, true);

      int checkXOR = 0;

      //i = 1 to exclude the '$'
      for (int i = 1; i < splitString.length(); i++) {
        assert(i < splitString.length());

        char character = splitString[i];
        //If the character is the first in the sentence, assign it to checkXOR
        if (checkXOR == 0) {
          checkXOR = character;
        }
        //Otherwise XOR it with the current contents of checkXOR
        else {
          checkXOR = checkXOR ^ character;
        }
      }

      //Compare decimal value of actual checksum with decimal value of XOR calculated checksum
      bool isValid = (hexToDec(checksum) == checkXOR);
      return isValid;
    }
    else {
      return false;
    }
  }

  SentenceData parseSentenceData(std::string sentence)
  {
      if (isWellFormedSentence(sentence)) {

        const std::string delimiter = "*";

        //Remove checksum and leading characters
        std::string splitString = splitSentence(sentence, delimiter, false);
        splitString = splitString.substr(3);

        SentenceData dataStructure;
        const std::string dataDelimiter = ",";

        int delimiterPos = 0;
        int count = 0;

        //WHile the position of the next delimiter is not -1
        while ((delimiterPos = splitString.find(dataDelimiter)) != std::string::npos) {
          assert(delimiterPos >= 0);
          assert(delimiterPos <= splitString.length());
          assert(count >= 0);

          //Assign the contents between the current position and the next delimiter to a token
          std::string token = splitString.substr(0, delimiterPos);
          if (count == 0) {
            //If the token is the first, it must be the format
            dataStructure.format = token;
          }
          else {
            dataStructure.dataFields.push_back(token);
          }
          //Remove the data between the current and next delimiter
          splitString.erase(0, delimiterPos + dataDelimiter.length());
          count++;
        }
        //Push the last datafield to the vector, as the while loop exits before the last
        dataStructure.dataFields.push_back(splitString);

        return dataStructure;
      }
  }

  GPS::Position interpretSentenceData(SentenceData data)
  {
      if (!isSupportedSentenceFormat(data.format)) {
        throw std::invalid_argument("Incompatible format, should be 'GGA', 'GLL', or 'RMC'");
      }

      try {
        //Map required indexes to their respective formats
        const std::map<std::string, std::vector<int>> posMap {
          { "GLL", {0, 1, 2, 3} },
          { "GGA", {1, 2, 3, 4, 8} },
          { "RMC", {2, 3, 4, 5} }
        };

        //Get indexes of the current format
        int latIndex = posMap.at(data.format)[0];
        int northIndex = posMap.at(data.format)[1];
        int lonIndex = posMap.at(data.format)[2];
        int eastIndex = posMap.at(data.format)[3];

        //GGA is only format with elevation
        if (data.format != "GGA") {
          return GPS::Position(data.dataFields.at(latIndex), data.dataFields.at(northIndex)[0],
            data.dataFields.at(lonIndex), data.dataFields.at(eastIndex)[0]);
        }
        else {
          int eleIndex = posMap.at(data.format)[4];
          return GPS::Position(data.dataFields.at(latIndex), data.dataFields.at(northIndex)[0],
            data.dataFields.at(lonIndex), data.dataFields.at(eastIndex)[0], data.dataFields.at(eleIndex));
        }
      }
      //If any of the data fields are invalid and cannot be used in GPS::Position()
      catch (const std::exception&) {
        throw std::invalid_argument("Invalid Data");
      }
  }

  std::vector<GPS::Position> positionsFromLog(std::istream & log)
  {
      std::vector<GPS::Position> sentences;

      //While the log has not reached its end
      while (!log.eof()) {

        //Get one line from the log and put it in 'line' variable
        std::string line;
        std::getline(log, line);

        //If the lines checksum is correct and its format is supported
        //Dont need to check for well formed sentence here as this is checked when the line is parsed and interpreted
        if (hasCorrectChecksum(line) && isSupportedSentenceFormat(line.substr(3, line.find(",")-3))) { //Get substring between '$GP' and first ','
          SentenceData parseData = parseSentenceData(line);
          try {
            sentences.push_back(interpretSentenceData(parseData));
          }
          //If interpretSentenceData throws invalid argument, skip current sentence
          catch (std::invalid_argument) {
            continue;
          }
        }
      }
      return sentences;
  }
}
