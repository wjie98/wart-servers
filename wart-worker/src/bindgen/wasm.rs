wit_bindgen_wasmtime::export!({
    paths: ["../interface/wasm/imports.wit"],
    async: [
        "get-future-row",
        "get-future-table",
        "async-query-kv",
        "async-query-node",
        "async-query-neighbors",
        "async-choice-nodes",
        // "select-nodes",
        // "select-edges",
    ],
});
