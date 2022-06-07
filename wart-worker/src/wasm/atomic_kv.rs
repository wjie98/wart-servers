use crate::bindgen::imports::MergeType;

use log;

use lazy_static::lazy_static;

const LUA_QUERY_BASE: &str = "
    local prefk = KEYS[1]
    local epoch = tonumber(ARGV[1])
    local field = ARGV[2]
    local defva = ARGV[3]
    for ep = epoch,0,-1 do
        local key = prefk .. ep
        local value = redis.hget(key, field)
        if (value) then
            redis.hdel(key, field)
            break
        end
    end
    if (not value) then
        value = defva
    end
    local key = prefk .. epoch
    redis.hset(key, field, value)
";

lazy_static! {
    static ref REDIS_QUERY_KV: redis::Script = {
        let lua_script = format!(
            "{} {}",
            LUA_QUERY_BASE,
            "
            return value
        "
        );
        redis::Script::new(&lua_script)
    };
    static ref REDIS_UPDATE_KV: redis::Script = {
        let lua_script = format!(
            "{} {}",
            LUA_QUERY_BASE,
            "
            local argum = ARGV[4]
            local merge = ARGV[5]
            if (merge == 'add') then
                local key = prefk .. (epoch + 1)
                local value = tonumber(value)
                local argum = tonumber(argum)
                return redis.hset(key, field, value + argum)
            elseif (merge == 'mov') then
                local key = prefk .. (epoch + 1)
                return redis.hset(key, field, argum)
            else
                return 0
            end
        "
        );
        redis::Script::new(&lua_script)
    };
}

pub async fn atomic_query_kv<T, C>(
    prefk: &str,
    epoch: u64,
    field: &str,
    defva: &T,
    con: &mut C,
) -> Option<T>
where
    T: redis::ToRedisArgs + redis::FromRedisValue,
    C: redis::aio::ConnectionLike,
{
    let val: T = REDIS_QUERY_KV
        .key(prefk)
        .arg(epoch)
        .arg(field)
        .arg(defva)
        .invoke_async(con)
        .await
        .map_err(|err| {
            log::error!("redis error: {}", err);
            err
        })
        .ok()?;
    Some(val)
}

pub async fn atomic_update_kv<T, C>(
    prefk: &str,
    epoch: u64,
    field: &str,
    defva: &T,
    argum: &T,
    merge: MergeType,
    con: &mut C,
) -> Option<i32>
where
    T: redis::ToRedisArgs,
    C: redis::aio::ConnectionLike,
{
    let merge = match merge {
        MergeType::Add => "add",
        MergeType::Mov => "mov",
    };
    let val: i32 = REDIS_QUERY_KV
        .key(prefk)
        .arg(epoch)
        .arg(field)
        .arg(defva)
        .arg(argum)
        .arg(merge)
        .invoke_async(con)
        .await
        .map_err(|err| {
            log::error!("redis error: {}", err);
            err
        })
        .ok()?;
    Some(val)
}
