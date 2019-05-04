#!/usr/bin/env node
const StoneServer = require("./index");
const readline = require("readline");

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: true
});

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

const executor = process.env.SENDER || "cli";

server.ready.then(async () => {
  rl.prompt(true);
  rl.on("line", async line => {
    process.stdout.write(
      await server.execute({ sender: executor, content: line })
    );
    rl.prompt(true);
  }).on("close", () => {
    console.log("Have a great day!");
    server.disconnect()
  });
});
