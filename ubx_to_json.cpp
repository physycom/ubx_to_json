
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include "jsoncons/json.hpp"

#define my_printf(...)

#define MAJOR_VERSION          0
#define MINOR_VERSION          5

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
        if (fread_size < sizeof(unsigned char)) {my_printf("ops1\n"); break;}
        my_printf("ubx_class : %02x\n", ubx_class);

        fread_size = std::fread(&ubx_id, sizeof(unsigned char), 1, inputfile);
        if (fread_size < sizeof(unsigned char)) {my_printf("ops2\n"); break;}
        my_printf("ubx_id : %02x\n", ubx_id);

          if (ubx_class == ubx_navpvt_class && ubx_id == ubx_navpvt_id) {
            fread_size = std::fread(&ubx_length, sizeof(int16_t), 1, inputfile);
            if (fread_size < sizeof(int16_t)) {my_printf("ops3\n"); break;}

            if (ubx_length > 0){
              payload = new char[ubx_length];
              fread_size = std::fread(payload, sizeof(char), ubx_length, inputfile);
              if (fread_size < sizeof(char)*ubx_length) {my_printf("ops4\n"); break;}

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
              delete[] payload;
            }


          fread_size = std::fread(&ubx_chk_A, sizeof(unsigned char), 1, inputfile);
          if (fread_size < sizeof(unsigned char)) {my_printf("ops5\n"); break;}
          fread_size = std::fread(&ubx_chk_B, sizeof(unsigned char), 1, inputfile);
          if (fread_size < sizeof(unsigned char)) {my_printf("ops6\n"); break;}

          found = true;
        }
      }
    }
  }
  return found;
}


void usage(char* progname) {
  std::cerr << "Usage: " << progname << " -i [input] -o [output.json]" << std::endl;
  std::cerr << "\t- [input] UBX binary file to parse" << std::endl;
  std::cerr << "\t- [output.json] json location to store parsed file" << std::endl;
}


int main(int argc, char** argv)
{
  std::cout << "ubx_to_json v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
  std::FILE *input_file;

  std::ofstream output_file;
  std::stringstream ss;
  std::string record_name;
  int gps_record_counter = 0;
  GNSSdata dato;
  jsoncons::json outjson;

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
          std::cerr << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
          usage(argv[0]);
          exit(1);
        }
      }
      else {
        std::cerr << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
        usage(argv[0]);
        exit(11);
      }
    }
  }
  else {
    usage(argv[0]);
    exit(111);
  }

  // Safety checks for file manipulations
  std::cout << "opening : " << input_name << std::endl;
  input_file = fopen(input_name.c_str(), "rb");
  if ( input_file == NULL ) {
    std::cerr << "Unable to open input file " << input_file << std::endl;
    exit(-1);
  }

  while (dato.readData(input_file))
  {
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
      std::cerr << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cerr << "Output redirected to stdout." << std::endl;
      std::cout << jsoncons::pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << jsoncons::pretty_print(outjson) << std::endl;
    output_file.close();
  }
  else std::cout << "No valid UBX data with \"ubx_navpvt = (0x01, 0x07)\" (class, id) found" << std::endl;

  fclose(input_file);

  return 0;
}

