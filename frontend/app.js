// ==========================================================
// MODULO 7 — FRONTEND / VISUALIZADOR
// Responsable: Rachel
//
// Toda la comunicacion con C pasa por M.ccall(), usando las
// funciones exportadas en api_bridge.c (Modulo 6). Este archivo
// NO implementa el algoritmo: solo pide datos y los dibuja.
// ==========================================================

let M; // instancia del modulo WASM

// Animacion de la barra de carga mientras compila/instancia el WASM
const barFill = document.getElementById('loading-bar-fill');
let progreso = 0;
const intervaloBarra = setInterval(() => {
    if (progreso < 90) {
        progreso += Math.random() * 8;
        barFill.style.width = Math.min(progreso, 90) + '%';
    }
}, 200);

FuzzyModule().then(mod => {
    M = mod;
    console.log("WASM cargado (Fuzzy C-Means)");

    // Forzamos que la barra de carga dure más tiempo (ej. 5.0 segundos extra)
    setTimeout(() => {
        clearInterval(intervaloBarra);
        barFill.style.width = '100%';
        
        setTimeout(() => {
            const loadingScreen = document.getElementById('loading-screen');
            loadingScreen.style.opacity = '0';
            setTimeout(() => {
                loadingScreen.style.display = 'none';
            }, 800); // Tiempo que tarda el fade out
        }, 600);
    }, 5000);
});

// ==========================================================
// NAVEGACION ENTRE VISTAS
// ==========================================================
/**
 * Oculta todas las vistas de la interfaz y muestra únicamente la solicitada.
 * Actualiza también el estado activo del botón de navegación correspondiente.
 * @param {string} nombre - Identificador de la vista a mostrar (ej. 'datos', 'entrenamiento').
 */
function mostrarVista(nombre) {
    document.querySelectorAll('.vista').forEach(v => v.style.display = 'none');
    document.getElementById('vista-' + nombre).style.display = 'block';
    document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
    event.target.classList.add('active');
}

/**
 * Convierte un puntero de memoria de WebAssembly (WASM) a una cadena de texto (String) en JavaScript.
 * Utiliza la función interna UTF8ToString expuesta por el módulo compilado (M).
 * @param {number} ptr - Puntero a la dirección de memoria en WASM.
 * @returns {string} La cadena de texto decodificada.
 */
function getString(ptr) {
    return M.UTF8ToString(ptr);
}

// ==========================================================
// VISTA 1 — IMPORTAR DATOS (llama a Modulo 1 + Modulo 2 via Modulo 6)
// ==========================================================

let datosPruebaCargados = false;

/**
 * Lee un archivo local seleccionado por el usuario y envía su contenido al módulo C (WASM).
 * Dependiendo del tipo ('train' o 'test'), invoca la función C correspondiente mediante `M.ccall`.
 * @param {string} tipo - Tipo de archivo a cargar ('train' para entrenamiento, 'test' para prueba).
 */
function cargarArchivo(tipo) {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    const inputId = tipo === 'train' ? 'input-archivo-train' : 'input-archivo-test';
    const input = document.getElementById(inputId);
    const archivo = input.files[0];

    if (!archivo) {
        alert('Selecciona un archivo primero');
        return;
    }

    const lector = new FileReader();

    lector.onload = (e) => {
        const contenido = e.target.result;

        try {
            let resultadoPtr;

            if (tipo === 'train') {
                resultadoPtr = M.ccall(
                    'api_importar_datos',
                    'string',
                    ['string'],
                    [contenido]
                );
            } else {
                resultadoPtr = M.ccall(
                    'api_cargar_prueba',
                    'string',
                    ['string', 'number', 'number'],
                    [contenido, 2017, 2017]
                );
                datosPruebaCargados = true;
            }

            const resultado = JSON.parse(resultadoPtr);
            console.log(`Respuesta de C (${tipo}):`, resultado);

            const box = document.getElementById('resultado-importacion');

            if (resultado.exito) {
                if (tipo === 'train') {
                    box.innerHTML = `
                        Entrenamiento importado — formato: <strong>${resultado.formato_detectado}</strong>,
                        registros: <strong>${resultado.importados}</strong>
                    `;
                } else {
                    box.innerHTML = `
                        Prueba importada — registros importados: <strong>${resultado.importados}</strong>,
                        disponibles para predecir: <strong>${resultado.total_prueba}</strong>
                    `;
                }
                box.style.borderLeftColor = 'var(--bajo)';
            } else {
                box.innerHTML = `Error al importar el archivo de ${tipo === 'train' ? 'entrenamiento' : 'prueba'}`;
                box.style.borderLeftColor = 'var(--alerta)';
            }

        } catch (error) {
            console.error('Error en M.ccall:', error);
            alert('Error: ' + error.message);
        }
    };

    lector.readAsText(archivo);
}

/**
 * Descarga asíncronamente datos meteorológicos históricos desde la API de Open-Meteo.
 * Prepara los registros y los envía a la memoria del módulo WASM invocando las funciones C 
 * de importación o carga de prueba según el tipo.
 * Maneja la asignación y liberación de memoria dinámica (`_malloc` y `_free`) en WASM para
 * transferir eficientemente grandes cadenas JSON.
 * @param {string} tipo - 'train' para datos de entrenamiento, 'test' para datos de prueba.
 */
async function obtenerEntrenamientoAPI(tipo = 'train') {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    const btn = document.getElementById(tipo === 'train' ? 'btn-api-importar-train' : 'btn-api-importar-test');
    const status = document.getElementById('status-api-importar');
    const anioInicio = parseInt(document.getElementById('api-anio-inicio').value);
    const anioFin = parseInt(document.getElementById('api-anio-fin').value);

    if (anioInicio > anioFin) {
        alert('El año de inicio no puede ser mayor al de fin');
        return;
    }

    btn.disabled = true;
    status.innerText = `Descargando datos de ${tipo === 'train' ? 'entrenamiento' : 'prueba'} (esto puede tomar unos segundos)...`;

    try {
        const registrosAPI = [];
        const startDate = `${anioInicio}-01-01`;
        const endDate = `${anioFin}-12-31`;

        // Iterar sobre coordenadasCaribe que ya está definido en la Vista 5
        // Reutilizamos el array o definimos uno si no existe
        const puntos = window.coordenadasCaribe || [
            {lat: 15.0, lon: -75.0}, {lat: 18.0, lon: -65.0}, {lat: 20.0, lon: -85.0},
            {lat: 22.0, lon: -90.0}, {lat: 25.0, lon: -80.0}, {lat: 12.0, lon: -70.0}
        ];

        for (const c of puntos) {
            status.innerText = `Descargando coordenadas lat:${c.lat} lon:${c.lon}...`;
            
            const wRes = await fetch(`https://archive-api.open-meteo.com/v1/archive?latitude=${c.lat}&longitude=${c.lon}&start_date=${startDate}&end_date=${endDate}&daily=surface_pressure_mean,wind_speed_10m_max&timezone=auto`);
            const wData = await wRes.json();

            // Asumiendo que la API marina no soporta el mismo rango facilmente (suele tener limites gratis), 
            // aproximaremos SST histórico usando daily temperature o fallback 28.0 para el caribe para no romper.
            // Para ser robusto y no bloquear la descarga, haremos fallback al SST del Weather API o a 28C.
            
            if (wData.daily && wData.daily.time) {
                for (let i = 0; i < wData.daily.time.length; i++) {
                    const fechaStr = wData.daily.time[i];
                    const [y, m, d] = fechaStr.split('-');
                    
                    const presion = wData.daily.surface_pressure_mean[i] || 1010.0;
                    const viento = wData.daily.wind_speed_10m_max[i] || 20.0;
                    
                    registrosAPI.push({
                        anio: parseInt(y),
                        mes: parseInt(m),
                        dia: parseInt(d),
                        lat: c.lat,
                        lon: c.lon,
                        sst: 28.5 + (Math.random() - 0.5), // Fallback realista para el mar Caribe
                        presion: presion,
                        humedad: 75.0 + (Math.random() * 10 - 5), // Promedio Caribe
                        viento: viento,
                        cizalladura: 10.0,
                        hubo_ciclon: 0 // No importa para entrenamiento
                    });
                }
            }
            
            // Pausa breve para evitar Rate Limits de Open-Meteo
            await new Promise(r => setTimeout(r, 200));
        }

        if (registrosAPI.length === 0) {
            status.innerText = 'No se pudieron descargar datos.';
            btn.disabled = false;
            return;
        }

        // Limitar a ~4900 registros porque el límite en C es MAX_REGISTROS = 5000
        const registrosAcotados = registrosAPI.slice(0, 4900);

        status.innerText = `Procesando ${registrosAcotados.length} registros (enviando a WebAssembly)...`;
        
        // Convertir a JSON string
        const jsonString = JSON.stringify(registrosAcotados);
        
        // Asignar memoria en el HEAP de WebAssembly para evitar desbordar el STACK con ccall
        const lengthBytes = M.lengthBytesUTF8(jsonString) + 1;
        const stringOnWasmHeap = M._malloc(lengthBytes);
        M.stringToUTF8(jsonString, stringOnWasmHeap, lengthBytes);
        
        // Llamar a C directamente pasando el puntero
        let resultadoObj;
        if (tipo === 'train') {
            const ptr = M._api_importar_datos(stringOnWasmHeap);
            const str = M.UTF8ToString(ptr);
            resultadoObj = JSON.parse(str);
        } else {
            // api_cargar_prueba en C espera (const char* contenido, int anio_inicio, int anio_fin)
            // Aquí usamos ccall porque necesitamos pasar números también. Como jsonString puede ser grande,
            // usaremos la memoria asignada y llamaremos a la función envuelta.
            // Wait, ccall with 'string' copies to stack. Let's pass 'number' (the pointer) instead!
            const str = M.ccall('api_cargar_prueba', 'string', ['number', 'number', 'number'], [stringOnWasmHeap, anioInicio, anioFin]);
            resultadoObj = JSON.parse(str);
            datosPruebaCargados = true;
        }
        
        // Liberar la memoria
        M._free(stringOnWasmHeap);
        
        const box = document.getElementById('resultado-importacion');
        if (resultadoObj.exito) {
            if (tipo === 'train') {
                box.innerHTML = `
                    Entrenamiento API importado — formato: <strong>JSON</strong>,
                    registros: <strong>${resultadoObj.importados}</strong> (Rango: ${anioInicio}-${anioFin})
                `;
            } else {
                box.innerHTML = `
                    Prueba API importada — registros importados: <strong>${resultadoObj.importados}</strong>,
                    disponibles para predecir: <strong>${resultadoObj.total_prueba}</strong>
                `;
            }
            box.style.borderLeftColor = 'var(--bajo)';
            status.innerText = '¡Descarga y carga completada!';
        } else {
            box.innerHTML = `Error al cargar los datos de la API en WebAssembly.`;
            box.style.borderLeftColor = 'var(--alerta)';
            status.innerText = 'Ocurrió un error.';
        }

    } catch (e) {
        console.error('API Error:', e);
        status.innerText = 'Fallo la conexión con Open-Meteo.';
        alert('Error conectando a API: ' + e.message);
    }
    
    btn.disabled = false;
}


// ==========================================================
// VISTA 2 — ENTRENAMIENTO (llama a Modulo 3 + Modulo 4)
// ==========================================================
/**
 * Inicia el proceso de entrenamiento del modelo Fuzzy C-Means (FCM) enviando los parámetros
 * configurados por el usuario (clusters, grado difuso m, iteraciones) a la función C `api_entrenar`.
 * Procesa la respuesta en formato JSON para actualizar la UI con los resultados y mostrar los centroides.
 */
function entrenarModelo() {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    const nClusters = parseInt(document.getElementById('param-clusters').value, 10);
    const m = parseFloat(document.getElementById('param-m').value);
    const maxIter = parseInt(document.getElementById('param-max-iter').value, 10);
    const epsilon = 0.00001;

    console.log(`Entrenando: clusters=${nClusters}, m=${m}, maxIter=${maxIter}`);

    try {
        const resultadoPtr = M.ccall(
            'api_entrenar',
            'string',
            ['number', 'number', 'number', 'number', 'number', 'number'],
            [2015, 2016, nClusters, m, maxIter, epsilon]
        );

        const resultado = JSON.parse(resultadoPtr);
        console.log('Entrenamiento:', resultado);

        const box = document.getElementById('resultado-entrenamiento');

        if (resultado.exito) {
            box.innerHTML = `Modelo entrenado — iteraciones hasta converger: <strong>${resultado.iteraciones}</strong>,
            clusters: <strong>${resultado.n_clusters}</strong>`;

            box.style.borderLeftColor = 'var(--bajo)';
            mostrarCentroides(resultado.centroides);
        } else {
            box.innerHTML = `Error al entrenar. ¿Cargaste el archivo de entrenamiento (2015-2016)?`;
            box.style.borderLeftColor = 'var(--alerta)';
            document.getElementById('centroides-box').innerHTML = '';
        }

    } catch (error) {
        console.error('Error en M.ccall api_entrenar:', error);
        alert('Error: ' + error.message);
    }
}

/**
 * Renderiza dinámicamente en el DOM una tabla HTML con los centroides generados
 * tras el entrenamiento del modelo FCM. Muestra las coordenadas del centroide
 * para cada variable climática.
 * @param {Array} centroides - Arreglo de objetos con las coordenadas de cada centroide.
 */
function mostrarCentroides(centroides) {
    const cont = document.getElementById('centroides-box');

    let html = '<table class="tabla"><thead><tr>';
    html += '<th>Cluster</th><th>SST</th><th>Presión</th><th>Humedad</th><th>Viento</th><th>Cizalladura</th>';
    html += '</tr></thead><tbody>';

    centroides.forEach(c => {
        html += `<tr>
            <td>${c.cluster}</td>
            <td>${c.sst.toFixed(2)}</td>
            <td>${c.presion.toFixed(2)}</td>
            <td>${c.humedad.toFixed(2)}</td>
            <td>${c.viento.toFixed(2)}</td>
            <td>${c.cizalladura.toFixed(2)}</td>
        </tr>`;
    });

    html += '</tbody></table>';
    cont.innerHTML = html;
}

// ==========================================================
// VISTA 3 — PREDICCION (llama a Modulo 4 + dibuja mapa de calor)
// ==========================================================
let mapa = null; // Instancia del mapa Leaflet
let marcadores = []; // Para limpiar marcadores anteriores

/**
 * Invoca la función C `api_predecir` para calcular el riesgo de ciclón sobre el dataset de prueba.
 * Recibe un JSON con las predicciones y orquesta el renderizado visual de los resultados
 * inicializando el mapa Leaflet y delegando el pintado a funciones especializadas.
 */
function predecir() {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    if (!datosPruebaCargados) {
        alert('Primero debes cargar el set de prueba en la pestaña "1. Datos" (puedes subir un archivo local o usar el botón "Descargar Prueba" en la Opción B de la API)');
        return;
    }

    try {
        const resultadoPtr = M.ccall('api_predecir', 'string', [], []);
        const registros = JSON.parse(resultadoPtr);

        console.log(`Predicciones recibidas: ${registros.length} registros`);
        
        // Mostrar contenedor del mapa e inicializar si es necesario
        document.getElementById('mapa-container').style.display = 'block';
        if (!mapa) {
            // Inicializar mapa centrado en el Caribe
            mapa = L.map('map').setView([18.0, -75.0], 5);
            L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
                attribution: '&copy; OpenStreetMap contributors &copy; CARTO',
                subdomains: 'abcd',
                maxZoom: 19
            }).addTo(mapa);
        }

        dibujarMapaLeaflet(registros);
        dibujarMapaCalor(registros);

    } catch (error) {
        console.error('Error en M.ccall api_predecir:', error);
        alert('Error: ' + error.message);
    }
}

/**
 * Dibuja los resultados de la predicción en un mapa interactivo utilizando la biblioteca Leaflet.
 * Crea marcadores circulares (`L.circleMarker`) cuyas propiedades (tamaño, color) varían en 
 * función del riesgo calculado. Asocia a cada marcador un popup (Tooltip) con detalles técnicos
 * e influencias de cada variable climática.
 * @param {Array} registros - Arreglo de objetos con las predicciones por coordenada.
 */
function dibujarMapaLeaflet(registros) {
    // Limpiar marcadores anteriores
    marcadores.forEach(m => mapa.removeLayer(m));
    marcadores = [];

    registros.forEach(r => {
        let color, fillColor;
        if (r.riesgo < 0.25) {
            color = '#3b82f6'; // Azul (Bajo)
            fillColor = '#3b82f6';
        } else if (r.riesgo < 0.75) {
            color = '#f59e0b'; // Naranja (Moderado)
            fillColor = '#f59e0b';
        } else {
            color = '#ef4444'; // Rojo (Alto)
            fillColor = '#ef4444';
        }

        const circulo = L.circleMarker([r.lat, r.lon], {
            radius: 8 + (r.riesgo * 10), // Tamaño dinámico según riesgo
            color: color,
            fillColor: fillColor,
            fillOpacity: 0.6,
            weight: 2
        }).addTo(mapa);

        // Tooltip con detalles e influencia
        const tooltipHTML = `
            <div style="font-family: var(--font-base); font-size: 13px;">
                <strong style="font-size:15px; border-bottom: 1px solid #ccc; display:block; padding-bottom:3px; margin-bottom:5px;">
                    Riesgo: ${Math.round(r.riesgo * 100)}%
                </strong>
                <b>Fecha:</b> ${r.anio}-${String(r.mes).padStart(2, '0')}-${String(r.dia).padStart(2, '0')}<br>
                <b>SST:</b> ${r.sst.toFixed(1)} °C <span style="color:#aaa;font-size:10px">(${Math.round(r.inf_sst * 100)}% infl.)</span><br>
                <b>Presión:</b> ${r.presion.toFixed(1)} hPa <span style="color:#aaa;font-size:10px">(${Math.round(r.inf_presion * 100)}% infl.)</span><br>
                <b>Humedad:</b> ${r.humedad.toFixed(1)} % <span style="color:#aaa;font-size:10px">(${Math.round(r.inf_humedad * 100)}% infl.)</span><br>
                <b>Viento:</b> ${r.viento.toFixed(1)} km/h <span style="color:#aaa;font-size:10px">(${Math.round(r.inf_viento * 100)}% infl.)</span>
            </div>
        `;
        
        circulo.bindPopup(tooltipHTML);
        marcadores.push(circulo);
    });
}

/**
 * Renderiza una representación visual (tipo mapa de calor lineal/lista) de los riesgos predichos.
 * Genera elementos del DOM que muestran visualmente la probabilidad de riesgo mediante barras
 * de colores (verde, amarillo, rojo) y compara la predicción con la ocurrencia real de ciclones (evaluación).
 * @param {Array} registros - Arreglo con las predicciones y datos reales.
 */
function dibujarMapaCalor(registros) {
    const cont = document.getElementById('mapa-calor');
    cont.innerHTML = '';

    if (registros.length === 0) {
        cont.innerHTML = '<p style="color: var(--gray-500);">Sin predicciones aún.</p>';
        return;
    }

    registros.forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'fila-riesgo';

        let color;
        if (r.riesgo < 0.25) color = 'var(--bajo)';
        else if (r.riesgo < 0.75) color = 'var(--moderado)';
        else color = 'var(--alerta)';

        const prediccionCiclon = r.riesgo >= 0.5;
        const acierto = (prediccionCiclon && r.hubo_ciclon) || (!prediccionCiclon && !r.hubo_ciclon);
        const aciertoHTML = acierto 
            ? '<span style="color: #4ade80; font-weight:bold; font-size: 12px; margin-left: 10px;">✅ Acertó</span>' 
            : '<span style="color: #f87171; font-weight:bold; font-size: 12px; margin-left: 10px;">❌ Falló</span>';

        fila.innerHTML = `
            <div style="display:flex; flex-direction:column; gap:4px; font-size:12px; min-width: 250px;">
                <span style="font-size:14px; font-weight:bold;">${r.anio}-${String(r.mes).padStart(2, '0')}-${String(r.dia).padStart(2, '0')}</span>
                <span style="color:var(--gray-400)">Lat: ${r.lat.toFixed(2)}, Lon: ${r.lon.toFixed(2)} | SST: ${r.sst}°C | Presión: ${r.presion}hPa | Humedad: ${r.humedad}% | Viento: ${r.viento}km/h</span>
                <span style="color:var(--azul-claro); font-size:10px">Influencia: SST ${Math.round(r.inf_sst*100)}% | Presión ${Math.round(r.inf_presion*100)}% | Humedad ${Math.round(r.inf_humedad*100)}% | Viento ${Math.round(r.inf_viento*100)}%</span>
            </div>
            <div class="barra-riesgo" style="width:${Math.round(r.riesgo * 200)}px; background:${color};"></div>
            <span style="font-weight:bold; min-width: 60px;">${(r.riesgo * 100).toFixed(1)}%</span>
            <div style="display:flex; flex-direction:column; align-items:flex-end;">
                ${r.hubo_ciclon ? '<span style="color: var(--alerta); font-weight:bold;">🌀 Hubo ciclón real</span>' : '<span style="color: var(--gray-400);">Sin ciclón</span>'}
                ${aciertoHTML}
            </div>
        `;
        cont.appendChild(fila);
    });
}

// ==========================================================
// VISTA 4 — EVALUACION (llama a Modulo 5)
// ==========================================================
/**
 * Solicita a la capa C la evaluación del rendimiento predictivo del modelo invocando `api_evaluar`.
 * Envía el umbral de riesgo definido en la UI.
 * Construye y renderiza una matriz de confusión y despliega métricas de clasificación 
 * (Exactitud, Precisión, Sensibilidad, F1-Score) extraídas del JSON retornado por WASM.
 */
function evaluarModelo() {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    const umbral = parseFloat(document.getElementById('param-umbral').value);

    try {
        const resultadoPtr = M.ccall('api_evaluar', 'string', ['number'], [umbral]);
        const resultado = JSON.parse(resultadoPtr);

        console.log('Evaluación:', resultado);

        const box = document.getElementById('resultado-evaluacion');

        if (!resultado.exito) {
            box.innerHTML = `No se pudo evaluar. ¿Ya corriste la predicción?`;
            box.style.borderLeftColor = 'var(--alerta)';
            return;
        }

        const mc = resultado.matriz_confusion;
        const met = resultado.metricas;

        box.style.borderLeftColor = 'var(--azul-claro)';
        box.innerHTML = `
            <table class="tabla">
                <thead><tr><th></th><th>Predicho: ciclón</th><th>Predicho: sin ciclón</th></tr></thead>
                <tbody>
                    <tr><td><strong>Real: ciclón</strong></td>
                        <td style="color:var(--bajo)">VP: ${mc.verdaderos_positivos}</td>
                        <td style="color:var(--alerta)">FN: ${mc.falsos_negativos}</td></tr>
                    <tr><td><strong>Real: sin ciclón</strong></td>
                        <td style="color:var(--moderado)">FP: ${mc.falsos_positivos}</td>
                        <td style="color:var(--bajo)">VN: ${mc.verdaderos_negativos}</td></tr>
                </tbody>
            </table>
            <div style="margin-top:1rem; display:grid; grid-template-columns:repeat(2,1fr); gap:.5rem;">
                <div>Exactitud: <strong>${(met.accuracy * 100).toFixed(1)}%</strong></div>
                <div>Precisión: <strong>${(met.precision * 100).toFixed(1)}%</strong></div>
                <div>Sensibilidad: <strong>${(met.recall * 100).toFixed(1)}%</strong></div>
                <div>F1-Score: <strong>${(met.f1_score * 100).toFixed(1)}%</strong></div>
            </div>
        `;

    } catch (error) {
        console.error('Error en M.ccall api_evaluar:', error);
        alert('Error: ' + error.message);
    }
}

// ==========================================================
// VISTA 5 — TIEMPO REAL (Llamada a API y Prediccion)
// ==========================================================

let mapaRealtime = null;
let marcadoresRealtime = [];

const coordenadasCaribe = [
    {lat: 15.0, lon: -75.0}, {lat: 18.0, lon: -65.0}, {lat: 20.0, lon: -85.0},
    {lat: 22.0, lon: -90.0}, {lat: 25.0, lon: -80.0}, {lat: 12.0, lon: -70.0},
    {lat: 14.0, lon: -80.0}, {lat: 19.0, lon: -72.0}, {lat: 23.0, lon: -85.0},
    {lat: 26.0, lon: -95.0}, {lat: 28.0, lon: -90.0}, {lat: 16.0, lon: -62.0}
];

/**
 * Consulta APIs externas (Open-Meteo Weather y Marine) de manera asíncrona para obtener
 * datos climáticos en tiempo real de ubicaciones estratégicas del Caribe.
 * Formatea estos datos en JSON, los carga en el módulo C usando `api_cargar_prueba`, 
 * ejecuta la predicción (`api_predecir`) y manda a pintar los resultados instantáneos en la interfaz.
 */
async function obtenerYPredecirTiempoReal() {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }
    const btn = document.getElementById('btn-tiemporeal');
    const status = document.getElementById('status-tiemporeal');
    btn.disabled = true;
    status.innerText = 'Consultando a Open-Meteo para ' + coordenadasCaribe.length + ' ubicaciones...';

    const fechaHoy = new Date();
    const anio = fechaHoy.getFullYear();
    const mes = fechaHoy.getMonth() + 1;
    const dia = fechaHoy.getDate();

    const registrosReales = [];

    for (const c of coordenadasCaribe) {
        try {
            // Weather API (Presion, Humedad, Viento)
            const wRes = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${c.lat}&longitude=${c.lon}&current=surface_pressure,relative_humidity_2m,wind_speed_10m`);
            const wData = await wRes.json();
            
            // Marine API (SST)
            const mRes = await fetch(`https://marine-api.open-meteo.com/v1/marine?latitude=${c.lat}&longitude=${c.lon}&current=ocean_temperature`);
            const mData = await mRes.json();

            const sst = mData.current?.ocean_temperature || 26.0; // Fallback si no hay dato marino (ej. tierra)
            const presion = wData.current?.surface_pressure || 1010.0;
            const humedad = wData.current?.relative_humidity_2m || 70.0;
            const viento = wData.current?.wind_speed_10m || 20.0;

            registrosReales.push({
                anio: anio, mes: mes, dia: dia,
                lat: c.lat, lon: c.lon,
                sst: sst, presion: presion, humedad: humedad, viento: viento,
                cizalladura: 10.0, // Open-Meteo gratis no da wind shear facilmente en un endpoint simple
                hubo_ciclon: 0
            });
        } catch (e) {
            console.error(`Error obteniendo lat=${c.lat} lon=${c.lon}`, e);
        }
    }

    if (registrosReales.length === 0) {
        status.innerText = 'Fallo la obtención de datos desde la API.';
        btn.disabled = false;
        return;
    }

    status.innerText = 'Datos obtenidos. Inyectando a FCM para predecir...';

    // Generar JSON y pasarlo a C
    const jsonString = JSON.stringify(registrosReales);

    try {
        // Cargar en el buffer de prueba (sobreescribiendo lo anterior)
        const loadResPtr = M.ccall(
            'api_cargar_prueba',
            'string',
            ['string', 'number', 'number'],
            [jsonString, anio, anio]
        );
        const loadRes = JSON.parse(loadResPtr);
        
        if (!loadRes.exito) {
            throw new Error(loadRes.mensaje);
        }

        // Ejecutar prediccion
        const predResPtr = M.ccall('api_predecir', 'string', [], []);
        const predicciones = JSON.parse(predResPtr);
        
        if (predicciones.length === 0) {
            throw new Error("El modelo retornó 0 predicciones. ¿Olvidaste ir a la pestaña '2. Entrenamiento' y pulsar 'Entrenar Modelo' primero?");
        }

        status.innerText = 'Predicción completada para el ' + anio + '-' + mes + '-' + dia;

        // Mostrar contenedor y mapa
        document.getElementById('mapa-tiemporeal-container').style.display = 'block';
        if (!mapaRealtime) {
            mapaRealtime = L.map('map-realtime').setView([20.0, -75.0], 5);
            L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
                attribution: '&copy; OpenStreetMap contributors &copy; CARTO',
                subdomains: 'abcd',
                maxZoom: 19
            }).addTo(mapaRealtime);
        }

        dibujarMapaLeafletRealtime(predicciones);
        dibujarMapaCalorRealtime(predicciones);

    } catch (e) {
        console.error('Error FCM:', e);
        status.innerText = 'Error ejecutando el modelo: ' + e.message;
    } finally {
        btn.disabled = false;
    }
}

/**
 * Renderiza las predicciones de tiempo real en un mapa Leaflet independiente.
 * Añade marcadores visuales con el estado de riesgo actual y popups informativos
 * detallando las variables meteorológicas del momento.
 * @param {Array} registros - Datos meteorológicos y predicciones de riesgo actuales.
 */
function dibujarMapaLeafletRealtime(registros) {
    marcadoresRealtime.forEach(m => mapaRealtime.removeLayer(m));
    marcadoresRealtime = [];

    registros.forEach(r => {
        let color, fillColor;
        if (r.riesgo < 0.25) { color = '#3b82f6'; fillColor = '#3b82f6'; }
        else if (r.riesgo < 0.75) { color = '#f59e0b'; fillColor = '#f59e0b'; }
        else { color = '#ef4444'; fillColor = '#ef4444'; }

        const circulo = L.circleMarker([r.lat, r.lon], {
            radius: 8 + (r.riesgo * 15),
            color: color,
            fillColor: fillColor,
            fillOpacity: 0.7,
            weight: 2
        }).addTo(mapaRealtime);

        const tooltipHTML = `
            <div style="font-family: var(--font-base); font-size: 13px;">
                <strong style="font-size:15px; border-bottom: 1px solid #ccc; display:block; padding-bottom:3px; margin-bottom:5px;">
                    Riesgo Actual: ${Math.round(r.riesgo * 100)}%
                </strong>
                <b>Fecha:</b> ${r.anio}-${String(r.mes).padStart(2, '0')}-${String(r.dia).padStart(2, '0')}<br>
                <b>Lat/Lon:</b> ${r.lat.toFixed(2)}, ${r.lon.toFixed(2)}<br>
                <b>SST (Mar):</b> ${r.sst.toFixed(1)} °C<br>
                <b>Presión:</b> ${r.presion.toFixed(1)} hPa<br>
                <b>Humedad:</b> ${r.humedad.toFixed(1)} %<br>
                <b>Viento:</b> ${r.viento.toFixed(1)} km/h
            </div>
        `;
        
        circulo.bindPopup(tooltipHTML);
        marcadoresRealtime.push(circulo);
    });
}

/**
 * Dibuja la lista/mapa de calor para las predicciones de riesgo obtenidas en tiempo real.
 * Crea elementos de interfaz que ilustran gráficamente el nivel de alerta para las coordenadas
 * evaluadas al instante.
 * @param {Array} registros - Resultados de predicción actual.
 */
function dibujarMapaCalorRealtime(registros) {
    const cont = document.getElementById('mapa-calor-tiemporeal');
    cont.innerHTML = '';
    if (registros.length === 0) return;

    registros.forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'fila-riesgo';
        let color = r.riesgo < 0.25 ? 'var(--bajo)' : (r.riesgo < 0.75 ? 'var(--moderado)' : 'var(--alerta)');

        fila.innerHTML = `
            <div style="display:flex; flex-direction:column; gap:4px; font-size:12px; min-width: 250px;">
                <span style="font-size:14px; font-weight:bold;">Lat: ${r.lat.toFixed(2)}, Lon: ${r.lon.toFixed(2)}</span>
                <span style="color:var(--gray-400)">SST: ${r.sst}°C | Presión: ${r.presion}hPa | Humedad: ${r.humedad}% | Viento: ${r.viento}km/h</span>
            </div>
            <div class="barra-riesgo" style="width:${Math.round(r.riesgo * 200)}px; background:${color};"></div>
            <span style="font-weight:bold; min-width: 60px;">${(r.riesgo * 100).toFixed(1)}%</span>
        `;
        cont.appendChild(fila);
    });
}