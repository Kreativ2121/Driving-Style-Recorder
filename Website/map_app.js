//Map definitions
let map = L.map('map').setView([53.4486, 14.4911], 18);

var Jawg_Streets = L.tileLayer('https://{s}.tile.jawg.io/jawg-streets/{z}/{x}/{y}{r}.png?access-token={accessToken}', {
	attribution: '<a href="http://jawg.io" title="Tiles Courtesy of Jawg Maps" target="_blank">&copy; <b>Jawg</b>Maps</a> &copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>',
	minZoom: 5,
	maxZoom: 22,
	subdomains: 'abcd',
	accessToken: 'I0qAyRbfORBQ0cGt8JRiYOuGug9TBOfVzdQXIFw4MwTnSAj5zoR9VepiON96MScV'
});
Jawg_Streets.addTo(map);

//Icon definitions
var redIcon = new L.Icon({
    iconUrl: "leaflet-1.7.1/images/marker-icon-red.png",
    shadowUrl: "leaflet-1.7.1/images/marker-shadow.png",
    iconSize: [25, 41],
    iconAnchor: [12, 41],
    popupAnchor: [1, -34],
    shadowSize: [41, 41]
});

var orangeIcon = new L.Icon({
    iconUrl: "leaflet-1.7.1/images/marker-icon-orange.png",
    shadowUrl: "leaflet-1.7.1/images/marker-shadow.png",
    iconSize: [25, 41],
    iconAnchor: [12, 41],
    popupAnchor: [1, -34],
    shadowSize: [41, 41]
});

var blueIcon = new L.Icon({
    iconUrl: "leaflet-1.7.1/images/marker-icon.png",
    shadowUrl: "leaflet-1.7.1/images/marker-shadow.png",
    iconSize: [25, 41],
    iconAnchor: [12, 41],
    popupAnchor: [1, -34],
    shadowSize: [41, 41]
});

//Other definitions
let myLocation = document.querySelector('#get_location');
let start = document.querySelector('#start');
let text = document.querySelector('#user_text');
var st = [];
var lat = [];
var lon = [];
var speed = [];
var rpm = [];
var counter = 1;

//Getting device coordinates and printing them on the map
myLocation.addEventListener('click', function (event) {
    event.preventDefault();
   
    if( ! navigator.geolocation){
       console.log("No geolocation.");
   }

   navigator.geolocation.getCurrentPosition(position =>{
       console.log(position);
       let lat = position.coords.latitude;
       let lon = position.coords.longitude;
       
       map.setView([lat, lon]);
   }, positionError => {
       console.error(positionError);
   });
});

//Draw road
function drawRoad(wpoints, roadColour) {
    counter = counter - 1;
    let Control = L.Routing.control({
        waypoints: wpoints,
        routeWhileDragging: true,
        draggableWaypoints: false,
        //fitSelectedRoutes: "smart",
        //createMarker: function() { return null; },
        createMarker: function(i, wp, nWps) {
            if (speed[counter] >= 70 || rpm[counter] >= 4500) {
              counter = counter + 1;
              return L.marker(wp.latLng, {
                icon: redIcon
              }).bindPopup("<b>Checkpoint</b><br>Speed: " + speed[counter-1] + " km/h<br>RPM: " + rpm[counter-1]);
            } else if(speed[counter] >= 50 || rpm[counter] >= 3500){
              counter = counter + 1;
              return L.marker(wp.latLng, {
                icon: orangeIcon
              }).bindPopup("<b>Checkpoint</b><br>Speed: " + speed[counter-1] + " km/h<br>RPM: " + rpm[counter-1]);
            } else {
                counter = counter + 1;
                return L.marker(wp.latLng, {
                icon: blueIcon
              }).bindPopup("<b>Checkpoint</b><br>Speed: " + speed[counter-1] + " km/h<br>RPM: " + rpm[counter-1]);
            }
          },
        lineOptions : {
            styles: [{color: 'black', opacity: 0.15, weight: 9}, {color: 'white', opacity: 0.8, weight: 6}, {color: roadColour, opacity: 1, weight: 2}],
            addWaypoints: false
        }
    }).addTo(map);
    Control.hide();
}

//Map data parser
start.addEventListener('click', function (event) {
    event.preventDefault();

    if (!text.value.length) {
        console.log("No data has been put in the text box.")
    } else {
        //Processing data to separate different types of data and create waypoints from them
        processData(text.value);
        let points = createWaypoints();

        //Gather information about the first localisation
        const firstValid = st.findIndex(e => e == 1);
        let lastLoc = L.latLng(lat[firstValid], lon[firstValid]);
        let lastChoice = 0;
        if (speed[firstValid] >= 70 || rpm[firstValid] >= 3500){
            lastChoice = 1;
        } else if (speed[firstValid] >= 50 || rpm[firstValid] >= 2800){
            lastChoice = 2;
        } else {
            lastChoice = 3;
        }

        //Loop over all the data and separate different behaviors during driving
        let choice = 0;
        let tmpPoints = [];
        tmpPoints.push(lastLoc);
        for (var i=1; i<points.length; i++) {
            let loc = L.latLng(lat[i], lon[i]);
            tmpPoints.push(loc);

            if (speed[i] >= 70 || rpm[i] >= 3500){
                choice = 1;
            } else if (speed[i] >= 50 || rpm[i] >= 2800){
                choice = 2;
            } else {
                choice = 3;
            }

            if(lastChoice != choice){
                if(lastChoice == 1){
                    drawRoad(tmpPoints,'red');
                    tmpPoints = [];
                    tmpPoints.push(loc);
                }
                else if(lastChoice == 2){
                    drawRoad(tmpPoints,'orange');
                    tmpPoints = [];
                    tmpPoints.push(loc);
                }
                else{
                    drawRoad(tmpPoints,'blue');
                    tmpPoints = [];
                    tmpPoints.push(loc);
                }
            }
            lastChoice = choice;
            lastLoc = loc;
        }
        //Handling last point
        if(lastChoice == 1){
            drawRoad(tmpPoints,'red');
        }
        else if(lastChoice == 2){
            drawRoad(tmpPoints,'orange');
        }
        else{
            drawRoad(tmpPoints,'blue');
        }
        counter = 1;
    }
});

//Processing user data to JS variables
function processData(allText) {
    var allTextLines = allText.split("; ");
    var lines = [];

    for (var i=0; i<allTextLines.length; i++) {
        var data = allTextLines[i].split(',');
        if(data[0] == 1){
            st.push(data[0]);
            lat.push(data[1]);
            lon.push(data[2]);
            speed.push(data[3]);
            rpm.push(data[4]);
        }
    }
    alert("Parsing done.");
    console.log("Parsing done.");
    console.log("Status: " + st);
    console.log("Latitude: " + lat);
}

//Creating waypoints for route creation
function createWaypoints() {
    var points = [];

    for (var i=0; i<st.length; i++) {
        let loc = L.latLng(lat[i], lon[i]);
        points.push(loc);
    }
    return points;
}