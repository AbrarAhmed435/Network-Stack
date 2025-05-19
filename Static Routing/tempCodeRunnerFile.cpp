// Create Router
    Router router("Router1");
    router.add_route("192.168.2.0", "192.168.1.1");
    router.add_route("192.168.3.0", "192.168.1.2");

    // Create RIP
    RIP rip;
    rip.exchange_routing_info(router);

    // ARP Request
    device1.arp_request("192.168.1.3");
    device2.arp_request("192.168.1.2");

    // Simulating static routing (simple decision)
    router.static_route("192.168.2.0");
    router.static_route("192.168.1.0");

    // Simulate Routing (Static Routing)
    router.static_route("192.168.1.3");

    // Simulate RIP Route Updates
    router.display_routing_table();