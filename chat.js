#!/usr/bin/env node
const StoneServer = require("./index");

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

server.ready.then(async () => {
  for await (item of server.chat) {
    console.log(item);
  }
  console.log("!!");
});
