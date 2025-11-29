# fill_year_data.py
import sqlite3, random, math
from datetime import datetime, timedelta

DB = "teplota.db"
STEP_MIN = 15          # interval měření (minuty)
DAYS = 365             # kolik dnů zpět

def seasonal_base(day_of_year):
    """Vrátí základní teplotu podle ročního období (sinus)."""
    # nejnižší kolem 15 °C (zima), nejvyšší 35 °C (léto)
    return 25 + 10 * math.sin(2 * math.pi * (day_of_year - 172) / 365)

conn = sqlite3.connect(DB)
cur  = conn.cursor()

start = datetime.now() - timedelta(days=DAYS)
samples = int((DAYS * 24 * 60) / STEP_MIN)

for i in range(samples):
    ts = start + timedelta(minutes=STEP_MIN * i)
    base = seasonal_base(ts.timetuple().tm_yday)
    value = round(base + random.uniform(-1.0, 1.0), 1)   # ±1 °C šum
    cur.execute(
        "INSERT INTO teplota (timestamp, value) VALUES (?, ?)",
        (ts.isoformat(), value)
    )

conn.commit()
conn.close()
print(f"Hotovo – vloženo {samples} řádků ({DAYS} dní).")
