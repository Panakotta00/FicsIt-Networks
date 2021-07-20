from pathlib import Path
import json
import os
import re
import io

class_redirect_table = {}
struct_redirect_table = {}

def redirect_class(clazz):
    while clazz in class_redirect_table:
        clazz = class_redirect_table[clazz]
    return clazz


def redirect_struct(struct):
    while struct in struct_redirect_table:
        struct = struct_redirect_table[struct]
    return struct


def ref_class(clazz):
    return "xref::Reflection.adoc#Class-" + redirect_class(clazz) + "[" + clazz + "]"

def ref_struct(struct):
    return "xref::Reflection.adoc#Struct-" + redirect_struct(struct) + "[" + struct + "]"


def get_adoc_files():
    result = Path(".").rglob("*.adoc")
    return result


def load_reflection_doc(path=""):
    if len(path) < 1:
        path = os.path.join(os.getenv('LOCALAPPDATA'), "FactoryGame/Saved/FINReflectionDocumentation.json")
    f = open(path, "r", encoding='utf16')
    refdoc = json.loads(f.read())
    f.close()
    return refdoc


def format_data_type(type):
    type_str = type["type"]
    inner = ""
    if "inner" in type:
        inner = type["inner"]
        if isinstance(inner, str):
            inner = ref_struct(inner)
    elif "subclass" in type:
        inner = ref_class(type["subclass"])
    if isinstance(inner, str):
        if len(inner) > 0:
            type_str = type_str + "(" + inner + ")"
    else:
        type_str = type_str + "(" + format_data_type(inner) + ")"
    return type_str


def write_description(f, obj):
    desc = obj["description"]
    desc = desc.replace("\n", " +\n")
    desc = re.sub(r"\n[^\S\n]* \+\n", "\n\n", desc)
    f.write(desc + "\n\n")


def write_property(f, prop, parent):
    section_id = parent
    if prop["Flag_ClassProp"]:
        section_id = section_id + "_Class"
    section_id = section_id + "-props-" + prop["internalName"]
    f.write("[#" + section_id + "]\n")
    f.write("// tag::" + section_id + "__title[]\n")
    f.write("===== _" + format_data_type(prop["type"]) + "_ *" + prop["displayName"] + "* `" + prop["internalName"] + "`\n")
    f.write("// tag::" + section_id + "[]\n")
    f.write("// tag::" + section_id + "__description[]\n")
    write_description(f, prop)
    f.write("// end::" + section_id + "__description[]\n")
    f.write("// tag::" + section_id + "__flags[]\n")
    f.write('[cols = "1,5a"]\n|===\n')
    f.write('| Flags\n')
    f.write('| +++')
    if prop["Flag_ClassProp"]:
        f.write("<span style='color:#5dafc5'><i>ClassProp</i></span> ")
    if prop["Flag_RT_Sync"]:
        f.write("<span style='color:#bb2828'><i>RuntimeSync</i></span> ")
    if prop["Flag_RT_Parallel"]:
        f.write("<span style='color:#bb2828'><i>RuntimeParallel</i></span> ")
    if prop["Flag_RT_Async"]:
        f.write("<span style='color:#bb2828'><i>RuntimeAsync</i></span> ")
    if prop["Flag_ReadOnly"]:
        f.write("<span style='color:#e59445'><i>ReadOnly</i></span> ")
    f.write('\n+++\n|===\n\n')
    f.write("// end::" + section_id + "__flags[]\n")
    f.write("// end::" + section_id + "[]\n")
    f.write("// end::" + section_id + "__title[]\n")


def write_parameters(f, list, prefix):
    f.write(prefix + '::\n+\n[%header,cols="1,1,4a"]\n|===\n|Name |Type |Description\n\n')
    for param in list:
        f.write("| *" + param["displayName"] + "* `" + param["internalName"] + "`\n")
        f.write("| " + format_data_type(param["type"]))
        if param["Flag_OutParam"] or param["Flag_RetVal"]:
            f.write(" _out_")
        f.write("\n")
        f.write("| ")
        write_description(f, param)
    f.write("|===\n\n")


def write_function(f, func, parent):
    section_id = parent
    if func["Flag_ClassFunc"]:
        section_id = section_id + "_Class"
    section_id = section_id + "-Funcs-" + func["internalName"]
    f.write("[#" + section_id + "]\n")
    f.write("// tag::" + section_id + "__title[]\n")
    f.write("===== *" + func["displayName"] + "* `" + func["internalName"] + "` (")
    in_params = []
    out_params = []
    firstparam = True
    for param in func["parameters"]:
        if not firstparam:
            f.write(", ")
        firstparam = False
        f.write("_" + format_data_type(param["type"]) + "_ *" + param["displayName"] + "* `" + param["internalName"] + "`")
        if param["Flag_OutParam"] or param["Flag_RetVal"]:
            f.write(" _out_")
            out_params.append(param)
        else:
            in_params.append(param)
    f.write(")\n")
    f.write("// tag::" + section_id + "[]\n")
    f.write("// tag::" + section_id + "__description[]\n")
    write_description(f, func)
    f.write("// end::" + section_id + "__description[]\n")

    f.write("// tag::" + section_id + "__flags[]\n")
    f.write('[cols = "1,5a"]\n|===\n')
    f.write('| Flags\n')
    f.write('| +++')
    if func["Flag_ClassFunc"]:
        f.write("<span style='color:#5dafc5'><i>ClassFunc</i></span> ")
    if func["Flag_StaticFunc"]:
        f.write("<span style='color:#5dafc5'><i>StaticFunc</i></span> ")
    if func["Flag_RT_Sync"]:
        f.write("<span style='color:#bb2828'><i>RuntimeSync</i></span> ")
    if func["Flag_RT_Parallel"]:
        f.write("<span style='color:#bb2828'><i>RuntimeParallel</i></span> ")
    if func["Flag_RT_Async"]:
        f.write("<span style='color:#bb2828'><i>RuntimeAsync</i></span> ")
    if func["Flag_VarArgs"]:
        f.write("<span style='color:#e59445'><i>VarArgs</i></span> ")
    if func["Flag_VarRets"]:
        f.write("<span style='color:#e59445'><i>VarReturns</i></span> ")
    f.write('\n+++\n|===\n\n')
    f.write("// end::" + section_id + "__flags[]\n")

    if len(in_params) > 0:
        f.write("// tag::" + section_id + "__params[]\n")
        write_parameters(f, in_params, "Parameters")
        f.write("// end::" + section_id + "__params[]\n")

    if len(out_params) > 0:
        f.write("// tag::" + section_id + "__retvals[]\n")
        write_parameters(f, out_params, "Return Values")
        f.write("// end::" + section_id + "__retvals[]\n")

    f.write("// end::" + section_id + "[]\n")
    f.write("// end::" + section_id + "__title[]\n")


def write_signal(f, signal, parent):
    section_id = parent + "-Signals-" + signal["internalName"]
    f.write("// tag::" + section_id + "__title[]\n")
    f.write("[#" + section_id + "]\n")
    f.write("===== *" + signal["displayName"] + "* `" + signal["internalName"] + "` (")
    firstparam = True
    for param in signal["parameters"]:
        if not firstparam:
            f.write(", ")
        firstparam = False
        f.write("_" + format_data_type(param["type"]) + "_ *" + param["displayName"] + "* `" + param["internalName"] + "`")
    f.write(")\n")
    f.write("// tag::" + section_id + "[]\n")
    f.write("// tag::" + section_id + "__description[]\n")
    write_description(f, signal)
    f.write("// end::" + section_id + "__description[]\n")

    f.write("// tag::" + section_id + "__flags[]\n")
    f.write('[cols = "1,5a"]\n|===\n')
    f.write('| Flags\n')
    f.write('| +++')
    if signal["isVarArgs"]:
        f.write("<span style='color:#e59445'><i>VarArgs</i></span> ")
    f.write('\n+++\n|===\n\n')
    f.write("// end::" + section_id + "__flags[]\n")

    if len(signal["parameters"]) > 0:
        f.write("// tag::" + section_id + "__params[]\n")
        write_parameters(f, signal["parameters"], "Parameters")
        f.write("// end::" + section_id + "__params[]\n")

    f.write("// end::" + section_id + "[]\n")
    f.write("// end::" + section_id + "__title[]\n")


def write_class(f, clazz):
    section_id = "Class-" + clazz["internalName"]
    f.write("// tag::" + section_id + "__title[]\n")
    f.write("[#" + section_id + "]\n")
    f.write("=== *" + clazz["displayName"] + "* `" + clazz["internalName"] + "`\n")
    f.write("// tag::" + section_id + "[]\n")
    f.write("// tag::" + section_id + "__parent[]\n")
    if "parent" in clazz and len(clazz["parent"]) > 0:
        f.write('[cols = "1,5a"]\n|===\n')
        f.write('| Parent\n')
        f.write('| ' + ref_class(clazz["parent"]) + '\n')
        f.write('|===\n\n')
    f.write("// end::" + section_id + "__parent[]\n")
    f.write("// tag::" + section_id + "__description[]\n")
    write_description(f, clazz)
    f.write("// end::" + section_id + "__description[]\n")
    if len(clazz["properties"]) > 0:
        f.write("// tag::" + section_id + "-Props__title[]\n")
        f.write("[#" + section_id + "-Props]\n")
        f.write("==== Properties\n")
        f.write("// tag::" + section_id + "-Props[]\n")
        for prop in clazz["properties"]:
            write_property(f, prop, section_id)
        f.write("// end::" + section_id + "-Props[]\n")
        f.write("// end::" + section_id + "-Props__title[]\n")
    if len(clazz["functions"]) > 0:
        f.write("// tag::" + section_id + "-Funcs__title[]\n")
        f.write("[#" + section_id + "-Funcs]\n")
        f.write("==== Functions\n")
        f.write("// tag::" + section_id + "-Funcs[]\n")
        for func in clazz["functions"]:
            write_function(f, func, section_id)
        f.write("// end::" + section_id + "-Funcs[]\n")
        f.write("// end::" + section_id + "-Funcs__title[]\n")
    if len(clazz["signals"]) > 0:
        f.write("// tag::" + section_id + "-Signals__title[]\n")
        f.write("[#" + section_id + "-Signals]\n")
        f.write("==== Signals\n")
        f.write("// tag::" + section_id + "-Signals[]\n")
        for signal in clazz["signals"]:
            write_signal(f, signal, section_id)
        f.write("// end::" + section_id + "-Signals[]\n")
        f.write("// end::" + section_id + "-Signals__title[]\n")
    f.write("// end::" + section_id + "[]\n")
    f.write("// end::" + section_id + "__title[]\n")


def write_struct(f, struct):
    section_id = "Struct-" + struct["internalName"]
    f.write("// tag::" + section_id + "__title[]\n")
    f.write("[#" + section_id + "]\n")
    f.write("=== *" + struct["displayName"] + "* `" + struct["internalName"] + "`\n")
    f.write("// tag::" + section_id + "[]\n")
    if "parent" in struct and len(struct["parent"]) > 0:
        f.write("// tag::" + section_id + "__parent[]\n")
        f.write('[cols = "1,5a"]\n|===\n')
        f.write('| Parent\n')
        f.write('| ' + ref_struct(struct["parent"]) + '\n')
        f.write('|===\n\n')
        f.write("// end::" + section_id + "__parent[]\n")
    f.write("// tag::" + section_id + "__description[]\n")
    write_description(f, struct)
    f.write("// end::" + section_id + "__description[]\n")
    if len(struct["properties"]) > 0:
        f.write("// tag::" + section_id + "__Prop[]\n")
        f.write("[#" + section_id + "-Props]\n")
        f.write("==== Properties\n")
        for prop in struct["properties"]:
            write_property(f, prop, section_id)
        f.write("// end::" + section_id + "__Props[]\n")
    if len(struct["functions"]) > 0:
        f.write("// tag::" + section_id + "__Funcs[]\n")
        f.write("[#" + section_id + "-Funcs]\n")
        f.write("==== Functions\n")
        for func in struct["functions"]:
            write_function(f, func, section_id)
        f.write("// end::" + section_id + "__Funcs[]\n")
    f.write("// end::" + section_id + "[]\n")
    f.write("// end::" + section_id + "__title[]\n")


def write_full_doc(f):
    doc = load_reflection_doc()
    f.write("== Classes\n")
    for clazz in doc["classes"]:
        if len(clazz["properties"]) > 0 or len(clazz["functions"]) > 0 or len(clazz["signals"]) > 0:
            write_class(f, clazz)
        else:
            class_redirect_table[clazz["internalName"]] = clazz["parent"]
    f.write("== Structs\n")
    for struct in doc["structs"]:
        if len(struct["properties"]) > 0 or len(struct["functions"]) > 0:
            write_struct(f, struct)
        elif "parent" in struct and len(struct["parent"]) > 0:
            struct_redirect_table[struct["internalName"]] = struct["parent"]


def is_line_block_start(line):
    return len(re.findall(r"^\s*//\s*#FINReflectionDocumentationBlock Begin\s*//\s*$", line)) > 0


def is_line_block_end(line):
    return len(re.findall(r"^\s*//\s*#FINReflectionDocumentationBlock End\s*//\s*$", line)) > 0


def read_block(file):
    while True:
        line = file.readline()
        if len(line) < 1:
            return
        if is_line_block_end(line):
            return


def get_block_header(line):
    matches = re.findall(r"^\s*//\s*#\s*FINReflectionDocumentation\s*//\s*$", line)
    return len(matches) > 0


def write_block_start(file):
    file.write("// #FINReflectionDocumentationBlock Begin //\n")


def write_block_end(file):
    file.write("// #FINReflectionDocumentationBlock End //\n")


def process_adoc_file(file):
    with open(file, "r") as f:
        contents = f.read()
    with io.StringIO(contents) as data:
        with io.StringIO() as f:
            was_header = False
            while True:
                line = data.readline()
                if was_header:
                    was_header = False
                    if is_line_block_start(line):
                        read_block(data)
                        continue
                f.write(line)
                if len(line) < 1:
                    break
                if get_block_header(line):
                    was_header = True
                    write_block_start(f)
                    write_full_doc(f)
                    write_block_end(f)
            with open(file, "wt", encoding="utf8") as out:
                out.write(f.getvalue())


def process_adoc_files():
    files = get_adoc_files()
    for file in files:
        process_adoc_file(file)


process_adoc_files()
