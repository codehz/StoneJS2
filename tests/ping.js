const StoneServer = require("../index");

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

server.ready.then(async () => {
  console.log("connected!");
  await server.ping();
  console.log("pong!");
  server.disconnect();
});
