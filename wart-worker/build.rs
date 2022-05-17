fn main() -> Result<(), Box<dyn std::error::Error>> {
    tonic_build::compile_protos("../interface/protobuf/wart-types.proto")?;
    tonic_build::compile_protos("../interface/protobuf/wart-worker.proto")?;
    tonic_build::compile_protos("../interface/protobuf/wart-storage.proto")?;
    Ok(())
}
