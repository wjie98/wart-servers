use tracing_subscriber::Layer;

pub struct WasmTracer;
pub struct PrintlnVisitor;

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
        // println!("got event:");
        // println!("  level = {:?}", event.metadata().level());
        // println!("  target = {:?}", event.metadata().target());
        // println!("  name = {:?}", event.metadata().name());
        // let mut visitor = PrintlnVisitor;
        // event.record(&mut visitor);
    }

    // fn on_new_span(
    //     &self,
    //     attrs: &tracing::span::Attributes<'_>,
    //     id: &tracing::span::Id,
    //     ctx: tracing_subscriber::layer::Context<'_, S>,
    // ) {
    //     let span = ctx.span(id).unwrap();
    //     println!("Got on_new_span!");
    //     println!("  level={:?}", span.metadata().level());
    //     println!("  target={:?}", span.metadata().target());
    //     println!("  name={:?}", span.metadata().name());

    //     let mut visitor = PrintlnVisitor;
    //     attrs.record(&mut visitor);
    // }
}

impl tracing::field::Visit for PrintlnVisitor {
    fn record_f64(&mut self, field: &tracing::field::Field, value: f64) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_i64(&mut self, field: &tracing::field::Field, value: i64) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_u64(&mut self, field: &tracing::field::Field, value: u64) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_bool(&mut self, field: &tracing::field::Field, value: bool) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_str(&mut self, field: &tracing::field::Field, value: &str) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_error(
        &mut self,
        field: &tracing::field::Field,
        value: &(dyn std::error::Error + 'static),
    ) {
        println!("  field={} value={}", field.name(), value)
    }

    fn record_debug(&mut self, field: &tracing::field::Field, value: &dyn std::fmt::Debug) {
        println!("  field={} value={:?}", field.name(), value)
    }
}
