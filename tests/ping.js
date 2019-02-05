const api = require("..")
console.log(1);
api.init();
console.log(2);
console.log(api.attach());
console.log(3);

console.log(api);

// api.core.log(console.log);
api.core.ping().then(console.log);
api.command.execute("test", "/help").then(console.log);
// (async () => {
//   console.log("config", await api.core.config);
// })();
