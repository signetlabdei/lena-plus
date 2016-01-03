/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Modified by Michele Polese from the lena-dual-stripe example.
 * The original lena-dual-stripe example was released with these comments:
 *
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 *
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store-module.h>
#include <ns3/buildings-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/applications-module.h>
#include <ns3/log.h>
#include <iomanip>
#include <ios>
#include <string>
#include <vector>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaAttachScenario");

bool AreOverlapping(Box a, Box b) {
	return !((a.xMin > b.xMax) || (b.xMin > a.xMax) || (a.yMin > b.yMax)
			|| (b.yMin > a.yMax));
}

class FemtocellBlockAllocator {
public:
	FemtocellBlockAllocator(Box area, uint32_t nApartmentsX, uint32_t nFloors);
	void CreateResidentialDistrict(uint32_t n);
	void CreateResidential(uint32_t buildingNumber);

private:
	bool OverlapsWithAnyPrevious(Box);
	Box m_area;
	uint32_t m_nApartmentsX;
	uint32_t m_nFloors;
	std::list<Box> m_previousBlocks;
	double m_xSize;
	double m_ySize;
	Ptr<UniformRandomVariable> m_xMinVar;
	Ptr<UniformRandomVariable> m_yMinVar;

};

FemtocellBlockAllocator::FemtocellBlockAllocator(Box area,
		uint32_t nApartmentsX, uint32_t nFloors) :
		m_area(area), m_nApartmentsX(nApartmentsX), m_nFloors(nFloors), m_xSize(
				nApartmentsX * 10 + 20), m_ySize(70) {
	m_xMinVar = CreateObject<UniformRandomVariable>();
	m_xMinVar->SetAttribute("Min", DoubleValue(area.xMin));
	m_xMinVar->SetAttribute("Max", DoubleValue(area.xMax - m_xSize));
	m_yMinVar = CreateObject<UniformRandomVariable>();
	m_yMinVar->SetAttribute("Min", DoubleValue(area.yMin));
	m_yMinVar->SetAttribute("Max", DoubleValue(area.yMax - m_ySize));
}

//these two methods create n buildings
// 360 m^2, delta 15 meters, 3 floors each
void FemtocellBlockAllocator::CreateResidentialDistrict(uint32_t n) {
	for (uint32_t i = 0; i < n; ++i) {
		CreateResidential(i);	
	}
}


void FemtocellBlockAllocator::CreateResidential(uint32_t buildingNumber) {
	//uint32_t column = buildingNumber % 4;
	uint32_t row = buildingNumber / 4;
	Box box;
	//set xMin and yMin, as in this map
	// ----______-----------------------______----______--------------------______----
	//where ----=15m, interspace between buildings
	//and ____=24m, building's widht
	//Each building is 360 m^2
	Ptr<GridBuildingAllocator> gridBuildingAllocator;
	if (buildingNumber % 4 == 0) {
		box.xMin = m_area.xMin;
		box.yMin = m_area.yMin + (12 + 15) * row;

		if (buildingNumber % 6 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(30));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(12));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(20));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Office));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else if (buildingNumber % 5 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(4 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			//create one of 10 residential buildings

			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Office));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else {
			NS_LOG_LOGIC ("allocated non overlapping block " << box);
			m_previousBlocks.push_back(box);
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		}

	} else if (buildingNumber % 4 == 1) {
		box.xMin = m_area.xMin + 24 + 15 + 20;
		box.yMin = m_area.yMin + (12 + 15) * row;

		if (buildingNumber % 6 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(30));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(12));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(20));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Office));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else if (buildingNumber % 5 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(4 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			//create one of 10 residential buildings

			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else {
			NS_LOG_LOGIC ("allocated non overlapping block " << box);
			m_previousBlocks.push_back(box);
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		}

	} else if (buildingNumber % 4 == 2) {
		box.xMin = m_area.xMin + 24 + 15 + 20 + 24 + 15;
		box.yMin = m_area.yMin + (12 + 15) * row;

		if (buildingNumber % 6 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(30));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(12));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(20));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Office));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else if (buildingNumber % 5 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(4 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			//create one of 10 residential buildings

			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else {
			NS_LOG_LOGIC ("allocated non overlapping block " << box);
			m_previousBlocks.push_back(box);
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		}
	} else if (buildingNumber % 4 == 3) {
		box.xMin = m_area.xMin + 24 + 15 + 20 + 24 + 15 + 24 + 15 + 20;
		box.yMin = m_area.yMin + (12 + 15) * row;

		if (buildingNumber % 6 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(30));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(12));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(20));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Office));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else if (buildingNumber % 5 == 0) {
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(4 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			//create one of 10 residential buildings

			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		} else {
			NS_LOG_LOGIC ("allocated non overlapping block " << box);
			m_previousBlocks.push_back(box);
			gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
			gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(1));
			gridBuildingAllocator->SetAttribute("LengthX", DoubleValue(24));
			gridBuildingAllocator->SetAttribute("LengthY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaX", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("DeltaY", DoubleValue(15));
			gridBuildingAllocator->SetAttribute("Height",
					DoubleValue(3 * m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsX",
					UintegerValue(m_nApartmentsX));
			gridBuildingAllocator->SetBuildingAttribute("NRoomsY",
					UintegerValue(2));
			gridBuildingAllocator->SetBuildingAttribute("NFloors",
					UintegerValue(m_nFloors));
			gridBuildingAllocator->SetBuildingAttribute("ExternalWallsType",
					EnumValue(Building::ConcreteWithoutWindows));
			gridBuildingAllocator->SetBuildingAttribute("Type",
					EnumValue(Building::Residential));
			gridBuildingAllocator->SetAttribute("MinX",
					DoubleValue(box.xMin + 7.5));
			gridBuildingAllocator->SetAttribute("MinY",
					DoubleValue(box.yMin + 7.5));
			gridBuildingAllocator->Create(1);
		}
	}

}

bool FemtocellBlockAllocator::OverlapsWithAnyPrevious(Box box) {
	for (std::list<Box>::iterator it = m_previousBlocks.begin();
			it != m_previousBlocks.end(); ++it) {
		if (AreOverlapping(*it, box)) {
			return true;
		}
	}
	return false;
}

void PrintGnuplottableBuildingListToFile(std::string filename) {
	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open()) {
		NS_LOG_ERROR ("Can't open file " << filename);
		return;
	}
	uint32_t index = 0;
	for (BuildingList::Iterator it = BuildingList::Begin();
			it != BuildingList::End(); ++it) {
		++index;
		Box box = (*it)->GetBoundaries();
		outFile << "set object " << index << " rect from " << box.xMin << ","
				<< box.yMin << " to " << box.xMax << "," << box.yMax
				<< " front fs empty " << std::endl;
	}
}

void PrintGnuplottableUeListToFile(std::string filename) {
	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open()) {
		NS_LOG_ERROR ("Can't open file " << filename);
		return;
	}
	for (NodeList::Iterator it = NodeList::Begin(); it != NodeList::End();
			++it) {
		Ptr<Node> node = *it;
		int nDevs = node->GetNDevices();
		for (int j = 0; j < nDevs; j++) {
			Ptr<LteUeNetDevice> uedev = node->GetDevice(j)->GetObject<
					LteUeNetDevice>();
			if (uedev) {
				Vector pos = node->GetObject<MobilityModel>()->GetPosition();
				outFile << "set label \" \" at "
						<< pos.x << "," << pos.y
						<< " left font \"Helvetica,6\" textcolor rgb \"slategray\" front point pt 2 ps 0.3 lc rgb \"slategray\" offset 0,0"
						<< std::endl; //uedev->GetImsi() 
			}
		}
	}
}

void PrintGnuplottableEnbListToFile(std::string filename) {
	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open()) {
		NS_LOG_ERROR ("Can't open file " << filename);
		return;
	}
	for (NodeList::Iterator it = NodeList::Begin(); it != NodeList::End();
			++it) {
		Ptr<Node> node = *it;
		int nDevs = node->GetNDevices();
		for (int j = 0; j < nDevs; j++) {
			Ptr<LteEnbNetDevice> enbdev = node->GetDevice(j)->GetObject<
					LteEnbNetDevice>();
			if (enbdev) {
				Vector pos = node->GetObject<MobilityModel>()->GetPosition();
				outFile << "set label \" \" at "
						<< pos.x << "," << pos.y
						<< " left font \"Helvetica,6\" textcolor rgb \"slategray\" front point pt 2 ps 0.3 lc rgb \"slategray\" offset 0,0"
						<< std::endl; 
			}
		}
	}
}

static uint16_t remote_port_ul = 10000;
static uint16_t remote_port_dl = 20000;

void 
deAttach(Ptr<NetDevice> ueDev, Ptr<NetDevice> enbDev) {
  // note: only 1 eNB in this scenario
  Ptr<LteUeNetDevice> ueLteDevice = ueDev->GetObject<LteUeNetDevice> ();
  Ptr<LteEnbNetDevice> enbDevice = enbDev->GetObject<LteEnbNetDevice>();
  // get some info
  uint16_t rnti = ueLteDevice->GetRrc()->GetRnti();
  uint16_t cellId = ueLteDevice->GetRrc()->GetCellId();
  NS_LOG_ERROR("The UE rnti is " << rnti << " and the cellId is " << cellId);
  uint16_t enb_cellId = enbDevice->GetCellId();
  NS_LOG_ERROR("The ENB cellId is " << enb_cellId);

  // remove the user from the eNB
  Ptr<LteEnbRrc> enbRrc = enbDevice->GetRrc();
  Ptr<LteEnbMac> enbMac = enbDevice->GetMac();
  bool isConnected = enbRrc->RemoveUeByImsi( rnti );
  if(!isConnected)
    {
      NS_LOG_ERROR("Ue disconnected");
      // force the UE to stop PHY and MAC for simulation purposes
      Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc(); // optimize this call
      ueRrc->StopConnectionAttempt();
    }
}

void
reAttach(Ptr<LteHelper> lteHelper, Ptr<EpcHelper> epcHelper, Ptr<NetDevice> ueDev, uint16_t remote_port_ul, uint16_t remote_port_dl) {
  Ptr<LteUeNetDevice> ueLteDevice = ueDev->GetObject<LteUeNetDevice> ();
  Ptr<EpcUeNas> ueNas = ueDev->GetObject<LteUeNetDevice> ()->GetNas();
  ueNas->Connect();
  // activate EPS bearer
  // radio bearer
  Ptr<EpcTft> tft = Create<EpcTft>(); 
  EpcTft::PacketFilter dlpf;
  dlpf.localPortStart = remote_port_dl;
  dlpf.localPortEnd = remote_port_dl;
  tft->Add(dlpf);
  EpcTft::PacketFilter ulpf;
  ulpf.remotePortStart = remote_port_ul;
  ulpf.remotePortEnd = remote_port_ul;
  tft->Add(ulpf);
  EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
  lteHelper->ActivateDedicatedEpsBearer(ueDev, bearer,
      tft);     
}

void 
reportUe(Ptr<NetDevice> ueDev) {
  
  Ptr<LteUeNetDevice> ueLteDevice = ueDev->GetObject<LteUeNetDevice> ();
  NS_LOG_ERROR("the state is " << ueLteDevice->GetRrc()->GetState());
  uint16_t rnti = ueLteDevice->GetRrc()->GetRnti();
  uint16_t cellId = ueLteDevice->GetRrc()->GetCellId();
  NS_LOG_ERROR("The UE rnti is " << rnti << " and the cellId is " << cellId);
}

void 
reportEnb(Ptr<NetDevice> enbDev) {
  Ptr<LteEnbNetDevice> enbLteDevice = enbDev->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  std::map<uint16_t, Ptr<UeManager> > ueMap = enbRrc->GetUeMap();
  for (std::map<uint16_t, Ptr<UeManager> > ::iterator ueIterator = ueMap.begin(); ueIterator != ueMap.end(); ++ueIterator) {
    NS_LOG_ERROR("To eNB " << enbLteDevice->GetCellId() << " connected UE " << ueIterator->second->GetRnti());
  }
}

// update this if numBearersPerUe > 1
static std::vector< uint16_t > remote_port_vector_ul;
static std::vector< uint16_t > remote_port_vector_dl;

static ns3::GlobalValue g_nBlocks("nBlocks", "Number of buildings",
		ns3::UintegerValue(12), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_nApartmentsX("nApartmentsX",
		"Number of apartments along the X axis in a building",
		ns3::UintegerValue(3), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_nFloors("nFloors", "Number of floors",
		ns3::UintegerValue(3), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_nMacroEnbSites("nMacroEnbSites",
		"How many macro sites there are", ns3::UintegerValue(1),
		ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_nMacroEnbSitesX("nMacroEnbSitesX",
		"(minimum) number of sites along the X-axis of the hex grid",
		ns3::UintegerValue(1), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_interSiteDistance("interSiteDistance",
		"min distance between two nearby macro cell sites",
		ns3::DoubleValue(500), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_macroUeDensity("macroUeDensity",
		"How many macrocell UEs there are per square meter",
		ns3::DoubleValue(0.0004), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_macroEnbTxPowerDbm("macroEnbTxPowerDbm",
		"TX power [dBm] used by macro eNBs", ns3::DoubleValue(43.0),
		ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_macroEnbDlEarfcn("macroEnbDlEarfcn",
		"DL EARFCN used by macro eNBs", ns3::UintegerValue(3650), // 3650 900 MHz
		ns3::MakeUintegerChecker<uint16_t>());
static ns3::GlobalValue g_macroEnbBandwidth("macroEnbBandwidth",
		"bandwidth [num RBs] used by macro eNBs", ns3::UintegerValue(25),
		ns3::MakeUintegerChecker<uint16_t>());
static ns3::GlobalValue g_simTime("simTime",
		"Total duration of the simulation [s]", ns3::DoubleValue(100),
		ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_generateRem("generateRem",
		"if true, will generate a REM and then abort the simulation;"
				"if false, will run the simulation normally (without generating any REM)",
		ns3::BooleanValue(false), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_epc("epc",
		"If true, will setup the EPC to simulate an end-to-end topology, "
				"with real IP applications over PDCP and RLC UM (or RLC AM by changing "
				"the default value of EpsBearerToRlcMapping e.g. to RLC_AM_ALWAYS). "
				"If false, only the LTE radio access will be simulated with RLC SM. ",
		ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_epcDl("epcDl",
		"if true, will activate data flows in the downlink when EPC is being used. "
				"If false, downlink flows won't be activated. "
				"If EPC is not used, this parameter will be ignored.",
		ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_epcUl("epcUl",
		"if true, will activate data flows in the uplink when EPC is being used. "
				"If false, uplink flows won't be activated. "
				"If EPC is not used, this parameter will be ignored.",
		ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_useUdp("useUdp",
		"if true, the UdpClient application will be used. "
				"Otherwise, the BulkSend application will be used over a TCP connection. "
				"If EPC is not used, this parameter will be ignored.",
		ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_fadingTrace("fadingTrace",
		"The path of the fading trace (by default no fading trace "
				"is loaded, i.e., fading is not considered)",
		ns3::StringValue(""), ns3::MakeStringChecker());
static ns3::GlobalValue g_numBearersPerUe("numBearersPerUe",
		"How many bearers per UE there are in the simulation",
		ns3::UintegerValue(1), ns3::MakeUintegerChecker<uint16_t>());
static ns3::GlobalValue g_srsPeriodicity("srsPeriodicity",
		"SRS Periodicity (has to be at least "
				"greater than the number of UEs per eNB)",
		ns3::UintegerValue(320), ns3::MakeUintegerChecker<uint16_t>());
static ns3::GlobalValue g_outdoorUeMinSpeed("outdoorUeMinSpeed",
		"Minimum speed value of macro UE with random waypoint model [m/s].",
		ns3::DoubleValue(0.0), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_outdoorUeMaxSpeed("outdoorUeMaxSpeed",
		"Maximum speed value of macro UE with random waypoint model [m/s].",
		ns3::DoubleValue(0.0), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_useApplication("useApplication",
		"if true, will activate applications on UEs and eventually the internet."
		"if epc is false this won't be considered",
		ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_nM2Mdev("nM2Mdev",
		"number of m2m devices",
		ns3::UintegerValue(49), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_runNumber ("runNumber",
		"Run number for rng",
		ns3::UintegerValue(1), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_useIdealPrach("useIdealPrach",
		"if true, will force the usage of ideal prach",
		ns3::BooleanValue(false), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_useIdealRrc("useIdealRrc",
		"if true, will force the usage of ideal rrc",
		ns3::BooleanValue(false), ns3::MakeBooleanChecker());

int main(int argc, char *argv[]) {
	// change some default attributes so that they are reasonable for
	// this scenario, but do this before processing command line
  	Config::SetDefault ("ns3::PfFfMacScheduler::HarqEnabled", BooleanValue(true));
  	Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue(5.0)); 
  	Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue(3.0));

	// arguments, so that the user is allowed to override these settings
	CommandLine cmd;
	cmd.Parse(argc, argv);
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();
	// parse again so you can override input file default values via command line
	cmd.Parse(argc, argv);

	// the scenario parameters get their values from the global attributes defined above
	UintegerValue uintegerValue;
	DoubleValue doubleValue;
	BooleanValue booleanValue;
	StringValue stringValue;
	GlobalValue::GetValueByName("nBlocks", uintegerValue);
	uint32_t nBlocks = uintegerValue.Get();
	GlobalValue::GetValueByName("nApartmentsX", uintegerValue);
	uint32_t nApartmentsX = uintegerValue.Get();
	GlobalValue::GetValueByName("nFloors", uintegerValue);
	uint32_t nFloors = uintegerValue.Get();
	GlobalValue::GetValueByName("nMacroEnbSites", uintegerValue);
	uint32_t nMacroEnbSites = uintegerValue.Get();
	GlobalValue::GetValueByName("nMacroEnbSitesX", uintegerValue);
	uint32_t nMacroEnbSitesX = uintegerValue.Get();
	GlobalValue::GetValueByName("interSiteDistance", doubleValue);
	double interSiteDistance = doubleValue.Get();
	GlobalValue::GetValueByName("macroEnbTxPowerDbm", doubleValue);
	double macroEnbTxPowerDbm = doubleValue.Get();
	GlobalValue::GetValueByName("macroEnbDlEarfcn", uintegerValue);
	uint16_t macroEnbDlEarfcn = uintegerValue.Get();
	GlobalValue::GetValueByName("macroEnbBandwidth", uintegerValue);
	uint16_t macroEnbBandwidth = uintegerValue.Get();
	GlobalValue::GetValueByName("simTime", doubleValue);
	double simTime = doubleValue.Get();
	GlobalValue::GetValueByName("epc", booleanValue);
	bool epc = booleanValue.Get();
	GlobalValue::GetValueByName("generateRem", booleanValue);
	bool generateRem = booleanValue.Get();
	GlobalValue::GetValueByName("fadingTrace", stringValue);
	std::string fadingTrace = stringValue.Get();
	GlobalValue::GetValueByName("numBearersPerUe", uintegerValue);
	uint16_t numBearersPerUe = uintegerValue.Get();
	GlobalValue::GetValueByName("srsPeriodicity", uintegerValue);
	uint16_t srsPeriodicity = uintegerValue.Get();
	GlobalValue::GetValueByName("useApplication", booleanValue);
	bool useApplication = booleanValue.Get();
	GlobalValue::GetValueByName("nM2Mdev", uintegerValue);
	uint32_t nM2MUes = uintegerValue.Get();
	GlobalValue::GetValueByName("runNumber", uintegerValue);
	uint32_t runNumber = uintegerValue.Get();
	GlobalValue::GetValueByName("useIdealPrach", booleanValue);
	bool useIdealPrach = booleanValue.Get();
	GlobalValue::GetValueByName("useIdealRrc", booleanValue);
	bool useIdealRrc = booleanValue.Get();

	Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity",
			UintegerValue(srsPeriodicity));
  	Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue(macroEnbDlEarfcn));

	Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue(useIdealRrc));
  	Config::SetDefault ("ns3::LteHelper::UseIdealPrach", BooleanValue(useIdealPrach));

  	// rng things
  	RngSeedManager::SetSeed (1);
  	RngSeedManager::SetRun (runNumber);  

	//create the macro box that will contain macro ues and buildings
	Box buildingsBox;
	double ueZ_low = 3;
	double ueZ = 3;
	buildingsBox = Box(0, 220, 0, 90, ueZ_low, ueZ);

	FemtocellBlockAllocator blockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);

	buildingsBox = Box(0, 220, 90, 180, ueZ_low, ueZ);
	blockAllocator = FemtocellBlockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);

	buildingsBox = Box(0, 220, 180, 280, ueZ_low, ueZ);
	blockAllocator = FemtocellBlockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);
	
	buildingsBox = Box(220, 440, 0, 90, ueZ_low, ueZ);
	blockAllocator = FemtocellBlockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);

	buildingsBox = Box(220, 440, 90, 180, ueZ_low, ueZ);
	blockAllocator = FemtocellBlockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);

	buildingsBox = Box(220, 440, 180, 280, ueZ_low, ueZ);
	blockAllocator = FemtocellBlockAllocator(buildingsBox, nApartmentsX, nFloors);
	blockAllocator.CreateResidentialDistrict(nBlocks);

	buildingsBox = Box(0, 440, 0, 280, ueZ_low, ueZ);

	NodeContainer macroEnbs;
	macroEnbs.Create(3 * nMacroEnbSites);
	NodeContainer m2mUes;
	m2mUes.Create(nM2MUes);

	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();
	lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
	lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (0));
	lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (8));
	lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (8));
	// use always NLOS model
	lteHelper->SetPathlossModelAttribute ("Los2NlosThr", DoubleValue (0));
	lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");

	if (!fadingTrace.empty ())
	{
	  lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
	  lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue (fadingTrace));
	}

	Ptr<PointToPointEpcHelper> epcHelper;
	if (epc)
	{
	  NS_LOG_LOGIC ("enabling EPC");
	  epcHelper = CreateObject<PointToPointEpcHelper> ();
	  lteHelper->SetEpcHelper (epcHelper);
	}

	// Macro eNBs in 3-sector hex grid

	mobility.Install(macroEnbs);
	BuildingsHelper::Install(macroEnbs);
	Ptr<LteHexGridEnbTopologyHelper> lteHexGridEnbTopologyHelper = CreateObject<
			LteHexGridEnbTopologyHelper>();
	lteHexGridEnbTopologyHelper->SetLteHelper(lteHelper);
	lteHexGridEnbTopologyHelper->SetAttribute("InterSiteDistance",
			DoubleValue(interSiteDistance));
	lteHexGridEnbTopologyHelper->SetAttribute("MinX", DoubleValue((buildingsBox.xMax - buildingsBox.xMin)/2));
	lteHexGridEnbTopologyHelper->SetAttribute("MinY", DoubleValue((buildingsBox.yMax - buildingsBox.yMin)/2));
	lteHexGridEnbTopologyHelper->SetAttribute("GridWidth",
			UintegerValue(nMacroEnbSitesX));
	Config::SetDefault("ns3::LteEnbPhy::TxPower",
			DoubleValue(macroEnbTxPowerDbm));
	lteHelper->SetEnbAntennaModelType("ns3::ParabolicAntennaModel");
	lteHelper->SetEnbAntennaModelAttribute("Beamwidth", DoubleValue(65));
	lteHelper->SetEnbAntennaModelAttribute("MaxAttenuation", DoubleValue(20.0));
	lteHelper->SetEnbDeviceAttribute("DlEarfcn",
			UintegerValue(macroEnbDlEarfcn));
	lteHelper->SetEnbDeviceAttribute("UlEarfcn",
			UintegerValue(macroEnbDlEarfcn + 18000));
	lteHelper->SetEnbDeviceAttribute("DlBandwidth",
			UintegerValue(macroEnbBandwidth));
	lteHelper->SetEnbDeviceAttribute("UlBandwidth",
			UintegerValue(macroEnbBandwidth));
	NetDeviceContainer macroEnbDevs =
			lteHexGridEnbTopologyHelper->SetPositionAndInstallEnbDevice(
					macroEnbs);

	// allocate UEs inside buildings
	Ptr<PositionAllocator> positionAlloc = CreateObject<
			RandomBuildingPositionAllocator>();
	mobility.SetPositionAllocator(positionAlloc);
	mobility.Install(m2mUes);
	BuildingsHelper::Install(m2mUes);	
	NetDeviceContainer m2mUeDevs = lteHelper->InstallUeDevice(m2mUes);

	Ipv4Address remoteHostAddr;
	NodeContainer ueNodes;
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ipv4InterfaceContainer ueIpIfaces;
	Ptr<Node> remoteHost;
	NetDeviceContainer ueDevs;

	PointToPointHelper p2ph;
	if (epc) {
		NS_LOG_LOGIC ("setting up internet and remote host");

		// Create a single RemoteHost
		NodeContainer remoteHostContainer;
		remoteHostContainer.Create(1);
		remoteHost = remoteHostContainer.Get(0);
		InternetStackHelper internet;
		internet.Install(remoteHostContainer);

		// Create the Internet

		p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
		p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
		p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
		Ptr<Node> pgw = epcHelper->GetPgwNode();
		NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
		Ipv4AddressHelper ipv4h;
		ipv4h.SetBase("1.0.0.0", "255.0.0.0");
		Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
		// in this container, interface 0 is the pgw, 1 is the remoteHost
		remoteHostAddr = internetIpIfaces.GetAddress(1);

		Ipv4StaticRoutingHelper ipv4RoutingHelper;
		Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
				ipv4RoutingHelper.GetStaticRouting(
						remoteHost->GetObject<Ipv4>());
		remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
				Ipv4Mask("255.0.0.0"), 1);

		// this code may be useful if 2 diff containers are used to place UEs in the simulation
		ueNodes.Add(m2mUes);
		ueDevs.Add(m2mUeDevs);

		// Install the IP stack on the UEs
		internet.Install(ueNodes);
		ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

		// Assign IP address to UEs, and install applications
		for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
			Ptr<Node> ueNode = ueNodes.Get(u);
			// Set the default gateway for the UE
			Ptr<Ipv4StaticRouting> ueStaticRouting =
					ipv4RoutingHelper.GetStaticRouting(
							ueNode->GetObject<Ipv4>());
			ueStaticRouting->SetDefaultRoute(
					epcHelper->GetUeDefaultGatewayAddress(), 1);
		}

		lteHelper->Attach(ueDevs);
	} 

	if(useApplication && epc)
	{
		// randomize a bit start times to avoid simulation artifacts
		// (e.g., buffer overflows due to packet transmissions happening
		// exactly at the same time)
		Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<
		UniformRandomVariable>();

		startTimeSeconds->SetAttribute("Min", DoubleValue(0));
		startTimeSeconds->SetAttribute("Max", DoubleValue(3));

		for (uint32_t u = 0; u < ueNodes.GetN(); ++u) 
		{
		Ptr<Node> ue = ueNodes.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting =
		    ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
		ueStaticRouting->SetDefaultRoute(
		    epcHelper->GetUeDefaultGatewayAddress(), 1);

		for (uint32_t b = 0; b < numBearersPerUe; ++b) 
		    {
		      ++remote_port_ul;
		      ++remote_port_dl;

		      ApplicationContainer clientApps;
		      ApplicationContainer serverApps;

		      
		      NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
		      UdpClientHelper ulClientHelper(remoteHostAddr, remote_port_ul);
		      clientApps.Add(ulClientHelper.Install(ue));
		      PacketSinkHelper ulPacketSinkHelper(
		          "ns3::UdpSocketFactory",
		          InetSocketAddress(Ipv4Address::GetAny(),
		              remote_port_ul));
		      serverApps.Add(ulPacketSinkHelper.Install(remoteHost));
		      remote_port_vector_ul.push_back(remote_port_ul);


		      NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
		      UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u),
		                remote_port_dl);
		      clientApps.Add(dlClientHelper.Install(remoteHost));
		      PacketSinkHelper dlPacketSinkHelper(
		          "ns3::UdpSocketFactory",
		          InetSocketAddress(Ipv4Address::GetAny(),
		              remote_port_dl));
		      serverApps.Add(dlPacketSinkHelper.Install(ue));
		      remote_port_vector_dl.push_back(remote_port_dl);

		      // radio bearer
		      Ptr<EpcTft> tft = Create<EpcTft>(); 
		      EpcTft::PacketFilter dlpf;
		      dlpf.localPortStart = remote_port_dl;
		      dlpf.localPortEnd = remote_port_dl;
		      tft->Add(dlpf);
		      EpcTft::PacketFilter ulpf;
		      ulpf.remotePortStart = remote_port_ul;
		      ulpf.remotePortEnd = remote_port_ul;
		      tft->Add(ulpf);
		      EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
		      lteHelper->ActivateDedicatedEpsBearer(ueDevs.Get(u), bearer,
		          tft);      

		      Time startTime = Seconds(startTimeSeconds->GetValue());
		      serverApps.Start(startTime);
		      clientApps.Start(startTime);

		    } // end for b
		} // end for cycle on ueNodes
	} // end of if(useApplication)


	/* The following can be used to force a user to detach and reattach to the network
	// randomize detach time
	Ptr<UniformRandomVariable> detachTimeSeconds = CreateObject<
	  UniformRandomVariable>();

	detachTimeSeconds->SetAttribute("Min", DoubleValue(0));
	detachTimeSeconds->SetAttribute("Max", DoubleValue(3));

	for (uint32_t u = 0; u < ueDevs.GetN(); ++u) 
	{
	Simulator::Schedule(Seconds(7 + detachTimeSeconds->GetValue()), deAttach, ueDevs.Get(u), enbDevs.Get(0));
	Simulator::Schedule(Seconds(10 + detachTimeSeconds->GetValue()), reAttach, lteHelper, epcHelper, ueDevs.Get(u), remote_port_vector_ul[u], remote_port_vector_dl[u]);
	Simulator::Schedule(Seconds(15 + detachTimeSeconds->GetValue()), deAttach, ueDevs.Get(u), enbDevs.Get(0));
	Simulator::Schedule(Seconds(25 + detachTimeSeconds->GetValue()), reAttach, lteHelper, epcHelper, ueDevs.Get(u), remote_port_vector_ul[u], remote_port_vector_dl[u]);
	Simulator::Schedule(Seconds(40 + detachTimeSeconds->GetValue()), deAttach, ueDevs.Get(u), enbDevs.Get(0));
	Simulator::Schedule(Seconds(60), reAttach, lteHelper, epcHelper, ueDevs.Get(u), remote_port_vector_ul[u], remote_port_vector_dl[u]);
	}
	*/

	BuildingsHelper::MakeMobilityModelConsistent();

	Ptr<RadioEnvironmentMapHelper> remHelper;
	if (generateRem) {
		PrintGnuplottableBuildingListToFile("buildings.txt");
		PrintGnuplottableEnbListToFile("enbs.txt");
		PrintGnuplottableUeListToFile("ues.txt");

		remHelper = CreateObject<RadioEnvironmentMapHelper>();
		remHelper->SetAttribute("ChannelPath", StringValue("/ChannelList/0"));
		remHelper->SetAttribute("OutputFile",
				StringValue("lena-dual-stripe.rem"));
		remHelper->SetAttribute("XMin", DoubleValue(buildingsBox.xMin));
		remHelper->SetAttribute("XMax", DoubleValue(buildingsBox.xMax));
		remHelper->SetAttribute("XRes", UintegerValue(300));
		remHelper->SetAttribute("YMin", DoubleValue(buildingsBox.yMin));
		remHelper->SetAttribute("YMax", DoubleValue(buildingsBox.yMax));
		remHelper->SetAttribute("YRes", UintegerValue(300));
		remHelper->SetAttribute("Z", DoubleValue(2));
		remHelper->SetAttribute("Earfcn", UintegerValue(macroEnbDlEarfcn)); // 900 MHz
		remHelper->SetAttribute("Bandwidth", UintegerValue(macroEnbBandwidth)); 
		remHelper->SetAttribute("UseDataChannel", BooleanValue(false));
		remHelper->Install();
		// simulation will stop right after the REM has been generated
	} else {
		Simulator::Stop(Seconds(simTime));
	}

	//lteHelper->EnablePhyTraces ();
	lteHelper->EnableMacTraces ();
	//lteHelper->EnableRlcTraces ();
	lteHelper->EnablePdcpTraces ();
	if(!useIdealPrach)
		{
			lteHelper->EnableRaPreambleTraces ();
		}
	lteHelper->EnableRaDelayTraces ();

	Simulator::Run();

	lteHelper = 0;
	Simulator::Destroy();
	return 0;
}
