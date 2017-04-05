exports.appendBuffer = function( buffer1, buffer2 ) {
    var tmp = new Uint8Array( buffer1.byteLength + buffer2.byteLength );
    tmp.set( buffer1, 0 );
    tmp.set( buffer2, buffer1.byteLength );
    return tmp;
};
