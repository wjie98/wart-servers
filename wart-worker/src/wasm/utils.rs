use crate::bindgen::*;

pub fn dump_to_imports_row(data: DataFrame) -> imports::RowResult {
    let DataFrame {
        headers,
        columns,
        comment: _,
    } = data;

    headers
        .into_iter()
        .zip(columns.into_iter())
        .map(|(h, s)| -> imports::ItemResult {
            match s.values {
                Some(v) => match v {
                    series::Values::BoolValues(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::Bol(x.data[0]),
                    },
                    series::Values::Int32Values(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::I32(x.data[0]),
                    },
                    series::Values::Int64Values(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::I64(x.data[0]),
                    },
                    series::Values::Float32Values(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::F32(x.data[0]),
                    },
                    series::Values::Float64Values(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::F64(x.data[0]),
                    },
                    series::Values::StringValues(x) if !x.data.is_empty() => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::Txt(x.data[0].clone()),
                    },
                    _ => imports::ItemResult {
                        key: h,
                        val: imports::ValueResult::Nil,
                    },
                },
                None => imports::ItemResult {
                    key: h,
                    val: imports::ValueResult::Nil,
                },
            }
        })
        // .filter(|s| !matches!(s.val, imports::ValueResult::Nil))
        .collect::<Vec<_>>()
}

pub fn dump_to_imports_table(data: DataFrame) -> imports::Table {
    let DataFrame {
        headers,
        columns,
        comment: _,
    } = data;

    headers
        .into_iter()
        .zip(columns.into_iter())
        .map(|(h, s)| -> imports::Series {
            match s.values {
                Some(v) => match v {
                    series::Values::BoolValues(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::Bol(x.data),
                    },
                    series::Values::Int32Values(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::I32(x.data),
                    },
                    series::Values::Int64Values(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::I64(x.data),
                    },
                    series::Values::Float32Values(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::F32(x.data),
                    },
                    series::Values::Float64Values(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::F64(x.data),
                    },
                    series::Values::StringValues(x) => imports::Series {
                        key: h,
                        val: imports::VectorResult::Txt(x.data),
                    },
                },
                None => imports::Series {
                    key: h,
                    val: imports::VectorResult::Nil,
                },
            }
        })
        // .filter(|s| !matches!(s.val, imports::Vector::Nil))
        .collect::<Vec<_>>()
}
