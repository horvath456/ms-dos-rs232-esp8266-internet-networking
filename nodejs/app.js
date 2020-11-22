const tls = require('tls');
const fs = require('fs');

const options = {
  key: fs.readFileSync('server-key.pem'),
  cert: fs.readFileSync('server-cert.pem'),
};

let clients = [];

function sendMessage(msg) {
  clients.forEach(client => {
    client.write(msg + '\n');
  })
  console.log(msg);
}

const server = tls.createServer(options, (socket) => {
  clients.push(socket);

  socket.on('data', data => {
    const dataString = data.toString().replace(/(\n)/gm, "");
    sendMessage(dataString);
  });

  socket.on('error', err => {
    console.log('An error occured: ' + err.code);
  });

  sendMessage('New client connected.');
  socket.setEncoding('ascii');
});

server.listen(8000, () => {
  console.log('Server started\n');
});