// CommandClient.h
#pragma once
#include <Arduino.h>

struct Command {
  String command;   // "start" / "stop"
  String patientId;
  bool valid = false;
};

class CommandClient {
public:
  CommandClient(const String& url): _url(url) {}
  Command poll(); // GET מהשרת
private:
  String _url;
};
