const fs = require('fs');
const FuzzyModule = require('./frontend/fcm.js');

FuzzyModule().then(M => {
    // 1. Import train data
    const trainData = fs.readFileSync('data/entrenamiento_2015_2016_real.csv', 'utf8');
    M.ccall('api_importar_datos', 'string', ['string'], [trainData]);
    
    const debugStr1 = M.ccall('api_debug_dataset', 'string', ['number'], [1]);
    console.log("Dataset before:", debugStr1);
    
    // 2. Train model
    const resEntrenar = M.ccall('api_entrenar', 'string', 
        ['number', 'number', 'number', 'number', 'number', 'number'], 
        [2015, 2016, 3, 2.0, 100, 0.00001]);
    console.log("Train result:", resEntrenar);
    
    const debugStr2 = M.ccall('api_debug_dataset', 'string', ['number'], [1]);
    console.log("Dataset after:", debugStr2);
    
    // 3. Dump vectors
    try {
        const debugStr = M.ccall('api_debug_train_vector', 'string', ['number'], [1]);
        console.log("Vector[1]:", debugStr);
    } catch(e) { console.log(e); }
    
});
