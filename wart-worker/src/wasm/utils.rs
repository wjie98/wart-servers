use crate::bindgen::*;

pub fn dump_to_imports_rows(data: DataFrame) -> (Vec<String>, Vec<imports::Row>) {
    let DataFrame {
        headers,
        columns,
        comment: _,
    } = data;

    let num_cols = columns
        .iter()
        .map(|col| match &col.values {
            Some(vs) => match vs {
                series::Values::BoolValues(x) => x.data.len(),
                series::Values::Int32Values(x) => x.data.len(),
                series::Values::Int64Values(x) => x.data.len(),
                series::Values::Float32Values(x) => x.data.len(),
                series::Values::Float64Values(x) => x.data.len(),
                series::Values::StringValues(x) => x.data.len(),
            },
            None => 0,
        })
        .collect::<Vec<usize>>();
    let num_rows = num_cols.iter().max();
    if let Some(num_rows) = num_rows {
        let num_rows = *num_rows;

        let mut rows = vec![];
        for _ in columns.iter() {
            rows.push(imports::Row::new());
        }

        for (j, col) in columns.iter().enumerate() {
            match &col.values {
                Some(vs) => match vs {
                    series::Values::BoolValues(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::Bol(x.data[i]));
                        }
                    }
                    series::Values::Int32Values(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::I32(x.data[i]));
                        }
                    }
                    series::Values::Int64Values(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::I64(x.data[i]));
                        }
                    }
                    series::Values::Float32Values(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::F32(x.data[i]));
                        }
                    }
                    series::Values::Float64Values(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::F64(x.data[i]));
                        }
                    }
                    series::Values::StringValues(x) => {
                        for i in 0..num_cols[j] {
                            rows[i].push(imports::Value::Txt(x.data[i].clone()));
                        }
                    }
                },
                None => (),
            };

            for i in num_cols[j]..num_rows {
                let i = i + num_cols[j];
                rows[i].push(imports::Value::Nil);
            }
        }
        (headers, rows)
    } else {
        (headers, vec![])
    }
}

pub fn dump_to_imports_table(data: DataFrame) -> (Vec<String>, imports::Table) {
    let DataFrame {
        headers,
        columns,
        comment: _,
    } = data;
    let table = columns
        .into_iter()
        .map(|s| match s.values {
            Some(vs) => match vs {
                series::Values::BoolValues(x) => imports::Series::Bol(x.data),
                series::Values::Int32Values(x) => imports::Series::I32(x.data),
                series::Values::Int64Values(x) => imports::Series::I64(x.data),
                series::Values::Float32Values(x) => imports::Series::F32(x.data),
                series::Values::Float64Values(x) => imports::Series::F64(x.data),
                series::Values::StringValues(x) => imports::Series::Txt(x.data),
            },
            None => imports::Series::Nil,
        })
        .collect::<imports::Table>();
    (headers, table)
}
