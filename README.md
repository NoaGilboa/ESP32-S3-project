# Mobile Gait Lab (WalkingLab)

## 📌 Overview
Mobile Gait Lab (WalkingLab) is a **portable, cost-effective system for gait monitoring and rehabilitation**.  
It combines **IoT sensors (ESP32-based controllers, load cells, distance sensors, and ESP32-CAM)** with a **cloud backend (Node.js + Azure SQL + Blob Storage)** and a **React.js frontend for physiotherapists**.  

The system enables:
- Continuous gait monitoring and walking stability analysis.
- Real-time video and sensor data collection.
- AI-driven treatment recommendations (via OpenAI API).
- Patient management, history tracking, and PDF report exports.

---

## ⚙️ Installation & Setup

### Backend (Node.js + Express)
```bash
git clone https://github.com/NoaGilboa/MobileWalkingLab_server.git
cd MobileWalkingLab_server
npm install
cp .env.example .env   # configure DB + Azure + OpenAI API keys
npm start
```

### Frontend (React.js)
```bash
cd MobileWalkingLab_client
npm install
npm start
```

### ESP32 Devices
- Flash code to `Seeed Studio XIAO ESP32-S3 Sense` (camera) and ESP32 controllers (sensors).
- Configure Wi-Fi credentials and server API endpoint in source code.
- Devices poll server every few seconds for **commands** (`start-session`, `stop-session`).

---

## 🔄 Typical Workflow

1. **Therapist Login** → via React frontend.  
2. **Patient Selection** → Search by name/ID, view history.  
3. **Start Measurement** → Therapist sends command from UI → Server → ESP32 device.  
4. **Data Collection**  
   - ESP32 sends walking metrics (speed, foot lifts, distance, hand pressure).  
   - ESP32-CAM streams/upload videos in real-time to Azure Blob Storage.  
5. **Data Storage**  
   - Sensor data → Azure SQL DB.  
   - Videos → Azure Blob Storage, linked by `measurement_id`.  
6. **Visualization**  
   - React frontend renders charts (speed, steps, distance, pressure).  
   - Clicking on chart points opens video popup if available.  
7. **AI Analysis**  
   - Server sends metrics to GPT → receives treatment recommendations.  
   - Therapist can edit recommendations.  
8. **Reporting**  
   - Export session (charts + recommendations + videos) to PDF.  

---

## 🖥️ System Architecture

```
                 ┌─────────────────────────┐
                 │ React Frontend (Therap.) │
                 │  - Login/Register        │
                 │  - Patient Dashboard     │
                 │  - Charts + Video Popup  │
                 └─────────────┬───────────┘
                               │ REST API
                               ▼
┌───────────────────────────────────────────────────┐
│              Node.js + Express Backend             │
│  - Patient & Therapist APIs                        │
│  - Commands API (/device/command)                  │
│  - Video Upload & Streaming (FFmpeg transcoding)   │
│  - GPT Integration (AI recommendations)            │
│  - SQL Database + Azure Blob connectors            │
└───────────────────────┬───────────────────────────┘
                        │
        ┌───────────────┴───────────────┐
        │                               │
        ▼                               ▼
┌─────────────────┐            ┌────────────────────┐
│ Azure SQL DB    │            │ Azure Blob Storage │
│ - Patients      │            │ - Raw Videos       │
│ - Sessions      │            │ - Processed MP4    │
│ - Metrics       │            │ - Linked by ID     │
└─────────────────┘            └────────────────────┘
        ▲
        │
        │ Commands & Data
        ▼
┌────────────────────────────┐
│ ESP32 Devices (IoT)        │
│  - Load Cells + HX711      │
│  - Distance Sensor (VL53L1X)│
│  - ESP32-CAM (video)       │
│ Poll server & upload data   │
└────────────────────────────┘
```

---

## ✨ Key Features / Functions

### Backend
- `POST /api/patients` → Add new patient.  
- `GET /api/patients/:id` → Fetch patient details.  
- `POST /api/video/:id/upload-video` → Upload patient video (MP4/AVI).  
- `GET /api/video/by-measurement/:id` → Fetch video by measurement.  
- `GET /api/device/command` → ESP32 polling endpoint.  
- AI Recommendations → `generateTreatmentPlan(metrics)`.

### Frontend
- Patient Dashboard → charts with walking metrics.  
- Video Popup → play synced video from Azure.  
- PDF Export → includes charts, notes, and AI plan.  
- Therapist Notes → editable treatment logs.

### ESP32 Firmware
- Polling loop to fetch command (`idle` / `start` / `stop`).  
- Capture frames from camera → stream/upload.  
- Collect metrics:  
  - Speed (distance sensor).  
  - Foot lifts (pressure sensors).  
  - Hand pressure (load cells).  
- Send data as JSON to backend.

---

## 🧪 Testing & QA
- **Unit Tests**: JUnit for backend APIs.  
- **Integration Tests**: ESP32 mock clients simulate device behavior.  
- **Frontend Tests**: React Testing Library.  
- **Load Testing**: Verify Azure handles concurrent patients.  
- **Video Streaming QA**: FFmpeg ensures MP4 with moov atom for browser compatibility.  

---

## 📡 Cloud Integration
- **Azure SQL Database**: Stores patients, sessions, metrics.  
- **Azure Blob Storage**: Stores videos/images, accessible via SAS tokens.  
- **Azure App Service**: Hosts Node.js backend.  
- **CI/CD**: GitHub Actions deploy backend/frontend to Azure.  

---

## 📜 License
MIT License © 2025 Mobile Walking Lab Team
