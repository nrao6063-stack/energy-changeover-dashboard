import React, { useEffect, useState } from "react";

const API_URL =
  "https://energy-changeover-flask.onrender.com/api/data";

function App() {
  const [data, setData] = useState({
    voltage: 0,
    pzem_current: 0,
    acs712_current: 0,
    power: 0,
    energy: 0,
    frequency: 0,
    power_factor: 0,
    ssr: false,
    system_status: "WAITING FOR ESP32",
    updated: null,
  });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch(API_URL);

        if (!response.ok) {
          throw new Error(`HTTP error: ${response.status}`);
        }

        const result = await response.json();
        setData(result);
      } catch (error) {
        console.error("Error fetching ESP32 data:", error);
      }
    };

    fetchData();

    const interval = setInterval(fetchData, 5000);

    return () => clearInterval(interval);
  }, []);

  return (
    <div>
      <h1>⚡ Energy Changeover Dashboard</h1>

      <p>
        System Status: <strong>{data.system_status}</strong>
      </p>

      <p>Voltage: {Number(data.voltage).toFixed(1)} V</p>

      <p>PZEM Current: {Number(data.pzem_current).toFixed(2)} A</p>

      <p>ACS712 Current: {Number(data.acs712_current).toFixed(2)} A</p>

      <p>Power: {Number(data.power).toFixed(1)} W</p>

      <p>Energy: {Number(data.energy).toFixed(2)} kWh</p>

      <p>Frequency: {Number(data.frequency).toFixed(2)} Hz</p>

      <p>Power Factor: {Number(data.power_factor).toFixed(2)}</p>

      <p>
        SSR:{" "}
        <strong>{data.ssr ? "ON" : "OFF"}</strong>
      </p>

      <p>Last update: {data.updated || "Waiting for ESP32..."}</p>
    </div>
  );
}

export default App;