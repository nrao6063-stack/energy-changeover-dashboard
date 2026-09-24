from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime

app = Flask(__name__)
CORS(app)

# Latest ESP32 data
latest_data = {
    "voltage": 0,
    "pzem_current": 0,
    "acs712_current": 0,
    "power": 0,
    "energy": 0,
    "frequency": 0,
    "power_factor": 0,
    "ssr": False,
    "system_status": "WAITING FOR ESP32",
    "updated": None
}


@app.route("/")
def home():
    return "Energy Changeover Flask API is running"


@app.route("/api/health", methods=["GET"])
def health():
    return jsonify({
        "status": "ok",
        "project": "energy-changeover-dashboard"
    })


@app.route("/api/data", methods=["GET"])
def get_data():
    return jsonify(latest_data)


@app.route("/api/data", methods=["POST"])
def receive_data():

    global latest_data

    data = request.get_json(silent=True)

    if not data:
        return jsonify({
            "status": "error",
            "message": "No JSON data received"
        }), 400

    latest_data = {
        "voltage": data.get("voltage", 0),
        "pzem_current": data.get("pzem_current", 0),
        "acs712_current": data.get("acs712_current", 0),
        "power": data.get("power", 0),
        "energy": data.get("energy", 0),
        "frequency": data.get("frequency", 0),
        "power_factor": data.get("power_factor", 0),
        "ssr": data.get("ssr", False),
        "system_status": data.get(
            "system_status",
            "UNKNOWN"
        ),
        "updated": datetime.now().isoformat()
    }

    return jsonify({
        "status": "success",
        "message": "ESP32 data received"
    })


if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )