#!/usr/bin/env node
const api = require("./index");

api.init();
api.attach();

api.chat.recv((err, msg) => {
  console.log(msg);
});