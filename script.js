
        // Firebase yapılandırması
        const firebaseConfig = {
		apiKey: "YOUR API KEY",
  		authDomain: "YOUR PROJECT.firebaseapp.com",
  		databaseURL: "https://YOUR DATABASE.firebasedatabase.app",
  		projectId: "YOUR PROJECT ID",
  		storageBucket: "YOUR PROJECT.firebasestorage.app",
  		messagingSenderId: "YOUR ID",
  		appId: "1:YOUR ID:web:YOUR WEB ID",
        };

        // Firebase'i başlat
        firebase.initializeApp(firebaseConfig);
        const database = firebase.database();

        // Sıcaklık ve nem değerlerini güncelle
        function updateData() {
            database.ref('/Sensor').on('value', (snapshot) => {
                const data = snapshot.val();
                document.getElementById('temperature').innerText = data.Sicaklik || '--';
                document.getElementById('humidity').innerText = data.Nem || '--';
            });
    firebase.database().ref("/Komut").on("value", (snapshot) => {
        const status = snapshot.val();
        if (status) {
            document.getElementById("status").innerText = "Durum: " + status;
        }
    });

        }


        // Veri güncellemelerini başlat
        updateData();

//OPENWEATHERMAP
city = "YOUR CITY";
apikey = "OPENWEATHER API KEY";
lang = "tr";
units = "metric";
link =
  "https://api.openweathermap.org/data/2.5/forecast/daily?q=" +
  city +
  "&appid=" +
  apikey +
  "&lang=" +
  lang +
  "&units=" +
  units;
var request = new XMLHttpRequest();
request.open("GET", link, true);
request.onload = function () {
  var data = JSON.parse(this.response);
  icon = data.list[0].weather[0].icon;
  let iconurl = "https://openweathermap.org/img/wn/" + icon + "@2x.png";
  icourl.src = iconurl;

};
request.send();



   function sendCommand(command) {
        // Komutu Firebase'e gönder
        firebase.database().ref("/Komut").set(command)
            .then(() => {
                document.getElementById("status").innerText = "Komut gönderildi: " + command;
            })
            .catch((error) => {
                document.getElementById("status").innerText = "Komut gönderilemedi: " + error.message;
            });
    }

    // Firebase'den durum bilgisini dinle
