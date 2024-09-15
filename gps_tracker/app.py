from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Store the latest GPS data
latest_data = {"lat": None, "lon": None}

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/receive-data', methods=['GET'])
def receive_data():
    global latest_data
    data = request.args.get('data')

    if data:
        try:
            lat, lon = data.split(",")
            latest_data['lat'] = float(lat)
            latest_data['lon'] = float(lon)
            return jsonify({"status": "success", "lat": lat, "lon": lon})
        except ValueError:
            return jsonify({"status": "error", "message": "Invalid data format"})
    else:
        return jsonify({"status": "error", "message": "No data received"})

@app.route('/get-latest-data', methods=['GET'])
def get_latest_data():
    return jsonify(latest_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
