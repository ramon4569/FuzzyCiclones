const fs = require('fs');
const fcm = require('./frontend/fcm.js');

fcm().then(M => {
    const jsonStr = JSON.stringify([{
        anio: 2026, mes: 7, dia: 29,
        lat: 15.0, lon: -75.0,
        sst: 26.0, presion: 1010.0, humedad: 70.0, viento: 20.0,
        cizalladura: 10.0, hubo_ciclon: 0
    }]);
    
    console.log("Calling api_cargar_prueba with string...");
    const res = M.ccall(
        'api_cargar_prueba',
        'string',
        ['string', 'number', 'number'],
        [jsonStr, 2026, 2026]
    );
    
    console.log("Result:", res);
}).catch(console.error);
