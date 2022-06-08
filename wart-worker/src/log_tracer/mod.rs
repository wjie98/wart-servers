use std::collections::{BTreeMap, VecDeque};
use std::sync::{Arc, Mutex};

use tracing_subscriber::Layer;

#[derive(Debug)]
pub struct JsonVisitor<'a>(&'a mut BTreeMap<String, serde_json::Value>);

impl<'a> tracing::field::Visit for JsonVisitor<'a> {
    fn record_f64(&mut self, field: &tracing::field::Field, value: f64) {
        self.0.insert(field.name().into(), serde_json::json!(value));
    }

    fn record_i64(&mut self, field: &tracing::field::Field, value: i64) {
        self.0.insert(field.name().into(), serde_json::json!(value));
    }

    fn record_u64(&mut self, field: &tracing::field::Field, value: u64) {
        self.0.insert(field.name().into(), serde_json::json!(value));
    }

    fn record_bool(&mut self, field: &tracing::field::Field, value: bool) {
        self.0.insert(field.name().into(), serde_json::json!(value));
    }

    fn record_str(&mut self, field: &tracing::field::Field, value: &str) {
        self.0.insert(field.name().into(), serde_json::json!(value));
    }

    fn record_error(
        &mut self,
        field: &tracing::field::Field,
        value: &(dyn std::error::Error + 'static),
    ) {
        self.0
            .insert(field.name().into(), serde_json::json!(value.to_string()));
    }

    fn record_debug(&mut self, field: &tracing::field::Field, value: &dyn std::fmt::Debug) {
        self.0.insert(
            field.name().into(),
            serde_json::json!(format!("{:?}", value)),
        );
    }
}

#[derive(Default)]
pub struct WasmTracer(Arc<Mutex<VecDeque<serde_json::Value>>>);

impl WasmTracer {
    pub fn get_logs(&self) -> Arc<Mutex<VecDeque<serde_json::Value>>> {
        self.0.clone()
    }
}

impl<S> Layer<S> for WasmTracer
where
    S: tracing::Subscriber,
    S: for<'lookup> tracing_subscriber::registry::LookupSpan<'lookup>,
{
    fn on_event(
        &self,
        event: &tracing::Event<'_>,
        _ctx: tracing_subscriber::layer::Context<'_, S>,
    ) {
        let mut fields = Default::default();
        let mut visitor = JsonVisitor(&mut fields);
        event.record(&mut visitor);

        fields.remove("log.file");
        fields.remove("log.line");
        fields.remove("log.module_path");
        let target = fields
            .remove("log.target")
            .unwrap_or_else(|| serde_json::json!(event.metadata().target()));

        let output = serde_json::json!({
            "target": target,
            "level": event.metadata().level().to_string(),
            "fields": fields,
        });

        if let Ok(mut it) = self.0.lock() {
            it.push_back(output);
        }
    }
}
