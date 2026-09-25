
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

  const fetchData = async () => {
    try {
      const response = await fetch(API_URL);

      if (!response.ok) {
        throw new Error("API error");
      }

      const result = await response.json();
      setData(result);
    } catch (error) {
      console.log("API connection error:", error);
    }
  };

  useEffect(() => {
    fetchData();

    const interval = setInterval(fetchData, 1000);

    return () => clearInterval(interval);
  }, []);

  const cards = [
    {
      title: "Voltage",
      value: `${Number(data.voltage || 0).toFixed(1)} V`,
      icon: "⚡",
      className: "voltage",
    },
    {
      title: "PZEM Current",
      value: `${Number(data.pzem_current || 0).toFixed(2)} A`,
      icon: "🔌",
      className: "pzem",
    },
    {
      title: "ACS712 Current",
      value: `${Number(data.acs712_current || 0).toFixed(2)} A`,
      icon: "📊",
      className: "acs",
    },
    {
      title: "Power",
      value: `${Number(data.power || 0).toFixed(1)} W`,
      icon: "🔋",
      className: "power",
    },
    {
      title: "Energy",
      value: `${Number(data.energy || 0).toFixed(2)} kWh`,
      icon: "💡",
      className: "energy",
    },
    {
      title: "Frequency",
      value: `${Number(data.frequency || 0).toFixed(2)} Hz`,
      icon: "〰️",
      className: "frequency",
    },
    {
      title: "Power Factor",
      value: Number(data.power_factor || 0).toFixed(2),
      icon: "📈",
      className: "pf",
    },
    {
      title: "SSR",
      value: data.ssr ? "ON" : "OFF",
      icon: "🔘",
      className: data.ssr ? "ssr-on" : "ssr-off",
    },
  ];

  return (
    <div style={styles.page}>
      <div style={styles.container}>

        <h1 style={styles.heading}>
          ⚡ Energy Changeover Dashboard
        </h1>

        <div style={styles.statusBox}>
          <span style={styles.statusLabel}>System Status:</span>

          <span
            style={{
              ...styles.statusValue,
              background:
                data.system_status === "NORMAL"
                  ? "#16a34a"
                  : "#f59e0b",
            }}
          >
            {data.system_status || "WAITING FOR ESP32"}
          </span>
        </div>

        <div style={styles.grid}>
          {cards.map((card) => (
            <div
              key={card.title}
              style={{
                ...styles.card,
                ...cardStyles[card.className],
              }}
            >
              <div style={styles.cardIcon}>{card.icon}</div>

              <div style={styles.cardTitle}>
                {card.title}
              </div>

              <div style={styles.cardValue}>
                {card.value}
              </div>
            </div>
          ))}
        </div>

        <div style={styles.updateBox}>
          <strong>Last Update:</strong>{" "}
          {data.updated
            ? new Date(data.updated).toLocaleString()
            : "Waiting for data..."}
        </div>

      </div>
    </div>
  );
}

const styles = {
  page: {
    minHeight: "100vh",
    background: "#f1f5f9",
    padding: "30px 15px",
    fontFamily: "Arial, sans-serif",
    boxSizing: "border-box",
  },

  container: {
    maxWidth: "1100px",
    margin: "auto",
  },

  heading: {
    textAlign: "center",
    color: "#111827",
    marginBottom: "25px",
    fontSize: "32px",
  },

  statusBox: {
    background: "#ffffff",
    borderRadius: "12px",
    padding: "18px",
    marginBottom: "25px",
    boxShadow: "0 3px 10px rgba(0,0,0,0.10)",
    textAlign: "center",
    fontSize: "20px",
  },

  statusLabel: {
    fontWeight: "bold",
    marginRight: "10px",
  },

  statusValue: {
    color: "white",
    padding: "6px 15px",
    borderRadius: "20px",
    fontWeight: "bold",
  },

  grid: {
    display: "grid",
    gridTemplateColumns: "repeat(auto-fit, minmax(220px, 1fr))",
    gap: "20px",
  },

  card: {
    borderRadius: "16px",
    padding: "25px",
    minHeight: "150px",
    boxSizing: "border-box",
    boxShadow: "0 5px 15px rgba(0,0,0,0.15)",
    transition: "transform 0.2s",
  },

  cardIcon: {
    fontSize: "32px",
    marginBottom: "10px",
  },

  cardTitle: {
    fontSize: "18px",
    fontWeight: "bold",
    color: "#1f2937",
    marginBottom: "12px",
  },

  cardValue: {
    fontSize: "30px",
    fontWeight: "bold",
    color: "#111827",
  },

  updateBox: {
    marginTop: "25px",
    background: "#ffffff",
    padding: "15px",
    borderRadius: "10px",
    textAlign: "center",
    color: "#475569",
    boxShadow: "0 3px 10px rgba(0,0,0,0.08)",
  },
};

const cardStyles = {
  voltage: {
    background: "#dbeafe",
    borderLeft: "7px solid #2563eb",
  },

  pzem: {
    background: "#dcfce7",
    borderLeft: "7px solid #16a34a",
  },

  acs: {
    background: "#ffedd5",
    borderLeft: "7px solid #ea580c",
  },

  power: {
    background: "#fef3c7",
    borderLeft: "7px solid #d97706",
  },

  energy: {
    background: "#ede9fe",
    borderLeft: "7px solid #7c3aed",
  },

  frequency: {
    background: "#cffafe",
    borderLeft: "7px solid #0891b2",
  },

  pf: {
    background: "#fce7f3",
    borderLeft: "7px solid #db2777",
  },

  "ssr-on": {
    background: "#dcfce7",
    borderLeft: "7px solid #16a34a",
  },

  "ssr-off": {
    background: "#fee2e2",
    borderLeft: "7px solid #dc2626",
  },
};

export default App;