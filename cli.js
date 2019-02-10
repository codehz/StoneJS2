#!/usr/bin/env node
const api = require("./index");
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

console.log("init", api.init());
console.log("attach", api.attach());

rl.prompt();

rl.on("line", async line => {
  console.log((await api.command.execute("cli", line)).trim());
  rl.prompt();
}).on('close', () => {
  console.log('Have a great day!');
  process.exit(0);
});