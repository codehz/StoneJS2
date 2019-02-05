const api = require("../index");

console.log("init", api.init());
console.log("attach", api.attach());

(async () => {
  console.log("ping!");
  await api.core.ping();
  console.log("pong!");
  console.log("config: " + (await api.core.config));
  api.chat.recv((err, data) => {
    console.log("chat: ", data);
  });
  api.core.log("*", (err, entry) => {
    console.log("log: ", entry);
  });
  api.core.online_players.onadd(info => {
    console.log("player added: ", info);
  });
  api.core.online_players.onremove(info => {
    console.log("player removed: ", info);
  });
  console.log("query codehz", await api.core.players.get("CodeHz"));
  console.log("executing /help");
  console.log(await api.command.execute("sender", "/help"));
  console.log("executing /id (maybe not exists)");
  console.log(await api.command.execute("sender", "/id"));
})();
