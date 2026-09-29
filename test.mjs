import createOliNat from './Oli_Nat.mjs';

const output = [];

//scripted answers for intake(), handed out in order
const inputs = ['Natan', '42'];
let inputIndex = 0;

const mod = await createOliNat({
    print: (line) => output.push(line),
    printErr: (line) => output.push('[error!] ' + line),

    //stands in for the user typing into the terminal
    requestInput: async () => {
        const answer = inputs[inputIndex++] ?? '';
        output.push(`[input] ${answer}`);
        return answer;
    },
});

const runFromSource = mod.cwrap('runFromSource', 'number', ['string'], { async: true });

//run each test to completion before starting the next one
const test = async (name, source) => {
    console.log(`${name}:`, await runFromSource(source));
};

await test('Test 1', '#pullf io\nprintln(10);');
await test('Test 2', '#pullf io\nprintln(20);');
await test('make test', '#pullf io\nmake int x = 3;\nprintln(x);');
await test('compile error', 'println(;');
await test('after errors', '#pullf io\nprintln(30);');

//input: one intake, then two in a row to check the VM resumes correctly each time
await test('input test', '#pullf io\nmake string answer = intake("name? ");\nprintln(answer);');
await test('double input', '#pullf io\nmake string a = intake();\nprintln(a);\nmake string b = intake();\nprintln(b);');

//runs normally again after input-pausing runs
await test('after input', '#pullf io\nprintln(40);');

console.log(output);
