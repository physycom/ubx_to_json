
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include "jsoncons/json.hpp"


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

class GNSSdata
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
  GNSSdata();
  bool readData(std::FILE * );
  int32_t lon;
  int32_t lat;
  int32_t alt;
  int32_t speed;
  uint32_t heading;
  int nano;
  struct tm gps_time;
};


GNSSdata::GNSSdata() {
  align_A = (unsigned char)0xB5;  // mu
  align_B = (unsigned char)0x62;  // b
  ubx_navpvt_class = 0x01;
  ubx_navpvt_id = 0x07;
}


bool GNSSdata::readData(std::FILE *inputfile)
{
  unsigned char buffer;
  bool found = false;
  char * payload;
  size_t fread_size = 0;

  while (!found) {
  	fread_size = std::fread(&buffer, sizeof(unsigned char), 1, inputfile);
    if (fread_size < sizeof(unsigned char)) break;

    if (buffer == align_A) {
  	fread_size = std::fread(&buffer, sizeof(unsigned char), 1, inputfile);
    if (fread_size < sizeof(unsigned char)) break;

      if (buffer == align_B) {
        fread_size = std::fread(&ubx_class, sizeof(unsigned char), 1, inputfile);
        if (fread_size < sizeof(unsigned char)) {printf("ops1"); break;}
        printf("ubx_class : %02x ", ubx_class);

        fread_size = std::fread(&ubx_id, sizeof(unsigned char), 1, inputfile);
        if (fread_size < sizeof(unsigned char)) {printf("ops2"); break;}
        printf("ubx_id : %02x\r", ubx_id);

          if (ubx_class == ubx_navpvt_class && ubx_id == ubx_navpvt_id) {
            fread_size = std::fread(&ubx_length, sizeof(int16_t), 1, inputfile);
            if (fread_size < sizeof(int16_t)) {printf("ops3"); break;}

            if (ubx_length > 0){
              payload = new char[ubx_length];
              fread_size = std::fread(payload, sizeof(char), ubx_length, inputfile);
              if (fread_size < sizeof(char)*ubx_length) {printf("ops4"); break;}

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

          fread_size = std::fread(&ubx_chk_A, sizeof(unsigned char), 1, inputfile);
          if (fread_size < sizeof(unsigned char)) {printf("ops5"); break;}
          fread_size = std::fread(&ubx_chk_B, sizeof(unsigned char), 1, inputfile);
          if (fread_size < sizeof(unsigned char)) {printf("ops6"); break;}

          found = true;
        }
      }
    }
  }
  return found;
}



int main(int argc, char** argv)
{
  std::FILE *input_file;

  std::ofstream output_file;
  std::stringstream ss;
  std::string record_name;
  int gps_record_counter = 0;
  GNSSdata dato;
  jsoncons::json outjson;

  std::cout << "ubx_to_json v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
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
  input_file = fopen(input_name.c_str(), "rb");
  if ( input_file == NULL ) {
    std::cout << "Unable to open input file." << std::endl;
    exit(-1);
  }

  while (dato.readData(input_file))
  {
  	std::cout << gps_record_counter << std::endl;
    gps_record_counter++;
    jsoncons::json ijson;
    ijson["lat"] = dato.lat;
    ijson["lon"] = dato.lon;
    ss << std::setfill('0') << std::setw(7) << gps_record_counter;
    record_name = "gps_record_" + ss.str();
    outjson[record_name] = ijson;
  }

  if (gps_record_counter) {
    std::cout << "found " << gps_record_counter << " records" << std::endl;
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    if (!output_file.is_open()) {
      std::cout << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cout << "Press q to quit, any other key to have a fallback output on stdout." << std::endl;
      char q;
      std::cin >> q;
      if (q == 'q') exit(333);
      else std::cout << jsoncons::pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << jsoncons::pretty_print(outjson) << std::endl;
    output_file.close();
  }
  else std::cout << "No valid UBX data with \"ubx_navpvt_class = 0x01\" and \"ubx_navpvt_id = 0x07\" found" << std::endl;

  fclose(input_file);

  return 0;
}

