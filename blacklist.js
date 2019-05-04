#!/usr/bin/env node
const StoneServer = require("./index");
const yargs = require("yargs");

const server = new StoneServer(
  process.env.API_ENDPOINT || "ws+unix://data/api.socket"
);

server.ready.then(async () => {
  yargs
    .usage("$0 <cmd> [args]")
    .command(
      "add",
      "Add target to blacklist",
      y =>
        y
          .choices("type", ["name", "uuid", "xuid"])
          .option("target", { type: "string" })
          .option("reason", { type: "string" })
          .demandOption(["type", "target", "reason"]),
      async ({ type, target, reason }) => {
        await server.blacklist_add({ [type]: target, reason });
        server.disconnect();
      }
    )
    .command(
      "remove",
      "Remove target from blacklist",
      y =>
        y
          .choices("type", ["name", "uuid", "xuid"])
          .option("target", { type: "string" })
          .demandOption(["type", "target"]),
      async ({ type, target }) => {
        await server.blacklist_add({ [type]: target });
        server.disconnect();
      }
    )
    .command(
      "kick",
      "Kick a target",
      y =>
        y
          .choices("type", ["name", "uuid", "xuid"])
          .option("target", { type: "string" })
          .option("reason", { type: "string" })
          .demandOption(["type", "target", "reason"]),
      async ({ type, target, reason }) => {
        await server.kick({ [type]: target, reason });
        server.disconnect();
      }
    )
    .command(
      "fetch",
      "Fetch blacklist",
      y => y,
      async () => {
        console.log(await server.blacklist);
        server.disconnect();
      }
    )
    .help().argv;
});
