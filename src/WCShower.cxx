#include "WireCellData/WCShower.h"
#include "TVector3.h"

using namespace WireCell;

WCShower::WCShower(WCVertex *vertex, WCTrack *track, MergeSpaceCellSelection& exclude_cells,MergeSpaceCellMap& mcells_map)
  : vertex(vertex)
  , track(track)
  , exclude_mcells(exclude_cells)
  , mcells_map(mcells_map)
{
  // constructor to construct things that only contain things connected to it ... 
  MergeSpaceCell *start_cell = vertex->get_msc();
  MergeSpaceCellSelection curr_cells = mcells_map[start_cell];
  Iterate(start_cell, curr_cells);
  SC_Hough(vertex->Center());
  //  std::cout << theta_hough << " " << phi_hough << std::endl;
  SC_proj_Hough(vertex->Center());
  //std::cout << ncell_out_range << " " << ncell_angle_out_range << " " << ncell_total << " " << nmcell_out_range << " " << nmcell_total << std::endl;
}

bool WCShower::Contain(WCTrack *track){
  bool result = false;
  int not_common = 0;
  for (int i=0;i!=track->get_centerVP_cells().size();i++){
    MergeSpaceCell *mcell = track->get_centerVP_cells().at(i);
    auto it  = find(all_mcells.begin(),all_mcells.end(),mcell);
    if (it == all_mcells.end())
      not_common ++;
  }
  
  if (not_common == 0 && track->get_centerVP_cells().size()>0)
    result = true;
  
  return result;
}

bool WCShower::IsContained(WCShower *shower){
  int nsame = 0;
  for (int i=0;i!=all_mcells.size();i++){
    MergeSpaceCell *mcell = all_mcells.at(i);
    auto it = find(shower->get_all_cells().begin(),shower->get_all_cells().end(),mcell);
    if (it != shower->get_all_cells().end())
      nsame ++;
  }
  if (nsame == all_mcells.size())
    return true;

  return false;
}


void WCShower::SC_proj_Hough(Point p){
  TVector3 l1_dir(sin(theta_hough)*cos(phi_hough),sin(theta_hough)*sin(phi_hough),cos(theta_hough));
  TVector3 dir_x(1,0,0);
  TVector3 l1_proj = dir_x.Cross(l1_dir);

  ncell_total = 0;
  ncell_out_range = 0;
  ncell_angle_out_range = 0;
  nmcell_total = 0;
  nmcell_out_range = 0;
  
  float rms = 0;

  for (int i=0;i!=all_mcells.size();i++){
    MergeSpaceCell *mcell = all_mcells.at(i);
    ncell_total += mcell->Get_all_spacecell().size();
    nmcell_total ++;

    TVector3 v1(mcell->Get_Center().x-p.x,mcell->Get_Center().y-p.y,mcell->Get_Center().z-p.z);
    
    if (v1.Dot(l1_dir) <0){
      ncell_out_range += mcell->Get_all_spacecell().size();
      nmcell_out_range ++;
    }else{
    // calculated projected distance, divided by the distance, would be the angle ...
        TVector3 v2 = v1 - l1_dir;
	
	TVector3 dist_dir = v1.Cross(v2).Cross(l1_dir);
	dist_dir *= 1./l1_dir.Mag2();
	
	TVector3 dist_proj;
	
	if (l1_proj.Mag2()!=0){
	  dist_proj = dist_dir - dist_dir.Dot(l1_proj)/l1_proj.Mag2() * l1_proj;
	}else{
	  dist_proj = dist_dir;
	}
	float dist = dist_proj.Mag();
	
	float dist1 = v1.Mag();
	
	float theta;
	if (dist1 !=0){
	  theta = asin(dist/dist1);
	}else{
	  theta = 0;
	}
	if (theta > 15./180.*3.1415926)
	  ncell_angle_out_range ++;
	
	rms += theta*theta * mcell->Get_all_spacecell().size();
	//    std::cout << theta << std::endl;
    }
  }

  rms = sqrt(rms/(ncell_total - ncell_out_range));
  //std::cout << rms << std::endl;

  

}


void WCShower::SC_Hough(Point p){
  TH2F *hough = new TH2F("","",180,0.,3.1415926,360,-3.1415926,3.1415926);
  double x0 = p.x;
  double y0 = p.y;
  double z0 = p.z;
  double x,y,z,q;
  for (int i=0;i!=all_mcells.size();i++){
    MergeSpaceCell *mcell = all_mcells.at(i);
    for (int j=0;j!=mcell->Get_all_spacecell().size();j++){
      SpaceCell *cell = mcell->Get_all_spacecell().at(j);
      x = cell->x();
      y = cell->y();
      z = cell->z();
      q = 1;
      TVector3 vec(x-x0,y-y0,z-z0);
      hough->Fill(vec.Theta(),vec.Phi(),q);
    }
  }
  int maxbin = hough->GetMaximumBin();
  int a,b,c;
  hough->GetBinXYZ(maxbin,a,b,c);
  theta_hough =  hough->GetXaxis()->GetBinCenter(a);
  phi_hough = hough->GetYaxis()->GetBinCenter(b);
  delete hough;
}


bool WCShower::IsShower(){
  bool result = true;
  int ncell_track = track->get_centerVP_cells().size();
  //std::cout << ncell_track << " " << all_mcells.size() << std::endl;
  if (ncell_track > 0.9 * all_mcells.size() && ncell_track > all_mcells.size() - 8)
    return false;
  if (nmcell_out_range > nmcell_total*0.05)
    return false;
  if (ncell_angle_out_range > 0.01 * ncell_total)
    return false;

  return result;
}

void WCShower::Iterate(MergeSpaceCell *curr_cell, MergeSpaceCellSelection &curr_cells){
  int flag = 0;
  auto it1 = find(all_mcells.begin(),all_mcells.end(),curr_cell);
    
  if (it1 == all_mcells.end()){
    // Not contained, do something
    auto it3 = find(track->get_centerVP_cells().begin(), track->get_centerVP_cells().end(), curr_cell);
    if (it3 != track->get_centerVP_cells().end()){
      flag = 1;
    }else{
      auto it2 = find(exclude_mcells.begin(),exclude_mcells.end(),curr_cell);
      if (it2 == exclude_mcells.end())
	flag = 1;
    }
  }
  
  if (flag==1){
    all_mcells.push_back(curr_cell);
    for (int i=0;i!=curr_cells.size();i++){
      MergeSpaceCell *curr_cell1 = curr_cells.at(i);
      MergeSpaceCellSelection curr_cells1 = mcells_map[curr_cell1];
      Iterate(curr_cell1,curr_cells1);
    }
  }

}

WCShower::~WCShower(){
  
}
