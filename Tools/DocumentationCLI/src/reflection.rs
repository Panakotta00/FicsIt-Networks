use crate::context::Context;
use crate::util::{escape_for_adoc_table, prefix_string_lines};
use serde::Deserialize;
use std::borrow::Borrow;
use std::cmp::Ordering;
use std::fs::File;
use std::io::{Result, Write};

#[derive(Deserialize, Debug)]
#[serde(tag = "type")]
pub enum ReflectionValueType {
	Bool,
	Int,
	Float,
	String,
	Object {
		subclass: Option<String>,
	},
	Trace {
		subclass: Option<String>,
	},
	Struct {
		inner: Option<String>,
	},
	Class {
		subclass: Option<String>,
	},
	Array {
		inner: Option<Box<ReflectionValueType>>,
	},
}

#[derive(Deserialize, Debug, Eq, PartialEq)]
pub enum ReflectionPropertyFlag {
	Attrib,
	ReadOnly,
	Param,
	OutParam,
	RetVal,
	#[serde(alias = "RT_Sync")]
	RTSync,
	#[serde(alias = "RT_Parallel")]
	RTParallel,
	#[serde(alias = "RT_Async")]
	RTAsync,
	ClassProp,
	StaticProp,
}

#[derive(Deserialize, Debug, PartialOrd, Eq)]
pub struct ReflectionBase {
	#[serde(alias = "internalName")]
	pub internal_name: String,
	#[serde(alias = "displayName")]
	pub display_name: Option<String>,
	pub description: String,
}

impl ReflectionBase {
	pub fn display_name(&self) -> &str {
		self.display_name.as_deref().unwrap_or(&self.internal_name)
	}

	pub fn name_adoc(&self) -> String {
		if let Some(display_name) = &self.display_name {
			format!("*{}* `{}`", display_name, self.internal_name)
		} else {
			format!("`{}`", self.internal_name)
		}
	}

	pub fn header_name_adoc(&self) -> String {
		format!("`{}`", self.internal_name)
	}
}

impl PartialEq<Self> for ReflectionBase {
	fn eq(&self, other: &Self) -> bool {
		self.internal_name.eq(&other.internal_name)
	}
}

impl Ord for ReflectionBase {
	fn cmp(&self, other: &Self) -> Ordering {
		self.internal_name.cmp(&other.internal_name)
	}
}

#[derive(Deserialize, Debug)]
pub struct ReflectionProperty {
	#[serde(flatten)]
	base: ReflectionBase,
	#[serde(alias = "type")]
	value_type: ReflectionValueType,
	flags: Vec<ReflectionPropertyFlag>,
}

#[derive(Deserialize, Debug, Eq, PartialEq)]
pub enum ReflectionFunctionFlag {
	VarArgs,
	#[serde(alias = "RT_Sync")]
	RTSync,
	#[serde(alias = "RT_Parallel")]
	RTParallel,
	#[serde(alias = "RT_Async")]
	RTAsync,
	MemberFunc,
	ClassFunc,
	StaticFunc,
	VarRets,
}

#[derive(Deserialize, Debug)]
pub struct ReflectionFunction {
	#[serde(flatten)]
	base: ReflectionBase,
	parameters: Vec<ReflectionProperty>,
	flags: Vec<ReflectionFunctionFlag>,
}

#[derive(Deserialize, Debug)]
pub struct ReflectionStruct {
	#[serde(flatten)]
	pub base: ReflectionBase,
	pub parent: Option<String>,
	pub properties: Vec<ReflectionProperty>,
	pub functions: Vec<ReflectionFunction>,
}

#[derive(Deserialize, Debug)]
pub struct ReflectionSignal {
	#[serde(flatten)]
	base: ReflectionBase,
	parameters: Vec<ReflectionProperty>,
	#[serde(alias = "isVarArgs")]
	is_var_args: bool,
}

#[derive(Deserialize, Debug)]
pub struct ReflectionClass {
	#[serde(flatten)]
	pub reflection_struct: ReflectionStruct,
	pub signals: Vec<ReflectionSignal>,
}

#[derive(Deserialize, Debug)]
pub struct ReflectionDocumentation {
	pub classes: Vec<ReflectionClass>,
	pub structs: Vec<ReflectionStruct>,
}

pub struct ReflectionContext<'a> {
	pub ctx: &'a Context,
}

impl<'a> ReflectionContext<'a> {
	pub fn new(context: &'a Context) -> Self {
		Self { ctx: context }
	}

	fn get_value_type(&self, value: &ReflectionValueType) -> String {
		match value {
			ReflectionValueType::Bool => "Bool".to_string(),
			ReflectionValueType::Int => "Int".to_string(),
			ReflectionValueType::Float => "Float".to_string(),
			ReflectionValueType::String => "String".to_string(),
			ReflectionValueType::Object { subclass } => {
				format!("Object<{}>", self.ctx.xref_class(subclass))
			}
			ReflectionValueType::Trace { subclass } => {
				format!("Trace<{}>", self.ctx.xref_class(subclass))
			}
			ReflectionValueType::Struct { inner } => {
				if let Some(inner) = inner {
					format!("Struct<{}>", self.ctx.xref_struct(&Some(inner.as_str())))
				} else {
					"Struct<any>".to_string()
				}
			}
			ReflectionValueType::Class { subclass } => {
				format!("Class<{}>", self.ctx.xref_class(subclass))
			}
			ReflectionValueType::Array { inner } => {
				if let Some(inner) = inner {
					format!("Array<{}>", self.get_value_type(inner))
				} else {
					"Array<any>".to_string()
				}
			}
		}
	}

	fn write_property(&self, file: &mut File, property: &ReflectionProperty) -> Result<()> {
		let flags: String = property
			.flags
			.iter()
			.map(|f| match f {
				ReflectionPropertyFlag::ReadOnly => {
					Some("<span style='color:#e59445'><i>ReadOnly</i></span>")
				}
				ReflectionPropertyFlag::RTSync => {
					Some("<span style='color:#bb2828'><i>RuntimeSync</i></span>")
				}
				ReflectionPropertyFlag::RTParallel => {
					Some("<span style='color:#bb2828'><i>RuntimeParallel</i></span>")
				}
				ReflectionPropertyFlag::RTAsync => {
					Some("<span style='color:#bb2828'><i>RuntimeAsync</i></span>")
				}
				ReflectionPropertyFlag::ClassProp => {
					Some("<span style='color:#5dafc5'><i>ClassProp</i></span>")
				}
				ReflectionPropertyFlag::StaticProp => {
					Some("<span style='color:#5dafc5'><i>StaticProp</i></span>")
				}
				_ => None,
			})
			.flatten()
			.intersperse(" ")
			.collect();

		let display_name_table = if let Some(display_name) = &property.base.display_name {
			format!("\n! Display Name ! {display_name}")
		} else {
			"".to_string()
		};

		writeln!(
			file,
			"=== {} : {}",
			property.base.header_name_adoc(),
			self.get_value_type(&property.value_type)
		)?;
		writeln!(file)?;
		writeln!(file, "{}", property.base.description)?;
		writeln!(file)?;
		writeln!(file, "[%collapsible]")?;
		writeln!(file, "====")?;
		writeln!(file, r#"[cols="1,5a",separator="!"]"#)?;
		writeln!(file, "!===")?;
		writeln!(file, "! Flags ! +++{flags}+++")?;
		writeln!(file, "{display_name_table}")?;
		writeln!(file, "!===")?;
		writeln!(file, "====")?;

		Ok(())
	}

	fn write_property_table<P: Borrow<ReflectionProperty>>(
		&self,
		file: &mut File,
		properties: &[P],
	) -> Result<()> {
		writeln!(file, r#"[%header,cols="1,1,4a",separator="!"]"#)?;
		writeln!(file, r#"!==="#)?;
		writeln!(file, r#"!Name !Type !Description"#)?;

		for property in properties {
			let property = property.borrow();
			writeln!(file)?;
			writeln!(file, "! {}", property.base.name_adoc())?;
			writeln!(
				file,
				"! {}",
				escape_for_adoc_table(&self.get_value_type(&property.value_type))
			)?;
			writeln!(
				file,
				"! {}",
				prefix_string_lines("  ", &escape_for_adoc_table(&property.base.description))
			)?;
		}

		writeln!(file, r"!===")?;

		Ok(())
	}

	fn write_function(&self, file: &mut File, function: &ReflectionFunction) -> Result<()> {
		let (parameters, return_values): (Vec<_>, Vec<_>) = function
			.parameters
			.iter()
			.map(|p| {
				(
					p,
					format!(
						"{} : {}",
						p.base.header_name_adoc(),
						self.get_value_type(&p.value_type)
					),
				)
			})
			.partition(|(p, _)| !p.flags.contains(&ReflectionPropertyFlag::OutParam));
		let (parameter_props, mut parameter_strings): (Vec<_>, Vec<_>) =
			parameters.into_iter().unzip();
		let (return_value_props, mut return_value_strings): (Vec<_>, Vec<_>) =
			return_values.into_iter().unzip();
		if function.flags.contains(&ReflectionFunctionFlag::VarArgs) {
			parameter_strings.push("...".to_string());
		}
		if function.flags.contains(&ReflectionFunctionFlag::VarRets) {
			return_value_strings.push("...".to_string());
		}
		let parameters = parameter_strings.join(", ");
		let return_values = match &return_value_strings[..] {
			[] => "".to_string(),
			[s] => format!(" -> {}", s),
			_ => format!(" -> ({})", return_value_strings.join(", ")),
		};

		let flags: String = function
			.flags
			.iter()
			.map(|f| match f {
				ReflectionFunctionFlag::VarArgs => {
					"<span style='color:#e59445'><i>VarArgs</i></span>"
				}
				ReflectionFunctionFlag::RTSync => {
					"<span style='color:#bb2828'><i>RuntimeSync</i></span>"
				}
				ReflectionFunctionFlag::RTParallel => {
					"<span style='color:#bb2828'><i>RuntimeParallel</i></span>"
				}
				ReflectionFunctionFlag::RTAsync => {
					"<span style='color:#bb2828'><i>RuntimeAsync</i></span>"
				}
				ReflectionFunctionFlag::MemberFunc => {
					"<span style='color:#5dafc5'><i>MemberFunc</i></span>"
				}
				ReflectionFunctionFlag::ClassFunc => {
					"<span style='color:#5dafc5'><i>ClassFunc</i></span>"
				}
				ReflectionFunctionFlag::StaticFunc => {
					"<span style='color:#5dafc5'><i>StaticFunc</i></span>"
				}
				ReflectionFunctionFlag::VarRets => {
					"<span style='color:#e59445'><i>VarReturns</i></span>"
				}
			})
			.intersperse(" ")
			.collect();

		let display_name_table = if let Some(display_name) = &function.base.display_name {
			format!("\n! Display Name ! {display_name}\n")
		} else {
			"".to_string()
		};

		writeln!(
			file,
			"=== {} ({parameters}){return_values}",
			function.base.header_name_adoc()
		)?;
		writeln!(file)?;
		writeln!(file, "{}", function.base.description)?;
		writeln!(file)?;
		writeln!(file, "[%collapsible]")?;
		writeln!(file, "====")?;
		writeln!(file, r#"[cols="1,5a",separator="!"]"#)?;
		writeln!(file, "!===")?;
		writeln!(file, "! Flags")?;
		writeln!(file, "! +++{flags}+++")?;
		write!(file, "{display_name_table}")?;
		writeln!(file, "!===")?;
		writeln!(file)?;

		if !parameter_props.is_empty() {
			writeln!(file, ".Parameters")?;
			self.write_property_table(file, &parameter_props)?;
			writeln!(file)?;
		}

		if !return_value_props.is_empty() {
			writeln!(file, ".Return Values")?;
			self.write_property_table(file, &return_value_props)?;
			writeln!(file)?;
		}

		writeln!(file, "====")?;

		Ok(())
	}

	fn write_file_base(&self, file: &mut File, base: &ReflectionBase) -> Result<()> {
		let title = base.display_name();
		let description = &base.description;

		writeln!(file, "= {title}")?;
		writeln!(file, ":table-caption!:")?;
		writeln!(file)?;
		writeln!(file, "{description}")?;
		writeln!(file)?;

		Ok(())
	}

	pub(crate) fn write_struct(
		&self,
		file: &mut File,
		ref_struct: &ReflectionStruct,
	) -> Result<()> {
		self.write_file_base(file, &ref_struct.base)?;

		if !ref_struct.properties.is_empty() {
			let mut properties: Vec<_> = ref_struct.properties.iter().collect();
			properties.sort_by(|p1, p2| p1.base.display_name().cmp(p2.base.display_name()));

			writeln!(file, "== Properties")?;
			writeln!(file)?;

			for property in &properties {
				self.write_property(file, property)?;
			}

			writeln!(file)?;
		}

		if !ref_struct.functions.is_empty() {
			let mut functions: Vec<_> = ref_struct.functions.iter().collect();
			functions.sort_by(|p1, p2| p1.base.display_name().cmp(p2.base.display_name()));

			writeln!(file, "== Functions")?;
			writeln!(file)?;

			for function in &functions {
				self.write_function(file, function)?;
			}

			writeln!(file)?;
		}

		Ok(())
	}

	fn write_signal(&self, file: &mut File, signal: &ReflectionSignal) -> Result<()> {
		let mut parameters = signal
			.parameters
			.iter()
			.map(|p| {
				format!(
					"_{}_ {}",
					self.get_value_type(&p.value_type),
					p.base.name_adoc()
				)
			})
			.collect::<Vec<String>>();
		if signal.is_var_args {
			parameters.push("...".to_string());
		}
		let parameters = parameters.join(", ");

		let name = signal.base.header_name_adoc();
		let description = &signal.base.description;

		writeln!(file, "=== {name} ({parameters})")?;
		writeln!(file)?;
		writeln!(file, "{description}")?;
		writeln!(file)?;

		if !signal.parameters.is_empty() {
			writeln!(file, "[%collapsible]")?;
			writeln!(file, "====")?;
			writeln!(file, ".Parameters")?;
			self.write_property_table(file, &signal.parameters)?;
			writeln!(file, "====")?;
			writeln!(file)?;
		}

		Ok(())
	}

	pub(crate) fn write_class(&self, file: &mut File, ref_class: &ReflectionClass) -> Result<()> {
		self.write_file_base(file, &ref_class.reflection_struct.base)?;

		writeln!(file, "// tag::interface[]")?;
		writeln!(file)?;

		if !ref_class.reflection_struct.properties.is_empty() {
			let mut properties: Vec<_> = ref_class.reflection_struct.properties.iter().collect();
			properties.sort_by(|p1, p2| p1.base.display_name().cmp(p2.base.display_name()));

			writeln!(file, "== Properties")?;
			writeln!(file)?;

			for property in &properties {
				self.write_property(file, property)?;
			}

			writeln!(file)?;
		}

		if !ref_class.reflection_struct.functions.is_empty() {
			let mut functions: Vec<_> = ref_class.reflection_struct.functions.iter().collect();
			functions.sort_by(|p1, p2| p1.base.display_name().cmp(p2.base.display_name()));

			writeln!(file, "== Functions")?;
			writeln!(file)?;

			for function in &functions {
				self.write_function(file, function)?;
			}

			writeln!(file)?;
		}

		if !ref_class.signals.is_empty() {
			let mut signals: Vec<_> = ref_class.signals.iter().collect();
			signals.sort_by(|p1, p2| p1.base.display_name().cmp(p2.base.display_name()));

			writeln!(file, "== Signals")?;
			writeln!(file)?;

			for signal in &signals {
				self.write_signal(file, &signal)?;
			}

			writeln!(file)?;
		}

		writeln!(file, "// end::interface[]")?;
		writeln!(file)?;

		Ok(())
	}
}
