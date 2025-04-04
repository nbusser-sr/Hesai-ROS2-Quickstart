#include "hesai_lidar_sdk.hpp"

#include"lasreader.hpp"
#include"laswriter.hpp"

#include<cfloat>

//#define SAVE_LAS_FILE
#define SAVE_LAZ_FILE             

std::mutex mex_viewer;
uint32_t last_frame_time;
uint32_t cur_frame_time;
//log info, display frame message
void lidarCallback(const LidarDecodedFrame<LidarPointXYZIRT>  &frame) {  
  cur_frame_time = GetMicroTickCount();
  if (cur_frame_time - last_frame_time > kMaxTimeInterval) {
    printf("Time between last frame and cur frame is: %u us\n", (cur_frame_time - last_frame_time));
  }
  last_frame_time = cur_frame_time;
  printf("frame:%d points:%u packet:%u start time:%lf end time:%lf\n",frame.frame_index, frame.points_num, frame.packet_num, frame.points[0].timestamp, frame.points[frame.points_num - 1].timestamp) ;

  mex_viewer.lock();
  LASwriteOpener laswriteropener;
std::string file_name1 = "./PointCloudFrame" + std::to_string(frame.frame_index) + "_" + std::to_string(frame.points[0].timestamp)+ ".las";
std::string file_name2 = "./PointCloudFrame" + std::to_string(frame.frame_index) + "_" + std::to_string(frame.points[0].timestamp)+ ".laz";

#ifdef SAVE_LAS_FILE
const char* las_name = file_name1.c_str();
#endif
#ifdef SAVE_LAZ_FILE
const char* las_name = file_name2.c_str();
  //laswriteropener.set_compress(true);
#endif
#if defined(SAVE_LAS_FILE) || defined(SAVE_LAZ_FILE)
  laswriteropener.set_file_name(las_name);
  std::time_t t = std::time(nullptr);
  std::tm *now = std::localtime(&t);
  uint16_t las_year = now->tm_year + 1900;
  uint16_t las_day = now->tm_mday;

  LASheader header;
  header.point_data_format = 1;
  header.point_data_record_length = 28;
  header.number_of_point_records = frame.points_num;
  header.file_creation_day = las_day;
  header.file_creation_year = las_year;
  strcpy(header.generating_software, "Hesai SDK");

  header.x_scale_factor = 0.00001;
  header.y_scale_factor = 0.00001;
  header.z_scale_factor = 0.00001;

  LASwriter* laswriter = laswriteropener.open(&header);

  LASpoint point;
  point.init(&header,header.point_data_format,header.point_data_record_length,&header);

  double max_x = -DBL_MAX, max_y = -DBL_MAX, max_z = -DBL_MAX;
  double min_x = DBL_MAX, min_y = DBL_MAX, min_z = DBL_MAX;


  for(uint32_t i=0; i < frame.points_num; i++){
    double las_x = frame.points[i].x * 10000;
    double las_y = frame.points[i].y * 10000;
    double las_z = frame.points[i].z * 10000;
    uint16_t las_intensity = frame.points[i].intensity;
    double las_time = frame.points[i].timestamp;
    uint16_t las_ring = frame.points[i].ring;

    point.set_x(las_x);
    point.set_y(las_y);
    point.set_z(las_z);
    point.set_intensity(las_intensity);
    point.set_point_source_ID(las_ring);

    if(las_x > max_x){
      max_x = las_x;
    }
    if(las_x < min_x){
      min_x = las_x;
    }
    if(las_y > max_y){
      max_y = las_y;
    }
    if(las_y < min_y){
      min_y = las_y;
    }
    if(las_z > max_z){
      max_z = las_z;
    }
    if(las_z < min_z){
      min_z = las_z;
    }

    point.set_gps_time(las_time);

    laswriter->write_point(&point);
    laswriter->update_inventory(&point);
  }
  header.set_bounding_box(min_x, min_y, min_z, max_x, max_y, max_z);
  laswriter->update_header(&header, TRUE);

  laswriter->close();
  delete laswriter;
#endif
  mex_viewer.unlock();
}

int main(int argc, char *argv[])
{
  HesaiLidarSdk<LidarPointXYZIRT> sample;
  DriverParam param;
  // assign param
  param.input_param.source_type = DATA_FROM_LIDAR;
  param.input_param.pcap_path = {"Your pcap file path"};
  param.input_param.correction_file_path = {"Your correction file path"};
  param.input_param.firetimes_path = {"Your firetime file path"};
  param.input_param.ptc_mode = PtcMode::tcp;
  param.input_param.certFile = "Your cert file";
  param.input_param.privateKeyFile = "Your privateKey file";
  param.input_param.caFile = "Your ca file";
  param.input_param.device_ip_address = "198.18.35.6";
  param.input_param.ptc_port = 9347;
  param.input_param.udp_port = 2368;
  param.input_param.host_ip_address = "";
  param.input_param.multicast_ip_address = "";
  // param.decoder_param.enable_packet_loss_tool = true;
  //init lidar with param
  sample.Init(param);

  //assign callback fuction
  sample.RegRecvCallback(lidarCallback);

  //star process thread
  last_frame_time = GetMicroTickCount();
  sample.Start();

  while (1)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
}