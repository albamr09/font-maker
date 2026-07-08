import React, { useMemo, useState } from 'react';
import clsx from 'clsx';

import { AppStatus, FontFileTreeItem } from '../../types/types.js';
import type { ComplexFont } from '../../data/data.js';
import { COMPLEX_BASE_FONT, COMPLEX_FONTS } from '../../data/data.js';
import { Button } from '../Inputs/Inputs.js';

import styles from './ComplexSection.module.css';


interface Props {
    status: AppStatus;
    stacks: FontFileTreeItem[];
    onConversionStart: (selectedFonts: ComplexFont[]) => void;
    onDownloadZip: () => void;
    onReset: () => void;
}

export function ComplexSection(props: Props) {
    const { status, stacks, onConversionStart, onDownloadZip, onReset } = props;
    const running = status === AppStatus.Running;
    const finished = status === AppStatus.Finished;

    const [selected, setSelected] = useState<Set<string>>(
        () => new Set(COMPLEX_FONTS.map(font => font.file)),
    );

    const selectedFonts = useMemo(
        () => COMPLEX_FONTS.filter(font => selected.has(font.file)),
        [selected],
    );

    const allSelected = selected.size === COMPLEX_FONTS.length;
    const canEdit = status === AppStatus.Ready;

    function toggle(file: string) {
        setSelected(prev => {
            const next = new Set(prev);
            if (next.has(file)) {
                next.delete(file);
            } else {
                next.add(file);
            }
            return next;
        });
    }

    function toggleAll() {
        setSelected(allSelected ? new Set() : new Set(COMPLEX_FONTS.map(f => f.file)));
    }

    function handleReset() {
        setSelected(new Set(COMPLEX_FONTS.map(f => f.file)));
        onReset();
    }

    return (
        <div className={styles.ComplexSection}>
            <p className={styles.Hint}>
                Each selected script is paired with the base font{' '}
                <strong>{COMPLEX_BASE_FONT.name}</strong>. One set of PBFs is
                generated per script, in a folder named after the complex font.
            </p>

            <label className={styles.SelectAll}>
                <input
                    type='checkbox'
                    checked={allSelected}
                    disabled={!canEdit}
                    onChange={toggleAll}
                />
                Select all
            </label>

            <ul className={styles.FontList}>
                {COMPLEX_FONTS.map(font => {
                    const stack = stacks.find(s => s.data.stackName === font.name);
                    const done = !!stack?.data.complete;
                    const count = stack?.data.glyphs?.length ?? 0;
                    const total = stack?.data.total;
                    const percent = done
                        ? 100
                        : total
                            ? Math.round((count / total) * 100)
                            : 0;
                    const isSelected = selected.has(font.file);
                    const showProgress = running || finished;

                    return (
                        <li key={font.file} className={styles.FontItem}>
                            <div className={styles.FontRow}>
                                <label className={styles.FontLabel}>
                                    <input
                                        type='checkbox'
                                        checked={isSelected}
                                        disabled={!canEdit}
                                        onChange={() => toggle(font.file)}
                                    />
                                    <span className={styles.FontName}>{font.name}</span>
                                </label>
                                <span className={styles.FontStatus}>
                                    {done
                                        ? 'done'
                                        : showProgress && isSelected
                                            ? `${percent}%`
                                            : ''}
                                </span>
                            </div>
                            {showProgress && isSelected && (
                                <div className={styles.ProgressTrack}>
                                    <div
                                        className={clsx(styles.ProgressBar, done && styles.done)}
                                        style={{ width: `${percent}%` }}
                                    />
                                </div>
                            )}
                        </li>
                    );
                })}
            </ul>

            <div className={styles.Actions}>
                {!finished ? (
                    <Button
                        className={clsx(styles.ActionButton, running && styles.loading)}
                        disabled={running || selectedFonts.length === 0}
                        onClick={() => onConversionStart(selectedFonts)}
                    >
                        {running
                            ? 'Generating…'
                            : `Generate ${selectedFonts.length} font${selectedFonts.length === 1 ? '' : 's'}`}
                    </Button>
                ) : (
                    <Button className={styles.ActionButton} onClick={onDownloadZip}>
                        Download .zip
                    </Button>
                )}
                <Button
                    outline
                    className={styles.ResetButton}
                    disabled={running}
                    onClick={handleReset}
                >
                    Reset
                </Button>
            </div>
        </div>
    );
}
