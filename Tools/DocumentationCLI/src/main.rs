#![feature(iter_intersperse)]

mod config;
mod context;
mod documentation;
mod lua;
mod reflection;
mod util;

use crate::config::Config;
use crate::context::Context;
use crate::documentation::Documentation;
use crate::lua::{LuaContext, LuaDocumentation};
use crate::reflection::{ReflectionContext, ReflectionDocumentation};
use regex::Regex;
use std::fs::{create_dir_all, remove_dir_all, File};
use std::io::Write;
use std::io::{BufRead, BufReader};
use std::path::{Path, PathBuf};

fn fatal<T>(error: &str) -> T {
	eprintln!("Fatal error occurred: {}", error);
	std::process::exit(-1)
}

fn remove_files_in_folder(path: &Path) {
	let _ = remove_dir_all(path);
	create_dir_all(path).unwrap_or_else(|e| {
		fatal(&format!(
			"Unable to create empty folder '{}': {e}",
			path.to_str().unwrap_or("")
		))
	});
}

fn write_reflection_files(context: &Context, doc: &ReflectionDocumentation) {
	remove_files_in_folder(&context.config.abs_reflection_path());

	let classes = doc.classes.iter().filter(|c| {
		context
			.class_pages
			.contains_key(&c.reflection_struct.base.internal_name)
	});
	let structs = doc
		.structs
		.iter()
		.filter(|s| context.struct_pages.contains_key(&s.base.internal_name));

	let context = ReflectionContext::new(context);

	create_dir_all(&context.ctx.abs_classes_path)
		.unwrap_or_else(|e| fatal(&format!("Unable to create structs folder: {e}")));
	for ref_class in classes {
		let path = context
			.ctx
			.abs_class_path(&ref_class.reflection_struct.base.internal_name);
		let mut file = File::create(&path).unwrap_or_else(|e| {
			fatal(&format!(
				"Failed to create class file '{}': {e}",
				path.to_str().unwrap_or("")
			))
		});
		context.write_class(&mut file, &ref_class).unwrap();
	}

	create_dir_all(&context.ctx.abs_structs_path)
		.unwrap_or_else(|e| fatal(&format!("Unable to create structs folder: {e}")));
	for ref_struct in structs {
		let path = context.ctx.abs_struct_path(&ref_struct.base.internal_name);
		let mut file = File::create(&path).unwrap_or_else(|e| {
			fatal(&format!(
				"Failed to create struct file '{}': {e}",
				path.to_str().unwrap_or("")
			))
		});
		context.write_struct(&mut file, &ref_struct).unwrap();
	}
}

fn write_lua_files(context: &Context, doc: &LuaDocumentation) {
	remove_files_in_folder(&context.abs_lua_path);

	let context = LuaContext::new(context);

	for module in &doc.modules {
		let path = context.ctx.abs_lua_module_path(&module.base.internal_name);
		let mut file = File::create(&path).unwrap_or_else(|e| {
			fatal(&format!(
				"Failed to create lua module file '{}': {e}",
				path.to_str().unwrap_or("")
			))
		});
		context.write_module(&mut file, module).unwrap();
	}
}

fn write_doc_files_to_nav<'a, 'b, T: Iterator<Item = (&'a String, &'b String)>>(
	nav_content: &mut Vec<u8>,
	prefix: &str,
	files: T,
) {
	for (title, path) in files {
		writeln!(nav_content, "{prefix} xref:{path}[{title}]").unwrap();
	}
}

fn update_navigation_file(context: &Context) {
	let file = File::open(context.config.abs_nav_file_path())
		.unwrap_or_else(|e| fatal(&format!("Unable to read navigation file: {e}")));
	let file_buf = BufReader::new(file);

	let regex = Regex::new(
		r"^\s*//\s+FIN((Reflection)|(Lua))Documentation((Begin\s+(\*+))|(End))\s+//\s*$",
	)
	.unwrap();

	let mut reflection_prefix: Option<String> = None;
	let mut lua_prefix: Option<String> = None;
	let mut new_contents = Vec::new();
	for line in file_buf.lines() {
		let line = if let Ok(line) = line {
			line
		} else {
			continue;
		};

		if let Some(captures) = regex.captures(&line) {
			if captures.get(2).is_some() {
				// a reflection begin/end
				if let Some(capture) = captures.get(6) {
					reflection_prefix = Some(capture.as_str().to_string());
					writeln!(&mut new_contents, "{line}").unwrap();
				} else if let Some(prefix) = reflection_prefix {
					writeln!(&mut new_contents, "{prefix} Classes").unwrap();
					let mut classes: Vec<_> = context.class_pages.iter().collect();
					classes.sort_by(|(c1, _), (c2, _)| c1.cmp(c2));
					write_doc_files_to_nav(
						&mut new_contents,
						&(prefix.clone() + "*"),
						classes.iter().cloned(),
					);
					let mut structs: Vec<_> = context.struct_pages.iter().collect();
					structs.sort_by(|(s1, _), (s2, _)| s1.cmp(s2));
					writeln!(&mut new_contents, "{prefix} Structs").unwrap();
					write_doc_files_to_nav(
						&mut new_contents,
						&(prefix + "*"),
						structs.iter().cloned(),
					);
					reflection_prefix = None;
				}
			} else if captures.get(3).is_some() {
				if let Some(capture) = captures.get(6) {
					lua_prefix = Some(capture.as_str().to_string());
					writeln!(&mut new_contents, "{line}").unwrap();
				} else if let Some(prefix) = lua_prefix {
					let mut lua_pages: Vec<_> = context.lua_pages.iter().collect();
					lua_pages.sort_by(|(p1, _), (p2, _)| p1.cmp(p2));
					write_doc_files_to_nav(&mut new_contents, &prefix, lua_pages.iter().cloned());
					lua_prefix = None;
				}
			}
		};

		if reflection_prefix.is_none() && lua_prefix.is_none() {
			writeln!(&mut new_contents, "{line}").unwrap();
		}
	}

	std::fs::write(context.config.abs_nav_file_path(), new_contents)
		.unwrap_or_else(|e| fatal(&format!("Unable to write changes to navigation file: {e}")));
}

fn exec_cmd(input: &Path, config: Config) {
	let input = std::path::absolute(input).unwrap();

	let doc = Documentation::load(&input)
		.unwrap_or_else(|e| fatal(&format!("Unable to load documentation: {e}")));

	let context = Context::new(config, &doc);

	write_reflection_files(&context, &doc.reflection);
	write_lua_files(&context, &doc.lua);
	update_navigation_file(&context);
}

fn main() {
	let args: Vec<String> = std::env::args().collect();
	let program = args[0].clone();

	let mut opts = getopts::Options::new();
	opts.optflag("h", "help", "print this help menu");
	opts.optopt("i", "config", "set the path to the config file", "NAME");
	let matches = match opts.parse(&args[1..]) {
		Ok(m) => m,
		Err(f) => panic!("{}", f.to_string()),
	};
	if matches.opt_present("h") {
		let brief = format!("Usage: {} FILE [options]", program);
		print!("{}", opts.usage(&brief));
		return;
	}
	let input = matches
		.free
		.get(0)
		.unwrap_or_else(|| fatal("No documentation input JSON file provided!"));
	let config_path = PathBuf::from(
		matches
			.opt_str("i")
			.unwrap_or("FINDocumentation.toml".to_string()),
	);

	let config = Config::load(&config_path)
		.unwrap_or_else(|e| fatal(&format!("Failed ot load config file: {e}")));

	exec_cmd(Path::new(&input), config);
}
