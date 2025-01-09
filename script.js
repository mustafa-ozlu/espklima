
        // Firebase yapılandırması
        const firebaseConfig = {
		apiKey: "AIzaSyDMjwPzCrOkrW4ARknWoUeD5HPMKjO2Rxw",
  		authDomain: "klima-hvl.firebaseapp.com",
  		databaseURL: "https://klima-hvl-default-rtdb.europe-west1.firebasedatabase.app",
  		projectId: "klima-hvl",
  		storageBucket: "klima-hvl.firebasestorage.app",
  		messagingSenderId: "560581085174",
  		appId: "1:560581085174:web:d742b22a031937211f29d7",
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


city = "Istanbul";
apikey = "6040d630cdeed286ab0cb1a2da7ed9b1";
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
//  console.log(iconurl);
//  document.getElementById("icourl").innerHTML = iconurl;
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
