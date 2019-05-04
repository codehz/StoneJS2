#!/usr/bin/env node
const StoneServer = require("./index");

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

const executor = process.env.SENDER || "cli";

const args = process.argv;

args.shift();
args.shift();
server.ready
  .then(async () => {
    console.log(
      await server.execute({ sender: executor, content: args.join(" ") })
    );
  })
  .catch(console.warn)
  .finally(() => server.disconnect());
