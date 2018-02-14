const RawBuffer = require('.');

r1 = new RawBuffer(Float32Array.from([1.5, 2.5, 3.5]).buffer);
a1 = r1.toAddress();
s = JSON.stringify(a1);
a2 = JSON.parse(s);
r2 = RawBuffer.fromAddress(a2);

console.log(new Float32Array(r2.getArrayBuffer()), r2.length);

r2.destroy();

console.log(r2.getArrayBuffer(), r2.length);
