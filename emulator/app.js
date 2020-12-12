const tls = require('tls');
const fs = require('fs');
const net = require('net');
const getMAC = require('getmac').default;
const ip = require('ip');
const ping = require('ping');


let openedTLSorTCPSocket = null;
let numberOfBytesToWriteOut = 0;


/**
 * pings a host
 * @param {String} host 
 * @returns {Promise} true for pingable
 */
function pingHost(host) {
    return new Promise((resolve, reject) => {
        ping.sys.probe(host, function (isAlive) {
            resolve(isAlive);
        });
    });
}


/**
 * pings a host and measures round trip time
 * @param {String} host 
 * @returns {Number|Boolean} RTT in ms or false if not successful
 */
async function pingHostWithRTT(host) {
    const before = Date.now();
    const res = await pingHost(host);
    const RTT = Date.now() - before;
    return res ? RTT : false;
}


/**
 * opens a new TLS connection
 * @param {String} host 
 * @param {Number} port
 * @param {Function} connectCallback
 * @param {Function} dataCallback
 * @param {Function} errorCallback
 * @returns {Object} socket
 */
function startTLSConnection(host, port, connectCallback, dataCallback, errorCallback) {
    const options = {
        rejectUnauthorized: false
    };

    const socket = tls.connect(port, host, options, async () => {
        console.log('client connected',
            socket.authorized ? 'authorized' : 'unauthorized');
        try { await connectCallback(); } catch (e) { }
    });

    socket.setEncoding('ascii');

    socket.on('data', async data => {
        console.log(data);
        try { await dataCallback(data); } catch (e) { }
    });

    socket.on('error', async err => {
        console.log(err);
        try { await errorCallback(err); } catch (e) { }
    });

    return socket;
}


/**
 * opens a new TCP connection
 * @param {String} host 
 * @param {Number} port
 * @param {Function} connectCallback 
 * @param {Function} dataCallback 
 * @param {Function} errorCallback 
 * @returns {Object} socket
 */
function startTCPConnection(host, port, connectCallback, dataCallback, errorCallback) {
    const socket = new net.Socket();

    socket.connect(port, host, async () => {
        console.log('TCP client connected to specified host');
        try { await connectCallback(); } catch (e) { }
    });

    socket.on('data', async data => {
        console.log(data);
        try { await dataCallback(data); } catch (e) { }
    });

    socket.on('error', async err => {
        console.log(err);
        try { await errorCallback(err); } catch (e) { }
    });

    return socket;
}


/**
 * handles incoming socket data
 * @param {Object} socket 
 * @param {String} data 
 */
async function handleData(socket, data) {
    if (numberOfBytesToWriteOut > 0) { // just assume data comes at max in 2 parts
        const numBytes = data.length > numberOfBytesToWriteOut ? numberOfBytesToWriteOut : data.length;
        const str = data.substr(0, numBytes);
        data = data.substr(numBytes);
        numberOfBytesToWriteOut = 0;
        openedTLSorTCPSocket.write(str);
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
    } else if (data.startsWith('AT+PING')) {
        const connStr = data.split('=')[1];
        const hostPart = connStr.split(',')[0];
        const host = hostPart.replace(/"/g, '').replace(/\n/g, '').replace(/\r/g, '');
        console.log('Pinging: ' + host + ' ...');
        const pingRes = await pingHostWithRTT(host);
        if (pingRes) {
            socket.write('REACHED HOST: ' + pingRes + ' ms\r\n');
            console.log('REACHED HOST: ' + pingRes + ' ms');
        } else {
            socket.write('HOST UNREACHABLE\r\n');
            console.log('HOST UNREACHABLE');
        }
    } else if (data.startsWith('AT+TLS_START_')) {
        const connStr = data.split('=')[1];
        const hostPart = connStr.split(',')[0];
        const portPart = connStr.split(',')[1];
        const host = hostPart.replace(/"/g, '');
        const port = parseInt(portPart.replace(/"/g, ''));

        console.log('starting TLS connection: ' + host + ':' + port + ' ...');

        openedTLSorTCPSocket = startTLSConnection(host, port, () => {
            socket.write('CONNECTED\n---\n');
        }, data => {
            // write data from tlssocket to tcpsocket
            socket.write(data);
        }, err => {
            if (err.syscall === 'connect') {
                socket.write(`*** Can't connect to ${host}:${port} ***\n---\n`);
            }
        });
    } else if (data.startsWith('AT+TCP_START_')) {
        const connStr = data.split('=')[1];
        const hostPart = connStr.split(',')[0];
        const portPart = connStr.split(',')[1];
        const host = hostPart.replace(/"/g, '');
        const port = parseInt(portPart.replace(/"/g, ''));

        console.log('starting TCP connection: ' + host + ':' + port + ' ...');

        openedTLSorTCPSocket = startTCPConnection(host, port, () => {
            socket.write('CONNECTED\n---\n');
        }, data => {
            // write data from this tcpsocket to tcpsocket
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
        openedTLSorTCPSocket.write(otherData);
        numberOfBytesToWriteOut = numberOfBytesToWrite - numberOfWrittenBytes;
    } else if (data === 'AT+TLS_CLOSE\n' || data === 'AT+TCP_CLOSE\n') {
        if (openedTLSorTCPSocket) {
            try {
                openedTLSorTCPSocket.close();
            } catch (e) {
                openedTLSorTCPSocket.end();
            }
            openedTLSorTCPSocket = null;
        }
        socket.write('OK\r\n');
    } else {
        socket.write('*** UNSUPPORTED COMMAND ***\r\n');
    }
}


console.log('ESP8266 Emulator is starting ...');

const server = net.createServer(socket => {
    console.log('client connected.');

    socket.setEncoding('ascii');

    socket.on('data', async data => {
        try { await handleData(socket, data); } catch (e) { }
    });

    socket.on('end', () => {
        console.log('client disconnected.');
    });
});

server.listen(7000, '127.0.0.1');

console.log('Listening on 127.0.0.1:7000\n');