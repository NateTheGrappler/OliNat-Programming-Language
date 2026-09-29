import createOliNat from './Oli_Nat.mjs';


const output = [];
const mod = await createOliNat({
    print: (line) => output.push(line),
                               printErr: (line) => output.push('[error!] ' + line),
});

const runFromSource = mod.cwrap('runFromSource', 'number', ['string']);


console.log('result for Test 1:', runFromSource('#pullf io\nprintln(10);'));
console.log('result for Test 2:', runFromSource('#pullf io\nprintln(20);'));
console.log('compile error:', runFromSource('println(;'));
console.log('runtime error:', runFromSource('#pullf io\nprintln(1 + "a");'));
console.log('make test:', runFromSource('#pullf io\nmake int x = 3;\n println(x);'));
console.log('make test missing Colon:', runFromSource('#pullf io\nmake int x = 3*0;\n println(x)'));
console.log('after errors:', runFromSource('#pullf io\nprintln(30);'));
console.log(output);
