const fs = require('fs');
const fcm = require('./frontend/fcm.js');

fcm().then(M => {
    // 1. Entrenar el modelo con datos dummy para que g_entrenado = 1
    console.log("Cargando y entrenando...");
    const dummyTrain = "anio,mes,dia,lat,lon,sst,presion,humedad,viento,cizalladura,hubo_ciclon\n" +
                       "2015,1,1,10,-60,26,1010,70,20,10,0\n" +
                       "2015,1,2,11,-61,27,1005,80,30,12,1\n";
    const loadTrain = M.ccall('api_cargar_prueba', 'string', ['string', 'number', 'number'], [dummyTrain, 2015, 2015]);
    console.log("Train load:", loadTrain);
    const trainRes = M.ccall('api_entrenar', 'string', ['number', 'number', 'number', 'number', 'number', 'number'], [2015, 2015, 2, 2.0, 100, 0.00001]);
    console.log("Train:", trainRes);

    // 2. Probar Realtime
    const jsonStr = JSON.stringify([{
        anio: 2026, mes: 7, dia: 29,
        lat: 15.0, lon: -75.0,
        sst: 26.0, presion: 1010.0, humedad: 70.0, viento: 20.0,
        cizalladura: 10.0, hubo_ciclon: 0
    }]);
    
    console.log("Calling api_cargar_prueba with realtime JSON...");
    const res = M.ccall('api_cargar_prueba', 'string', ['string', 'number', 'number'], [jsonStr, 2026, 2026]);
    console.log("Result Load Realtime:", res);

    const predRes = M.ccall('api_predecir', 'string', [], []);
    console.log("Result Prediccion:", predRes);
}).catch(console.error);
