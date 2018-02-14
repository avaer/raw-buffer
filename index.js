const path = require('path');
const {RawBuffer} = require(path.join(__dirname, 'build', 'Release', 'raw_buffer.node'));
module.exports = RawBuffer;
