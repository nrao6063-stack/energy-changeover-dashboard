from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime,timezone

app = Flask(__name__)
CORS(app)

# =====================================================
# LATEST ESP32 DATA
# =====================================================

latest_data = {
    "voltage": 0.0,
    "pzem_current": 0.0,
    "acs712_current": 0.0,
    "power": 0.0,
    "energy": 0.0,
    "frequency": 0.0,
    "power_factor": 0.0,

    # Environment
    "temp": 0.0,
    "humidity": 0.0,
    "vibration": "NORMAL",

    # Changeover
    "ssr": False,
    "system_status": "NORMAL",

    "updated": ""
}


# =====================================================
# ESP32 → FLASK
# =====================================================

@app.route("/api/data", methods=["POST"])
def receive_data():

    global latest_data

    try:

        data = request.get_json()

        if not data:

            return jsonify({
                "status": "error",
                "message": "No JSON data received"
            }), 400

        print()
        print("====================================")
        print("ESP32 DATA RECEIVED")
        print("====================================")
        print(data)

        # =================================================
        # ELECTRICAL DATA
        # =================================================

        latest_data["voltage"] = float(
            data.get("voltage", 0)
        )

        latest_data["pzem_current"] = float(
            data.get("pzem_current", 0)
        )

        latest_data["acs712_current"] = float(
            data.get("acs712_current", 0)
        )

        latest_data["power"] = float(
            data.get("power", 0)
        )

        latest_data["energy"] = float(
            data.get("energy", 0)
        )

        latest_data["frequency"] = float(
            data.get("frequency", 0)
        )

        latest_data["power_factor"] = float(
            data.get("power_factor", 0)
        )

        # =================================================
        # TEMPERATURE
        # Accept both "temp" and "temperature"
        # =================================================

        latest_data["temp"] = float(
            data.get(
                "temp",
                data.get("temperature", 0)
            )
        )

        # =================================================
        # HUMIDITY
        # =================================================

        latest_data["humidity"] = float(
            data.get("humidity", 0)
        )

        # =================================================
        # VIBRATION
        # =================================================

        latest_data["vibration"] = str(
            data.get(
                "vibration",
                "NORMAL"
            )
        )

        # =================================================
        # SSR
        # =================================================

        latest_data["ssr"] = bool(
            data.get("ssr", False)
        )

        # =================================================
        # SYSTEM STATUS
        # =================================================

        latest_data["system_status"] = str(
            data.get(
                "system_status",
                "NORMAL"
            )
        )

        # =================================================
        # TIME
        # =================================================

        latest_data["updated"] = (
            datetime.now().isoformat()
        )

        print()
        print("STORED DATA:")
        print(latest_data)

        print("====================================")

        return jsonify({
            "status": "success",
            "message": "ESP32 data received"
        }), 200

    except Exception as e:

        print("ERROR:", e)

        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500


# =====================================================
# MOBILE APP → GET DATA
# =====================================================

@app.route("/api/data", methods=["GET"])
def get_data():

    return jsonify(latest_data), 200


# =====================================================
# HEALTH CHECK
# =====================================================

@app.route("/", methods=["GET"])
def home():

    return jsonify({
        "status": "online",
        "message": "Energy Changeover Flask API is running"
    })


# =====================================================
# RUN
# =====================================================

if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )