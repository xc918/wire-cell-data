#include "WireCellData/SignalROI.h"

#include <algorithm>

using namespace WireCell;

SignalROI::SignalROI(WirePlaneType_t plane, int chid, int start_bin, int end_bin, TH1F *h1)
  : plane(plane)
  , chid(chid)
  , start_bin(start_bin)
  , end_bin(end_bin)
{
  float start_content = h1->GetBinContent(start_bin+1);
  float end_content = h1->GetBinContent(end_bin+1);
  contents.resize(end_bin-start_bin+1);

  
  for (int i=start_bin; i<= end_bin; i++){
    float content = h1->GetBinContent(i+1) - ((end_content - start_content)*(i-start_bin)/(end_bin-start_bin) + start_content);
    contents.at(i-start_bin) = content;
  }
}

SignalROI::SignalROI(SignalROI *roi){
  plane = roi->get_plane();
  chid = roi->get_chid();
  start_bin = roi->get_start_bin();
  end_bin = roi->get_end_bin();
  for (int i=start_bin; i<=end_bin;i++){
    contents.push_back(roi->get_contents().at(i-start_bin));
  }
}

bool SignalROI::overlap(SignalROI* roi){

  int min_start_bin = start_bin;
  if (start_bin < roi->get_start_bin())
    min_start_bin = roi->get_start_bin();
  int min_end_bin = end_bin;
  if (end_bin > roi->get_end_bin())
    min_end_bin = roi->get_end_bin();
  if (min_end_bin > min_start_bin){
    return true;
  }else{
    return false;
  }
  // if (start_bin < roi->get_end_bin() && end_bin >= roi->get_end_bin())
  //   return true;
  // if (start_bin <= roi->get_start_bin() && end_bin > roi->get_start_bin())
  //   return true;
  // if (start_bin >= roi->get_start_bin() && end_bin <= roi->get_end_bin())
  //   return true;
  //return false;
}



SignalROI::~SignalROI(){
}



std::vector<std::pair<int,int>> SignalROI::get_above_threshold(float th){
  std::vector<std::pair<int,int>> bins;
  for (int i=0;i<contents.size();i++){
    if (contents.at(i) > th){
      int start = i;
      int end = i;
      for (int j=i+1;j<contents.size();j++){
	if (contents.at(j) > th){
	  end = j;
	}else{
	  break;
	}
      }
      bins.push_back(std::make_pair(start,end));
      i = end;
    }
  }

  return bins;
}
