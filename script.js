// ESP8266'nın IP adresini buraya yazın
const ESP8266_IP = "http://192.168.1.33";

document.addEventListener("DOMContentLoaded", () => {
  const btnOn = document.getElementById("btn-on");
  const btnOff = document.getElementById("btn-off");
  const statusElement = document.getElementById("device-status");
  const tempElement = document.getElementById("temperature");
  const humidityElement = document.getElementById("humidity");

  // Cihazı aç
  btnOn.addEventListener("click", async () => {
    try {
      const response = await fetch(`${ESP8266_IP}/control`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ action: "on" }),
      });
      const result = await response.json();
      statusElement.textContent = `Cihaz durumu: ${result.status}`;
    } catch (error) {
      statusElement.textContent = "Bağlantı hatası!";
    }
  });

  // Cihazı kapat
  btnOff.addEventListener("click", async () => {
    try {
      const response = await fetch(`https://api.thingspeak.com/channels/2801292/feeds.json?results=2`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ action: "off" }),
      });
      const result = await response.json();
      statusElement.textContent = `Cihaz durumu: ${result.status}`;
    } catch (error) {
      statusElement.textContent = "Bağlantı hatası!";
    }
  });

  // Sensör verilerini güncelle
  async function fetchSensorData() {
  
    const url = "https://api.thingspeak.com/channels/2801292/feeds.json?results=2";

    fetch(url)
      .then(response => {
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        return response.json();
      })
      .then(data => {
        const feeds = data.feeds;
    
        // İlk feed'in değerlerini al
        if (feeds.length > 0) {
          const firstFeed = feeds[0];
          const temp = firstFeed.field1;
          const humidity = firstFeed.field2;
    
          // HTML elementlerine değerleri yerleştir
          document.getElementById("tempElement").textContent = temp || "N/A";
          document.getElementById("humidityElement").textContent = humidity || "N/A";
        } else {
          console.warn("No feeds found in the data.");
        }
      })
      .catch(error => {
        console.error('Error fetching data:', error);
        document.getElementById("tempElement").textContent = "Error";
        document.getElementById("humidityElement").textContent = "Error";
      });
    

  // Sensör verilerini her 5 saniyede bir güncelle
  setInterval(fetchSensorData, 5000);
  fetchSensorData();
);
