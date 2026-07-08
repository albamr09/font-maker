export const DEFAULT_FONT = 'Noto Sans Regular';

export interface ComplexFont {
    file: string; // path under public/
    name: string; // output fontstack / folder name
}

// The base (unicode-indexed) font paired with every complex-script font.
export const COMPLEX_BASE_FONT: ComplexFont = {
    file: 'complex/NotoSans-Regular.ttf',
    name: 'Noto Sans',
};

// One pbf set is generated per entry, pairing the base font above with each of
// these complex-script fonts. Files live in app/public/complex/.
export const COMPLEX_FONTS: ComplexFont[] = [
    { file: 'complex/NotoSansBengali-Regular.ttf', name: 'Noto Sans Bengali' },
    { file: 'complex/NotoSansDevanagari-Regular.ttf', name: 'Noto Sans Devanagari' },
    { file: 'complex/NotoSansGujarati-Regular.ttf', name: 'Noto Sans Gujarati' },
    { file: 'complex/NotoSansKannada-Regular.ttf', name: 'Noto Sans Kannada' },
    { file: 'complex/NotoSansMalayalam-Regular.ttf', name: 'Noto Sans Malayalam' },
    { file: 'complex/NotoSansMyanmar-Regular.ttf', name: 'Noto Sans Myanmar' },
    { file: 'complex/NotoSansTamil-Regular.ttf', name: 'Noto Sans Tamil' },
    { file: 'complex/NotoSansTelugu-Regular.ttf', name: 'Noto Sans Telugu' },
];

export const EXAMPLES = [
    {
        name: 'Font stack "Barlow Regular"',
        files: [
            'Barlow-Regular.ttf',
        ],
    },
    {
        name: 'Font stack "Lato Bold"',
        files: [
            'Lato-Bold.ttf',
        ],
    },
    {
        name: 'Font stack "Lato Bold,Barlow Regular"',
        files: [
            'Lato-Bold.ttf',
            'Barlow-Regular.ttf',
        ],
    },
];

export const LANGUAGES: [code: string, label: string][] = [
    ['name', 'default'],
    ['name:en', 'en'],
    ['name:ru', 'ru'],
    ['name:ar', 'ar'],
    ['name:zh-Hant', 'zh-Hant'],
    ['name:zh-Hans', 'zh-Hans'],
    ['name:ja', 'ja'],
    ['name:ko', 'ko'],
    ['name:fr', 'fr'],
    ['name:uk', 'uk'],
    ['name:de', 'de'],
    ['name:fi', 'fi'],
    ['name:pl', 'pl'],
    ['name:es', 'es'],
    ['name:be', 'be'],
    ['name:br', 'br'],
    ['name:he', 'he'],
    ['name:sr', 'sr'],
    ['name:sv', 'sv'],
    ['name:it', 'it'],
    ['name:ga', 'ga'],
    ['name:el', 'el'],
    ['name:kn', 'kn'],
    ['name:th', 'th'],
    ['name:nl', 'nl'],
    ['name:ca', 'ca'],
    ['name:hu', 'hu'],
    ['name:eu', 'eu'],
    ['name:oc', 'oc'],
    ['name:lt', 'lt'],
    ['name:cs', 'cs'],
    ['name:ro', 'ro'],
    ['name:hi', 'hi'],
    ['name:ka', 'ka'],
    ['name:fa', 'fa'],
    ['name:pt', 'pt'],
    ['name:my', 'my'],
    ['name:nan', 'nan'],
    ['name:ku', 'ku'],
    ['name:ur', 'ur'],
    ['name:vi', 'vi'],
    ['name:bg', 'bg'],
    ['name:hsb', 'hsb'],
    ['name:bo', 'bo'],
    ['name:hak', 'hak'],
    ['name:tay', 'tay'],
    ['name:szy', 'szy'],
    ['name:ami', 'ami'],
    ['name:se', 'se'],
];
