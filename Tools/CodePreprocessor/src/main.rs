use walkdir::WalkDir;
use simple_error::SimpleError;
use clang::*;

fn main() -> Result<(), SimpleError> {
    let clang = Clang::new().unwrap();

    // Create a new `Index`
    let index = Index::new(&clang, false, false);

    for entry in WalkDir::new(".\\FicsItNetworksLua\\Private\\FINLua\\API")
        .follow_links(true)
        .into_iter()
        .filter_map(|e| e.ok()) {
        let f_name = entry.file_name().to_string_lossy();

        if f_name.ends_with(".cpp") || f_name.ends_with(".h") {
            println!("{}", f_name);
            let tu = index.parser(entry.path()).arguments(&["-x", "c++", "-CC", "-I", ".\\FicsItNetworksLua\\Public"]).detailed_preprocessing_record(true).parse().unwrap();

           /* let field_functions = tu.get_entity().get_children().into_iter().filter(|e| {
                e.get_kind() == EntityKind::Namespace && e.get_name() == Some("FINLua"),
                e.get_location().map(|l| l.get_file_location().file).flatten().map(|f| f.get_path().to_str().map(|s| s.to_string())).flatten().unwrap_or("".to_string()) == ".\\FicsItNetworksLua\\Private\\FINLua\\API\\LuaComponentAPI.cpp"
            }).map(|e| e.get_children()).flatten().filter(|e| {
                true //e.get_name() == Some("FieldFunction".to_string())
            }).map(|e| e.get_children()).flatten().collect::<Vec<_>>();
            for macro_expansion in macro_expansion {
                //println!("{:?}", macro_expansion.get_location().map(|l| l.get_file_location().file).flatten().map(|f| f.get_path().to_str().map(|s| s.to_string())).flatten().unwrap_or("".to_string()));
                println!("{:?}: {:?}", macro_expansion.get_name(), macro_expansion.get_kind());
            }*/

            tu.get_entity().visit_children(|e, _| {
                if e.get_kind() == EntityKind::MacroExpansion && e.get_name() == Some("FieldFunction".to_string()) {
                    //println!("{:?}", e);
                    //println!("{:?}", e.get_comment());
                    EntityVisitResult::Continue
                } else {
                    EntityVisitResult::Recurse
                }
            });
        }
    }

    let tu = index.parser(".\\FicsItNetworksLua\\Private\\FINLua\\API\\LuaComponentAPI.cpp").arguments(&["-x", "c++", "-E", "-CC", "-I", ".\\FicsItNetworksLua\\Public"]).detailed_preprocessing_record(true).parse().unwrap();
    tu.get_entity().

    tu.get_entity().visit_children(|e, _| {
        if let Some(comment) = e.get_comment() {
            println!("{}", comment);
        }
        EntityVisitResult::Recurse
    });

    /*let entities = tu.get_entity().get_children().into_iter()
        .map(|e| e.get_children()).flatten()
        .collect::<Vec<_>>();
    for entity in entities {
        println!("{:?}", entity);
        println!("Print: '{}'", entity.get_pretty_printer().print());
        //println!("{:?}", entity.get_location().map(|l| l.get_file_location().file).flatten().map(|f| f.get_path().to_str().map(|s| s.to_string())).flatten().unwrap_or("".to_string()));
        //println!("{:?}: {:?}", entity.get_name(), entity.get_kind());
    }*/

    Ok(())
}
