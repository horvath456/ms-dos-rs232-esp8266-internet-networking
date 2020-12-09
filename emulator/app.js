const tls = require('tls');
const fs = require('fs');
const net = require('net');
const getMAC = require('getmac').default;
const ip = require('ip');


let openedTLSSocket = null;
let numberOfBytesToWriteOut = 0;


/**
 * opens a new TLS connection
 * @param {String} host 
 * @param {Number} port
 * @param {Function} dataCallback 
 * @returns {Object} socket
 */
function startTLSConnection(host, port, connectCallback, dataCallback, errorCallback) {
    const options = {
        rejectUnauthorized: false
    };

    const socket = tls.connect(port, host, options, () => {
        console.log('client connected',
            socket.authorized ? 'authorized' : 'unauthorized');
        connectCallback();
    });

    socket.setEncoding('ascii');

    socket.on('data', data => {
        console.log(data);
        dataCallback(data);
    });

    socket.on('error', err => {
        console.log(err);
        errorCallback(err);
    });

    return socket;
}


/**
 * handles incoming socket data
 * @param {Object} socket 
 * @param {String} data 
 */
function handleData(socket, data) {
    if (numberOfBytesToWriteOut > 0) { // just assume data comes at max in 2 parts
        const numBytes = data.length > numberOfBytesToWriteOut ? numberOfBytesToWriteOut : data.length;
        const str = data.substr(0, numBytes);
        data = data.substr(numBytes);
        numberOfBytesToWriteOut = 0;
        openedTLSSocket.write(str);
        return; // consider data handled ;)
    }

    if (data === 'AT\n') {
        socket.write('OK\r\n');
    } else if (data === 'AT+INFO\n') {
        socket.write('ESP8266_EMULATOR_FOR_DOSBOX\r\n');
    } else if (data === 'AT+RESET\n') {
        socket.write('OK\r\n');
    } else if (data === 'AT+GET_MAC\n') {
        socket.write(getMAC() + '\r\n');
    } else if (data === 'AT+GET_IP\n') {
        socket.write(ip.address() + '\r\n');
    } else if (data === 'AT+INIT_TLS\n') {
        socket.write('OK\r\n');
    } else if (data.startsWith('AT+TLS_START_')) {
        const connStr = data.split('=')[1];
        const hostPart = connStr.split(',')[0];
        const portPart = connStr.split(',')[1];
        const host = hostPart.replace(/"/g, '');
        const port = parseInt(portPart.replace(/"/g, ''));

        console.log('starting TLS connection: ' + host + ':' + port + ' ...');

        openedTLSSocket = startTLSConnection(host, port, () => {
            socket.write('CONNECTED\n---\n');
        }, data => {
            // write data from tlssocket to tcpsocket
            socket.write(data);
        }, err => {
            if (err.syscall === 'connect') {
                socket.write(`*** Can't connect to ${host}:${port} ***\n---\n`);
            }
        });
    } else if (data.startsWith('AT+SEND')) {
        const firstLine = data.split('\n')[0];
        const otherData = data.split('\n').slice(1).join('\n');
        const numberOfBytesToWrite = parseInt(firstLine.split('=')[1]);
        const numberOfWrittenBytes = otherData.length;
        openedTLSSocket.write(otherData);
        numberOfBytesToWriteOut = numberOfBytesToWrite - numberOfWrittenBytes;
    } else if (data === 'AT+TLS_CLOSE\n') {
        if (openedTLSSocket) {
            openedTLSSocket.close();
            openedTLSSocket = null;
        }
        socket.write('OK\r\n');
    } else {
        socket.write('*** UNSUPPORTED COMMAND ***\r\n');
    }
}


console.log('Listening on 127.0.0.1:7000\n');


const server = net.createServer(socket => {
    socket.setEncoding('ascii');

    socket.on('data', (data) => {
        handleData(socket, data);
    });

    socket.on('end', () => {
        console.log('Closed.');
    });
});

server.listen(7000, '127.0.0.1');