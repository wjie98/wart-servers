wit_bindgen_wasmtime::export!({
    paths: ["../interface/wasm/imports.wit"],
    async: [
        "log",
        "log-enabled",
        "storage::new",
        "storage::choice-nodes",
        "storage::query-nodes",
        "storage::query-neighbors",
        "storage::query-kv",
        "storage::update-kv",
    ],
});
