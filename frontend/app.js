// ==========================================================
// MODULO 7 — FRONTEND / VISUALIZADOR
// Responsable: (asignar integrante)
//
// Toda la comunicacion con C pasa por M.ccall(), usando las
// funciones exportadas en api_bridge.c (Modulo 6). Este archivo
// NO implementa el algoritmo: solo pide datos y los dibuja.
// ==========================================================

let M; // instancia del modulo WASM (equivalente a "BisectModule" en ProyectoBisect)

// Animacion de la barra de carga mientras compila/instancia el WASM
const barFill = document.getElementById('loading-bar-fill');
let progreso = 0;
const intervaloBarra = setInterval(() => {
    if (progreso < 90) {
        progreso += Math.random() * 8;
        barFill.style.width = Math.min(progreso, 90) + '%';
    }
}, 400);

// TODO: el nombre "FuzzyModule" debe coincidir con -s EXPORT_NAME="FuzzyModule"
// usado en compilar.bat al generar frontend/fcm.js
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
function cargarArchivo(tipo) {
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

        // TODO: llamar a api_importar_datos(contenido) via M.ccall
        // const resultadoPtr = M.ccall('api_importar_datos', 'string', ['string'], [contenido]);
        // const resultado = JSON.parse(resultadoPtr);
        // mostrar resultado.mensaje / resultado.importados en #resultado-importacion
        // y, si tipo === 'test', guardar los registros para pasarlos a api_predecir

        document.getElementById('resultado-importacion').textContent =
            'TODO: conectar con api_importar_datos() del Modulo 6';
    };
    lector.readAsText(archivo);
}

// ==========================================================
// VISTA 2 — ENTRENAMIENTO (llama a Modulo 3 + Modulo 4)
// ==========================================================
function entrenarModelo() {
    const nClusters = parseInt(document.getElementById('param-clusters').value, 10);
    const m = parseFloat(document.getElementById('param-m').value);
    const maxIter = parseInt(document.getElementById('param-max-iter').value, 10);

    // TODO: llamar a api_entrenar(anio_inicio, anio_fin, n_clusters, m, max_iter, epsilon)
    // const resultadoPtr = M.ccall('api_entrenar', 'string',
    //     ['number','number','number','number','number','number'],
    //     [2015, 2016, nClusters, m, maxIter, 0.00001]);
    // const resultado = JSON.parse(resultadoPtr);
    // pintar resultado.centroides en #centroides-box

    document.getElementById('resultado-entrenamiento').textContent =
        'TODO: conectar con api_entrenar() del Modulo 6';
}

// ==========================================================
// VISTA 3 — PREDICCION (llama a Modulo 4 + dibuja mapa de calor)
// ==========================================================
function predecir() {
    // TODO: const resultadoPtr = M.ccall('api_predecir', 'string', [], []);
    // const registros = JSON.parse(resultadoPtr);
    // dibujarMapaCalor(registros);
    document.getElementById('mapa-calor').textContent =
        'TODO: conectar con api_predecir() del Modulo 6';
}

// Dibuja una fila por registro, coloreada segun el riesgo (mu al cluster
// de alto riesgo). riesgo va de 0.0 (verde/bajo) a 1.0 (rojo/alto).
function dibujarMapaCalor(registros) {
    const cont = document.getElementById('mapa-calor');
    cont.innerHTML = '';
    registros.forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'fila-riesgo';
        fila.innerHTML = `
            <span>${r.anio}-${r.mes}-${r.dia}</span>
            <div class="barra-riesgo" style="width:${Math.round(r.riesgo * 200)}px"></div>
            <span>${(r.riesgo * 100).toFixed(1)}%</span>
        `;
        cont.appendChild(fila);
    });
}

// ==========================================================
// VISTA 4 — EVALUACION (llama a Modulo 5)
// ==========================================================
function evaluarModelo() {
    const umbral = parseFloat(document.getElementById('param-umbral').value);

    // TODO: const resultadoPtr = M.ccall('api_evaluar', 'string', ['number'], [umbral]);
    // const resultado = JSON.parse(resultadoPtr);
    // mostrar matriz de confusion + metricas en #resultado-evaluacion

    document.getElementById('resultado-evaluacion').textContent =
        'TODO: conectar con api_evaluar() del Modulo 6';
}
