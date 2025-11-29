import streamlit as st
import sqlite3
import pandas as pd
import plotly.express as px
from streamlit_autorefresh import st_autorefresh
from datetime import datetime, timedelta

st.set_page_config(page_title="📊 Teplota – přehled", page_icon="🌡️", layout="wide")
st.title("🌱 Historie teplot ve skleníku")

# ---------- Auto‑refresh ----------
st_autorefresh(interval=3000, key="data_refresh")

# ---------- DB connection ----------
conn = sqlite3.connect("teplota.db", check_same_thread=False)
cursor = conn.cursor()

# ----- Správa databáze (mazání) -----
with st.expander("🗑️ Správa databáze"):
    col_all, col_day = st.columns(2)

    with col_all:
        if st.button("Smazat VŠECHNA data"):
            cursor.execute("DELETE FROM teplota")
            cursor.execute("DELETE FROM stav")
            conn.commit()
            st.success("Všechna data byla odstraněna.")
            st.experimental_rerun()

    with col_day:
        sel_date = st.date_input("Datum pro smazání")
        if st.button("Smazat vybraný den"):
            cursor.execute("DELETE FROM teplota WHERE substr(timestamp,1,10)=?", (sel_date.isoformat(),))
            conn.commit()
            st.success(f"Záznamy pro {sel_date} byly odstraněny.")
            st.experimental_rerun()

# ---------- Parametry uživatele ----------
if "threshold" not in st.session_state:
    st.session_state.threshold = 30.0

col_thr, col_range = st.columns(2)

with col_thr:
    st.session_state.threshold = st.number_input(
        "⚠️ Prahová teplota (°C)",
        min_value=-40.0, max_value=100.0, step=0.5,
        value=float(st.session_state.threshold)
    )
with col_range:
    range_option = st.selectbox(
        "Zobrazit období",
        ["Vše", "Poslední hodina", "Poslední den", "Poslední týden", "Poslední měsíc", "Poslední rok"],
        index=1
    )

# ---------- Načtení dat ----------
df = pd.read_sql_query("SELECT timestamp, value FROM teplota ORDER BY timestamp ASC", conn)

# ---------- Filtrování podle času ----------
if not df.empty:
    df["timestamp"] = pd.to_datetime(df["timestamp"])
    now = datetime.now()
    delta_map = {
        "Poslední hodina": timedelta(hours=1),
        "Poslední den": timedelta(days=1),
        "Poslední týden": timedelta(weeks=1),
        "Poslední měsíc": timedelta(days=30),
        "Poslední rok": timedelta(days=365),
    }
    if range_option != "Vše":
        start_time = now - delta_map[range_option]
        df_show = df[df["timestamp"] >= start_time]
    else:
        df_show = df.copy()
else:
    df_show = pd.DataFrame()

# ---------- Zobrazení metrik a grafu ----------
if not df_show.empty:
    latest_temp = df_show.iloc[-1]["value"]
    min_temp = df_show["value"].min()
    max_temp = df_show["value"].max()
    avg_temp = df_show["value"].mean()
    delta_temp = latest_temp - df_show.iloc[-2]["value"] if len(df_show) > 1 else 0

    if latest_temp >= st.session_state.threshold:
        st.error(f"🚨 Aktuální teplota {latest_temp:.1f} °C překročila práh {st.session_state.threshold:.1f} °C!")

    col1, col2, col3, col4 = st.columns(4)
    col1.metric("🌡️ Aktuální", f"{latest_temp:.1f} °C", delta=f"{delta_temp:+.1f} °C")
    col2.metric("🔽 Minimum", f"{min_temp:.1f} °C")
    col3.metric("🔼 Maximum", f"{max_temp:.1f} °C")
    col4.metric("📉 Průměr", f"{avg_temp:.1f} °C")

    fig = px.line(df_show, x="timestamp", y="value",
                  labels={"timestamp": "Čas", "value": "Teplota (°C)"},
                  title=f"Vývoj teploty – {range_option.lower()}")
    fig.update_layout(margin=dict(l=0, r=0, t=40, b=0))
    st.plotly_chart(fig, use_container_width=True)

    with st.expander("📄 Posledních 50 záznamů"):
        st.dataframe(df_show.tail(50), use_container_width=True)

    # ---------- Export CSV (dole pod grafem) ----------
    csv = df_show.to_csv(index=False).encode("utf-8")
    st.download_button("📥 Export CSV", data=csv, file_name="teplota_export.csv", mime="text/csv")
else:
    st.info("V databázi zatím nejsou žádná data pro vybraný rozsah.")

conn.close()
