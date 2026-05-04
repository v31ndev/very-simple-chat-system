#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <enet/enet.h>

#define MAX_USERNAME_CHARS  15
#define MAX_MSG 512
#define PING_INTERVAL 3

typedef struct {
    ENetHost *client;
    ENetPeer *peer;
    bool running;
} NetData;

void* net_thread(void *arg) {
    NetData *data = (NetData *)arg;
    ENetEvent event;
    time_t lastPing = time(NULL);

    while (data->running) {
        // Handle network events
        while (enet_host_service(data->client, &event, 20) > 0) {
            switch(event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("\nDisconnected from server.\n");
                    data->running = 0;
                    break;
                default:
                    break;
            }
        }

        // Send periodic ping
        time_t now = time(NULL);
        if (difftime(now, lastPing) >= PING_INTERVAL) {
            ENetPacket *pingPacket = enet_packet_create("PING:", 6, 0);
            enet_peer_send(data->peer, 0, pingPacket);
            enet_host_flush(data->client);
            lastPing = now;
        }

        usleep(1000); // avoid busy-waiting
    }

    return NULL;
}

int main(void) {
    
    if(enet_initialize() != 0) {
        fprintf(stderr, "Failed to initialize ENet. Exiting program...\n");
    }
    
    char username[MAX_USERNAME_CHARS];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

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
    for(int i = 0; i < 3; i++) {
        printf("Connection attemp no. %d\n", i + 1);
        if(enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            char packetData[strlen(username) + strlen("CLIENT:") + 1];
            snprintf(packetData, sizeof(packetData), "CLIENT:%s", username);
            
            ENetPacket *packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(peer, 0, packet);

            enet_host_flush(client);

            printf("Connected to server %s on port %u.\nConnected as '%s'\n", serverIP, event.peer->address.port, username);
            break;
        }
        else {
            printf("Connection failed.\n");
            if(i == 2) return 1;
        }
    }

    printf("\n\n");

    NetData netData = {client, peer, true};
    pthread_t netThread;
    pthread_create(&netThread, NULL, net_thread, &netData);

    char msg[strlen("MSG:") + MAX_MSG];
    while(netData.running) {
        printf("%s >> ", username);
        snprintf(msg, sizeof(msg), "MSG:");

        if(fgets(msg + strlen(msg), sizeof(msg) - strlen(msg), stdin) == NULL) {
            break;
        }
        msg[strcspn(msg, "\n")] = '\0';

        ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    }

    netData.running = false;
    pthread_join(netThread, NULL);

    enet_peer_reset(peer);
    enet_host_destroy(client);
    enet_deinitialize();
    return 0;
}