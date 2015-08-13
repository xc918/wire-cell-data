#ifndef WCVertex_h
#define WCVertex_h

#include "WireCellData/Point.h"
#include "WireCellData/MergeSpaceCell.h"
//#include "WireCellData/WCTrack.h"

#include <vector>
#include <map>

namespace WireCell {
  class WCVertex {
    
  public:
    Point Center();
    WCVertex(MergeSpaceCell& msc);
    ~WCVertex();
  protected:
    Point center;
    MergeSpaceCell& msc;
  };
  
  typedef std::vector<WCVertex*> WCVertexSelection;
  typedef std::map<MergeSpaceCell*, WCVertex*> MSC_WCV_Map;
  //typedef std::map<WCTrack*, WCVertexSelection> WCT_WCVs_Map;
}

#endif