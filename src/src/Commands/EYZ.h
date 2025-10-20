#ifndef COMMAND_EYZ_H
#define COMMAND_EYZ_H

class String;

String Command_Eyz_Test(struct EventStruct* event, const char* Line);
String Command_Eyz_Art(struct EventStruct* event, const char* Line);
String Command_Eyz_Tek(struct EventStruct* event, const char* Line);
String Command_Eyz_Serino(struct EventStruct* event, const char* Line);
String Command_Eyz_Sensor(struct EventStruct* event, const char* Line);
String Command_Eyz_Plu_Art(struct EventStruct* event, const char* Line);
String Command_Eyz_Yaz(struct EventStruct* event, const char* Line);
String Command_Eyz_Yaz_Satir(struct EventStruct* event, const char* Line);
String Command_Eyz_Net(struct EventStruct* event, const char* Line);
String Command_Eyz_tspl(struct EventStruct* event, const char* Line);

#endif  // COMMAND_EYZ_H