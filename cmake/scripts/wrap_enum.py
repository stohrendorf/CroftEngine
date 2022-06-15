import argparse
from pathlib import Path
import re
from typing import Iterable, Tuple, List


def read_enum_txt(name: Path) -> Iterable[Tuple[str, str]]:
    for line in name.read_text().splitlines():
        if re.fullmatch(r"(\s*//.+)?", line):
            pass
        matches = re.fullmatch(r"(.+?)\s*=\s*(.+?)(\s*//.+)?", line)
        if matches:
            yield matches.group(1), matches.group(2)
            continue

        matches = re.fullmatch(r"(.+?)(\s*//.+)?", line)
        if matches:
            yield matches.group(1), None


def generate_header_lines(args, enums: List[Tuple[str, str]]) -> Iterable[str]:
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield "#include <gsl/gsl-lite.hpp>"
    yield "#include <map>"
    yield "#include <string>"
    for include in args.with_includes:
        yield f"#include {include}"
    yield ""

    yield f"namespace {args.namespace}"
    yield "{"
    yield f"enum class {args.name} : {args.type}"
    yield "{"
    for identifier, value in enums:
        if value is not None:
            yield f"    {identifier} = {value},"
        else:
            yield f"    {identifier},"
    yield "};"

    yield ""
    yield f"extern gsl::czstring toString({args.name} val);"

    yield ""
    yield "template<typename T> struct EnumUtil;"

    yield ""
    yield f"template<> struct EnumUtil<{args.name}>"
    yield "{"
    yield f"    static {args.name} fromString(const std::string& value);"
    yield f"    static std::map<{args.name}, std::string> all();"
    yield "    static const char* name()"
    yield "    {"
    yield f"        return \"{args.name}\";"
    yield "    }"

    yield "};"

    yield f"}} // namespace {args.namespace}"


def generate_impl_lines(args, enums: List[Tuple[str, str]], include_path: Path) -> Iterable[str]:
    yield f"#include \"{include_path.as_posix()}\""
    yield ""
    yield "#include <cstdint>"
    yield "#include <gsl/gsl-lite.hpp>"
    yield "#include <map>"
    yield "#include <stdexcept>"
    yield "#include <string>"

    yield f"namespace {args.namespace}"
    yield "{"

    yield f"gsl::czstring toString({args.name} val)"
    yield "{"
    yield "    switch(val)"
    yield "    {"
    for identifier, value in enums:
        yield f"        case {args.name}::{identifier}: return \"{identifier}\";"
    yield "        default: return nullptr;"
    yield "    }"
    yield "}"

    yield f"{args.name} EnumUtil<{args.name}>::fromString(const std::string& value)"
    yield "{"
    for identifier, value in enums:
        yield f"    if(value == \"{identifier}\") return {args.name}::{identifier};"
    yield f"    throw std::domain_error(\"Value '\" + value + \"' invalid for enum '{args.name}'\");"
    yield "}"

    yield ""
    yield f"std::map<{args.name}, std::string> EnumUtil<{args.name}>::all()"
    yield "{"
    yield "    return {"
    for identifier, value in enums:
        yield f"        {{{args.name}::{identifier}, \"{identifier}\"}},"
    yield "    };"
    yield "}"
    yield f"}} // namespace {args.namespace}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', type=str)
    parser.add_argument('--namespace', type=str)
    parser.add_argument('--type', type=str)
    parser.add_argument('--source', type=str)
    parser.add_argument('--dest', type=str)
    parser.add_argument('--dest-impl', type=str)
    parser.add_argument('--with-includes', type=str, nargs="*")

    args = parser.parse_args()

    enums = list(read_enum_txt(Path(args.source)))
    Path(args.dest).write_text("\n".join(generate_header_lines(args, enums)))
    Path(args.dest_impl).write_text(
        "\n".join(generate_impl_lines(args, enums, Path(args.dest).relative_to(Path(args.dest_impl).parent))))


if __name__ == '__main__':
    main()
