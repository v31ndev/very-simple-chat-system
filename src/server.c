#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <enet/enet.h>

#define MAX_CLIENTS 20
#define MAX_USERNAME_CHARS  15

typedef struct {
    ENetPeer *peer;
    char username[MAX_USERNAME_CHARS];
} User;

int main(void) {
    
    if(enet_initialize() != 0) {
        fprintf(stderr, "Failed to initialize ENet. Exiting program...\n");
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 2256;

    ENetHost *server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

    if(server == NULL) {
        fprintf(stderr, "Failed to initialize server. Exiting program...\n");
        return 1;
    }

    User userList[MAX_CLIENTS];
    for(int i = 0; i < MAX_CLIENTS; i++) {
        userList[i].peer = NULL;
        userList[i].username[0] = '\0';
    }


    printf("Server is running on port %u!\n", address.port);
    ENetEvent event;
    while(1) {
        while(enet_host_service(server, &event, 20) > 0) {
            char clientIP[64];
            enet_address_get_host_ip(&event.peer->address, clientIP, sizeof(clientIP));
            
            switch(event.type) {
                case ENET_EVENT_TYPE_CONNECT : {
                    printf("Client/Listener has connected and is awaiting server validation/registration.\n");
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE : {
                    if(!strchr((char *)event.packet->data, ':')){
                        printf("Invalid packet from %s\n", clientIP);
                        enet_packet_destroy(event.packet);
                        break;
                    }

                    char* action = (char *) malloc(strcspn((char *)event.packet->data, ":") + 1);
                    if(action == NULL) {
                        printf("Error processing received data. Destroying packet and skipping...\n");
                        enet_packet_destroy(event.packet);
                        break;
                    }

                    char msg[512];

                    memcpy(action, event.packet->data, (int)strcspn((char *)event.packet->data, ":"));
                    action[strcspn((char *)event.packet->data, ":")] = '\0';
                    snprintf(msg, sizeof(msg), "%s", event.packet->data + strcspn(event.packet->data, ":") + 1);
                    
                    char result[MAX_USERNAME_CHARS + 4 + 512];
                    result[0] = '\0';

                    if(!strcmp(action, "LISTENER")) {
                        printf("Listener (%s) connected.\n", clientIP);
                        snprintf(result, sizeof(result), "Listener (%s) connected.", clientIP);
                    }
                    else if(!strcmp(action, "CLIENT")) {
                        for(int i = 0; i < MAX_CLIENTS; i++) {
                            if(userList[i].peer == NULL) {
                                userList[i].peer = event.peer;

                                strncpy(userList[i].username, msg, sizeof(userList[i].username) - 1);
                                userList[i].username[sizeof(userList[i].username)-1] = '\0';
                                break;
                            }
                        }
                        printf("Client '%s' (%s) connected.\n", msg, clientIP);
                        snprintf(result, sizeof(result), "Client '%s' (%s) connected.", msg, clientIP);
                    }
                    else if(!strcmp(action, "MSG")) {
                        char *username = "unknown";
                        for(int i = 0; i < MAX_CLIENTS; i++) {
                            if(userList[i].peer == event.peer) {
                                username = userList[i].username;
                                break;
                            }
                        }

                        snprintf(result , sizeof(result), "%s >> %.*s", username, (int)strlen(msg), msg);
                    }
                    else if(!strcmp(action, "PING")) {
                        for(int i = 0; i < MAX_CLIENTS; i++) {
                            if(userList[i].peer == event.peer) {
                                printf("Client '%s' (%s) pinged the server - probably to stay connected.\n", userList[i].username, clientIP);
                                break;
                            }
                        }
                    }
                    else {
                        printf("Client (%s) sent us an invalid packet.\n", clientIP);
                    }
                    
                    if(result[0] != '\0') {
                        ENetPacket *packet = enet_packet_create(result, strlen(result) + 1, ENET_PACKET_FLAG_RELIABLE);

                        enet_host_broadcast(server, 0, packet);
                    }

                    free(action);
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT : {
                    for(int i = 0; i < MAX_CLIENTS; i++) {
                        if(userList[i].peer == event.peer) {
                            char msg[256];
                            snprintf(msg, sizeof(msg), "Client '%s' (%s) disconnected.", userList[i].username, clientIP);
                            printf("%s\n", msg);

                            ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                            enet_host_broadcast(server, 0, packet);

                            userList[i] = (User){ NULL, "\0" };
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}