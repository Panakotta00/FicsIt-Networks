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
    return "<<Class-" + redirect_class(clazz) + "," + clazz + ">>"

def ref_struct(struct):
    return "<<Struct-" + redirect_struct(struct) + "," + struct + ">>"


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


def write_property(f, prop):
    f.write(
        "===== _" + format_data_type(prop["type"]) + "_ *" + prop["displayName"] + "* `" + prop["internalName"] + "`\n")
    write_description(f, prop)
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


def write_parameters(f, list, prefix):
    f.write(prefix + '::\n+\n[%header,cols="1,1,4a"]\n|===\n|Name |Type |Description\n\n')
    for param in list:
        f.write("| *" + param["displayName"] + "* `" + param["internalName"] + "`\n")
        f.write("| " + format_data_type(param["type"]) + "\n")
        f.write("| ")
        write_description(f, param)
    f.write("|===\n\n")


def write_function(f, func):
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
    write_description(f, func)

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

    if len(in_params) > 0:
        write_parameters(f, in_params, "Parameters")

    if len(out_params) > 0:
        write_parameters(f, out_params, "Return Values")


def write_signal(f, signal):
    f.write("===== *" + signal["displayName"] + "* `" + signal["internalName"] + "` (")
    firstparam = True
    for param in signal["parameters"]:
        if not firstparam:
            f.write(", ")
        firstparam = False
        f.write("_" + format_data_type(param["type"]) + "_ *" + param["displayName"] + "* `" + param["internalName"] + "`")
    f.write(")\n")
    write_description(f, signal)

    f.write('[cols = "1,5a"]\n|===\n')
    f.write('| Flags\n')
    f.write('| +++')
    if signal["isVarArgs"]:
        f.write("<span style='color:#e59445'><i>VarArgs</i></span> ")
    f.write('\n+++\n|===\n\n')

    if len(signal["parameters"]) > 0:
        write_parameters(f, signal["parameters"], "Parameters")


def write_class(f, clazz):
    f.write("[#Class-" + clazz["internalName"] + "]\n")
    f.write("=== *" + clazz["displayName"] + "* `" + clazz["internalName"] + "`\n")
    if "parent" in clazz and len(clazz["parent"]) > 0:
        f.write('[cols = "1,5a"]\n|===\n')
        f.write('| Parent\n')
        f.write('| ' + ref_class(clazz["parent"]) + '\n')
        f.write('|===\n\n')
    write_description(f, clazz)
    if len(clazz["properties"]) > 0:
        f.write("==== Properties\n")
        for prop in clazz["properties"]:
            write_property(f, prop)
    if len(clazz["functions"]) > 0:
        f.write("==== Functions\n")
        for func in clazz["functions"]:
            write_function(f, func)
    if len(clazz["signals"]) > 0:
        f.write("==== Signals\n")
        for signal in clazz["signals"]:
            write_signal(f, signal)


def write_struct(f, struct):
    f.write("[#Struct-" + struct["internalName"] + "]\n")
    f.write("=== *" + struct["displayName"] + "* `" + struct["internalName"] + "`\n")
    if "parent" in struct and len(struct["parent"]) > 0:
        f.write('[cols = "1,5a"]\n|===\n')
        f.write('| Parent\n')
        f.write('| ' + ref_struct(struct["parent"]) + '\n')
        f.write('|===\n\n')
    write_description(f, struct)
    if len(struct["properties"]) > 0:
        f.write("==== Properties\n")
        for prop in struct["properties"]:
            write_property(f, prop)
    if len(struct["functions"]) > 0:
        f.write("==== Functions\n")
        for func in struct["functions"]:
            write_function(f, func)


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
