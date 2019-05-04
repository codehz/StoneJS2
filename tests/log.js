const StoneServer = require("../index");

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

server.ready.then(async () => {
  console.log("connected!");
  for await (const entry of server.log) {
    console.log(entry);
  }
});
