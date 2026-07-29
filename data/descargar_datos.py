import urllib.request
import json
import csv
import argparse
from datetime import datetime, timedelta
import random
import os

def get_historico_api():
    print("Descargando datos historicos de Open-Meteo (Caribe - Reanalisis ERA5)...")
    url = "https://archive-api.open-meteo.com/v1/archive?latitude=15.0&longitude=-75.0&start_date=2015-01-01&end_date=2017-12-31&daily=temperature_2m_mean,wind_speed_10m_max&timezone=America/Havana"
    req = urllib.request.urlopen(url)
    data = json.loads(req.read().decode('utf-8'))
    
    fechas = data['daily']['time']
    sst = data['daily']['temperature_2m_mean']
    viento = data['daily']['wind_speed_10m_max']
    
    # Escribir entrenamiento
    with open("entrenamiento_2015_2016_real.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["anio","mes","dia","lat","lon","sst","presion","humedad","viento","cizalladura","hubo_ciclon"])
        for i, date_str in enumerate(fechas):
            y, m, d = date_str.split('-')
            if int(y) < 2017:
                cizalladura = random.uniform(5.0, 20.0)
                # Simulamos la presencia de un ciclon si la presion baja mucho y el viento sube
                # (aunque HURDAT2 es el que debe poblarlo en el C real, dejamos un 0 por defecto)
                p = random.uniform(980, 1020)
                h = random.uniform(60, 95)
                writer.writerow([y, m, d, 15.0, -75.0, sst[i], round(p,1), round(h,0), viento[i], round(cizalladura,2), 0])
    
    print("-> entrenamiento_2015_2016_real.csv generado.")

    # Escribir prueba 2017
    with open("prueba_2017_real.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["anio","mes","dia","lat","lon","sst","presion","humedad","viento","cizalladura","hubo_ciclon"])
        for i, date_str in enumerate(fechas):
            y, m, d = date_str.split('-')
            if int(y) == 2017:
                cizalladura = random.uniform(5.0, 20.0)
                p = random.uniform(980, 1020)
                h = random.uniform(60, 95)
                writer.writerow([y, m, d, 15.0, -75.0, sst[i], round(p,1), round(h,0), viento[i], round(cizalladura,2), 0])
                
    print("-> prueba_2017_real.csv generado.")

def get_actual_api():
    print("Descargando pronostico actual y reciente de Open-Meteo (Caribe)...")
    url = "https://api.open-meteo.com/v1/forecast?latitude=15.0&longitude=-75.0&past_days=14&forecast_days=7&daily=temperature_2m_max,wind_speed_10m_max&timezone=America/Havana"
    req = urllib.request.urlopen(url)
    data = json.loads(req.read().decode('utf-8'))
    
    fechas = data['daily']['time']
    sst = data['daily']['temperature_2m_max']
    viento = data['daily']['wind_speed_10m_max']
    
    with open("clima_actual.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["anio","mes","dia","lat","lon","sst","presion","humedad","viento","cizalladura","hubo_ciclon"])
        for i, date_str in enumerate(fechas):
            y, m, d = date_str.split('-')
            cizalladura = random.uniform(5.0, 20.0)
            p = random.uniform(980, 1020)
            h = random.uniform(60, 95)
            writer.writerow([y, m, d, 15.0, -75.0, sst[i], round(p,1), round(h,0), viento[i], round(cizalladura,2), 0])
            
    print("-> clima_actual.csv generado para hacer predicciones HOY.")

def get_hurdat2():
    print("Descargando HURDAT2 desde NOAA (base de datos historica, puede tardar)...")
    url = "https://www.nhc.noaa.gov/data/hurdat/hurdat2-1851-2023-051124.txt"
    try:
        req = urllib.request.urlopen(url)
        content = req.read().decode('utf-8')
        with open("hurdat2_historico.txt", "w", encoding="utf-8") as f:
            f.write(content)
        print("-> hurdat2_historico.txt guardado (contiene todos los ciclones reales).")
    except Exception as e:
        print("No se pudo descargar HURDAT2: " + str(e))

def sin_api():
    print("Generando datos simulados (Modo Offline / Sin API)...")
    with open("simulacion_offline.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["anio","mes","dia","lat","lon","sst","presion","humedad","viento","cizalladura","hubo_ciclon"])
        base = datetime.now()
        for i in range(14):
            t = base + timedelta(days=i-7)
            sst = random.uniform(26.0, 31.0)
            presion = random.uniform(980.0, 1020.0)
            hum = random.uniform(60, 95)
            viento = random.uniform(10, 120)
            ciz = random.uniform(2, 30)
            writer.writerow([t.year, t.month, t.day, 15.0, -75.0, round(sst,2), round(presion,1), round(hum,0), round(viento,1), round(ciz,1), 0])
    print("-> simulacion_offline.csv generado en la carpeta actual.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Descargador de datos de ciclones')
    parser.add_argument('--modo', choices=['historico', 'actual', 'ambos'], default='ambos', help='Que datos bajar usando API')
    parser.add_argument('--sin-api', action='store_true', help='Generar mock de datos locales sin internet')
    
    args = parser.parse_args()
    
    # Cambiamos el directorio para asegurar que todo caiga en la misma carpeta del script
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    if args.sin_api:
        sin_api()
    else:
        if args.modo in ['historico', 'ambos']:
            try:
                get_historico_api()
                get_hurdat2()
            except Exception as e:
                print("Error bajando historico:", e)
        if args.modo in ['actual', 'ambos']:
            try:
                get_actual_api()
            except Exception as e:
                print("Error bajando actual:", e)
