// Expose enum values.
const enums = [
    'heif_error_code',
    'heif_suberror_code',
];
for (const e of enums) {
    for (const key in Module[e]) {
        if (!Module[e].hasOwnProperty(key) || key === 'values') {
            continue;
        }
        Module[key] = Module[e][key];
    }
}