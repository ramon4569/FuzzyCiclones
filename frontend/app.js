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
}, 400);

FuzzyModule().then(mod => {
    M = mod;
    console.log("WASM cargado (Fuzzy C-Means)");

    clearInterval(intervaloBarra);
    barFill.style.width = '100%';
    setTimeout(() => {
        document.getElementById('loading-screen').style.display = 'none';
    }, 300);
});

// ==========================================================
// NAVEGACION ENTRE VISTAS
// ==========================================================
function mostrarVista(nombre) {
    document.querySelectorAll('.vista').forEach(v => v.style.display = 'none');
    document.getElementById('vista-' + nombre).style.display = 'block';
    document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
    event.target.classList.add('active');
}

function getString(ptr) {
    return M.UTF8ToString(ptr);
}

// ==========================================================
// VISTA 1 — IMPORTAR DATOS (llama a Modulo 1 + Modulo 2 via Modulo 6)
// ==========================================================

let datosPruebaCargados = false;

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

// ==========================================================
// VISTA 2 — ENTRENAMIENTO (llama a Modulo 3 + Modulo 4)
// ==========================================================
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
function predecir() {
    if (!M) {
        alert('Espera a que el modulo WASM termine de cargar');
        return;
    }

    if (!datosPruebaCargados) {
        alert('Primero carga el archivo de prueba (2017) en la pestaña "1. Datos"');
        return;
    }

    try {
        const resultadoPtr = M.ccall('api_predecir', 'string', [], []);
        const registros = JSON.parse(resultadoPtr);

        console.log(`Predicciones recibidas: ${registros.length} registros`);
        dibujarMapaCalor(registros);

    } catch (error) {
        console.error('Error en M.ccall api_predecir:', error);
        alert('Error: ' + error.message);
    }
}

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

        fila.innerHTML = `
            <span>${r.anio}-${String(r.mes).padStart(2, '0')}-${String(r.dia).padStart(2, '0')}</span>
            <div class="barra-riesgo" style="width:${Math.round(r.riesgo * 200)}px; background:${color};"></div>
            <span>${(r.riesgo * 100).toFixed(1)}%</span>
            ${r.hubo_ciclon ? '<span style="color: var(--alerta);">🌀 ciclón real</span>' : ''}
        `;
        cont.appendChild(fila);
    });
}

// ==========================================================
// VISTA 4 — EVALUACION (llama a Modulo 5)
// ==========================================================
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