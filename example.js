const RawBuffer = require('.');

r = new RawBuffer(Float32Array.from([1.5, 2.5, 3.5]).buffer);
a1 = r.toAddress();
s = JSON.stringify(a1);
a2 = JSON.parse(s);

console.log(new Float32Array(RawBuffer.fromAddress(a2).getArrayBuffer()));
