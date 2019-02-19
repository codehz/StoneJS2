#!/usr/bin/env node
const api = require("./index");

api.init();
api.attach();

const args = process.argv;

args.shift();
args.shift();
api.command.execute("server", args.join(" ")).then(result => {
  console.log(result.trim());
  process.exit(0);
});
