use crate::bindgen::imports;

use anyhow::Result;
use std::sync::Arc;

use wasmtime::{Config, Engine, Instance, Linker, Module, Store, Trap};
use wasmtime_wasi::WasiCtx;

pub struct Context<T>
where
    T: imports::Imports,
{
    pub wasi_ctx: WasiCtx,
    pub imports: T,
    imports_tables: imports::ImportsTables<T>,
}

impl<T> Context<T>
where
    T: imports::Imports,
{
    fn new(wasi_ctx: WasiCtx, imports: T) -> Self {
        let imports_tables = imports::ImportsTables::default();
        Self {
            wasi_ctx,
            imports,
            imports_tables,
        }
    }
}

#[derive(Clone)]
pub struct SandboxManager<T>
where
    T: imports::Imports,
{
    pub engine: Engine,
    pub module: Module,
    #[allow(dead_code)]
    linker: Arc<Linker<Context<T>>>,
}

impl<T> SandboxManager<T>
where
    T: imports::Imports + Send + 'static,
    T::FutureRow: Send + 'static,
    T::FutureTable: Send + 'static,
{
    pub fn from_module(module: &[u8], config: &Config) -> Result<Self> {
        let engine = Engine::new(config)?;
        let module = unsafe { Module::deserialize(&engine, module)? };
        let linker = Self::add_to_linker(&engine)?;
        Ok(Self {
            engine,
            module,
            linker: Arc::new(linker),
        })
    }

    pub fn from_bytes(program: &[u8], config: &Config) -> Result<Self> {
        let engine = Engine::new(config)?;
        let module = Module::new(&engine, program)?;
        let linker = Self::add_to_linker(&engine)?;
        Ok(Self {
            engine,
            module,
            linker: Arc::new(linker),
        })
    }

    pub fn instantiate(&self, wasi_ctx: WasiCtx, imports: T) -> Result<Sandbox<T>> {
        let mut store = Store::new(&self.engine, Context::<T>::new(wasi_ctx, imports));
        let instance = self.linker.instantiate(&mut store, &self.module)?;
        Ok(Sandbox::<T> { store, instance })
    }

    fn add_to_linker(engine: &Engine) -> Result<Linker<Context<T>>> {
        let mut linker: Linker<Context<T>> = Linker::new(&engine);
        wasmtime_wasi::add_to_linker(&mut linker, |cx| &mut cx.wasi_ctx)?;
        imports::add_to_linker(&mut linker, |cx| (&mut cx.imports, &mut cx.imports_tables))?;
        Ok(linker)
    }
}

pub struct Sandbox<T>
where
    T: imports::Imports,
{
    pub store: Store<Context<T>>,
    instance: Instance,
}

impl<T> Sandbox<T>
where
    T: imports::Imports + Send + 'static,
    T::FutureRow: Send + 'static,
    T::FutureTable: Send + 'static,
{
    // pub fn block_on<F, R>(&mut self, signal: F) -> Result<(), Trap>
    // where
    //     F: Future<Output = R> + Send + 'static,
    //     F::Output: Send + 'static,
    // {
    //     use futures::task::noop_waker;
    //     use std::task::{Context, Poll};

    //     let _start = self.instance.get_typed_func(&mut self.store, "_start")?;

    //     let mut future = Box::pin(_start.call_async(&mut self.store, ()));
    //     let mut signal = Box::pin(signal);

    //     let waker = noop_waker();
    //     let mut cx = Context::from_waker(&waker);

    //     loop {
    //         if let Poll::Ready(_) = signal.as_mut().poll(&mut cx) {
    //             return Err(Trap::new("canceled by rpc server"));
    //         }

    //         if let Poll::Ready(ret) = future.as_mut().poll(&mut cx) {
    //             return ret;
    //         }
    //     }
    // }

    pub fn call_ctors(&mut self) -> Result<(), Trap> {
        let __wasm_call_ctors = self
            .instance
            .get_typed_func(&mut self.store, "__wasm_call_ctors")?;
        __wasm_call_ctors.call(&mut self.store, ())
    }

    pub fn call_dtors(&mut self) -> Result<(), Trap> {
        let __wasm_call_dtors = self
            .instance
            .get_typed_func(&mut self.store, "__wasm_call_dtors")?;
        __wasm_call_dtors.call(&mut self.store, ())
    }

    pub fn call_start(&mut self) -> Result<(), Trap> {
        let _start = self.instance.get_typed_func(&mut self.store, "_start")?;
        _start.call(&mut self.store, ())
    }

    pub fn call_all(&mut self) -> Result<(), Trap> {
        self.call_ctors()?;
        self.call_start()?;
        self.call_dtors()?;
        Ok(())
    }
}
