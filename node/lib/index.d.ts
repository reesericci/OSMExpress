export interface Location {
    lon: number;
    lat: number;
    version: number;
}

export interface Metadata {
    version: number;
    timestamp: number;
    changeset: number;
    uid: number;
    user?: string;
}

export interface NodeData {
    tags: Record<string, string>;
    metadata?: Metadata;
}

export interface NodeIterData {
    id: string;
    tags: Record<string, string>;
    metadata?: Metadata;
}

export interface WayData {
    nodes: string[];
    tags: Record<string, string>;
    metadata?: Metadata;
}

export interface WayIterData {
    id: string;
    nodes: string[];
    tags: Record<string, string>;
    metadata?: Metadata;
}

export interface RelationMember {
    ref: string;
    type: 'node' | 'way' | 'relation';
    role?: string;
}

export interface RelationData {
    tags: Record<string, string>;
    members: RelationMember[];
    metadata?: Metadata;
}

export interface RelationIterData {
    id: string;
    tags: Record<string, string>;
    members: RelationMember[];
    metadata?: Metadata;
}

export declare class Environment {
    constructor(path: string);
    close(): void;
}

export declare class Transaction {
    constructor(env: Environment);
    abort(): void;
}

export declare class Locations {
    constructor(txn: Transaction);
    get(nodeId: number | string): Location | null;
    exists(nodeId: number | string): boolean;
}

export declare class Nodes {
    constructor(txn: Transaction);
    get(nodeId: number | string): NodeData | null;
    exists(nodeId: number | string): boolean;
    iterate(callback: (node: NodeIterData) => boolean | void): number;
}

export declare class Ways {
    constructor(txn: Transaction);
    get(wayId: string): WayData | null;
    exists(wayId: string): boolean;
    iterate(callback: (way: WayIterData) => boolean | void): number;
}

export declare class Relations {
    constructor(txn: Transaction);
    get(relationId: number | string): RelationData | null;
    exists(relationId: number | string): boolean;
    iterate(callback: (relation: RelationIterData) => boolean | void): number;
}

export declare function tagDict(tagObject: Record<string, string>): Record<string, string>;
