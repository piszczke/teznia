from flask import Flask, render_template, request, redirect, url_for
import requests

app = Flask(__name__)

ESP8266_IP = '192.168.0.11'  # Replace with the IP address of your ESP8266

@app.route('/')
def index():
    response = requests.get(ESP8266_IP + '/data')
    return response.text

@app.route('/relay/on/1min')
def relay_on_1min():
    requests.get(ESP8266_IP + '/relay/on/1min')
    return redirect(url_for('index'))

@app.route('/relay/on/3min')
def relay_on_3min():
    requests.get(ESP8266_IP + '/relay/on/3min')
    return redirect(url_for('index'))

@app.route('/relay/on/5min')
def relay_on_5min():
    requests.get(ESP8266_IP + '/relay/on/5min')
    return redirect(url_for('index'))

@app.route('/relay/on/10min')
def relay_on_10min():
    requests.get(ESP8266_IP + '/relay/on/10min')
    return redirect(url_for('index'))

@app.route('/relay/on/humidity')
def relay_on_humidity():
    requests.get(ESP8266_IP + '/relay/on/humidity')
    return redirect(url_for('index'))

@app.route('/relay/off')
def relay_off():
    requests.get(ESP8266_IP + '/relay/off')
    return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
