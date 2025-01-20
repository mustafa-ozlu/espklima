
        // Firebase yapılandırması
const firebaseConfig = {
		apiKey: "API KEY",
  		authDomain: "*.firebaseapp.com",
  		databaseURL: "https://*.europe-west1.firebasedatabase.app",
  		projectId: "*",
  		storageBucket: "*.firebasestorage.app",
  		messagingSenderId: "*",
  		appId: "1:*:web:*",
        };

        // Firebase'i başlat
        firebase.initializeApp(firebaseConfig);
        const database = firebase.database();

        // Sıcaklık ve nem değerlerini güncelle
        function updateData() {
            database.ref('/Sensor').on('value', (snapshot) => {
                const data = snapshot.val();
                document.getElementById('temperature').innerText = `${data.Sicaklik} °C` || '--';
                document.getElementById('humidity').innerText = `${data.Nem} % ` || '--';
            });
    firebase.database().ref("/KomutDurumu").on("value", (snapshot) => {
        const status = snapshot.val();
        if (status) {
            document.getElementById("status").innerText = "Durum: " + status;
        }
    });
	database.ref('/Derece').on('value', (snapshot) => {
                const derece= snapshot.val();
                document.getElementById('spinButton').value = derece;
                
            });

        }


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
function changeValue(delta) { 
   const spinButton = document.getElementById('spinButton'); 
   let value = parseInt(spinButton.value) || 18; value += delta; 
   if (value >= 18 && value <= 30) { 
	spinButton.value = value; 
	sendToFirebase(value); } } 
function sendToFirebase(value) { 
   firebase.database().ref('Derece').set(value)
	.then(() => { 
	   console.log(''); })
	.catch((error) => { 
	   console.error('Hata:', error); }); } 
document.getElementById('spinButton').addEventListener('input', function() { 
   const settemp = this.value; 
   sendToFirebase(settemp); });

updateData();

city = "Istanbul";
apikey = "*";
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
  var temp=Math.round(data.list[0].temp.day);
document.getElementById("opentemp").innerText =`${temp} °C`;
openwnem=data.list[0].humidity;
document.getElementById("openh").innerText = `${openwnem} %`;
  icon = data.list[0].weather[0].icon;
  let iconurl = "https://openweathermap.org/img/wn/" + icon + "@2x.png";
  icourl.src = iconurl;
  
//  document.getElementById("icourl").innerHTML = iconurl;
};
request.send();
