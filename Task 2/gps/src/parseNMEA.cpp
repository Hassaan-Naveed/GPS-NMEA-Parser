#include "earth.h"
#include "parseNMEA.h"

namespace NMEA
{

// Determine whether the parameter is the three-character code for a sentence format that is currently supported.
// Supported formats are GLL, GGA + RMC
  bool isSupportedSentenceFormat(std::string format)
  {
      return format == "GLL" || format == "GGA" || format == "RMC";
  }


// Determine whether the parameter is a well-formed NMEA sentence using sentenceFormat (see header)
  bool isWellFormedSentence(std::string candidateSentence)
  {
      return std::regex_match(candidateSentence, sentenceFormatting);
  }


 // Determine whether a sentence has a correct checksum.
  bool hasCorrectChecksum(std::string sentence)
  {
      assert(sentence.length() >= 2);

      // converts string hex checksum to decimal int
      int checksum = stoi(sentence.substr(sentence.length()-2, 2), 0, 16);

      int totalXOR = sentence[1];
      for(int i = 2; i < int(sentence.length()); i++){
          if(sentence[i] != '*'){
              totalXOR = totalXOR ^ sentence[i];
          } else {
              break;
          }
      }
      return totalXOR == checksum;
  }


// Seperates NMEA Sentence data SentenceData struct
  SentenceData parseSentenceData(std::string sentence)
  {   
      assert(sentence.length() >= 7);

      struct SentenceData parsedSentence = {};
      parsedSentence.format = sentence.substr(3, 3);

      std::string currentField = "";
      // Start from i = 7 to ignore the first ','
      for(int i = 7; i < int(sentence.length()); i++){
          if(sentence[i] == ','){
              parsedSentence.dataFields.push_back(currentField);
              currentField.clear();
          } else if(sentence[i] == '*') {
              parsedSentence.dataFields.push_back(currentField);
              break;
          } else {
              currentField.push_back(sentence[i]);
          }
      }
      return parsedSentence;
  }


  // Interprets the sentence data using the position() function/format
  GPS::Position interpretSentenceData(SentenceData data)
  {
      assert(!data.format.empty());

      if(!isSupportedSentenceFormat(data.format)) throw std::invalid_argument("Unsupported Sentence Format");

      try {
          // using dataIndexByFormat (see header) map for for indexes based on data format
          std::vector<int> dataIndexes = dataIndexByFormat.at(data.format);

          std::string latitude = data.dataFields.at(dataIndexes.at(0));
          char latitudeDir = data.dataFields.at(dataIndexes.at(1))[0];

          std::string longitude = data.dataFields.at(dataIndexes.at(2));
          char longitudeDir = data.dataFields.at(dataIndexes.at(3))[0];

          // if theres a 5th value for format, it has elevation to include (allows new formats to be added easily)
          if(dataIndexes.size() == 5){
              std::string elevation = data.dataFields.at(dataIndexes.at(4));
              return GPS::Position(latitude, latitudeDir, longitude, longitudeDir, elevation);
          } else {
              return GPS::Position(latitude, latitudeDir, longitude, longitudeDir);
          }

      } catch (const std::exception&) {
          throw std::invalid_argument("Data Fields Incorrect");
      }
  }


// Takes in a stream of NMEA sentences and returns a vector containing the positions of
// of only the valid sentences
  std::vector<GPS::Position> positionsFromLog(std::istream & log)
  {
      // check log is not empty / starts with null character \0
      assert(log.peek() != '\0');

      std::vector<GPS::Position> positions = {};

      while(!log.eof()){
          std::string sentenceLine = "";
          std::getline(log, sentenceLine);

          // checking whether sentenceLine is valid, and adding it to positions if it is
          if(isWellFormedSentence(sentenceLine) && hasCorrectChecksum(sentenceLine)){
              SentenceData formattedData = parseSentenceData(sentenceLine);
              try {
                  positions.push_back(interpretSentenceData(formattedData));
              } catch (const std::invalid_argument&) {
                  continue;
              }   
          }
      }
      return positions;
  }
}
