#include "UdpTraceBasedApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/lifecycle/ModuleOperations.h"

Define_Module(UdpTraceBasedApp);

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
//Declaring Vector to store the ground station coordinates and ethernet


void UdpTraceBasedApp::initialize(int stage)
{
    // initialize with UdpBasicApp
    UdpBasicApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *traceFile = par("traceFile");
        // read the timeStamp vector and in case it it empty or the file does not exist the module will not be started,
        // as specified in handleStartOperation method.
        parseTraceFile2Vector(traceFile,timeStamp);
        //////////////////////////////////////////////////////////////////////////
        // Allow multuple groundstations (Musab) 
        //////////////////////////////////////////////////////////////////////////
        multiGroundStationUsed = par("multiGroundStationUsed").boolValue();
        //new code added here
        if (multiGroundStationUsed){
            host = getContainingNode(this);
            mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
            const char *file_name = par("groundstationsTraceFile");
            readTraceFile(file_name);
            // destination_index = findClosestGroundStation();
        }
        
        // GSx = par("GSx");
        // GSy = par("GSy");
        // GSz = par("GSz");
        // GS2x = par("GS2x");
        // GS2y = par("GS2y");
        // GS2z = par("GS2z");
        // std::vector<std::vector<double>> locations
        // {
        //     { GSx, GSy, GSz },
        //     { GS2x, GS2y, GS2z }
        // };
        // GSLocations = locations;
        // if(multiGroundStationUsed){
        //     host = getContainingNode(this);
        //     mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        // }
        //////////////////////////////////////////////////////////////////////////
        // Emit application packet sent Signal (Musab)
        //////////////////////////////////////////////////////////////////////////
        appPacketSentSignal = registerSignal("appPacketSent");
    }
}

void UdpTraceBasedApp::parseTraceFile2Vector(const char* filename, std::vector<double>& vecOfDoubles)
{
    std::vector<std::string> vecOfStrs;
    std::ifstream in(filename, std::ios::in);
    // Check if the file is opened (we modified the error message here to just  return in order to enable scripting the application)
    if (in.fail())
            return;
    std::string lineStr;
    // Read the file line by line until the end.
    while (std::getline(in, lineStr))
    {
        // Append the contents of a non-empty line to the vector of strings
        if (lineStr.size() > 0){
            double lineDouble = stod(lineStr);
            simtime_t lineStart = lineDouble;
            // only add line if it is within boundaries [startTime,stopTime]
            if ((lineStart >= startTime && (lineStart <= stopTime || stopTime == -1))){
                vecOfDoubles.push_back(lineDouble);
            // vecOfStrs.push_back(lineStr);
            }
        }
    }
    // Close The File
    in.close();
    // Assign the elements of vecOfStrs to vecOfDoubles after converting it from string to double
    // vecOfDoubles.resize(vecOfStrs.size());
    // std::transform(vecOfStrs.begin(), vecOfStrs.end(), vecOfDoubles.begin(),
    //     [](std::string const& val) {return stod(val); });
}

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
////The following function reads a trace file to store the coordinates of the ground stations
void UdpTraceBasedApp::readTraceFile(const char* file_name)
{
    //reading the .txt file to list of ground_stations_coordinates
    int position_of_first_comma,position_of_second_comma,position_of_third_comma,searching_position_output_textfile,position_at_output_textfile,assigning_position_z_coordinate,assigning_position_x_coordinate,assigning_position_y_coordinate,assigning_position_ethernet;

    // Creating a text string, which is used to output the text file
    std::string output_textfile;

    // Read from the text file
    std::ifstream MyReadFile(file_name);
    //ifstream MyReadFile("ground_stations_coordinates.txt");
    line_number=0;
    
    // Using a while loop together with the getline() function to read the file line by line
    while (getline (MyReadFile, output_textfile)){               
        // finding the position of the first ","
        for( searching_position_output_textfile = 0; searching_position_output_textfile < output_textfile.length(); searching_position_output_textfile++){
            if(output_textfile[searching_position_output_textfile]==','){
                position_of_first_comma = searching_position_output_textfile;
                break;
            }
        }
        
        // finding the position of the second ","
        for( searching_position_output_textfile = searching_position_output_textfile+1; searching_position_output_textfile < output_textfile.length(); searching_position_output_textfile++){
            if(output_textfile[searching_position_output_textfile]==','){
                position_of_second_comma = searching_position_output_textfile;
                break;
            }
        }

        // finding the position of the third ","
        for( searching_position_output_textfile = searching_position_output_textfile+1; searching_position_output_textfile < output_textfile.length(); searching_position_output_textfile++){
            if(output_textfile[searching_position_output_textfile]==','){
                position_of_third_comma = searching_position_output_textfile;
                break;
            }
        }

        char x_coordinate_Part[position_of_first_comma];
        char y_coordinate_Part[position_of_second_comma-position_of_first_comma];
        char z_coordinate_Part[position_of_third_comma-position_of_second_comma];
        char ethernet_Part[output_textfile.length()-position_of_third_comma];


        //assigning the x coordinate part of the trace file into the vector
        position_at_output_textfile=0;
        for(assigning_position_x_coordinate = 0;assigning_position_x_coordinate<position_of_first_comma;assigning_position_x_coordinate++){
            x_coordinate_Part[assigning_position_x_coordinate]=output_textfile[position_at_output_textfile];
            position_at_output_textfile++;
        }
        x_coordinate_Part[assigning_position_x_coordinate]='\0';
        std::vector<double> temp;
        
        // convert string to double
        temp.push_back(std::stod(x_coordinate_Part));
        position_at_output_textfile++;

        //assigning the y coordinate part of the trace file into the vector
        for( assigning_position_y_coordinate=0;assigning_position_y_coordinate<position_of_second_comma-position_of_first_comma-1;assigning_position_y_coordinate++){
            y_coordinate_Part[assigning_position_y_coordinate]=output_textfile[position_at_output_textfile];
            position_at_output_textfile++;
        }
        y_coordinate_Part[assigning_position_y_coordinate]='\0';
        
        // convert string to double
        temp.push_back(std::stod(y_coordinate_Part));
        position_at_output_textfile++;

        //assigning the z coordinate part of the trace file into vector
        for(assigning_position_z_coordinate=0;assigning_position_z_coordinate<position_of_third_comma-position_of_second_comma-1;assigning_position_z_coordinate++){
            z_coordinate_Part[assigning_position_z_coordinate]=output_textfile[position_at_output_textfile];
            position_at_output_textfile++;
        }
        z_coordinate_Part[assigning_position_z_coordinate]='\0';
        
        // convert string to double
        temp.push_back(std::stod(z_coordinate_Part));
        position_at_output_textfile++;

        //assigning the ethernet part of the trace file into vector
        for(assigning_position_ethernet=0;assigning_position_ethernet<output_textfile.length()-position_of_third_comma-1;assigning_position_ethernet++){
            ethernet_Part[assigning_position_ethernet]=output_textfile[position_at_output_textfile];
            position_at_output_textfile++;
        }
        ethernet_Part[assigning_position_ethernet]='\0';
        ethernet_vector.push_back(ethernet_Part);

        ground_stations_coordinates_array.push_back(temp);
        temp.clear();
        line_number++;

    }
    
    // Closing the file
    MyReadFile.close();
    
    //printing ground stations coordinates vector 
    EV << "List of ground stations coordinates: " << " \n";
    std::vector<std::vector<double>> :: iterator test_vector_iterator;
    test_vector_iterator = ground_stations_coordinates_array.begin();
    for (test_vector_iterator; test_vector_iterator != ground_stations_coordinates_array.end(); test_vector_iterator++) {
        std::vector<double> :: iterator inner_vector_iterator = (*test_vector_iterator).begin();
        for(inner_vector_iterator; inner_vector_iterator != (*test_vector_iterator).end(); inner_vector_iterator++){
            std::cout << *inner_vector_iterator <<" ";
        }
        std::cout << std::endl;
    }
    
    //printing ethernet vector 
    EV << "List of Ethernets: " << " \n";
    for(std::vector<std::string>::iterator e=ethernet_vector.begin();e!=ethernet_vector.end();++e){
        std::cout << *e;
        std::cout << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
////The following function calculates the distance to all ground stations and finds the closest one
int UdpTraceBasedApp::findClosestGroundStation()
{
    int closest_ground_station=0;
    //getting the position of the current aircraft
    // Coord aircraft_position = check_and_cast<IMobility *>(getContainingNode(this)->getSubmodule("mobility"))->getCurrentPosition();
    Coord aircraft_position = mobility->getCurrentPosition();
    EV << " aircraft_position: " << aircraft_position << " \n";
    
    std::vector<double> distance_vector;
    //applying the formula sqrt((x_2-x_1)^2+(y_2-y_1)^2+(z_2-z1)^2) to get the distance between aircraft and all ground stations
    double min_distance = sqrt(pow((aircraft_position.x-ground_stations_coordinates_array[0][0]),2) + pow((aircraft_position.y-ground_stations_coordinates_array[0][1]),2)+ pow((aircraft_position.z-ground_stations_coordinates_array[0][2]),2));
    
    for(int i = 0; i < line_number; i++){
        double distance = sqrt(pow((aircraft_position.x-ground_stations_coordinates_array[i][0]),2) + pow((aircraft_position.y-ground_stations_coordinates_array[i][1]),2)+ pow((aircraft_position.z-ground_stations_coordinates_array[i][2]),2));
        distance_vector.push_back(distance);

        //find the closest ground station
        if (distance < min_distance){
            min_distance = distance;
            closest_ground_station=i;
        }
    }
    EV << " closest_ground_station is: " << closest_ground_station << " \n";
    EV << " Corresponding Ethernet is: " << ethernet_vector[closest_ground_station] << " \n";
    //new code upto here
    return closest_ground_station;
}

void UdpTraceBasedApp::handleStartOperation(LifecycleOperation *operation)
{
    if (timeStamp.empty()){
        return;
    }
    startTime = timeStamp.at(0);
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void UdpTraceBasedApp::processStart(){
    // a counter for the index of the timeStamp vector
    timeIndex = 0;
    UdpBasicApp::processStart();
}

void UdpTraceBasedApp::processSend()
{
    sendPacket();
    //////////////////////////////////////////////////////////////////////////
    // Emit application packet sent Signal (Musab) (Musab)
    //////////////////////////////////////////////////////////////////////////
    emit(appPacketSentSignal, simTime());
    // Calculate the next packet transmission time from the timeStamp vector
    timeIndex = timeIndex + 1;
    // to ensure that the if statement will be checked if and only if the timeStamp has at least 2 elements.
    if (timeIndex < timeStamp.size()){
        simtime_t nextSendTime = simTime() + (timeStamp.at(timeIndex)  -timeStamp.at(timeIndex-1));
        selfMsg->setKind(SEND);
        scheduleAt(nextSendTime, selfMsg);
    } else {
        selfMsg->setKind(STOP);
        scheduleAt(simTime(), selfMsg);
    }

}

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
L3Address UdpTraceBasedApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if(multiGroundStationUsed) {
        destination_index = findClosestGroundStation();
        return destAddresses[destination_index];
        // const Coord GroundStationLocation = Coord(GSx, GSy, GSz);
        // const Coord GroundStationLocation2 = Coord(GS2x, GS2y, GS2z);
        // m distanceToGroundStation = m(mobility->getCurrentPosition().distance(GroundStationLocation));
        // m distanceToGroundStation2 = m(mobility->getCurrentPosition().distance(GroundStationLocation2));
        // if (distanceToGroundStation < distanceToGroundStation2)
        //     return destAddresses[0];
        // else
        //     return destAddresses[1];
        // for (int i = 0; i < GSLocations.size(); i++)
        // {
        //     EV_INFO << "Ground station (Destination) position x = " << GSLocations[i][0] << " y = " << GSLocations[i][1] << " z = " << GSLocations[i][2] << endl;
        // }
        // return;
    }
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void UdpTraceBasedApp::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if(dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}
