import type { AppMode, AppState, FontFileTreeItem, MapConfig, RenderedGlyphs } from '../../types/types.js';
import { AppStatus } from '../../types/types.js';
import { DEFAULT_FONT } from '../../data/data.js';
import { isStackConverted, resetFontStack } from './utilities.js';


export type AppAction =
    | { type: 'addFontStacks'; stacks: FontFileTreeItem[]; }
    | { type: 'setFontStacks'; stacks: FontFileTreeItem[]; modifiedStackIds?: string[]; }
    | { type: 'setMode'; mode: AppMode; }
    | { type: 'reset'; }
    | { type: 'startConversion'; toConvert: FontFileTreeItem[]; }
    | { type: 'updateConversionStatus'; data: WebWorkerDataPackage; }
    | { type: 'updateMapConfig'; changes: Partial<MapConfig>; };

export interface WebWorkerDataPackage {
    stackId: string;
    stackName?: string;
    glyph?: RenderedGlyphs;
    total?: number; // total number of .pbf files this stack will produce
    done?: boolean; // final message: the worker has finished this stack
}


export const initialState: AppState = {
    stacks: [],
    status: AppStatus.Ready,
    mode: 'normal',
    config: {
        font: DEFAULT_FONT,
        fontSize: 14,
        langCode: 'name',
        customText: '',
    },
};

export function appReducer(state: AppState, action: AppAction): AppState {
    switch (action.type) {

        case 'addFontStacks': {
            return {
                ...state,
                status: AppStatus.Ready,
                stacks: [...state.stacks, ...action.stacks],
            };
        }

        case 'setFontStacks': {
            const newStacks = action.stacks.map(stack => (
                action.modifiedStackIds?.includes(stack.id)
                    ? resetFontStack(stack)
                    : stack
            ));
            const convertedStacks = newStacks.filter(isStackConverted);
            const activeFont = convertedStacks.some(stack => stack.id === state.config.font)
                ? state.config.font
                : convertedStacks.at(-1)?.id || DEFAULT_FONT;
            return {
                ...state,
                status: (newStacks.length && newStacks.every(isStackConverted))
                    ? AppStatus.Finished
                    : AppStatus.Ready,
                stacks: newStacks,
                config: (state.config.font !== activeFont)
                    ? { ...state.config, font: activeFont }
                    : state.config,
            };
        }

        case 'setMode': {
            if (action.mode === state.mode) {
                return state;
            }
            // switching modes starts from a clean slate
            return {
                ...state,
                mode: action.mode,
                status: AppStatus.Ready,
                stacks: [],
                config: { ...state.config, font: DEFAULT_FONT },
            };
        }

        case 'reset': {
            return {
                ...state,
                status: AppStatus.Ready,
                stacks: [],
                config: { ...state.config, font: DEFAULT_FONT },
            };
        }

        case 'startConversion': {
            return {
                ...state,
                status: action.toConvert.length
                    ? AppStatus.Running
                    : AppStatus.Finished,
                stacks: state.stacks.map(stack => {
                    if (action.toConvert.includes(stack)) {
                        return { ...stack, data: { ...stack.data, glyphs: [], complete: false } };
                    }
                    return stack;
                }),
            };
        }

        case 'updateConversionStatus': {
            const { stackId, stackName, glyph, total, done } = action.data;
            let setAsNewActiveFont = false;
            const updatedStacks = state.stacks.map(stack => {
                if (stack.id === stackId) {
                    const glyphs = glyph ? [...stack.data.glyphs!, glyph] : stack.data.glyphs;
                    const updated = {
                        ...stack,
                        data: {
                            ...stack.data,
                            stackName: stackName ?? stack.data.stackName,
                            glyphs,
                            total: total ?? stack.data.total,
                            complete: done ? true : stack.data.complete,
                        },
                    };
                    setAsNewActiveFont = isStackConverted(updated);
                    return updated;
                }
                return stack;
            });
            return {
                ...state,
                status: updatedStacks.length && updatedStacks.every(isStackConverted)
                    ? AppStatus.Finished
                    : AppStatus.Running,
                stacks: updatedStacks,
                config: setAsNewActiveFont
                    ? { ...state.config, font: stackId }
                    : state.config,
            };
        }

        case 'updateMapConfig': {
            return {
                ...state,
                config: { ...state.config, ...action.changes },
            };
        }

        default:
            return state;

    }
}
