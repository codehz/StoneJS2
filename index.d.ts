PlayerInfo;

export type NormalMessage = {
  sender: string;
  content: string;
};

export type PlayerInfo = {
  name: string;
  uuid: string;
  xuid: string;
};

export type LogEntry = {
  tag: string;
  level: 0 | 1 | 2 | 3 | 4;
  content: string;
};

export type BlacklistOP = {
  type: string;
  content: string;
};

export type BlacklistOPWithReason = {
  type: string;
  content: string;
  reason: string;
};

export type CommandRequest = {
  sender: string;
  content: string;
};

export type EventData = {
  name: string;
  data: string;
};

class StoneServer {
  constructor(address: string);
  ready: Promise<void>;
  disconnect(): void;

  stop(): Promise<void>;
  ping(): Promise<void>;
  get tps(): Promise<number>;
  get config(): Promise<string>;
  get online_players(): Promise<PlayerInfo[]>;
  get log(): AsyncIterable<LogEntry>;
  get player_join(): AsyncIterable<LogEntry>;
  get player_left(): AsyncIterable<LogEntry>;

  broadcast_message(msg: NormalMessage): Promise<void>;
  broadcast_raw(raw: string): Promise<void>;
  get chat(): AsyncIterable<NormalMessage>;

  blacklist_add(op: BlacklistOPWithReason): Promise<void>;
  blacklist_remove(op: BlacklistOP): Promise<void>;
  kick(op: BlacklistOPWithReason): Promise<void>;
  get blacklist(): Promise<BlacklistOPWithReason[]>;

  execute(req: CommandRequest): Promise<string>;

  script_emit(data: EventData): Promise<void>;
  get script_event(): Promise<EventData>;
}

export = StoneServer;
