#!/usr/bin/env node
const api = require("./index");

api.init();
api.attach();

api.core.tps().then(tps => {
  console.log(tps);
  process.exit(0);
});
