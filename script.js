// ESP8266'nın IP adresini buraya yazın
const ESP8266_IP = "http://192.168.1.33";

document.addEventListener("DOMContentLoaded", () => {
  const btnOn = document.getElementById("btn-on");
  const btnOff = document.getElementById("btn-off");
  const statusElement = document.getElementById("device-status");

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
      const response = await fetch(`${ESP8266_IP}/control`, {
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
});
