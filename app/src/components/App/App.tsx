import React, { useEffect, useMemo, useReducer, useRef } from 'react';
import maplibregl, { RequestParameters, ResponseCallback } from 'maplibre-gl';
import JSZip from 'jszip';
import { saveAs } from 'file-saver';

import type { AppMode, Example, FontFileTreeItem, MapConfig } from '../../types/types.js';
import type { ComplexFont } from '../../data/data.js';
import { COMPLEX_BASE_FONT, DEFAULT_FONT } from '../../data/data.js';
import { exampleToFontStackTreeItem, fileToFontStackTreeItem, isStackConverted } from './utilities.js';
import { flattenTree } from '../FilesSection/utilities.js';
import { appReducer, initialState, WebWorkerDataPackage } from './appReducer.js';
import { PermittedGitHubLogo } from '../PermittedGitHubLogo/PermittedGitHubLogo.js';
import { ExamplesSection } from '../ExamplesSection/ExamplesSection.js';
import { FilesSection } from '../FilesSection/FilesSection.js';
import { ComplexSection } from '../ComplexSection/ComplexSection.js';
import { MapSettingsSection } from '../MapSettingsSection/MapSettingsSection.js';
import { MapPreview } from '../MapPreview/MapPreview.js';

import styles from './App.module.css';


const worker = new Worker('worker.js');
const complexWorker = new Worker('complex-worker.js');

export function App() {
    const [state, dispatch] = useReducer(appReducer, initialState);
    const { status, stacks, mode, config } = state;

    // make the state accessible in protocol hook.
    const stacksRef = useRef(stacks);
    stacksRef.current = stacks;

    useEffect(() => {
        const onWorkerMessage = function (message: MessageEvent<WebWorkerDataPackage>) {
            dispatch({ type: 'updateConversionStatus', data: message.data });
        };
        worker.onmessage = onWorkerMessage;
        complexWorker.onmessage = onWorkerMessage;

        maplibregl.addProtocol(
            'memfont',
            (params: RequestParameters, callback: ResponseCallback<any>) => {
                const re = new RegExp(/memfont:\/\/(.+)\/(\d+)-(\d+)\.pbf/);
                const urlMatch = params.url.match(re);
                if (urlMatch) {
                    const [_, fontId, rangeStart, rangeEnd] = urlMatch;
                    const rangeName = `${rangeStart}-${rangeEnd}.pbf`;
                    if (fontId === DEFAULT_FONT) {
                        fetch(`https://demotiles.maplibre.org/font/${fontId}/${rangeName}`)
                            .then(resp => resp.arrayBuffer())
                            .then(ab => callback(null, new Uint8Array(ab), null, null));
                    } else {
                        const matchingRange = stacksRef.current
                            .find(stack => stack.id === fontId)?.data.glyphs
                            ?.find(glyph => glyph.name === rangeName);
                        setTimeout(() => {
                            if (!matchingRange) throw Error(`Can't find range "${params.url}"`);
                            callback(null, new Uint8Array(matchingRange.buffer), null, null);
                        }, 0);
                    }
                }
                return {
                    cancel: () => {},
                };
            },
        );

        return () => {
            worker.onmessage = null;
            complexWorker.onmessage = null;
            maplibregl.removeProtocol('memfont');
        };
    }, []);

    const handle = useMemo(() => ({
        exampleLoad: async (example: Example) => {
            const loadedExample = await exampleToFontStackTreeItem(example);
            dispatch({ type: 'addFontStacks', stacks: [loadedExample] });
        },
        fontFilesUpload: (uploadedFiles: File[]) => {
            if (uploadedFiles.length) {
                const asFontStackItems = uploadedFiles
                    .sort((a, b) => a.name.localeCompare(b.name))
                    .map(fileToFontStackTreeItem);
                dispatch({ type: 'addFontStacks', stacks: asFontStackItems });
            }
        },
        fontStacksChange: (newStacks: FontFileTreeItem[], modifiedStackIds?: string[]) => {
            dispatch({ type: 'setFontStacks', stacks: newStacks, modifiedStackIds });
        },
        mapConfigChange: (changes: Partial<MapConfig>) => {
            dispatch({ type: 'updateMapConfig', changes });
        },
        modeChange: (newMode: AppMode) => {
            dispatch({ type: 'setMode', mode: newMode });
        },
        reset: () => {
            dispatch({ type: 'reset' });
        },
    }), []);

    async function startFilesConversion() {
        const toConvert = stacks
            .filter(stack => !isStackConverted(stack));
        dispatch({ type: 'startConversion', toConvert });

        for (const stack of toConvert) {
            const stackFiles = flattenTree([stack]);
            const buffers = await Promise.all(
                stackFiles.map(item => item.data.file.arrayBuffer()),
            );
            worker.postMessage({ stackId: stack.id, buffers }, buffers);
        }
    }

    async function startComplexConversion(selectedFonts: ComplexFont[]) {
        if (!selectedFonts.length) return;

        // The base font is shared across every complex script.
        const baseBuffer = await fetch(COMPLEX_BASE_FONT.file).then(resp => resp.arrayBuffer());

        const items: FontFileTreeItem[] = await Promise.all(
            selectedFonts.map(async font => {
                const blob = await fetch(font.file).then(resp => resp.blob());
                return {
                    id: font.name,
                    parent: undefined,
                    children: [],
                    data: { file: new File([blob], font.name), stackName: font.name },
                };
            }),
        );

        dispatch({ type: 'addFontStacks', stacks: items });
        dispatch({ type: 'startConversion', toConvert: items });

        for (const [index, font] of selectedFonts.entries()) {
            const complexBuffer = await items[index].data.file.arrayBuffer();
            // Clone the base buffer per stack since transferred buffers get detached.
            const baseClone = baseBuffer.slice(0);
            complexWorker.postMessage(
                { stackId: font.name, name: font.name, buffers: [baseClone, complexBuffer] },
                [baseClone, complexBuffer],
            );
        }
    }

    async function downloadZip() {
        const zip = new JSZip();
        for (const { data: { stackName, glyphs } } of stacks) {
            const folder = zip.folder(stackName!)!;
            for (const { name, buffer } of glyphs!) {
                folder.file(name, buffer);
            }
        }
        const content = await zip.generateAsync({ type: 'blob' });
        saveAs(content, `font-maker-${new Date().toISOString()}.zip`);
    }

    return (
        <main className={styles.FullPage}>
            <aside className={styles.SidePanel}>
                <header>
                    <h1>
                        <a href='/font-maker' target='_blank'>
                            Font Maker
                        </a>
                    </h1>
                    <a href='https://github.com/maplibre/font-maker' target='_blank'>
                        <PermittedGitHubLogo />
                    </a>
                </header>

                <section className={styles.SectionWrapper}>
                    <div className={styles.ModeToggle} role='tablist'>
                        <button
                            type='button'
                            role='tab'
                            aria-selected={mode === 'normal'}
                            data-active={mode === 'normal'}
                            onClick={() => handle.modeChange('normal')}
                        >
                            Normal
                        </button>
                        <button
                            type='button'
                            role='tab'
                            aria-selected={mode === 'complex'}
                            data-active={mode === 'complex'}
                            onClick={() => handle.modeChange('complex')}
                        >
                            Complex scripts
                        </button>
                    </div>
                </section>

                {mode === 'normal' ? (
                    <>
                        <section className={styles.SectionWrapper}>
                            <h2>Load Examples</h2>
                            <ExamplesSection
                                status={status}
                                onExampleLoad={handle.exampleLoad}
                            />
                        </section>

                        <section className={styles.SectionWrapper}>
                            <h2>Convert .otf or .ttf files</h2>
                            <FilesSection
                                status={status}
                                stacks={stacks}
                                onFilesUpload={handle.fontFilesUpload}
                                onStacksChange={handle.fontStacksChange}
                                onConversionStart={startFilesConversion}
                                onDownloadZip={downloadZip}
                            />
                        </section>
                    </>
                ) : (
                    <section className={styles.SectionWrapper}>
                        <h2>Generate complex script fonts</h2>
                        <ComplexSection
                            status={status}
                            stacks={stacks}
                            onConversionStart={startComplexConversion}
                            onDownloadZip={downloadZip}
                            onReset={handle.reset}
                        />
                    </section>
                )}

                <section className={styles.SectionWrapper}>
                    <h2>Map settings</h2>
                    <MapSettingsSection
                        stacks={stacks}
                        mapConfig={config}
                        onMapConfigChange={handle.mapConfigChange}
                    />
                </section>
            </aside>
            <div className={styles.MapPreview}>
                <MapPreview mapConfig={config} />
            </div>
        </main>
    );
}
