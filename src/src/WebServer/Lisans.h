#ifndef WEBSERVER_WEBSERVER_LISANS_H
#define WEBSERVER_WEBSERVER_LISANS_H

#include "../WebServer/common.h"

void handle_lisans();

void handle_lisans_SERiALSERVER();
void handle_lisans_SERiALCLiENT();
void handle_lisans_SERiALTCPMODBUSSLAVE();
void handle_lisans_TCPMODBUSHRCMESAJ();
void handle_lisans_HRC();
void handle_lisans_FYZ();
void handle_lisans_FYZPLU();
void handle_lisans_EYZ();
void handle_lisans_EYZPLU();
void handle_lisans_IND();
void handle_lisans_BLE_CLiENT();
void handle_lisans_BLE_SERVER();

void appLisans(int key, String lisans_name);

#endif
