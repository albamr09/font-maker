import { EXAMPLES } from '../data/data.js';


export enum AppStatus {
    Ready = 'Ready',
    Running = 'Running',
    Finished = 'Finished',
}

export type AppMode = 'normal' | 'complex';

export interface AppState {
    stacks: FontFileTreeItem[];
    status: AppStatus;
    mode: AppMode;
    config: MapConfig;
}

export interface MapConfig {
    font: string;
    fontSize: number;
    langCode: string;
    customText: string;
}


export interface TreeItem<T = unknown> {
    id: string;
    parent?: TreeItem<T>; // items under root do not have a parent
    children: TreeItem<T>[];
    data: T;
}

// tree flatten to an array
export interface FlattenedTreeItem<T = unknown> extends TreeItem<T> {
    depth: number;
}


export interface RenderedGlyphs {
    name: string;
    buffer: ArrayBuffer;
}

export interface FontFileData {
    file: File; // .otf,.ttf file
    stackName?: string;
    glyphs?: RenderedGlyphs[]; // generated .pbf files
    total?: number; // expected number of .pbf files (known once the worker starts)
    complete?: boolean; // set once the worker signals it is done with this stack
}

export type FontFileTreeItem = TreeItem<FontFileData>;


export type Example = typeof EXAMPLES[number];
