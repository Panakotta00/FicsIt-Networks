use serde::Deserialize;
use std::path::{Path, PathBuf};

#[derive(Deserialize, Debug)]
pub struct Config {
	pub nav_file_path: PathBuf,
	pub reflection_folder: PathBuf,
	pub lua_folder: PathBuf,
	#[serde(skip)]
	pub config_path: PathBuf,
}

impl Config {
	pub fn load(config_path: &Path) -> std::io::Result<Config> {
		let contents = std::fs::read_to_string(config_path)?;
		let mut config: Config = toml::from_str(&contents)
			.map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
		config.config_path = std::path::absolute(config_path).unwrap();
		Ok(config)
	}

	pub fn abs_nav_file_path(&self) -> PathBuf {
		self.config_path.parent().unwrap().join(&self.nav_file_path)
	}

	pub fn abs_reflection_path(&self) -> PathBuf {
		self.config_path
			.parent()
			.unwrap()
			.join(&self.reflection_folder)
	}

	pub fn abs_lua_path(&self) -> PathBuf {
		self.config_path.parent().unwrap().join(&self.lua_folder)
	}
}
