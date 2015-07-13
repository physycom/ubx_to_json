#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include "jsoncons/json.hpp"

using namespace jsoncons;

#define MAJOR_VERSION          0
#define MINOR_VERSION          1

#define UBX_YEAR_OFFSET    4
#define UBX_MONTH_OFFSET   6
#define UBX_DAY_OFFSET     7
#define UBX_HOUR_OFFSET    8
#define UBX_MIN_OFFSET     9
#define UBX_SEC_OFFSET    10
#define UBX_NANO_OFFSET   16
#define UBX_FIX_OFFSET    20
#define UBX_LON_OFFSET    24
#define UBX_LAT_OFFSET    28
#define UBX_ALT_OFFSET    36
#define UBX_SPEED_OFFSET  60
#define UBX_HEAD_OFFSET   60

/***********************************************************************
 * ((Read_UBX)) Reads lat-lon-alt data from UBX binary file            *
 ***********************************************************************/

class GPSData
{
private:
  unsigned char align_A;
  unsigned char align_B;
  unsigned char ubx_class;
  unsigned char ubx_id;
  int16_t ubx_length;
  std::vector<char> payload;
  unsigned char ubx_chk_A;
  unsigned char ubx_chk_B;
  unsigned char ubx_navpvt_class;
  unsigned char ubx_navpvt_id;
  unsigned char fix;
public:
  GPSData();
  void readData(std::ifstream& inputfile);
  int32_t lon;
  int32_t lat;
  int32_t alt;
  int32_t speed;
  uint32_t heading;
  int nano;
  struct tm gps_time;
};


GPSData::GPSData() {
  align_A = (unsigned char)0xB5;  // mu
  align_B = (unsigned char)0x62;  // b
  ubx_navpvt_class = 0x01;
  ubx_navpvt_id = 0x07;
}


void GPSData::readData(std::ifstream& inputfile)
{
  unsigned char buffer;
  bool found = false;
  char * payload;
//  std::cout << "ciao" << std::endl;

  while (!found) {
    inputfile.read((char*)&buffer, sizeof(buffer));
    printf("%02x ", buffer);
    if (buffer == align_A) {
      if (inputfile.read((char*)&buffer, sizeof(buffer)).gcount() < sizeof(buffer)) break;
      if (buffer == align_B) {
        std::cout << "Trovato messaggio " << align_A << align_B << std::endl;
        if (inputfile.read((char*)&ubx_class, sizeof(ubx_class)).gcount() < sizeof(ubx_class)) break;
        if (inputfile.read((char*)&ubx_id, sizeof(ubx_id)).gcount() < sizeof(ubx_id)) break;
        if (inputfile.read((char*)&ubx_length, sizeof(ubx_length)).gcount() < sizeof(ubx_length)) break;

        if (ubx_length > 0){
          payload = new char[ubx_length];
          if (inputfile.read(payload, ubx_length*sizeof(char)).gcount() < ubx_length*sizeof(char)) break;

          if (ubx_class == ubx_navpvt_class && ubx_id == ubx_navpvt_id){
            unsigned char uc_temp; unsigned short us_temp;

            memcpy((void *)&us_temp, &payload[UBX_YEAR_OFFSET], sizeof(us_temp));
            gps_time.tm_year = ((int)us_temp) - 1900;
            memcpy((void *)&uc_temp, &payload[UBX_MONTH_OFFSET], sizeof(uc_temp));
            gps_time.tm_mon = (int)uc_temp;
            memcpy((void *)&uc_temp, &payload[UBX_DAY_OFFSET], sizeof(uc_temp));
            gps_time.tm_mday = (int)uc_temp;
            memcpy((void *)&uc_temp, &payload[UBX_HOUR_OFFSET], sizeof(uc_temp));
            gps_time.tm_hour = (int)uc_temp;
            memcpy((void *)&uc_temp, &payload[UBX_MIN_OFFSET], sizeof(uc_temp));
            gps_time.tm_min = (int)uc_temp;
            memcpy((void *)&uc_temp, &payload[UBX_SEC_OFFSET], sizeof(uc_temp));
            gps_time.tm_sec = (int)uc_temp;
            memcpy((void *)&nano, &payload[UBX_NANO_OFFSET], sizeof(nano));
            memcpy((void *)&fix, &payload[UBX_FIX_OFFSET], sizeof(fix));
            memcpy((void *)&lon, &payload[UBX_LON_OFFSET], sizeof(lon));
            memcpy((void *)&lat, &payload[UBX_LAT_OFFSET], sizeof(lat));
            memcpy((void *)&alt, &payload[UBX_ALT_OFFSET], sizeof(alt));
            memcpy((void *)&speed, &payload[UBX_SPEED_OFFSET], sizeof(speed));
            memcpy((void *)&heading, &payload[UBX_HEAD_OFFSET], sizeof(heading));
          }

          delete[] payload;
        }

        if (inputfile.read((char*)&ubx_chk_A, sizeof(ubx_chk_A)).gcount() < sizeof(ubx_chk_A)) break;
        if (inputfile.read((char*)&ubx_chk_B, sizeof(ubx_chk_B)).gcount() < sizeof(ubx_chk_B)) break;

        found = true;
        printf("%02x - %02x:%02x - %02x:%02x - Len: %hi - %02x:%02x\n", found, align_A, align_B, ubx_class, ubx_id, ubx_length, ubx_chk_A, ubx_chk_B);
      }
//      else printf("align_B not valid: %02x:%02x\n", align_A, align_B);
    }
//    else printf("align_A not valid : % 02x : % 02x\n", align_A, align_B);
  }
}



int main(int argc, char** argv)
{
  std::ifstream input_file;
  std::ofstream output_file;
  std::stringstream ss;
  std::string record_name;
  int gps_record_counter = 0;
  GPSData dato;
  json outjson;

  std::cout << "Nmea_to_UBX v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
  std::cout << "Usage: " << argv[0] << " -i [input] -o [output.json]" << std::endl;
  std::cout << "\t- [input] UBX binary file to parse" << std::endl;
  std::cout << "\t- [output.json] json location to store parsed file" << std::endl;

  // Parsing command line
  std::string input_name, output_name;
  if (argc > 2){ /* Parse arguments, if there are arguments supplied */
    for (int i = 1; i < argc; i++){
      if ((argv[i][0] == '-') || (argv[i][0] == '/')){       // switches or options...
        switch (tolower(argv[i][1])){
        case 'i':
          input_name = argv[++i];
          break;
        case 'o':
          output_name = argv[++i];
          break;
        default:    // no match...
          std::cout << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
          exit(1);
        }
      }
      else {
        std::cout << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
        exit(11);
      }
    }
  }
  else {
    std::cout << "No flags specified. Read usage and relaunch properly." << std::endl;
    exit(111);
  }

  // Safety checks for file manipulations
  std::cout << "opening : " << input_name << std::endl;
  input_file.open(input_name.c_str() , std::ios_base::in | std::ios_base::binary);

  std::cout << "is open: " << input_file.is_open() << std::endl;
  std::cout << "eof : " << input_file.eof() << std::endl;
  std::cout << "good : " << input_file.good() << std::endl;
  std::cout << "fail : " << input_file.fail() << std::endl;
  std::cout << "bad : " << input_file.bad() << std::endl;

  input_file.clear( input_file.goodbit );

  std::cout << "is open: " << input_file.is_open() << std::endl;
  std::cout << "eof : " << input_file.eof() << std::endl;
  std::cout << "good : " << input_file.good() << std::endl;
  std::cout << "fail : " << input_file.fail() << std::endl;
  std::cout << "bad : " << input_file.bad() << std::endl;

  while (!input_file.eof()) {
//  while (input_file.good()) {

//    std::cout << "ciao2" << std::endl;

    dato.readData(input_file);
    gps_record_counter++;
    json ijson;
    ijson["lat"] = dato.lat;
    ijson["lon"] = dato.lon;
    ss << std::setfill('0') << std::setw(7) << gps_record_counter;
    record_name = "gps_record_" + ss.str();
//    outjson[record_name] = ijson;
  }

/*
  if (gps_record_counter) {
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    if (!output_file.is_open()) {
      std::cout << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cout << "Press q to quit, any other key to have a fallback output on stdout." << std::endl;
      char q;
      std::cin.get(&q,1);
      if (q == 'q') exit(333);
      else std::cout << pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << pretty_print(outjson) << std::endl;
    output_file.close();
  }
  */

  if( gps_record_counter ) std::cout << "record : " << gps_record_counter << std::endl;
  else std::cout << "No valid UBX data with \"ubx_navpvt_class = 0x01\" and \"ubx_navpvt_id = 0x07\" found" << std::endl;

  input_file.close();

  return 0;
}

