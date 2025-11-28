#include <napi.h>
#include <string>
#include <vector>
#include <cstdint>

#include "lmdb.h"
#include "osmx/storage.h"
#include "osmx/messages.capnp.h"

class Environment : public Napi::ObjectWrap<Environment> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Environment(const Napi::CallbackInfo& info);
    ~Environment();

    MDB_env* GetEnv() { return mEnv; }

private:
    static Napi::FunctionReference constructor;
    MDB_env* mEnv;

    void Close(const Napi::CallbackInfo& info);
};

class Transaction : public Napi::ObjectWrap<Transaction> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Transaction(const Napi::CallbackInfo& info);
    ~Transaction();

    MDB_txn* GetTxn() { return mTxn; }
    Environment* GetEnvironment() { return mEnvironment; }

private:
    static Napi::FunctionReference constructor;
    MDB_txn* mTxn;
    Environment* mEnvironment;
    Napi::Reference<Napi::Object> mEnvRef;

    void Abort(const Napi::CallbackInfo& info);
};

class Locations : public Napi::ObjectWrap<Locations> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Locations(const Napi::CallbackInfo& info);
    ~Locations();

private:
    static Napi::FunctionReference constructor;
    osmx::db::Locations* mLocations;
    Transaction* mTransaction;
    Napi::Reference<Napi::Object> mTxnRef;

    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Exists(const Napi::CallbackInfo& info);
};

class Nodes : public Napi::ObjectWrap<Nodes> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Nodes(const Napi::CallbackInfo& info);
    ~Nodes();

private:
    static Napi::FunctionReference constructor;
    osmx::db::Elements* mElements;
    Transaction* mTransaction;
    Napi::Reference<Napi::Object> mTxnRef;
    MDB_dbi mDbi;

    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Exists(const Napi::CallbackInfo& info);
    Napi::Value Iterate(const Napi::CallbackInfo& info);
};

class Ways : public Napi::ObjectWrap<Ways> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Ways(const Napi::CallbackInfo& info);
    ~Ways();

private:
    static Napi::FunctionReference constructor;
    osmx::db::Elements* mElements;
    Transaction* mTransaction;
    Napi::Reference<Napi::Object> mTxnRef;
    MDB_dbi mDbi;

    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Exists(const Napi::CallbackInfo& info);
    Napi::Value Iterate(const Napi::CallbackInfo& info);
};

class Relations : public Napi::ObjectWrap<Relations> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Relations(const Napi::CallbackInfo& info);
    ~Relations();

private:
    static Napi::FunctionReference constructor;
    osmx::db::Elements* mElements;
    Transaction* mTransaction;
    Napi::Reference<Napi::Object> mTxnRef;
    MDB_dbi mDbi;

    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Exists(const Napi::CallbackInfo& info);
    Napi::Value Iterate(const Napi::CallbackInfo& info);
};

Napi::FunctionReference Environment::constructor;
Napi::FunctionReference Transaction::constructor;
Napi::FunctionReference Locations::constructor;
Napi::FunctionReference Nodes::constructor;
Napi::FunctionReference Ways::constructor;
Napi::FunctionReference Relations::constructor;

// Environment implementation
Napi::Object Environment::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Environment", {
        InstanceMethod("close", &Environment::Close)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Environment", func);
    return exports;
}

Environment::Environment(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Environment>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected for database path").ThrowAsJavaScriptException();
        return;
    }
    std::string path = info[0].As<Napi::String>().Utf8Value();
    mEnv = osmx::db::createEnv(path);
}

Environment::~Environment() {
    if (mEnv) {
        mdb_env_close(mEnv);
        mEnv = nullptr;
    }
}

void Environment::Close(const Napi::CallbackInfo& info) {
    if (mEnv) {
        mdb_env_close(mEnv);
        mEnv = nullptr;
    }
}

// Transaction implementation
Napi::Object Transaction::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Transaction", {
        InstanceMethod("abort", &Transaction::Abort)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Transaction", func);
    return exports;
}

Transaction::Transaction(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Transaction>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Environment object expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object envObj = info[0].As<Napi::Object>();
    mEnvironment = Napi::ObjectWrap<Environment>::Unwrap(envObj);
    mEnvRef = Napi::Persistent(envObj);

    int rc = mdb_txn_begin(mEnvironment->GetEnv(), nullptr, MDB_RDONLY, &mTxn);
    if (rc != 0) {
        Napi::Error::New(env, mdb_strerror(rc)).ThrowAsJavaScriptException();
        return;
    }
}

Transaction::~Transaction() {
    if (mTxn) {
        mdb_txn_abort(mTxn);
        mTxn = nullptr;
    }
}

void Transaction::Abort(const Napi::CallbackInfo& info) {
    if (mTxn) {
        mdb_txn_abort(mTxn);
        mTxn = nullptr;
    }
}

// Locations implementation
Napi::Object Locations::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Locations", {
        InstanceMethod("get", &Locations::Get),
        InstanceMethod("exists", &Locations::Exists)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Locations", func);
    return exports;
}

Locations::Locations(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Locations>(info), mLocations(nullptr) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Transaction object expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object txnObj = info[0].As<Napi::Object>();
    mTransaction = Napi::ObjectWrap<Transaction>::Unwrap(txnObj);
    mTxnRef = Napi::Persistent(txnObj);

    mLocations = new osmx::db::Locations(mTransaction->GetTxn());
}

Locations::~Locations() {
    delete mLocations;
}

Napi::Value Locations::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Node ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t nodeId = info[0].As<Napi::Number>().Int64Value();
    osmx::db::Location loc = mLocations->get(nodeId);

    if (loc.is_undefined()) {
        return env.Null();
    }

    Napi::Object result = Napi::Object::New(env);
    result.Set("lon", Napi::Number::New(env, loc.coords.lon()));
    result.Set("lat", Napi::Number::New(env, loc.coords.lat()));
    result.Set("version", Napi::Number::New(env, loc.version));
    return result;
}

Napi::Value Locations::Exists(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Node ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t nodeId = info[0].As<Napi::Number>().Int64Value();
    return Napi::Boolean::New(env, mLocations->exists(nodeId));
}

// Helper function to convert metadata to JS object
Napi::Object MetadataToJs(Napi::Env env, ::Metadata::Reader metadata) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("version", Napi::Number::New(env, metadata.getVersion()));
    obj.Set("timestamp", Napi::Number::New(env, static_cast<double>(metadata.getTimestamp())));
    obj.Set("changeset", Napi::Number::New(env, metadata.getChangeset()));
    obj.Set("uid", Napi::Number::New(env, metadata.getUid()));
    if (metadata.hasUser()) {
        obj.Set("user", Napi::String::New(env, metadata.getUser().cStr()));
    }
    return obj;
}

// Helper function to convert tags to JS object
Napi::Object TagsToJs(Napi::Env env, capnp::List<capnp::Text>::Reader tags) {
    Napi::Object obj = Napi::Object::New(env);
    for (size_t i = 0; i + 1 < tags.size(); i += 2) {
        obj.Set(tags[i].cStr(), Napi::String::New(env, tags[i + 1].cStr()));
    }
    return obj;
}

// Nodes implementation
Napi::Object Nodes::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Nodes", {
        InstanceMethod("get", &Nodes::Get),
        InstanceMethod("exists", &Nodes::Exists),
        InstanceMethod("iterate", &Nodes::Iterate)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Nodes", func);
    return exports;
}

Nodes::Nodes(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Nodes>(info), mElements(nullptr) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Transaction object expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object txnObj = info[0].As<Napi::Object>();
    mTransaction = Napi::ObjectWrap<Transaction>::Unwrap(txnObj);
    mTxnRef = Napi::Persistent(txnObj);

    mElements = new osmx::db::Elements(mTransaction->GetTxn(), "nodes");
    
    CHECK_LMDB(mdb_dbi_open(mTransaction->GetTxn(), "nodes", MDB_INTEGERKEY, &mDbi));
}

Nodes::~Nodes() {
    delete mElements;
}

Napi::Value Nodes::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Node ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t nodeId = info[0].As<Napi::Number>().Int64Value();

    if (!mElements->exists(nodeId)) {
        return env.Null();
    }

    try {
        auto message = mElements->getReader(nodeId);
        auto node = message.getRoot<Node>();

        Napi::Object result = Napi::Object::New(env);
        result.Set("tags", TagsToJs(env, node.getTags()));
        if (node.hasMetadata()) {
            result.Set("metadata", MetadataToJs(env, node.getMetadata()));
        }
        return result;
    } catch (const std::exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value Nodes::Exists(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Node ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t nodeId = info[0].As<Napi::Number>().Int64Value();
    return Napi::Boolean::New(env, mElements->exists(nodeId));
}

Napi::Value Nodes::Iterate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    
    MDB_cursor* cursor;
    int rc = mdb_cursor_open(mTransaction->GetTxn(), mDbi, &cursor);
    if (rc != 0) {
        Napi::Error::New(env, mdb_strerror(rc)).ThrowAsJavaScriptException();
        return env.Null();
    }

    MDB_val key, data;
    uint64_t count = 0;
    
    while ((rc = mdb_cursor_get(cursor, &key, &data, count == 0 ? MDB_FIRST : MDB_NEXT)) == 0) {
        uint64_t nodeId = *((uint64_t*)key.mv_data);
        
        try {
            auto arr = kj::ArrayPtr<const capnp::word>((const capnp::word*)data.mv_data, data.mv_size / sizeof(capnp::word));
            capnp::FlatArrayMessageReader message(arr);
            auto node = message.getRoot<Node>();

            Napi::Object result = Napi::Object::New(env);
            result.Set("id", Napi::String::New(env, std::to_string(nodeId)));
            result.Set("tags", TagsToJs(env, node.getTags()));

            if (node.hasMetadata()) {
                result.Set("metadata", MetadataToJs(env, node.getMetadata()));
            }

            Napi::Value callbackResult = callback.Call({result});
            
            if (callbackResult.IsBoolean() && !callbackResult.As<Napi::Boolean>().Value()) {
                break;
            }
        } catch (const std::exception& e) {
            mdb_cursor_close(cursor);
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
        
        count++;
    }

    mdb_cursor_close(cursor);
    return Napi::Number::New(env, static_cast<double>(count));
}

// Ways implementation
Napi::Object Ways::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Ways", {
        InstanceMethod("get", &Ways::Get),
        InstanceMethod("exists", &Ways::Exists),
        InstanceMethod("iterate", &Ways::Iterate)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Ways", func);
    return exports;
}

Ways::Ways(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Ways>(info), mElements(nullptr) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Transaction object expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object txnObj = info[0].As<Napi::Object>();
    mTransaction = Napi::ObjectWrap<Transaction>::Unwrap(txnObj);
    mTxnRef = Napi::Persistent(txnObj);

    mElements = new osmx::db::Elements(mTransaction->GetTxn(), "ways");
    
    CHECK_LMDB(mdb_dbi_open(mTransaction->GetTxn(), "ways", MDB_INTEGERKEY, &mDbi));
}

Ways::~Ways() {
    delete mElements;
}

Napi::Value Ways::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Way ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t wayId = info[0].As<Napi::Number>().Int64Value();

    if (!mElements->exists(wayId)) {
        return env.Null();
    }

    try {
        auto message = mElements->getReader(wayId);
        auto way = message.getRoot<Way>();

        Napi::Object result = Napi::Object::New(env);

        auto nodes = way.getNodes();
        Napi::Array nodeArray = Napi::Array::New(env, nodes.size());
        for (size_t i = 0; i < nodes.size(); i++) {
            nodeArray.Set(i, Napi::String::New(env, std::to_string(static_cast<uint64_t>(nodes[i]))));
        }
        result.Set("nodes", nodeArray);

        result.Set("tags", TagsToJs(env, way.getTags()));

        if (way.hasMetadata()) {
            result.Set("metadata", MetadataToJs(env, way.getMetadata()));
        }

        return result;
    } catch (const std::exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value Ways::Exists(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Way ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t wayId = info[0].As<Napi::Number>().Int64Value();
    return Napi::Boolean::New(env, mElements->exists(wayId));
}

Napi::Value Ways::Iterate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    
    MDB_cursor* cursor;
    int rc = mdb_cursor_open(mTransaction->GetTxn(), mDbi, &cursor);
    if (rc != 0) {
        Napi::Error::New(env, mdb_strerror(rc)).ThrowAsJavaScriptException();
        return env.Null();
    }

    MDB_val key, data;
    uint64_t count = 0;
    
    while ((rc = mdb_cursor_get(cursor, &key, &data, count == 0 ? MDB_FIRST : MDB_NEXT)) == 0) {
        uint64_t wayId = *((uint64_t*)key.mv_data);
        
        try {
            auto arr = kj::ArrayPtr<const capnp::word>((const capnp::word*)data.mv_data, data.mv_size / sizeof(capnp::word));
            capnp::FlatArrayMessageReader message(arr);
            auto way = message.getRoot<Way>();

            Napi::Object result = Napi::Object::New(env);
            result.Set("id", Napi::String::New(env, std::to_string(wayId)));

            auto nodes = way.getNodes();
            Napi::Array nodeArray = Napi::Array::New(env, nodes.size());
            for (size_t i = 0; i < nodes.size(); i++) {
                nodeArray.Set(i, Napi::String::New(env, std::to_string(static_cast<uint64_t>(nodes[i]))));
            }
            result.Set("nodes", nodeArray);

            result.Set("tags", TagsToJs(env, way.getTags()));

            if (way.hasMetadata()) {
                result.Set("metadata", MetadataToJs(env, way.getMetadata()));
            }

            Napi::Value callbackResult = callback.Call({result});
            
            if (callbackResult.IsBoolean() && !callbackResult.As<Napi::Boolean>().Value()) {
                break;
            }
        } catch (const std::exception& e) {
            mdb_cursor_close(cursor);
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
        
        count++;
    }

    mdb_cursor_close(cursor);
    return Napi::Number::New(env, static_cast<double>(count));
}

// Relations implementation
Napi::Object Relations::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Relations", {
        InstanceMethod("get", &Relations::Get),
        InstanceMethod("exists", &Relations::Exists),
        InstanceMethod("iterate", &Relations::Iterate)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Relations", func);
    return exports;
}

Relations::Relations(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Relations>(info), mElements(nullptr) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Transaction object expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Object txnObj = info[0].As<Napi::Object>();
    mTransaction = Napi::ObjectWrap<Transaction>::Unwrap(txnObj);
    mTxnRef = Napi::Persistent(txnObj);

    mElements = new osmx::db::Elements(mTransaction->GetTxn(), "relations");
    
    CHECK_LMDB(mdb_dbi_open(mTransaction->GetTxn(), "relations", MDB_INTEGERKEY, &mDbi));
}

Relations::~Relations() {
    delete mElements;
}

Napi::Value Relations::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Relation ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t relationId = info[0].As<Napi::Number>().Int64Value();

    if (!mElements->exists(relationId)) {
        return env.Null();
    }

    try {
        auto message = mElements->getReader(relationId);
        auto relation = message.getRoot<Relation>();

        Napi::Object result = Napi::Object::New(env);

        result.Set("tags", TagsToJs(env, relation.getTags()));

        auto members = relation.getMembers();
        Napi::Array memberArray = Napi::Array::New(env, members.size());
        for (size_t i = 0; i < members.size(); i++) {
            auto member = members[i];
            Napi::Object memberObj = Napi::Object::New(env);
            memberObj.Set("ref", Napi::String::New(env, std::to_string(static_cast<uint64_t>(member.getRef()))));

            const char* typeStr = "node";
            switch (member.getType()) {
                case RelationMember::Type::NODE: typeStr = "node"; break;
                case RelationMember::Type::WAY: typeStr = "way"; break;
                case RelationMember::Type::RELATION: typeStr = "relation"; break;
            }
            memberObj.Set("type", Napi::String::New(env, typeStr));

            if (member.hasRole()) {
                memberObj.Set("role", Napi::String::New(env, member.getRole().cStr()));
            }
            memberArray.Set(i, memberObj);
        }
        result.Set("members", memberArray);

        if (relation.hasMetadata()) {
            result.Set("metadata", MetadataToJs(env, relation.getMetadata()));
        }

        return result;
    } catch (const std::exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value Relations::Exists(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Relation ID (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t relationId = info[0].As<Napi::Number>().Int64Value();
    return Napi::Boolean::New(env, mElements->exists(relationId));
}

Napi::Value Relations::Iterate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    
    MDB_cursor* cursor;
    int rc = mdb_cursor_open(mTransaction->GetTxn(), mDbi, &cursor);
    if (rc != 0) {
        Napi::Error::New(env, mdb_strerror(rc)).ThrowAsJavaScriptException();
        return env.Null();
    }

    MDB_val key, data;
    uint64_t count = 0;
    
    while ((rc = mdb_cursor_get(cursor, &key, &data, count == 0 ? MDB_FIRST : MDB_NEXT)) == 0) {
        uint64_t relationId = *((uint64_t*)key.mv_data);
        
        try {
            auto arr = kj::ArrayPtr<const capnp::word>((const capnp::word*)data.mv_data, data.mv_size / sizeof(capnp::word));
            capnp::FlatArrayMessageReader message(arr);
            auto relation = message.getRoot<Relation>();

            Napi::Object result = Napi::Object::New(env);
            result.Set("id", Napi::String::New(env, std::to_string(relationId)));
            result.Set("tags", TagsToJs(env, relation.getTags()));

            auto members = relation.getMembers();
            Napi::Array memberArray = Napi::Array::New(env, members.size());
            for (size_t i = 0; i < members.size(); i++) {
                auto member = members[i];
                Napi::Object memberObj = Napi::Object::New(env);
                memberObj.Set("ref", Napi::String::New(env, std::to_string(static_cast<uint64_t>(member.getRef()))));

                const char* typeStr = "node";
                switch (member.getType()) {
                    case RelationMember::Type::NODE: typeStr = "node"; break;
                    case RelationMember::Type::WAY: typeStr = "way"; break;
                    case RelationMember::Type::RELATION: typeStr = "relation"; break;
                }
                memberObj.Set("type", Napi::String::New(env, typeStr));

                if (member.hasRole()) {
                    memberObj.Set("role", Napi::String::New(env, member.getRole().cStr()));
                }
                memberArray.Set(i, memberObj);
            }
            result.Set("members", memberArray);

            if (relation.hasMetadata()) {
                result.Set("metadata", MetadataToJs(env, relation.getMetadata()));
            }

            Napi::Value callbackResult = callback.Call({result});
            
            if (callbackResult.IsBoolean() && !callbackResult.As<Napi::Boolean>().Value()) {
                break;
            }
        } catch (const std::exception& e) {
            mdb_cursor_close(cursor);
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
        
        count++;
    }

    mdb_cursor_close(cursor);
    return Napi::Number::New(env, static_cast<double>(count));
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Environment::Init(env, exports);
    Transaction::Init(env, exports);
    Locations::Init(env, exports);
    Nodes::Init(env, exports);
    Ways::Init(env, exports);
    Relations::Init(env, exports);
    return exports;
}

NODE_API_MODULE(osmx_native, Init)
