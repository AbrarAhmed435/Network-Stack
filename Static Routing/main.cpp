#include <iostream>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <queue>
#include<ctime>
#include<windows.h>
#include <thread>
#include <unistd.h>
#include<thread>
#include<chrono>
#include <chrono>
#include <ctime>
#include <bitset>
#include<sstream>

using namespace std;
string find_CS(string);
class Switch;
class Device {
public:
    string name;
    string ip_address;
    string mac_address;
    string subnet_mask;
    string gateway;
    Switch *connected_switch;
    int port=-1;
    void *connected_to;

    unordered_map<string, string> arp_cache;

    Device(string n, string ip, string mac) : name(n), ip_address(ip), mac_address(mac) {}

    void set_subnet_mask(string mask) {
        subnet_mask = mask;
    }
     void receiving_Data(string data) {
        size_t pos = data.find('#');
        if (pos != string::npos) {    // Ensure that '#' was found
            string checksum = data.substr(pos + 1);
            string actual_data = data.substr(0, pos);


            string new_checksum = find_CS(actual_data);


            if (new_checksum == checksum) {
                cout << this->name << " received data: " << actual_data << endl;
                cout << "Sending ack " << endl;
            } else {
                cout << "Error detected in data" << endl;
            }
        } else {
            cout << "Invalid data, no checksum found" << endl;
        }
    }

    void set_gateway(string gw) {
        gateway = gw;
    }

    void display_info() {
        cout << "Device: " << name << "\nIP: " << ip_address << "\nMAC: " << mac_address << endl;
    }

    void arp_request(string ip_to_find, vector<Device*>& all_devices) {
    if (arp_cache.find(ip_to_find) != arp_cache.end()) {
        cout << "Found MAC address for " << ip_to_find << ": " << arp_cache[ip_to_find] << endl;
    } else {
        cout << "ARP request for " << ip_to_find << ": MAC address not found. Broadcasting...\n";

        bool found = false;
        for (Device* d : all_devices) {
            if (d->ip_address == ip_to_find) {
                arp_cache[ip_to_find] = d->mac_address;
                cout << "Received ARP response from " << d->name << ": " << d->mac_address << endl;
                found = true;
                break;
            }
        }

        if (!found) {
            cout << "No device responded to ARP request. Host unreachable.\n";
        }
    }
}

};

class Router {
public:
    string name;
    unordered_map<string, string> routing_table;
    unordered_map<string, string> arp_cache;

    Router(string n) : name(n) {}

    void add_route(string destination_ip, string next_hop_ip="Directetly connected") {
        routing_table[destination_ip] = next_hop_ip;
        cout << "Route added: " << destination_ip << " via " << next_hop_ip << endl;
    }

    void display_routing_table() {
        cout << "Routing Table for " << name << ":\n";
        for (const auto& route : routing_table) {
            cout << "Destination: " << route.first << " Next Hop: " << route.second << endl;
        }
    }

    void static_route(string destination_ip) {
        if (routing_table.find(destination_ip) != routing_table.end()) {
            cout << "Routing to " << destination_ip << " via " << routing_table[destination_ip] << endl;
        } else {
            cout << "No route found for " << destination_ip << ". Sending to default gateway.\n";
        }
    }

    void arp_request(string ip_to_find) {
        if (arp_cache.find(ip_to_find) != arp_cache.end()) {
            cout << "Found MAC address for " << ip_to_find << ": " << arp_cache[ip_to_find] << endl;
        } else {
            cout << "ARP request for " << ip_to_find << ": MAC address not found. Requesting...\n";
            arp_cache[ip_to_find] = "00:1A:2B:3C:4D:6F";  // Simulating MAC address
            cout << "Received ARP response for " << ip_to_find << ": " << arp_cache[ip_to_find] << endl;
        }
    }
};

// Simple RIP Protocol (Routing Information Protocol) Simulation
class RIP {
public:
    unordered_map<string, string> routing_table;

    void exchange_routing_info(Router& router) {
        // Simulate exchanging routes (for simplicity, just print current routing table)
        cout << "Exchanging routing information...\n";
        for (const auto& route : router.routing_table) {
            routing_table[route.first] = route.second;
            cout << "Route: " << route.first << " Next Hop: " << route.second << endl;
        }
    }
};


class Switch {
public:
    vector<Device*> connected_end_Device; // Connected devices (hosts)
  unordered_map<int, Router*> connected_routers;   // Single connected router
    unordered_map<string, int> mac_table; // MAC address to port mapping


    // Connect a device to the switch
    void connect_device(Device* device, int port) {
        connected_end_Device.push_back(device);
        device->port = port;
        device->connected_to = this;
        cout << "Device " << device->name << " connected on port " << port << endl;
    }

    // Connect a router to the switch
    void connect_router(Router* router, int port) {
    connected_routers[port] = router;
    cout << "Router " << router->name << " connected to the switch on port " << port << endl;
}

    // Learn MAC address (typically from source address of frame)
    void learn_address(const string& mac_address, int port) {
        if (mac_table.find(mac_address) == mac_table.end()) {
            mac_table[mac_address] = port;
            cout << "Switch learned MAC address " << mac_address << " on port " << port << endl;
        }
    }
     int find_port_by_mac(const string& mac_address) {
        if (mac_table.find(mac_address) != mac_table.end()) {
            return mac_table[mac_address];  // Return corresponding port
        } else {
            cout << "MAC address not found!" << endl;
            return -1;  // Return -1 if MAC address is not found
        }
    }
    void forward_frame(const string& source_mac, const string& destination_mac, const string& frame) {
        // Forwarding logic based on destination MAC address (if it's known)
        int port = find_port_by_mac(destination_mac);
        if (port != -1) {
            cout << "Forwarding frame to port: " << port << endl;
            for(auto device:connected_end_Device){
                if(device->mac_address==destination_mac){
                    device->receiving_Data(frame);
                }
            }

        } else {
            cout << "Destination MAC not found. Broadcasting to all ports." << endl;
            for (auto device : connected_end_Device) {
                cout << "Forwarding frame to " << device->name << endl;
                device->receiving_Data(frame);
            }
        }
    }

    // Forward frame from source MAC to destination MAC
    void forward_frame(const string& src_mac, const string& dest_mac, int incoming_port) {

        if (mac_table.find(dest_mac) != mac_table.end()) {
            int out_port = mac_table[dest_mac];
            cout << "Forwarding frame from " << src_mac << " to " << dest_mac
                 << " on port " << out_port << endl;
        } else {
            cout << "Destination MAC " << dest_mac << " unknown. Flooding frame to all ports "
                 << "except incoming port " << incoming_port << endl;
        }
    }

    // Print the MAC address table
    void print_mac_table() {
        cout << "\n--- Switch MAC Address Table ---\n";
        for (const auto& entry : mac_table) {
            cout << "MAC Address: " << entry.first << ", Port: " << entry.second << endl;
        }
        cout << "--------------------------------\n";
    }
};

class CSMA_CD {
private:
    bool busy_channel = false;

public:
    CSMA_CD() {
        srand(time(0));
    }

    bool sense_channel() {
        busy_channel = (rand() % 10) <3;
        return busy_channel;
    }

    void transmit(const string& data) {
        cout<<data<<endl;
        cout << "Sensing channel..." << endl;
        if (sense_channel()) {
            cout << "Channel is busy. Waiting..." << endl;
            sleep(2);
            transmit(data); // Retry transmission
            return;
        }

        if (rand() % 10 < 3) {  // 20% chance of collision
            cout << "Collision detected! Backing off..." << endl;
            sleep(1); //waiting for 1 second
            transmit(data); // Retry transmission
            return;
        }

        cout << "Transmitting data: " << data << endl;
    }
};
string find_CS(string data) {
    int checksum = 0;
    for (char ch : data) {
        checksum += static_cast<int>(ch);
    }
    checksum %= 256;
    return to_string(checksum);
}

vector<uint8_t> split_ip(const string &ip) {
    vector<uint8_t> octets;
    stringstream ss(ip);
    string octet;

    while (getline(ss, octet, '.')) {
        octets.push_back(stoi(octet));
    }
    return octets;
}

string calculate_netid(const string &ip, const string &subnet) {
    vector<uint8_t> ip_octets = split_ip(ip);
    vector<uint8_t> subnet_octets = split_ip(subnet);

    vector<uint8_t> netid_octets;
    for (int i = 0; i < 4; i++) {
        netid_octets.push_back(ip_octets[i] & subnet_octets[i]);
    }

    stringstream netid;
    for (int i = 0; i < 4; i++) {
        netid << to_string(netid_octets[i]);
        if (i < 3) netid << ".";
    }

    return netid.str();
}
void send_packet(Device* src, Device* des, const string& message, const vector<Device*>& all_devices) {
    string src_net = calculate_netid(src->ip_address, src->subnet_mask);
    string dest_net = calculate_netid(des->ip_address, des->subnet_mask);

    // Append checksum
    string checksum = find_CS(message);
    string final_message = message + "#" + checksum;

    if (src_net == dest_net) {
        cout << "Communication within the same network" << endl;

        // ARP for destination
        if (src->arp_cache.find(des->ip_address) == src->arp_cache.end()) {
            cout << "ARP request for " << des->ip_address << ": MAC address not found. Broadcasting..." << endl;
            src->arp_cache[des->ip_address] = des->mac_address;
            cout << "Received ARP response from " << des->name << ": " << des->mac_address << endl;
        }

        Switch* sw = src->connected_switch;
        sw->learn_address(src->mac_address, src->port);
        sw->learn_address(des->mac_address, des->port);

        cout << src->name << " (" << src->ip_address << ") is sending data to "
             << des->name << " (" << des->ip_address << ")" << endl;
        cout << "Message: " << final_message << endl;

        CSMA_CD cd;
        cd.transmit(final_message);
        sw->forward_frame(src->mac_address, des->mac_address, final_message);
    } else {
        cout << "Communication across different networks" << endl;

        // Step 1: ARP for source gateway
        string gateway_ip = src->gateway;
        if (src->arp_cache.find(gateway_ip) == src->arp_cache.end()) {
            cout << "ARP request for gateway " << gateway_ip << ": MAC not found. Broadcasting..." << endl;
            for (auto dev : all_devices) {
                if (dev->ip_address == gateway_ip) {
                    src->arp_cache[gateway_ip] = dev->mac_address;
                    cout << "Received ARP response from " << dev->name << ": " << dev->mac_address << endl;
                    break;
                }
            }
        }

        Switch* sw1 = src->connected_switch;
        sw1->learn_address(src->mac_address, src->port);

        string gateway_mac = src->arp_cache[gateway_ip];
        int router_port = -1;
        for (auto it = sw1->mac_table.begin(); it != sw1->mac_table.end(); ++it) {
    string mac = it->first;
    int port = it->second;
    if (mac == gateway_mac) {
        router_port = port;
        break;
    }
}
        if (router_port != -1)
            sw1->learn_address(gateway_mac, router_port);

        cout << src->name << " is sending packet to gateway " << gateway_ip << " for forwarding..." << endl;
        CSMA_CD cd;
        cd.transmit(final_message);
        sw1->forward_frame(src->mac_address, gateway_mac, final_message);

        // Simulate router forwarding
        cout << "Router1 forwarding packet to Router2...\n";
        cout << "Router2 received packet for " << des->ip_address << endl;

        // Step 2: ARP for destination device
        if (des->arp_cache.find(des->ip_address) == des->arp_cache.end()) {
            cout << "ARP request for " << des->ip_address << ": MAC not in cache. Broadcasting..." << endl;
            des->arp_cache[des->ip_address] = des->mac_address;
            cout << "Received ARP response from " << des->name << ": " << des->mac_address << endl;
        }

        // Step 3: Destination switch learns MACs
        Switch* sw2 = des->connected_switch;
        Device* router2_iface = nullptr;
        for (auto dev : all_devices) {
            if (dev->ip_address == des->gateway) {
                router2_iface = dev;
                break;
            }
        }

        if (router2_iface) {
            sw2->learn_address(router2_iface->mac_address, router2_iface->port);
        }
        sw2->learn_address(des->mac_address, des->port);

        cout << "Final transmission from Router2 to " << des->name << endl;
        cd.transmit(final_message);
        sw2->forward_frame(router2_iface->mac_address, des->mac_address, final_message);
    }
}


int main() {
    // Create Devices
    vector<Device*> all_devices;
     Device device1("Device1", "192.168.1.2", "00:1A:2B:3C:4D:5A");
    device1.set_subnet_mask("255.255.255.0");
    device1.set_gateway("192.168.1.1");
    device1.display_info();

    Device device2("Device2", "192.168.1.3", "00:1A:2B:3C:4D:5B");
    device2.set_subnet_mask("255.255.255.0");
    device2.set_gateway("192.168.1.1");
        device2.display_info();

    // Create Devices in Network 2 (connected to Router2 via Switch2)
    Device device3("Device3", "192.168.2.2", "00:1A:2B:3C:4D:5C");
    device3.set_subnet_mask("255.255.255.0");
    device3.set_gateway("192.168.2.1");
        device3.display_info();

    Device device4("Device4", "192.168.2.3", "00:1A:2B:3C:4D:5D");
    device4.set_subnet_mask("255.255.255.0");
    device4.set_gateway("192.168.2.1");
        device4.display_info();

    // Create router interfaces as Devices (for ARP support)
    Device router1_iface1("Router1-eth0", "192.168.1.1", "00:1A:2B:3C:4D:01");
    Device router2_iface1("Router2-eth0", "192.168.2.1", "00:1A:2B:3C:4D:02");

    // Add all devices to global list
    all_devices.push_back(&device1);
    all_devices.push_back(&device2);
    all_devices.push_back(&device3);
    all_devices.push_back(&device4);
    all_devices.push_back(&router1_iface1);
    all_devices.push_back(&router2_iface1);

    // Create Routers
    Router router1("Router1");
    Router router2("Router2");

    // Add static routes
    router1.add_route("192.168.2.0", "192.168.3.2");
    router1.add_route("192.168.3.0");
    router1.add_route("192.168.1.0");

    router2.add_route("192.168.2.0");
    router2.add_route("192.168.3.0");
    router2.add_route("192.168.1.0", "192.168.3.1");

    // Display routing tables
    router1.display_routing_table();
    router2.display_routing_table();

    // Create Switches
    Switch switch1;
    Switch switch2;

    // Connect Devices to Switches
    switch1.connect_device(&device1, 1);
    switch1.connect_device(&device2, 2);
    switch1.connect_device(&router1_iface1, 3);  // Router1 interface as device

    switch2.connect_device(&device3, 1);
    switch2.connect_device(&device4, 2);
    switch2.connect_device(&router2_iface1, 3);  // Router2 interface as device

    // Connect Routers to Switches (for info display only)
   // switch1.connect_router(&router1);
    //switch2.connect_router(&router2);

    // Switch learns MAC addresses
    switch1.learn_address(device1.mac_address, device1.port);
    switch1.learn_address(device2.mac_address, device2.port);
    switch1.learn_address(router1_iface1.mac_address, router1_iface1.port);

    switch2.learn_address(device3.mac_address, device3.port);
    switch2.learn_address(device4.mac_address, device4.port);
    switch2.learn_address(router2_iface1.mac_address, router2_iface1.port);
    device1.connected_switch=&switch1;
    device2.connected_switch=&switch1;
    device3.connected_switch=&switch2;
    device4.connected_switch=&switch2;

    // Perform ARP Requests
    cout << "\n--- ARP Request Simulation ---\n";
    device1.arp_request("192.168.1.3", all_devices); // Device2
    device2.arp_request("192.168.1.1", all_devices); // Router1 iface
    device3.arp_request("192.168.2.3", all_devices); // Device4
    device4.arp_request("192.168.2.1", all_devices); // Router2 iface

    // Print MAC tables
    switch1.print_mac_table();
    switch2.print_mac_table();
    send_packet(&device1,&device4,"he",all_devices);

    return 0;
}
