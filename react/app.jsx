import React, { useEffect, useState } from "react";

const API_URL =
  "https://energy-changeover-dashboard.onrender.com/api/data";

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

  const [error, setError] = useState("");

  const getData = async () => {
    try {
      const response = await fetch(API_URL);

      if (!response.ok) {
        throw new Error("API connection failed");
      }

      const result = await response.json();
      setData(result);
      setError("");
    } catch (err) {
      console.error(err);
      setError("Unable to connect to Flask API");
    }
  };

  useEffect(() => {
    getData();

    const interval = setInterval(getData, 3000);

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="dashboard">
      <header className="header">
        <h1>⚡ Energy Changeover Dashboard</h1>
        <p>ESP32 • PZEM-004T • ACS712 • SSR</p>
      </header>

      {error && <div className="error">{error}</div>}

      <div className="status-card">
        <span>System Status</span>
        <strong>{data.system_status}</strong>
      </div>

      <div className="cards">

        <div className="card">
          <h2>Voltage</h2>
          <div className="value">
            {Number(data.voltage || 0).toFixed(1)}
            <span> V</span>
          </div>
        </div>

        <div className="card">
          <h2>PZEM Current</h2>
          <div className="value">
            {Number(data.pzem_current || 0).toFixed(2)}
            <span> A</span>
          </div>
        </div>

        <div className="card">
          <h2>ACS712 Current</h2>
          <div className="value">
            {Number(data.acs712_current || 0).toFixed(2)}
            <span> A</span>
          </div>
        </div>

        <div className="card">
          <h2>Power</h2>
          <div className="value">
            {Number(data.power || 0).toFixed(1)}
            <span> W</span>
          </div>
        </div>

        <div className="card">
          <h2>Energy</h2>
          <div className="value">
            {Number(data.energy || 0).toFixed(2)}
            <span> kWh</span>
          </div>
        </div>

        <div className="card">
          <h2>Frequency</h2>
          <div className="value">
            {Number(data.frequency || 0).toFixed(1)}
            <span> Hz</span>
          </div>
        </div>

        <div className="card">
          <h2>Power Factor</h2>
          <div className="value">
            {Number(data.power_factor || 0).toFixed(2)}
          </div>
        </div>

        <div className="card">
          <h2>SSR</h2>
          <div className={data.ssr ? "ssr-on" : "ssr-off"}>
            {data.ssr ? "ON" : "OFF"}
          </div>
        </div>

      </div>

      <div className="updated">
        Last update: {data.updated || "Waiting for ESP32"}
      </div>
    </div>
  );
}

export default App;