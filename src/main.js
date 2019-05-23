const WebSocket = require("rpc-websockets").Client;
const AsyncQueue = require("./async-queue");

function genfun(clazz, ns, name, altname = name) {
  clazz.prototype[altname] = function(arg = {}) {
    return this.socket.call(`${ns}.${name}`, arg);
  };
}

function gennotify(clazz, ns, name, altname = name) {
  clazz.prototype[altname] = function(arg = {}) {
    return this.socket.notify(`${ns}.${name}`, arg);
  };
}

function genprop(clazz, ns, name, altname = name) {
  Object.defineProperty(clazz.prototype, altname, {
    get() {
      return this.socket.call(`${ns}.${name}`, []);
    }
  });
}

function genevent(clazz, ns, name, altname = name) {
  Object.defineProperty(clazz.prototype, altname, {
    get() {
      const mix = `${ns}.${name}`;
      return new AsyncQueue(async ({ next, throw: fail }) => {
        await this.socket.subscribe(mix);
        this.socket.on(mix, next);
        this.socket.once("error", err => {
          this.socket.off(mix, next);
          this.socket.unsubscribe(mix);
          fail(err);
        });
      });
    }
  });
}

class StoneServer {
  constructor(address) {
    this.socket = new WebSocket(address, {
      reconnect: false
    });
    let resolve, reject;
    this.ready = new Promise((res, rej) => {
      resolve = res;
      reject = rej;
    });
    this.socket.once("open", resolve);
    this.socket.once("error", reject);
  }

  disconnect() {
    this.socket.close();
  }
}

gennotify(StoneServer, "core", "stop");
genfun(StoneServer, "core", "ping");
genprop(StoneServer, "core", "tps");
genprop(StoneServer, "core", "config");
genprop(StoneServer, "core", "online_players");
genevent(StoneServer, "core", "log");
genevent(StoneServer, "core", "player_join");
genevent(StoneServer, "core", "player_left");

gennotify(StoneServer, "chat", "send", "broadcast_message");
gennotify(StoneServer, "chat", "raw", "broadcast_raw");
genevent(StoneServer, "chat", "recv", "chat");

gennotify(StoneServer, "blacklist", "add", "blacklist_add");
gennotify(StoneServer, "blacklist", "remove", "blacklist_remove");
gennotify(StoneServer, "blacklist", "kick", "kick");
genprop(StoneServer, "blacklist", "fetch", "blacklist");

genfun(StoneServer, "command", "execute");

gennotify(StoneServer, "script", "emit", "script_emit");
genevent(StoneServer, "script", "broadcast", "script_event");

module.exports = StoneServer;
