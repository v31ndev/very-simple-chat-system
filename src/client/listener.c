#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

int main(void) {
    
    if(enet_initialize() != 0) {
        fprintf(stderr, "Failed to initialize ENet. Exiting program...\n");
        return 1;
    }

    char serverIP[64];
    printf("Enter a valid IPv4 address: ");
    fgets(serverIP, sizeof(serverIP), stdin);
    serverIP[strcspn(serverIP, "\n")] = '\0';

    ENetAddress address;
    if(enet_address_set_host_ip(&address, serverIP) != 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        return 1;
    }
    address.port = 2256;

    ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);
    if(client == NULL) {
        fprintf(stderr, "Error initializing core client component. Exiting program...\n");
        return 1;
    }

    ENetPeer *peer = enet_host_connect(client, &address, 2, 0);
    if(peer == NULL) {
        fprintf(stderr, "Failed to create peer. Exiting program...\n");
        return 1;
    }

    ENetEvent event;
    while(1) {
        while(enet_host_service(client, &event, 20) > 0) {
            switch(event.type) {
                case ENET_EVENT_TYPE_CONNECT : {
                    printf("Connected to server %s on port %u.\n\n", serverIP, event.peer->address.port);

                    ENetPacket *packet = enet_packet_create("LISTENER:X", strlen("LISTENER:X") + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(peer, 0, packet);

                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE : {
                    printf("%s\n", (char *) event.packet->data);
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT : {
                    printf("Disconnected from server.\n");
                    break;
                }
            }
        }
    }

    enet_peer_reset(peer);
    enet_host_destroy(client);
    enet_deinitialize();
    return 0;
}