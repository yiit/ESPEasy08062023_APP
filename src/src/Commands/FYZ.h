#ifndef COMMAND_FYZ_H
#define COMMAND_FYZ_H

class String;

String Command_Fyz_Test(struct EventStruct *event, const char* Line);
String Command_Fyz_Plu_Art(struct EventStruct *event, const char* Line);
String Command_Fyz_Art(struct EventStruct *event, const char* Line);
String Command_Fyz_Top(struct EventStruct *event, const char* Line);
String Command_Fyz_Kopya(struct EventStruct *event, const char* Line);
String Command_Fyz_Yaz(struct EventStruct *event, const char* Line);
String Command_Fyz_Yaz_Satir(struct EventStruct *event, const char* Line);
String Command_Fyz_Net(struct EventStruct *event, const char* Line);
String Command_Fyz_Serino(struct EventStruct *event, const char* Line);
String Command_Fyz_Esc_Pos(struct EventStruct *event, const char* Line);

#endif // COMMAND_FYZ_H