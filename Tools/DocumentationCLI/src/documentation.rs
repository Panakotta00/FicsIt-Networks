use crate::lua::LuaDocumentation;
use crate::reflection::ReflectionDocumentation;
use serde::Deserialize;
use std::io::BufReader;
use std::path::Path;

#[derive(Deserialize, Debug)]
pub struct Documentation {
	pub reflection: ReflectionDocumentation,
	pub lua: LuaDocumentation,
}

impl Documentation {
	pub fn load(path: &Path) -> std::io::Result<Self> {
		let file = std::fs::File::open(path)
			.map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
		let buf = BufReader::new(file);
		let documentation: Self = serde_json::from_reader(buf)
			.map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
		Ok(documentation)
	}
}
