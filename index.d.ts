export function init(): void;
export function init(path: string): void;
export function init(host: string, port: number): void;

export type x = ReturnType<typeof init>;

export function attach(): void;

type ecb<T> =
  | ((error: null, input: T) => void)
  | ((error: string, input: null) => void);

export type ProxiedAction<T> = (...input: InputType<T>) => void;
export type ProxiedMethod<T, R> = (
  ...input: InputType<T>
) => Promise<OutputType<R>>;
export type ProxiedProperty<T> = Promise<OutputType<T>>;
export type ProxiedBoardcast<T> = (callback: ecb<OutputType<T>>) => void;
export type ProxiedPatternBoardcast<T> = (
  filter: string,
  callback: ecb<OutputType<T>>
) => void;
export type ProxiedSet<T> = {
  items: Promise<OutputType<T>[]>;
  onclear: (cb: (n: null) => void) => void;
  onadd: (cb: (out: OutputType<T>) => void) => void;
  onremove: (cb: (out: OutputType<T>) => void) => void;
};
export type ProxiedHash<K, V> = {
  get: (...key: InputType<K>) => Promise<OutputType<V>>;
};

export type Empty = void;

export type LogEntry = {
  tag: string;
  level: 0 | 1 | 2 | 3 | 4;
  content: string;
};

export type PlayerInfo = {
  name: string;
  uuid: string;
  xuid: string;
};

export type NormalMessage = {
  sender: string;
  content: string;
};

export type CommandRequest = {
  sender: string;
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

export type InputType<T> = T extends Empty
  ? []
  : T extends NormalMessage
  ? [string, string]
  : T extends CommandRequest
  ? [string, string]
  : T extends BlacklistOP
  ? [string, string]
  : T extends BlacklistOPWithReason
  ? [string, string, string]
  : [T];
export type OutputType<T> = T extends Empty ? null : T;

export declare const core: {
  stop: ProxiedAction<Empty>;
  ping: ProxiedMethod<Empty, Empty>;
  config: ProxiedProperty<string>;
  log: ProxiedPatternBoardcast<LogEntry>;
  players: ProxiedHash<string, PlayerInfo>;
  online_players: ProxiedSet<PlayerInfo>;
};

export declare const chat: {
  send: ProxiedAction<NormalMessage>;
  recv: ProxiedBoardcast<NormalMessage>;
  raw: ProxiedAction<string>;
};

export declare const command: {
  execute: ProxiedMethod<CommandRequest, string>;
};

export declare const blacklist: {
  add: ProxiedAction<BlacklistOPWithReason>;
  kick: ProxiedAction<BlacklistOPWithReason>;
  remove: ProxiedAction<BlacklistOP>;
};
