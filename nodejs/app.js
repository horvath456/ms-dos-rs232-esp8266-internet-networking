const tls = require('tls');
const fs = require('fs');

const options = {
  key: fs.readFileSync('server-key.pem'),
  cert: fs.readFileSync('server-cert.pem'),
};

const server = tls.createServer(options, (socket) => {
  console.log('server connected',
    socket.authorized ? 'authorized' : 'unauthorized');

  socket.on('data', function (data) {

    console.log('Received: %s [it is %d bytes long]',
      data.toString().replace(/(\n)/gm, ""),
      data.length);

      socket.write('Server got data!\n');

  });

  // Let us know when the transmission is over
  socket.on('end', function () {

    console.log('EOT (End Of Transmission)');

  });


  socket.write('welcome!\n');
  socket.setEncoding('ascii');
});

server.listen(8000, () => {
  console.log('server bound');
});