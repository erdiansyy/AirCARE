from flask import Flask, request, jsonify
from flask_cors import CORS
from flask_mysqldb import MySQL
from datetime import datetime

app = Flask(__name__)
CORS(app)

# Konfigurasi MySQL
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'your_username'
app.config['MYSQL_PASSWORD'] = 'your_password'
app.config['MYSQL_DB'] = 'iot_db'

mysql = MySQL(app)

# Buat tabel jika belum ada
def init_db():
    cur = mysql.connection.cursor()
    cur.execute('''
        CREATE TABLE IF NOT EXISTS sensor_data (
            id INT AUTO_INCREMENT PRIMARY KEY,
            temperature FLOAT,
            humidity FLOAT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    mysql.connection.commit()
    cur.close()

@app.route('/sensor', methods=['POST'])
def add_sensor_data():
    data = request.get_json()
    
    cur = mysql.connection.cursor()
    cur.execute(
        "INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)",
        (data['temperature'], data['humidity'])
    )
    mysql.connection.commit()
    cur.close()
    
    return jsonify({"message": "Data berhasil ditambahkan"}), 201

@app.route('/sensor/<int:id>', methods=['PUT'])
def update_sensor_data(id):
    data = request.get_json()
    
    cur = mysql.connection.cursor()
    cur.execute(
        "UPDATE sensor_data SET temperature=%s, humidity=%s WHERE id=%s",
        (data['temperature'], data['humidity'], id)
    )
    mysql.connection.commit()
    cur.close()
    
    return jsonify({"message": "Data berhasil diupdate"}), 200

@app.route('/sensor', methods=['GET'])
def get_sensor_data():
    cur = mysql.connection.cursor()
    cur.execute("SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 10")
    rows = cur.fetchall()
    cur.close()
    
    data = []
    for row in rows:
        data.append({
            'id': row[0],
            'temperature': row[1],
            'humidity': row[2],
            'timestamp': row[3].strftime("%Y-%m-%d %H:%M:%S")
        })
    
    return jsonify(data)

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=True)