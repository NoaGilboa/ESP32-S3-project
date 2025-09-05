#ifndef DUALLOGGER_H
#define DUALLOGGER_H

#include <FS.h>
#include <vector>
#include <Arduino.h>
#include "Measurement.h"

enum class Mode {
    ONLINE,
    OFFLINE
};

class DualLogger {
public:
    DualLogger(fs::FS &fs, const String &filename, Mode mode = Mode::OFFLINE);

    bool begin(bool resetFile = false);
    void log(const String &line);
    void log(const Measurement &m);
    void flush();
    void setPath(const char *path);
    
    String getFilename() const;  // ← Getter ציבורי
    void writeHeaderIfNeeded();  // נפתח לשימוש חיצוני
    std::vector<float> getColumnFloat(int colIndex); // ✔️ לצורך ניתוחים

private:
    fs::FS &_fs;
    String _filename;
    File _file;
    Mode _mode;
    bool _headerWritten;
};

#endif // DUALLOGGER_H
