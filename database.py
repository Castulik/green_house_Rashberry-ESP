import sqlite3
from datetime import datetime
import paho.mqtt.client as mqtt

# DB setup
db = sqlite3.connect("teplota.db")
cursor = db.cursor()
cursor.execute("""
CREATE TABLE IF NOT EXISTS teplota (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    value REAL NOT NULL
)
""")
db.commit()

# MQTT config
BROKER = "public.cloud.shiftr.io" ## CHANGE
PORT = 1883 
USERNAME = "VASE_UZIVATELSKE_JMENO" ## CHANGE
PASSWORD = "VASE_HESLO_NEBO_TOKEN" ## CHANGE
TOPIC = "sensor/sklenik/teplota" ## CHANGE

def on_message(client, userdata, msg):
    try:
        temperature = float(msg.payload.decode())
    except ValueError:
        print("⚠️ Špatný formát:", msg.payload)
        return

    now = datetime.now().isoformat()
    cursor.execute("INSERT INTO teplota (timestamp, value) VALUES (?, ?)", (now, temperature))
    db.commit()
    print(f"💾 Uloženo: {now} – {temperature} °C")

client = mqtt.Client(client_id="SklenikLogger")
client.username_pw_set(USERNAME, PASSWORD)
client.on_message = on_message
client.connect(BROKER, PORT, 60)
client.subscribe(TOPIC)

print("📡 Logger běží a sbírá data…")
client.loop_forever()