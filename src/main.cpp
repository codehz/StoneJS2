#include <nan.h>
#include <stone-api/Blacklist.h>
#include <stone-api/Chat.h>
#include <stone-api/Command.h>
#include <stone-api/Core.h>
#include <stone-api/Script.h>

#include <cstdio>
#include <cstdlib>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <typeinfo>
#include <utility>

template <typename T> static T &GetInstance() {
  static T storage;
  return storage;
}

v8::Local<v8::String> operator"" _v8(char const *str, size_t) { return Nan::New(str).ToLocalChecked(); }

template <typename F> auto makePromise(F f) {
  Nan::EscapableHandleScope scope;
  auto res = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
  f(
      [iso = v8::Isolate::GetCurrent(), persistent = new Nan::Persistent<v8::Promise::Resolver>(res)](auto thing) {
        Nan::HandleScope scope;
        auto ctx = v8::Context::New(iso);
        auto res = persistent->Get(iso);
        res->Resolve(ctx, thing).ToChecked();
        iso->RunMicrotasks();
        delete persistent;
      },
      [iso = v8::Isolate::GetCurrent(), persistent = new Nan::Persistent<v8::Promise::Resolver>(res)](auto thing) {
        Nan::HandleScope scope;
        auto ctx = v8::Context::New(iso);
        auto res = persistent->Get(iso);
        res->Reject(ctx, thing).ToChecked();
        iso->RunMicrotasks();
        delete persistent;
      });
  return res->GetPromise();
}

#define CALLBACK (const Nan::FunctionCallbackInfo<v8::Value> &info)
#define V8CALLBACK (const v8::FunctionCallbackInfo<v8::Value> &info)

#define PROPERTY (v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &info)

struct APIGenerator;

namespace api {
template <template <typename> typename Service> struct ProxiedService<::APIGenerator, Service> : Named {
  static Service<ClientSide> impl;
  v8::Local<v8::Object> obj;
  ProxiedService(std::string const &name)
      : Named(name)
      , obj(Nan::New<v8::Object>()) {}
  template <typename T> void provide(T target) { Nan::Set(target, Nan::New(name).ToLocalChecked(), obj); }
  template <typename T> inline void $(T &target) { target.$(obj, name); }
};
} // namespace api

template <int length, typename O, typename F> void SetFunc(O obj, std::string name, F f) {
  static std::vector<F> list;
  v8::Local<v8::Function> t = v8::Function::New(Nan::GetCurrentContext(), +[] V8CALLBACK { list[info.Data()->ToInteger()->Value()](info); },
                                                Nan::New((int32_t)list.size()), length, v8::ConstructorBehavior::kThrow)
                                  .ToLocalChecked();
  list.emplace_back(std::move(f));
  v8::Local<v8::String> fn_name = Nan::New(name).ToLocalChecked();
  t->SetName(fn_name);
  Nan::Set(obj, fn_name, t);
}

template <typename O, typename F> void SetProp(O obj, std::string name, F f) {
  static std::vector<F> list;
  v8::Local<v8::String> fn_name = Nan::New(name).ToLocalChecked();
  Nan::SetAccessor(obj, fn_name, [] PROPERTY { list[info.Data()->ToInteger()->Value()](name, info); }, 0, Nan::New((int32_t)list.size()));
  list.emplace_back(std::move(f));
}

template <typename T> struct V8IO;

template <> struct V8IO<api::Empty> {
  static constexpr auto length = 0;
  template <typename Info> static char const *read(Info const &info) { return api::Buffer::ZERO; }
  static auto write(char const *) { return Nan::Null(); }
};

template <> struct V8IO<int> {
  static constexpr auto length = 1;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsNumber()) Nan::ThrowTypeError("wrong argument");
    return api::Serializble<int>::write(info[0]->NumberValue());
  }
  static auto write(char const *data) { return Nan::New(atoi(data)); }
};

template <> struct V8IO<std::string> {
  static constexpr auto length = 1;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String str(info[0]);
    return api::Buffer::copy(*str);
  }
  static auto write(char const *data) { return Nan::New(data).ToLocalChecked(); }
};

template <> struct V8IO<api::PlayerInfo> {
  static auto write(char const *data) {
    auto info = api::Serializble<api::PlayerInfo>::read(data);
    auto obj  = Nan::New<v8::Object>();
    Nan::Set(obj, "name"_v8, Nan::New(info.name).ToLocalChecked());
    Nan::Set(obj, "uuid"_v8, Nan::New(info.uuid).ToLocalChecked());
    Nan::Set(obj, "xuid"_v8, Nan::New(info.xuid).ToLocalChecked());
    return obj;
  }
};

template <> struct V8IO<api::CommandRequest> {
  static constexpr auto length = 2;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString() || !info[1]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String sender(info[0]);
    Nan::Utf8String command(info[1]);
    return api::Buffer::format("%s\n%s", *sender, *command);
  }
};

template <> struct V8IO<api::NormalMessage> {
  static constexpr auto length = 2;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString() || !info[1]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String sender(info[0]);
    Nan::Utf8String content(info[1]);
    return api::Buffer::format("%s\n%s", *sender, *content);
  }
  static auto write(char const *data) {
    auto msg = api::Serializble<api::NormalMessage>::read(data);
    auto obj = Nan::New<v8::Object>();
    Nan::Set(obj, "sender"_v8, Nan::New(msg.sender).ToLocalChecked());
    Nan::Set(obj, "content"_v8, Nan::New(msg.content).ToLocalChecked());
    return obj;
  }
};

template <> struct V8IO<api::BlacklistOP<false>> {
  static constexpr auto length = 2;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString() || !info[1]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String type(info[0]);
    Nan::Utf8String content(info[1]);
    return api::Buffer::format("%s\n%s", *type, *content);
  }
};

template <> struct V8IO<api::BlacklistOP<true>> {
  static constexpr auto length = 3;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString() || !info[1]->IsString() || !info[2]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String type(info[0]);
    Nan::Utf8String content(info[1]);
    Nan::Utf8String reason(info[2]);
    return api::Buffer::format("%s\n%s\n%s", *type, *content, *reason);
  }
};

template <> struct V8IO<api::LogEntry> {
  static auto rebuild(char const *identify, char const *data) {
    auto entry = api::LogEntry::rebuild(identify, data);
    auto obj   = Nan::New<v8::Object>();
    Nan::Set(obj, "tag"_v8, Nan::New(entry.tag).ToLocalChecked());
    Nan::Set(obj, "level"_v8, Nan::New(entry.level));
    Nan::Set(obj, "content"_v8, Nan::New(entry.content).ToLocalChecked());
    return obj;
  }
};

template <> struct V8IO<api::EventData> {
  static constexpr auto length = 2;
  template <typename Info> static api::Buffer read(Info const &info) {
    if (info.Length() < length || !info[0]->IsString() || !info[1]->IsString()) Nan::ThrowTypeError("wrong argument");
    Nan::Utf8String sender(info[0]);
    Nan::Utf8String content(info[1]);
    return api::Buffer::format("%s\n%s", *sender, *content);
  }
  static auto rebuild(char const *identify, char const *data) {
    auto entry = api::EventData::rebuild(identify, data);
    auto obj   = Nan::New<v8::Object>();
    Nan::Set(obj, "name"_v8, Nan::New(entry.name).ToLocalChecked());
    Nan::Set(obj, "data"_v8, Nan::New(entry.data).ToLocalChecked());
    return obj;
  }
};

struct APIGenerator {
  template <typename T> struct proxied_action : api::Named {
    proxied_action(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      SetFunc<V8IO<T>::length>(obj, name,
                               [=] V8CALLBACK { apid_invoke(NULL, NULL, api::Buffer::buildKeyName(service_name, name), V8IO<T>::read(info)); });
    }
  };
  template <typename T, typename R> struct proxied_method : api::Named {
    proxied_method(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      SetFunc<V8IO<T>::length>(obj, name, [=, buffer = api::Buffer::buildKeyName(service_name, name)] V8CALLBACK {
        info.GetReturnValue().Set(makePromise([=, &buffer](auto resolve, auto reject) {
          apid_invoke_method(
              [](char const *data, void *privdata) {
                Nan::HandleScope scope;
                auto xresolve = (decltype(resolve) *)privdata;
                (*xresolve)(V8IO<R>::write(data));
                delete xresolve;
              },
              new auto(resolve), buffer, V8IO<T>::read(info));
        }));
      });
    }
  };
  template <typename T> struct proxied_property : api::Named {
    proxied_property(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      SetProp(obj, name, [=, buffer = api::Buffer::buildKeyName(service_name, name)] PROPERTY {
        info.GetReturnValue().Set(makePromise([=, &buffer](auto resolve, auto reject) {
          apid_kv_get(
              [](char const *data, void *privdata) {
                Nan::HandleScope scope;
                auto xresolve = (decltype(resolve) *)privdata;
                (*xresolve)(V8IO<T>::write(data));
                delete xresolve;
              },
              new auto(resolve), buffer);
        }));
      });
    }
  };
  template <typename T> struct proxied_broadcast : api::Named {
    proxied_broadcast(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      SetFunc<1>(obj, name, [=, buffer = api::Buffer::buildKeyName(service_name, name)] V8CALLBACK {
        if (info.Length() < 1 || !info[0]->IsFunction()) Nan::ThrowTypeError("wrong argument");
        apid_subscribe(
            [](char const *data, void *privdata) {
              Nan::HandleScope scope;
              auto &cb                    = *(Nan::Callback *)privdata;
              v8::Local<v8::Value> argv[] = { Nan::Null(), V8IO<T>::write(data) };
              Nan::Call(cb, 2, argv);
            },
            new Nan::Callback(info[0].template As<v8::Function>()), buffer);
      });
    }
  };
  template <typename T> struct proxied_pattern_broadcast : api::Named {
    proxied_pattern_broadcast(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      SetFunc<2>(obj, name, [=, buffer = api::Buffer::buildKeyName(service_name, name)] V8CALLBACK {
        if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsFunction()) Nan::ThrowTypeError("wrong argument");
        Nan::Utf8String pat{ info[0] };
        std::ostringstream oss;
        oss << const_cast<char *>(buffer.data) << "." << *pat;
        apid_subscribe_pattern(
            [](char const *identify, char const *data, void *privdata) {
              Nan::HandleScope scope;
              auto &cb                    = *(Nan::Callback *)privdata;
              v8::Local<v8::Value> argv[] = { Nan::Null(), V8IO<T>::rebuild(identify, data) };
              Nan::Call(cb, 2, argv);
            },
            new Nan::Callback(info[1].template As<v8::Function>()), oss.str().data());
      });
    }
  };
  template <typename T> struct proxied_set : api::Named {
    proxied_set(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      auto temp = Nan::New<v8::Object>();
      SetProp(temp, "items", [=, buffer = api::Buffer::buildKeyName(service_name, name)] PROPERTY {
        info.GetReturnValue().Set(makePromise([=, &buffer](auto resolve, auto reject) {
          apid_set_all(
              [](int size, char const **vec, void *privdata) {
                Nan::HandleScope scope;
                auto xresolve = (decltype(resolve) *)privdata;
                auto arr      = Nan::New<v8::Array>(size);
                for (int i = 0; i < size; i++) Nan::Set(arr, i, V8IO<T>::write(vec[i]));
                (*xresolve)(arr);
                delete xresolve;
              },
              new auto(resolve), buffer);
        }));
      });
      SetFunc<1>(temp, "onclear", [=, buffer = api::Buffer::buildKeyName(service_name, name + "!clear")] V8CALLBACK {
        if (info.Length() < 1 || !info[0]->IsFunction()) Nan::ThrowTypeError("wrong argument");
        apid_subscribe(
            [](char const *data, void *privdata) {
              Nan::HandleScope scope;
              auto &cb                    = *(Nan::Callback *)privdata;
              v8::Local<v8::Value> argv[] = { Nan::Null() };
              Nan::Call(cb, 1, argv);
            },
            new Nan::Callback(info[0].template As<v8::Function>()), buffer);
      });
      SetFunc<1>(temp, "onadd", [=, buffer = api::Buffer::buildKeyName(service_name, name + "!add")] V8CALLBACK {
        if (info.Length() < 1 || !info[0]->IsFunction()) Nan::ThrowTypeError("wrong argument");
        apid_subscribe(
            [](char const *data, void *privdata) {
              Nan::HandleScope scope;
              auto &cb                    = *(Nan::Callback *)privdata;
              v8::Local<v8::Value> argv[] = { Nan::Null(), V8IO<T>::write(data) };
              Nan::Call(cb, 2, argv);
            },
            new Nan::Callback(info[0].template As<v8::Function>()), buffer);
      });
      SetFunc<1>(temp, "onremove", [=, buffer = api::Buffer::buildKeyName(service_name, name + "!remove")] V8CALLBACK {
        if (info.Length() < 1 || !info[0]->IsFunction()) Nan::ThrowTypeError("wrong argument");
        apid_subscribe(
            [](char const *data, void *privdata) {
              Nan::HandleScope scope;
              auto &cb                    = *(Nan::Callback *)privdata;
              v8::Local<v8::Value> argv[] = { Nan::Null(), V8IO<T>::write(data) };
              Nan::Call(cb, 2, argv);
            },
            new Nan::Callback(info[0].template As<v8::Function>()), buffer);
      });
      Nan::Set(obj, Nan::New(name).ToLocalChecked(), temp);
    }
  };
  template <typename K, typename V> struct proxied_hash : api::Named {
    proxied_hash(std::string const &name)
        : Named(name) {}
    void $(v8::Local<v8::Object> &obj, std::string service_name) {
      auto temp = Nan::New<v8::Object>();
      SetFunc<V8IO<K>::length>(temp, "get", [=, buffer = api::Buffer::buildKeyName(service_name, name)] V8CALLBACK {
        info.GetReturnValue().Set(makePromise([=, &buffer](auto resolve, auto reject) {
          apid_hash_get(
              [](bool flag, char const *data, void *privdata) {
                Nan::HandleScope scope;
                auto tp                   = (std::tuple<decltype(resolve), decltype(reject)> *)privdata;
                auto &[xresolve, xreject] = *tp;
                if (flag)
                  xresolve(V8IO<V>::write(data));
                else
                  xreject("Not Found"_v8);
                delete tp;
              },
              new (std::tuple<decltype(resolve), decltype(reject)>)(resolve, reject), buffer, V8IO<K>::read(info));
        }));
      });
      Nan::Set(obj, Nan::New(name).ToLocalChecked(), temp);
    }
  };
};

static void check_result(int value, char const *err) {
  if (value != 0) Nan::ThrowError(err);
}

NAN_MODULE_INIT(InitAll) {
  using namespace api;
  Nan::SetMethod(target, "init", [] CALLBACK {
    if (info.Length() == 0) {
      check_result(apid_init(), "init apid failed");
    } else if (info.Length() == 1 && info[0]->IsString()) {
      Nan::Utf8String str(info[0]);
      check_result(apid_init_unix(*str), "init apid failed");
    } else if (info.Length() == 2 && info[0]->IsString() && info[1]->IsNumber()) {
      Nan::Utf8String str(info[0]);
      check_result(apid_init_tcp(*str, info[1]->NumberValue()), "init apid failed");
    } else {
      Nan::ThrowTypeError("init methods requires () or (string) or (string, number)");
    }
  });
  Nan::SetMethod(target, "attach", [] CALLBACK {
    if (info.Length() == 0) {
      check_result(apid_attach(Nan::GetCurrentEventLoop()), "apid attach failed");
    } else {
      Nan::ThrowTypeError("attach methods requires ()");
    }
  });
  api::CoreService<APIGenerator>().provide(target);
  api::ChatService<APIGenerator>().provide(target);
  api::CommandService<APIGenerator>().provide(target);
  api::BlacklistService<APIGenerator>().provide(target);
  api::ScriptService<APIGenerator>().provide(target);
}

NODE_MODULE(stonejs2, InitAll)