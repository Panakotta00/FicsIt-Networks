use crate::context::Context;
use crate::util::prefix_string_lines;
use regex::{Captures, Regex};
use serde::Deserialize;
use serde_with::{serde_as, KeyValueMap};
use std::fs::File;
use std::io::{Result, Write};

#[derive(Deserialize, Debug)]
pub struct LuaBase {
	#[serde(rename = "$key$")]
	pub internal_name: String,
	#[serde(alias = "displayName")]
	pub display_name: Option<String>,
	pub description: String,
}

impl LuaBase {
	pub fn display_name(&self) -> &str {
		self.display_name.as_deref().unwrap_or(&self.internal_name)
	}

	pub fn name_adoc(&self) -> String {
		if let Some(display_name) = &self.display_name {
			format!("*{display_name}* `{}`", self.internal_name)
		} else {
			format!("`{}`", self.internal_name)
		}
	}
}

#[derive(Deserialize, Debug)]
pub struct LuaTableField {
	#[serde(flatten)]
	pub base: LuaBase,
	#[serde(flatten)]
	pub value: LuaValue,
}

#[derive(Deserialize, Debug)]
pub struct LuaFunctionParameter {
	#[serde(flatten)]
	pub base: LuaBase,
	#[serde(alias = "type")]
	pub lua_type: String,
}

#[serde_as]
#[derive(Deserialize, Debug)]
#[serde(tag = "valueType")]
pub enum LuaValue {
	#[serde(alias = "table")]
	Table {
		#[serde_as(as = "KeyValueMap<_>")]
		fields: Vec<LuaTableField>,
	},
	#[serde(alias = "function")]
	Function {
		#[serde(alias = "parameterSignature")]
		parameter_signature: String,
		#[serde(alias = "returnValueSignature")]
		return_value_signature: String,
		#[serde_as(as = "KeyValueMap<_>")]
		parameters: Vec<LuaFunctionParameter>,
		#[serde(alias = "returnValues")]
		#[serde_as(as = "KeyValueMap<_>")]
		return_values: Vec<LuaFunctionParameter>,
	},
	#[serde(alias = "bareValue")]
	BareValue {
		#[serde(alias = "luaType")]
		lua_type: String,
	},
}

#[derive(Deserialize, Debug)]
pub struct LuaMetatable {
	#[serde(flatten)]
	pub base: LuaBase,
	#[serde(flatten)]
	pub table: LuaValue,
}

#[derive(Deserialize, Debug)]
pub struct LuaGlobal {
	#[serde(flatten)]
	pub base: LuaBase,
	#[serde(flatten)]
	pub value: LuaValue,
}

#[serde_as]
#[derive(Deserialize, Debug)]
pub struct LuaModule {
	#[serde(flatten)]
	pub base: LuaBase,
	pub dependencies: Vec<String>,
	#[serde_as(as = "KeyValueMap<_>")]
	pub metatables: Vec<LuaMetatable>,
	#[serde_as(as = "KeyValueMap<_>")]
	pub globals: Vec<LuaGlobal>,
}

#[serde_as]
#[derive(Deserialize, Debug)]
pub struct LuaDocumentation {
	#[serde_as(as = "KeyValueMap<_>")]
	pub modules: Vec<LuaModule>,
}

pub struct LuaContext<'a> {
	pub ctx: &'a Context,
}

impl<'a> LuaContext<'a> {
	pub fn new(context: &'a Context) -> Self {
		Self { ctx: context }
	}

	fn lua_type_identifier_adoc(&self, identifier: &str) -> String {
		if self.ctx.classes.contains(identifier) {
			self.ctx.xref_class(&Some(identifier))
		} else if self.ctx.structs.contains(identifier) {
			self.ctx.xref_struct(&Some(identifier))
		} else if self.ctx.lua_types.contains(identifier) {
			self.ctx.xref_lua_type(identifier)
		} else {
			identifier.to_string()
		}
	}

	fn patch_lua_types(&self, str: &str) -> String {
		let regex = Regex::new(r"[\w\-]+").unwrap();
		regex
			.replace_all(str, |captures: &Captures| -> String {
				let str = captures.get(0).unwrap().as_str();
				self.lua_type_identifier_adoc(str)
			})
			.to_string()
	}

	fn write_function_parameter_table<'b, I: Iterator<Item = &'b LuaFunctionParameter>>(
		&self,
		file: &mut File,
		parameters: I,
	) -> Result<()> {
		writeln!(file, r#"[%header,cols="1,1,4a"]"#)?;
		writeln!(file, r#"|==="#)?;
		writeln!(file, r#"|Name |Type |Description"#)?;
		writeln!(file)?;

		for parameter in parameters {
			writeln!(file, "| {}", parameter.base.name_adoc())?;
			writeln!(file, "| {}", self.patch_lua_types(&parameter.lua_type))?;
			writeln!(
				file,
				"| {}",
				prefix_string_lines("  ", &parameter.base.description)
			)?;
			writeln!(file)?;
		}

		writeln!(file, r#"|==="#)?;

		Ok(())
	}

	fn write_value(
		&self,
		file: &mut File,
		header: &str,
		identifier: &str,
		base: &LuaBase,
		value: &LuaValue,
	) -> Result<()> {
		if base.internal_name.starts_with("__") {
			return Ok(());
		}

		let suffix = match value {
			LuaValue::Table { .. } => "".to_string(),
			LuaValue::Function {
				parameter_signature,
				return_value_signature,
				parameters: _,
				return_values: _,
			} => {
				let parameters = self.patch_lua_types(&parameter_signature);
				if return_value_signature.is_empty() {
					format!(" ({parameters})")
				} else {
					let return_values = self.patch_lua_types(&return_value_signature);
					format!(" ({parameters}) -> {return_values}")
				}
			}
			LuaValue::BareValue { lua_type } => format!(" : {}", self.patch_lua_types(lua_type)),
		};

		let identifier = if identifier.is_empty() {
			identifier.to_string()
		} else {
			format!("{identifier}.")
		};

		let header_identifier = if identifier.is_empty() {
			"".to_string()
		} else {
			format!("__{identifier}__")
		};

		writeln!(
			file,
			"{header} {header_identifier}**{}**{suffix}",
			base.internal_name
		)?;
		writeln!(file, "{}", base.description)?;
		writeln!(file)?;

		let header = format!("{header}=");
		let identifier = format!("{identifier}{}", base.internal_name);

		match value {
			LuaValue::Table { fields } => {
				for field in fields {
					self.write_value(file, &header, &identifier, &field.base, &field.value)?;
				}
			}
			LuaValue::Function {
				parameters,
				return_values,
				parameter_signature: _,
				return_value_signature: _,
			} => {
				if !parameters.is_empty() {
					writeln!(file, "Parameters::")?;
					self.write_function_parameter_table(file, parameters.iter())?;
					writeln!(file)?;
				}
				if !return_values.is_empty() {
					writeln!(file, "Return Values::")?;
					self.write_function_parameter_table(file, return_values.iter())?;
					writeln!(file)?;
				}
			}
			_ => {}
		}

		Ok(())
	}

	fn write_global(&self, file: &mut File, global: &LuaGlobal) -> Result<()> {
		self.write_value(file, "===", "", &global.base, &global.value)
	}

	fn write_metatable(&self, file: &mut File, metatable: &LuaMetatable) -> Result<()> {
		self.write_value(file, "===", "", &metatable.base, &metatable.table)
	}

	pub fn write_module(&self, file: &mut File, module: &LuaModule) -> Result<()> {
		writeln!(file, "= {}", module.base.display_name())?;
		writeln!(file, "{}", module.base.description)?;
		writeln!(file)?;
		
		if !module.dependencies.is_empty() {
			let dependencies: String = module
				.dependencies
				.iter()
				.map(|s| self.ctx.xref_lua_module(s.as_str()))
				.intersperse(", ".to_string())
				.collect();
			
			writeln!(file, r#"[cols="1,5a"]"#)?;
			writeln!(file, "|===")?;
			writeln!(file, "|Dependencies")?;
			writeln!(file, "| {dependencies}")?;
			writeln!(file, "|===")?;
			writeln!(file)?;
		}

		if !module.globals.is_empty() {
			writeln!(file, "== Globals")?;
			writeln!(file)?;

			for global in &module.globals {
				self.write_global(file, global)?;
			}
		}

		if !module.metatables.is_empty() {
			writeln!(file, "== Types")?;
			writeln!(file)?;

			for metatable in &module.metatables {
				self.write_metatable(file, metatable)?;
			}
		}

		Ok(())
	}
}
