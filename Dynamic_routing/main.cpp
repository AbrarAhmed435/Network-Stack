#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <climits>
using namespace std;

class RouteEntry {
public:
    string next_hop_ip;
    int hop_count;
    RouteEntry(string ip = "", int hop = INT_MAX) : next_hop_ip(ip), hop_count(hop) {}
};

class Router;

class Device {
public:
    string name;
    string ip_address;
    string mac_address;
    string gateway; // router IP
    Router* connected_router;

    Device(string name, string ip, string mac)
        : name(name), ip_address(ip), mac_address(mac), connected_router(NULL) {}

    void connect_router(Router* router, string gateway_ip);
};

class Router {
public:
    string name;
    string ip_address;
    map<string, RouteEntry> routing_table;
    vector<Router*> neighbors;
    vector<Device*> connected_devices;

    Router(string name, string ip) : name(name), ip_address(ip) {}

    void connect_neighbor(Router* neighbor) {
        neighbors.push_back(neighbor);
    }

    void connect_device(Device* dev) {
        connected_devices.push_back(dev);
        dev->connect_router(this, ip_address);

        string net = get_network(dev->ip_address);
        routing_table[net] = RouteEntry(dev->ip_address, 1);
    }

    void exchange_routes() {
        for (int i = 0; i < neighbors.size(); i++) {
            Router* neighbor = neighbors[i];
            for (map<string, RouteEntry>::iterator it = routing_table.begin(); it != routing_table.end(); ++it) {
                string dest_net = it->first;
                RouteEntry entry = it->second;
                int new_hops = entry.hop_count + 1;

                if (new_hops > 15) continue;

                if (neighbor->routing_table.find(dest_net) == neighbor->routing_table.end() ||
                    new_hops < neighbor->routing_table[dest_net].hop_count) {
                    neighbor->routing_table[dest_net] = RouteEntry(ip_address, new_hops);
                    cout << neighbor->name << " learned route to " << dest_net << " via " << ip_address << " (hops: " << new_hops << ")" << endl;
                }
            }
        }
    }

    void print_table() {
        cout << "Routing Table for " << name << ":\n";
        for (map<string, RouteEntry>::iterator it = routing_table.begin(); it != routing_table.end(); ++it) {
            cout << "Destination: " << it->first << ", Next Hop: " << it->second.next_hop_ip
                 << ", Hops: " << it->second.hop_count << endl;
        }
        cout << "--------------------------\n";
    }

    string get_next_hop(string dest_ip) {
        string dest_net = get_network(dest_ip);
        if (routing_table.find(dest_net) != routing_table.end()) {
            return routing_table[dest_net].next_hop_ip;
        }
        return "unreachable";
    }

    static string get_network(string ip) {
        size_t pos = ip.rfind('.');
        return ip.substr(0, pos) + ".0";
    }
};

void Device::connect_router(Router* router, string gateway_ip) {
    connected_router = router;
    gateway = gateway_ip;
}

void send_data(Device* src, Device* dst) {
    cout << "\nSending data from " << src->name << " to " << dst->name << "...\n";

    string dst_net = Router::get_network(dst->ip_address);
    string next_hop = src->connected_router->get_next_hop(dst->ip_address);

    if (next_hop == "unreachable") {
        cout << "Destination unreachable\n";
    } else {
        cout << "Next hop to " << dst->ip_address << ": " << next_hop << "\n";
        cout << "Data delivered to " << dst->name << "\n";
    }
}

int main() {
    Router router1("Router1", "192.168.1.1");
    Router router2("Router2", "192.168.2.1");
    Router router3("Router3", "192.168.3.1");

    router1.connect_neighbor(&router2);
    router2.connect_neighbor(&router1);
    router2.connect_neighbor(&router3);
    router3.connect_neighbor(&router2);

    Device dev1("Device1", "192.168.1.2", "00:1A:2B:3C:4D:01");
    Device dev2("Device2", "192.168.2.2", "00:1A:2B:3C:4D:02");
    Device dev3("Device3", "192.168.3.2", "00:1A:2B:3C:4D:03");

    router1.connect_device(&dev1);
    router2.connect_device(&dev2);
    router3.connect_device(&dev3);

    // Simulate periodic RIP updates
    for (int i = 0; i < 3; ++i) {
        cout << "\n--- RIP Round " << i+1 << " ---\n";
        router1.exchange_routes();
        router2.exchange_routes();
        router3.exchange_routes();
    }

    router1.print_table();
    router2.print_table();
    router3.print_table();

    send_data(&dev1, &dev3);
    send_data(&dev3, &dev2);

    return 0;
}
