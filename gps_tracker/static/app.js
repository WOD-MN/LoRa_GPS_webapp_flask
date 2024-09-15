// Initialize the map
var map = L.map('map', { 
    center: [0, 0], 
    zoom: 13,
    zoomControl: true 
});

// Add OpenStreetMap tiles
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '© OpenStreetMap contributors'
}).addTo(map);

// Marker and Polyline layers
var marker = L.marker([0, 0], { draggable: true }).addTo(map)
    .bindPopup('Latitude: 0<br>Longitude: 0')
    .openPopup();
var polyline = L.polyline([], { color: 'blue', weight: 5 }).addTo(map);

// Function to update map and path
function updateMap(lat, lon) {
    var newLatLng = [lat, lon];
    marker.setLatLng(newLatLng);
    marker.setPopupContent('Latitude: ' + lat + '<br>Longitude: ' + lon);
    var latLngs = polyline.getLatLngs();
    latLngs.push(newLatLng);
    polyline.setLatLngs(latLngs);
    document.getElementById('latitude').textContent = lat;
    document.getElementById('longitude').textContent = lon;

    // Smooth scroll animation
    document.querySelector('#info').classList.add('highlight');
    setTimeout(() => {
        document.querySelector('#info').classList.remove('highlight');
    }, 1000);
}

// Fetch latest GPS data
function fetchData() {
    fetch('http://192.168.1.68:5001/get-latest-data')
        .then(response => response.json())
        .then(data => {
            if (data.lat && data.lon) {
                updateMap(data.lat, data.lon);
            }
        })
        .catch(error => console.error('Error fetching data:', error));
}

// Clear path on button click
document.getElementById('menuClear').addEventListener('click', function() {
    polyline.setLatLngs([]); // Clear the polyline
});

// Sidebar toggle
document.getElementById('sidebarCollapse').addEventListener('click', function() {
    document.getElementById('sidebar').classList.toggle('active');
});

// Fetch data periodically
fetchData();
setInterval(fetchData, 5000); // Every 5 seconds
