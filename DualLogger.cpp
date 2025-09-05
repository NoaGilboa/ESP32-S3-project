#include "DualLogger.h"
#include "Measurement.h"

DualLogger::DualLogger(fs::FS &fs, const String &filename, Mode mode)
    : _fs(fs), _filename(filename), _mode(mode), _headerWritten(false) {}

bool DualLogger::begin(bool resetFile) {
    if (resetFile) {
        _fs.remove(_filename);
    }

    _file = _fs.open(_filename, FILE_APPEND);
    if (!_file) {
        Serial.println("❌ Failed to open log file.");
        return false;
    }

    writeHeaderIfNeeded();
    return true;
}

void DualLogger::log(const String &line) {
    if (!_file) return;
    _file.println(line);
    _file.flush();
}

void DualLogger::log(const Measurement &m) {
    if (!_file) return;

    String line = String(m.timestamp) + "," +
                  String(m.distance, 3) + "," +
                  String(m.speed, 3) + "," +
                  String(m.handPressureL, 2) + "," +
                  String(m.handPressureR, 2) + "," +
                  String(m.footLiftL) + "," +
                  String(m.footLiftR);

    _file.println(line);
    _file.flush();
}

void DualLogger::flush() {
    if (_file) _file.flush();
}

void DualLogger::setPath(const char *path) {
    _filename = String(path);
}
String DualLogger::getFilename() const {
    return _filename;
}


void DualLogger::writeHeaderIfNeeded() {
    if (_headerWritten || !_file) return;

    _file.println("timestamp_ms,distance_m,speed_mps,handL_kg,handR_kg,liftsL,liftsR");
    _headerWritten = true;
}

std::vector<float> DualLogger::getColumnFloat(int colIndex) {
    std::vector<float> values;
    File f = _fs.open(_filename, FILE_READ);
    if (!f) return values;

    if (f.available()) f.readStringUntil('\n'); // דלג על header

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.isEmpty()) continue;

        int start = 0;
        for (int i = 0; i <= colIndex; ++i) {
            int comma = line.indexOf(',', start);
            if (comma == -1 && i < colIndex) {
                start = -1;
                break;
            }
            if (i == colIndex) {
                String valStr = (comma == -1) ? line.substring(start) : line.substring(start, comma);
                valStr.trim();
                values.push_back(valStr.toFloat());
                break;
            }
            start = comma + 1;
        }
    }

    f.close();
    return values;
}
