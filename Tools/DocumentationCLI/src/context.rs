use crate::config::Config;
use crate::documentation::Documentation;
use crate::util::path_to_forward_slash;
use std::borrow::Borrow;
use std::collections::{HashMap, HashSet};
use std::path::PathBuf;

pub struct Context {
	pub config: Config,
	pub abs_classes_path: PathBuf,
	pub classes: HashSet<String>,
	pub class_pages: HashMap<String, String>,
	pub abs_structs_path: PathBuf,
	pub class_redirects: HashMap<String, String>,
	pub structs: HashSet<String>,
	pub struct_pages: HashMap<String, String>,
	pub struct_redirects: HashMap<String, String>,
	pub abs_lua_path: PathBuf,
	pub lua_pages: HashMap<String, String>,
	pub lua_types: HashSet<String>,
	pub lua_type_pages: HashMap<String, String>,
}

impl Context {
	pub fn new(config: Config, documentation: &Documentation) -> Self {
		let abs_pages_path = config.abs_nav_file_path().parent().unwrap().join("pages");
		let abs_classes_path = config
			.config_path
			.parent()
			.unwrap()
			.join(&config.reflection_folder)
			.join("classes");
		let abs_structs_path = config
			.config_path
			.parent()
			.unwrap()
			.join(&config.reflection_folder)
			.join("structs");
		let abs_lua_path = config
			.config_path
			.parent()
			.unwrap()
			.join(&config.lua_folder);
		let classes_path = abs_classes_path.strip_prefix(&abs_pages_path).unwrap();
		let structs_path = abs_structs_path.strip_prefix(&abs_pages_path).unwrap();
		let lua_path = abs_lua_path.strip_prefix(&abs_pages_path).unwrap();

		let classes: HashSet<_> = documentation
			.reflection
			.classes
			.iter()
			.map(|c| c.reflection_struct.base.internal_name.clone())
			.collect();
		let structs: HashSet<_> = documentation
			.reflection
			.structs
			.iter()
			.map(|s| s.base.internal_name.clone())
			.collect();
		let lua_types: HashSet<_> = documentation
			.lua
			.modules
			.iter()
			.map(|m| &m.metatables)
			.flatten()
			.map(|m| m.base.internal_name.clone())
			.collect();

		let (class_redirects, class_pages): (Vec<_>, Vec<_>) =
			documentation.reflection.classes.iter().partition(|c| {
				c.reflection_struct.properties.is_empty()
					&& c.reflection_struct.functions.is_empty()
					&& c.signals.is_empty()
					&& c.reflection_struct.parent.is_some()
			});
		let class_redirects = class_redirects
			.into_iter()
			.map(|c| {
				(
					c.reflection_struct.base.internal_name.clone(),
					c.reflection_struct.parent.clone().unwrap(),
				)
			})
			.collect();
		let class_pages = class_pages
			.into_iter()
			.map(|c| &c.reflection_struct.base.internal_name)
			.map(|s| {
				(
					s.clone(),
					path_to_forward_slash(&classes_path.join(format!("{s}.adoc"))).unwrap(),
				)
			})
			.collect();

		let (struct_redirects, struct_pages): (Vec<_>, Vec<_>) =
			documentation.reflection.structs.iter().partition(|s| {
				s.properties.is_empty() && s.functions.is_empty() && s.parent.is_some()
			});
		let struct_redirects = struct_redirects
			.into_iter()
			.map(|s| (s.base.internal_name.clone(), s.parent.clone().unwrap()))
			.collect();
		let struct_pages = struct_pages
			.into_iter()
			.map(|s| &s.base.internal_name)
			.map(|s| {
				(
					s.clone(),
					path_to_forward_slash(&structs_path.join(format!("{s}.adoc"))).unwrap(),
				)
			})
			.collect();

		let lua_pages = documentation
			.lua
			.modules
			.iter()
			.map(|m| &m.base.internal_name)
			.map(|s| {
				(
					s.clone(),
					path_to_forward_slash(&lua_path.join(format!("{s}.adoc"))).unwrap(),
				)
			})
			.collect();
		let lua_type_pages = documentation
			.lua
			.modules
			.iter()
			.map(|m| {
				m.metatables.iter().map(|t| {
					(
						t.base.internal_name.clone(),
						path_to_forward_slash(&lua_path.join(format!(
							"{}.adoc#_{}",
							m.base.internal_name,
							t.base.internal_name.to_lowercase()
						)))
						.unwrap(),
					)
				})
			})
			.flatten()
			.collect();

		Self {
			config,
			abs_classes_path,
			classes,
			class_pages,
			abs_structs_path,
			structs,
			struct_pages,
			abs_lua_path,
			lua_pages,
			lua_types,
			lua_type_pages,
			class_redirects,
			struct_redirects,
		}
	}

	pub fn abs_class_path(&self, class: &str) -> PathBuf {
		self.abs_classes_path.join(format!("{class}.adoc"))
	}

	pub fn abs_struct_path(&self, struct_name: &str) -> PathBuf {
		self.abs_structs_path.join(format!("{struct_name}.adoc"))
	}

	pub fn abs_lua_module_path(&self, module: &str) -> PathBuf {
		self.abs_lua_path.join(format!("{module}.adoc"))
	}

	pub fn xref_class<S: Borrow<str>>(&self, class: &Option<S>) -> String {
		let class = class.as_ref().map(|s| s.borrow()).unwrap_or("Object");
		let mut parent = class;
		while let Some(redirected) = self.class_redirects.get(parent) {
			parent = redirected;
		}
		if let Some(path) = self.class_pages.get(parent) {
			format!("xref:/{path}[{class}]")
		} else {
			class.to_string()
		}
	}

	pub fn xref_struct<S: Borrow<str>>(&self, struct_type: &Option<S>) -> String {
		let struct_type = struct_type
			.as_ref()
			.map(|s| {
				let mut struct_type = s.borrow();
				while let Some(redirected) = self.struct_redirects.get(struct_type) {
					struct_type = redirected;
				}
				self.struct_pages.get(struct_type).map(|p| (s.borrow(), p))
			})
			.flatten();
		if let Some((struct_type, path)) = struct_type {
			format!("xref:/{path}[{struct_type}]")
		} else {
			"any".to_string()
		}
	}

	pub fn xref_lua_module<S: Borrow<str>>(&self, lua_module: S) -> String {
		let lua_module = lua_module.borrow();
		if let Some(path) = self.lua_pages.get(lua_module) {
			format!("xref:/{path}[{lua_module}]")
		} else {
			lua_module.to_string()
		}
	}

	pub fn xref_lua_type<S: Borrow<str>>(&self, lua_type: S) -> String {
		let lua_type = lua_type.borrow();
		if let Some(path) = self.lua_type_pages.get(lua_type) {
			format!("xref:/{path}[{lua_type}]")
		} else {
			lua_type.to_string()
		}
	}
}
