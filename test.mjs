import createOliNat from './Oli_Nat.mjs';


const output = [];
const mod = await createOliNat({
    print: (line) => output.push(line),
    printErr: (line) => output.push('[error!] ' + line),
});

const runFromSource = mod.cwrap('runFromSource', 'number', ['string']);


console.log('result for Test 1:', runFromSource('#pullf io\nprintln(10);'));
console.log('result for Test 2:', runFromSource('#pullf io\nprintln(20);'));
console.log('compile error:', runSource('println(;'));
console.log('runtime error:', runSource('#pullf io\nprintln(1 + "a");'));
console.log('after errors:', runSource('#pullf io\nprintln(30);'));
console.log(output);
