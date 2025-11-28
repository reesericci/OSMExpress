const native = require('../build/Release/osmx_native.node');

class Environment {
    constructor(path) {
        this._handle = new native.Environment(path);
    }

    close() {
        this._handle.close();
    }
}

class Transaction {
    constructor(env) {
        if (!(env instanceof Environment)) {
            throw new TypeError('Environment instance expected');
        }
        this._env = env;
        this._handle = new native.Transaction(env._handle);
    }

    abort() {
        this._handle.abort();
    }
}

class Locations {
    constructor(txn) {
        if (!(txn instanceof Transaction)) {
            throw new TypeError('Transaction instance expected');
        }
        this._txn = txn;
        this._handle = new native.Locations(txn._handle);
    }

    get(nodeId) {
        return this._handle.get(nodeId);
    }

    exists(nodeId) {
        return this._handle.exists(nodeId);
    }
}

class Nodes {
    constructor(txn) {
        if (!(txn instanceof Transaction)) {
            throw new TypeError('Transaction instance expected');
        }
        this._txn = txn;
        this._handle = new native.Nodes(txn._handle);
    }

    get(nodeId) {
        return this._handle.get(nodeId);
    }

    exists(nodeId) {
        return this._handle.exists(nodeId);
    }

    iterate(callback) {
        return this._handle.iterate(callback);
    }
}

class Ways {
    constructor(txn) {
        if (!(txn instanceof Transaction)) {
            throw new TypeError('Transaction instance expected');
        }
        this._txn = txn;
        this._handle = new native.Ways(txn._handle);
    }

    get(wayId) {
        return this._handle.get(wayId);
    }

    exists(wayId) {
        return this._handle.exists(wayId);
    }

    iterate(callback) {
        return this._handle.iterate(callback);
    }
}

class Relations {
    constructor(txn) {
        if (!(txn instanceof Transaction)) {
            throw new TypeError('Transaction instance expected');
        }
        this._txn = txn;
        this._handle = new native.Relations(txn._handle);
    }

    get(relationId) {
        return this._handle.get(relationId);
    }

    exists(relationId) {
        return this._handle.exists(relationId);
    }

    iterate(callback) {
        return this._handle.iterate(callback);
    }
}

function tagDict(tagObject) {
    return tagObject;
}

module.exports = {
    Environment,
    Transaction,
    Locations,
    Nodes,
    Ways,
    Relations,
    tagDict
};
