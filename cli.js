#!/usr/bin/env node
const api = require("./index");
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: true
});

api.init();
api.attach();

rl.prompt(true);

rl.on("line", async line => {
  process.stdout.write(await api.command.execute("cli", line));
  rl.prompt(true);
}).on('close', () => {
  console.log('Have a great day!');
  process.exit(0);
});