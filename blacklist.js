#!/usr/bin/env node
const api = require("./index");
const yargs = require("yargs");

api.init();
api.attach();

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
    ({ type, target, reason }) => {
      api.blacklist.add(type, target, reason);
      setImmediate(() => process.exit(0));
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
    ({ type, target }) => {
      api.blacklist.remove(type, target);
      setImmediate(() => process.exit(0));
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
    ({ type, target, reason }) => {
      api.blacklist.kick(type, target, reason);
      setImmediate(() => process.exit(0));
    }
  )
  .command(
    "fetch",
    "Fetch blacklist",
    y => y,
    async () => {
      console.log(await api.blacklist.fetch());
      process.exit(0);
    }
  )
  .help().argv;

