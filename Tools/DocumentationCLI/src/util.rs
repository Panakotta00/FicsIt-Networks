use std::ffi::OsStr;
use std::path::Path;

pub fn prefix_string_lines(prefix: &str, string: &str) -> String {
	string
		.lines()
		.intersperse(&("\n".to_string() + prefix))
		.collect()
}

pub fn path_to_forward_slash(path: &Path) -> Option<String> {
	path.iter()
		.intersperse(OsStr::new("/"))
		.map(|s| s.to_str())
		.collect()
}
