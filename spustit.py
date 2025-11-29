import subprocess
import time

# Spustí logger v novém terminálu
subprocess.Popen(["start", "cmd", "/k", "python database.py"], shell=True)

# Počká 2 sekundy
time.sleep(2)

# Spustí dashboard
subprocess.Popen(["start", "cmd", "/k", "streamlit run Main.py"], shell=True)
