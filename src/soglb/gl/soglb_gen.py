import logging
import os
import re
import urllib.request
from collections import defaultdict
from copy import deepcopy
from dataclasses import dataclass, field
from typing import Dict, Set, Iterable
from typing import Optional
from typing import TextIO
from typing import Tuple
from xml.etree import ElementTree
from xml.etree.ElementTree import Element

logging.basicConfig(level=logging.INFO)

API_VERSION_PREFIX = 'API_LEVEL'
VENDOR_PREFIX = 'WITH_GL_API_VENDOR'
EXTENSION_PREFIX = 'WITH_API_EXTENSION'
ENABLED_EXTENSIONS = (
    'GL_ARB_bindless_texture',
    'GL_ARB_texture_filter_anisotropic',
    'GL_EXT_texture_filter_anisotropic',
    'GL_ARB_sync',
)
ENABLED_APIS = ('gl',)

XML_NAME = os.path.join("api", "gl.xml")

API_LEVEL_FILTER = "API_LEVEL_GL_VERSION_4_5_core"


def strip_ext_suffix(name: str) -> str:
    strip_suffixes = ("ARB",)
    for suffix in strip_suffixes:
        if name.endswith(suffix):
            return name[:-len(suffix)]
    return name


def normalize_constant_name(name: str) -> str:
    if name.startswith('GL_'):
        name = name[3:]
    name = strip_ext_suffix(name).rstrip("_")
    name = ''.join([x[:1].upper() + x[1:].lower() for x in name.split('_')])
    if name[:1].isnumeric():
        name = '_' + name
    return name


def normalize_fn_name(name: str) -> str:
    if name.startswith('gl'):
        name = name[2:]
    name = strip_ext_suffix(name[:1].lower() + name[1:])

    if not any(name.lower().endswith(x)
               for x in
               ('buffers', 'elements', 'shaders', 'textures', 'status', 'arrays', 'attrib', 'instanced', 'enabled',
                'queries', 'indexed', 'indexedv')):
        # remove type specs
        if not name.startswith('getQueryBufferObject'):
            name = re.sub(r'([1-9]?)(u?(b|s|i|i64)|f|d)(v?)$', r'\1', name)
    if any(name.lower().endswith(s) for s in
           ('booleanv', 'doublev', 'floatv', 'integerv', 'integer64v', 'pointerv', 'createshaderprogramv', 'arrayv',
            'indexedv')):
        name = name[:-1]
    if name.lower().endswith("i_v"):
        name = name[:-3] + "I"
    elif name.lower().endswith("i64_v"):
        name = name[:-5] + "I"
    if name[:1].isnumeric():
        name = '_' + name
    return name


def _patch_integral_types(*, xml_ptype: Element, enum_name: Optional[str]) -> Optional[str]:
    if xml_ptype is None:
        return None
    orig = xml_ptype.text.strip()
    if orig == 'GLbitfield':
        if enum_name is not None:
            xml_ptype.text = 'core::Bitfield<{}>'.format(strip_ext_suffix(enum_name))
        else:
            xml_ptype.text = 'uint32_t'
            orig = 'uint32_t'
    elif orig == 'GLboolean':
        xml_ptype.text = 'bool'
    elif orig == 'GLubyte':
        xml_ptype.text = 'uint8_t'
    elif orig == 'GLushort':
        xml_ptype.text = 'uint16_t'
    elif orig == 'GLuint':
        xml_ptype.text = 'uint32_t'
    elif orig == 'GLuint64':
        xml_ptype.text = 'uint64_t'
    elif orig == 'GLuint64EXT':
        xml_ptype.text = 'uint64_t'
    elif orig == 'GLbyte':
        xml_ptype.text = 'int8_t'
    elif orig == 'GLshort':
        xml_ptype.text = 'int16_t'
    elif orig == 'GLint':
        xml_ptype.text = 'int32_t'
    elif orig == 'GLint64':
        xml_ptype.text = 'int64_t'
    elif orig == 'GLint64EXT':
        xml_ptype.text = 'int64_t'
    elif orig == 'GLfloat':
        xml_ptype.text = 'float'
    elif orig == 'GLdouble':
        xml_ptype.text = 'double'
    elif orig == 'GLvoid':
        xml_ptype.text = 'void'
    elif orig in ('GLchar', 'GLcharARB'):
        xml_ptype.text = 'char'
    elif orig == 'GLenum':
        xml_ptype.text = strip_ext_suffix(enum_name) if enum_name else 'core::EnumType'
    elif orig == 'GLsizei':
        xml_ptype.text = 'core::SizeType'
    elif orig in 'GLfixed':
        xml_ptype.text = 'core::Fixed'
    elif orig in ('GLhalf', 'GLhalfNV'):
        xml_ptype.text = 'core::Half'
    elif orig in ('GLintptr', 'GLintptrARB'):
        xml_ptype.text = 'std::intptr_t'
    elif orig in ('GLsizeiptr', 'GLsizeiptrARB'):
        xml_ptype.text = 'std::size_t'
    elif orig == 'GLclampf':
        xml_ptype.text = 'core::ClampedFloat'
    elif orig == 'GLclampd':
        xml_ptype.text = 'core::ClampedDouble'
    elif orig == 'GLhandleARB':
        xml_ptype.text = 'core::Handle'
    elif orig == 'GLsync':
        xml_ptype.text = 'core::Sync'
    elif orig in ('GLDEBUGPROC', 'GLDEBUGPROCARB', 'GLDEBUGPROCKHR', 'GLDEBUGPROCAMD'):
        xml_ptype.text = 'core::DebugProc'
        return None
    else:
        return None
    return orig


class Command:
    def __init__(self, *, xml_command: Element):
        self.comment = xml_command.attrib.get('comment', None)
        self.aliases = set([e.attrib['name'] for e in xml_command.findall('alias')])
        self.proto: Element = deepcopy(xml_command.find('proto'))
        self.raw_name = self.proto.findtext('name')

        # XXX PATCH
        if self.raw_name in ('glTexImage2D', 'glTexImage3D'):
            xml_command.find('./param[@group="InternalFormat"]/ptype').text = 'GLenum'
        elif self.raw_name in ('glNamedBufferData',):
            for param in xml_command.findall('./param[@group="VertexBufferObjectUsage"]'):
                param.attrib['group'] = 'BufferUsageARB'

        self.orig_return_type = _patch_integral_types(xml_ptype=self.proto.find('ptype'),
                                                      enum_name=self.proto.attrib.get('group'))
        self.params = []
        self.call_params = []
        for param in map(deepcopy, xml_command.findall('param')):  # type: Element
            name = param.findtext('name')
            param_copy = deepcopy(param)
            orig_type = _patch_integral_types(xml_ptype=param.find('ptype'), enum_name=param.attrib.get('group'))

            param_copy.remove(param_copy.find('name'))
            cast_target = ''.join(param_copy.itertext()).strip()
            if orig_type is not None:
                if orig_type != 'GLbitfield':
                    if '*' in cast_target:
                        self.call_params.append('detail::constAway(reinterpret_cast<{}>({}))'.format(cast_target, name))
                    else:
                        self.call_params.append('static_cast<{}>({})'.format(cast_target, name))
                else:
                    self.call_params.append('{}.value()'.format(name))
            else:
                if '*' in cast_target:
                    self.call_params.append('detail::constAway({})'.format(name))
                elif cast_target == 'GLDEBUGPROC':
                    self.call_params.append('reinterpret_cast<GLDEBUGPROC>({})'.format(name))
                else:
                    self.call_params.append(name)

            self.params.append(''.join(param.itertext()))

    def print_code(self, *, file: TextIO, impl: bool = False):
        if self.comment:
            file.write('// {}\n'.format(self.comment))

        proto_copy = deepcopy(self.proto)
        proto_copy.find('name').text = normalize_fn_name(proto_copy.findtext('name'))

        proto = ''.join(proto_copy.itertext())
        proto_copy = deepcopy(proto_copy)
        proto_copy.remove(proto_copy.find('name'))
        return_cast = ''.join(proto_copy.itertext())

        if not impl:
            file.write('extern {}({});\n'.format(proto, ', '.join(self.params)))
        else:
            file.write('{}({})\n'.format(proto, ', '.join(self.params)))
            file.write('{\n')
            if self.orig_return_type is None:
                file.write(
                    '    return {}({});\n'.format(self.proto.find('name').text, ', '.join(self.call_params)))
            else:
                file.write(
                    '    return static_cast<{}>({}({}));\n'.format(return_cast,
                                                                   self.proto.find('name').text,
                                                                   ', '.join(self.call_params)))
            file.write('}\n')


@dataclass
class ConstantsCommands:
    constants: Set[str] = field(default_factory=set)
    commands: Set[str] = field(default_factory=set)


@dataclass
class Enum:
    name: str
    constants: Set[Tuple[str, str]] = field(default_factory=set)
    is_bitmask: bool = False


def _make_version_macro(version: str, profile_name: Optional[str]) -> str:
    if profile_name is None:
        return '{}_{}'.format(API_VERSION_PREFIX, version)
    else:
        return '{}_{}_{}'.format(API_VERSION_PREFIX, version, profile_name)


def _make_guard(defines: Iterable[str]) -> str:
    return ' || '.join(['defined({})'.format(x) for x in sorted(defines)])


def build_guarded_commands(
        versions_profiles: Dict[str, Dict[Optional[str], ConstantsCommands]]) -> Dict[Tuple[str, ...], Set[str]]:
    command_guards: Dict[str, Set[str]] = defaultdict(set)

    for version, profiles in versions_profiles.items():
        for profile_name, profile_data in profiles.items():
            for command_name in sorted(profile_data.commands):
                command_guards[command_name].add(_make_version_macro(version, profile_name))

    # reverse mapping dir, for less if/endif pairs
    guards_commands: Dict[Tuple[str, ...], Set[str]] = defaultdict(set)
    for command_name, guards in command_guards.items():
        guards_commands[tuple(sorted(guards))].add(command_name)

    return guards_commands


def build_guarded_constants(versions_profiles: Dict[str, Dict[Optional[str], ConstantsCommands]],
                            enum: Enum,
                            constants: Dict[str, Tuple[str, str]]) -> Optional[Dict[Tuple[str, ...], Set[str]]]:
    constant_guards: Dict[str, Set[str]] = defaultdict(set)
    for version, profiles in versions_profiles.items():
        for profile_name, profile_data in profiles.items():
            constant_guard = _make_version_macro(version, profile_name)
            for constant_name, constant_type in enum.constants:
                if not constant_name.startswith('GL_'):
                    constant_name = 'GL_' + constant_name
                constant_val_type = constants.get(constant_name)
                if constant_val_type is None:
                    logging.warning(
                        'Constant referenced by enum {} not found: {}'.format(enum.name, constant_name))
                    continue
                if constant_name in profile_data.constants:
                    constant_guards[constant_name].add(constant_guard)

    if len(constant_guards) == 0:
        return None

    guards_constants: Dict[Tuple[str, ...], Set[str]] = defaultdict(set)
    for constant_name, guards in constant_guards.items():
        guards_constants[tuple(sorted(guards))].add(constant_name)

    return guards_constants


def load_xml():
    # load gl.xml
    xml: ElementTree = ElementTree.parse(XML_NAME)

    # api, api_version, profile, (enums,commands)
    logging.info('Loading APIs')
    apis_versions_profiles: Dict[str, Dict[str, Dict[Optional[str], ConstantsCommands]]] = defaultdict(dict)
    for api_name in {e.attrib['api'] for e in xml.iterfind('./feature[@api]')}:
        logging.info("Loading API {}".format(api_name))
        profile_data: Dict[Optional[str], ConstantsCommands] = {None: ConstantsCommands()}

        version_names = map(lambda x: x[0], sorted(
            [(feature.attrib['name'], feature.attrib['number']) for feature in
             xml.iterfind('./feature[@api="{}"]'.format(api_name))],
            key=lambda x: x[1]
        ))
        for version_name in version_names:
            logging.info("  Loading API version {}".format(version_name))
            xml_feature = next(filter(lambda x: x.attrib['name'] == version_name,
                                      xml.iterfind('./feature[@api="{}"]'.format(api_name))))

            for profiled_tag in xml_feature.iterfind('./*[@profile]'):
                profile = profiled_tag.attrib['profile']
                if profile not in profile_data:
                    # new profile encountered
                    profile_data[profile] = deepcopy(profile_data[None])

            for xml_require in xml_feature.iterfind('./require'):
                profile = xml_require.attrib.get('profile', None)

                for xml_ref in xml_require:
                    if xml_ref.tag == 'enum':
                        if profile is None:
                            for pd in profile_data.values():
                                pd.constants.add(xml_ref.attrib['name'])
                        else:
                            profile_data[profile].constants.add(xml_ref.attrib['name'])
                    elif xml_ref.tag == 'command':
                        if profile is None:
                            for pd in profile_data.values():
                                pd.commands.add(xml_ref.attrib['name'])
                        else:
                            profile_data[profile].commands.add(xml_ref.attrib['name'])
                    elif xml_ref.tag == 'type':
                        # ignore
                        pass
                    else:
                        raise KeyError('Unexpected tag {}'.format(xml_ref.tag))

            for xml_remove in xml_feature.iterfind('./remove'):
                profile = xml_remove.attrib.get('profile', None)

                for xml_ref in xml_remove:
                    if xml_ref.tag == 'enum':
                        if profile is None:
                            for pd in profile_data.values():
                                pd.constants.remove(xml_ref.attrib['name'])
                        else:
                            profile_data[profile].constants.remove(xml_ref.attrib['name'])
                    elif xml_ref.tag == 'command':
                        if profile is None:
                            for pd in profile_data.values():
                                pd.commands.remove(xml_ref.attrib['name'])
                        else:
                            profile_data[profile].commands.remove(xml_ref.attrib['name'])
                    else:
                        raise KeyError('Unexpected tag {}'.format(xml_ref.tag))

            apis_versions_profiles[api_name][version_name] = deepcopy(profile_data)

    logging.info('APIs summary')
    for api_name, versions_profiles in sorted(apis_versions_profiles.items()):
        logging.info('  API {}'.format(api_name))
        for version_name, profiles in sorted(versions_profiles.items()):
            logging.info('    API version {}'.format(version_name))

            if len(profiles) > 1:
                # drop non-profile in case we have profiles
                del profiles[None]

            for profile_name, profile_data in sorted(profiles.items()):
                logging.info('      profile {}: {} constants, {} commands'.format(profile_name or "*",
                                                                                  len(profile_data.constants),
                                                                                  len(profile_data.commands)))

    logging.info('Loading extensions')
    # extension, api, profile, (enums,commands)
    extensions_apis_profiles: Dict[str, Dict[str, Dict[str, ConstantsCommands]]] = defaultdict(lambda: defaultdict(
        lambda: defaultdict(ConstantsCommands)))

    for xml_extension in xml.iterfind('./extensions/extension'):
        extension_name = xml_extension.attrib['name']
        extension_support = {*xml_extension.attrib['supported'].split('|')}

        for xml_require in xml_extension.iterfind('./require'):
            require_api = xml_require.attrib.get('api')
            if require_api is not None:
                api_targets = {require_api}
            else:
                api_targets = extension_support
            if "glcore" in api_targets:
                api_targets.remove("glcore")

            require_profiles = {xml_require.attrib.get('profile', None)} | {None}

            extension_data = extensions_apis_profiles[extension_name]

            for xml_ref in xml_require:
                if xml_ref.tag == 'enum':
                    for ext_api_name in api_targets:
                        for profile in require_profiles:
                            extension_data[ext_api_name][profile].constants.add(xml_ref.attrib['name'])
                elif xml_ref.tag == 'command':
                    for ext_api_name in api_targets:
                        for profile in require_profiles:
                            extension_data[ext_api_name][profile].commands.add(xml_ref.attrib['name'])
                elif xml_ref.tag == 'type':
                    # ignored
                    pass
                else:
                    raise KeyError('Unexpected tag {}'.format(xml_ref.tag))

    logging.info('Extensions summary')
    for extension_name, ext_apis_profiles in extensions_apis_profiles.items():
        logging.info('  Extension {}'.format(extension_name))
        for ext_api_name, ext_profiles in ext_apis_profiles.items():
            logging.info('    API {}'.format(ext_api_name))
            for ext_profile_name, ext_profile_data in ext_profiles.items():
                logging.info('      profile {}: {} constants, {} commands'.format(ext_profile_name or "*",
                                                                                  len(ext_profile_data.constants),
                                                                                  len(ext_profile_data.commands)))
                if extension_name in ENABLED_EXTENSIONS:
                    for profile_data in apis_versions_profiles[ext_api_name].values():
                        profiles = (profile_data[ext_profile_name],) if ext_profile_name else profile_data.values()
                        for profile in profiles:
                            for ext_constant in ext_profile_data.constants:
                                profile.constants.add(ext_constant)
                            for ext_command in ext_profile_data.commands:
                                profile.commands.add(ext_command)

    disabled_constants = {
                             enum.attrib['name']
                             for extension in xml.iterfind('./extensions/extension[@name]')
                             if extension.attrib['name'] not in ENABLED_EXTENSIONS
                             for enum in extension.iterfind('./require/enum')
                         } - {
                             enum.attrib['name']
                             for enum in xml.iterfind('./feature/require/enum')
                         }

    constants = {
        e.attrib['name']: (e.attrib['value'], e.attrib.get('type', ''))
        for e in xml.iterfind('./enums/enum')
        if e.attrib['name'] not in disabled_constants
    }

    enums: Dict[str, Enum] = {}
    for e in xml.iterfind('./enums/enum[@group]'):
        for enum_name in e.attrib["group"].split(","):
            if e.attrib["name"] not in constants:
                continue

            if enum_name in enums:
                enum = enums[enum_name]
            else:
                enum = enums[enum_name] = Enum(enum_name)

            enum.constants.add((e.attrib["name"], e.attrib.get("type", "")))
            enum.bitmask = any(g for g in xml.iterfind('./enums[@bitmask]') if g.attrib["group"] == enum.name)

    for bm in xml.iterfind('./enums[@type="bitmask"]'):
        for enum_name in bm.attrib['group'].split(","):
            enum = enums.get(enum_name)
            if enum is not None:
                enum.is_bitmask = True

    commands: Dict[str, Command] = {}
    for cmd in map(lambda x: Command(xml_command=x), xml.findall('./commands/command')):
        assert cmd.raw_name not in commands
        # XXX PATCH
        if cmd.raw_name == "glShaderBinary":
            # there are no default formats available, so the required enum will not be generated
            continue
        commands[cmd.raw_name] = cmd

    for api_name, versions_profiles in apis_versions_profiles.items():
        if api_name not in ENABLED_APIS:
            continue

        logging.info('Writing header for API {}...'.format(api_name))
        with open('api/{}.hpp'.format(api_name), 'w') as f:
            f.write('#pragma once\n')
            f.write('#include "soglb_core.hpp" // IWYU pragma: export\n')
            f.write('namespace {}::api\n'.format(api_name))
            f.write('{\n')

            if API_LEVEL_FILTER is None:
                f.write('// API feature levels\n')
                for version, profiles in sorted(versions_profiles.items()):
                    for profile_name in sorted(profiles.keys()):
                        f.write('// #define {}\n'.format(_make_version_macro(version, profile_name)))
            else:
                f.write('// API feature level: {}\n'.format(API_LEVEL_FILTER))
            f.write('\n')

            total_guards = sum([len(p) for p in versions_profiles.values()])

            '''
            TODO
            f.write('// API extensions\n')
            for extension_name, apis_profiles in sorted(extensions_apis_profiles.items()):
                if api_name in sorted(apis_profiles):
                    f.write('// #define {}_{}\n'.format(EXTENSION_PREFIX, extension_name))
            f.write('\n')
            '''

            f.write('// special numbers \n')
            if "SpecialNumbers" in enums:
                logging.info('special numbers')
                enum_data = enums["SpecialNumbers"]

                guards_constants = build_guarded_constants(versions_profiles, enum_data, constants)
                if guards_constants is None:
                    logging.warning('Skipping {} - no members'.format(enum_name))
                    continue
                written_enumerator = []
                for guards, constant_names in sorted(guards_constants.items()):
                    if API_LEVEL_FILTER is not None and API_LEVEL_FILTER not in guards:
                        continue
                    elif API_LEVEL_FILTER is None and len(guards) != total_guards:
                        f.write('#if {}\n'.format(_make_guard(guards)))
                    for constant_name in sorted(constant_names):
                        constant_value, constant_type = constants[constant_name]
                        enumerator_name = normalize_constant_name(constant_name)
                        if (enumerator_name, constant_value) in written_enumerator:
                            continue
                        if any(name == enumerator_name for name, value in written_enumerator if
                               value != constant_value):
                            logging.error("Conflicting value for constant {}".format(enumerator_name))
                        written_enumerator.append((enumerator_name, constant_value))
                        f.write('constexpr auto {} = {}{};\n'.format(enumerator_name, constant_value, constant_type))
                    if API_LEVEL_FILTER is None and len(guards) != total_guards:
                        f.write('#endif\n')

                if enum_data.is_bitmask:
                    f.write(
                        'constexpr core::Bitfield<{0}> operator|({0} left, {0} right) {{ return core::Bitfield<{0}>(left) | right;}}\n'.format(
                            enum_name))

                f.write('\n')

            f.write('// enums\n')
            for enum_name, enum_data in sorted(enums.items()):
                if enum_name == "SpecialNumbers":
                    continue

                enum_name = strip_ext_suffix(enum_name)
                logging.info('enum {}'.format(enum_name))

                guards_constants = build_guarded_constants(versions_profiles, enum_data, constants)
                if guards_constants is None:
                    logging.warning('Skipping {} - no members'.format(enum_name))
                    continue

                if len(guards_constants) == 1:
                    # only one guard around all constants - promote around enum
                    guards, constant_names = next(iter(guards_constants.items()))
                    if API_LEVEL_FILTER is not None and API_LEVEL_FILTER not in guards:
                        continue
                    elif API_LEVEL_FILTER is None and len(guards) != total_guards:
                        f.write('#if {}\n'.format(_make_guard(guards)))
                    f.write('enum class {} : core::EnumType\n'.format(enum_name))
                    f.write('{\n')
                    written_enumerator = []
                    for constant_name in sorted(constant_names):
                        constant_value, constant_type = constants[constant_name]
                        enumerator_name = normalize_constant_name(constant_name)
                        if (enumerator_name, constant_value) in written_enumerator:
                            continue
                        if any(name == enumerator_name for name, value in written_enumerator if
                               value != constant_value):
                            logging.error("Conflicting value for enumerator {}".format(enumerator_name))
                        written_enumerator.append((enumerator_name, constant_value))
                        f.write('    {} = {}{},\n'.format(enumerator_name, constant_value, constant_type))
                    f.write('};\n')

                    if enum_data.is_bitmask:
                        f.write(
                            'constexpr core::Bitfield<{0}> operator|({0} left, {0} right) {{ return core::Bitfield<{0}>(left) | right;}}\n'.format(
                                enum_name))

                    if API_LEVEL_FILTER is None and len(guards) != total_guards:
                        f.write('#endif\n')
                else:
                    f.write('enum class {} : core::EnumType\n'.format(enum_name))
                    f.write('{\n')
                    written_enumerator = []
                    for guards, constant_names in sorted(guards_constants.items()):
                        if API_LEVEL_FILTER is not None and API_LEVEL_FILTER not in guards:
                            continue
                        elif API_LEVEL_FILTER is None and len(guards) != total_guards:
                            f.write('#if {}\n'.format(_make_guard(guards)))
                        for constant_name in sorted(constant_names):
                            constant_value, constant_type = constants[constant_name]
                            enumerator_name = normalize_constant_name(constant_name)
                            if (enumerator_name, constant_value) in written_enumerator:
                                continue
                            if any(name == enumerator_name for name, value in written_enumerator if
                                   value != constant_value):
                                logging.error("Conflicting value for enumerator {}".format(enumerator_name))
                            written_enumerator.append((enumerator_name, constant_value))
                            f.write('    {} = {}{},\n'.format(enumerator_name, constant_value, constant_type))
                        if API_LEVEL_FILTER is None and len(guards) != total_guards:
                            f.write('#endif\n')
                    f.write('};\n')

                    if enum_data.is_bitmask:
                        f.write(
                            'constexpr core::Bitfield<{0}> operator|({0} left, {0} right) {{ return core::Bitfield<{0}>(left) | right;}}\n'.format(
                                enum_name))

                f.write('\n')

            f.write('// commands\n')
            for guards, command_names in sorted(build_guarded_commands(versions_profiles).items()):
                if API_LEVEL_FILTER is not None and API_LEVEL_FILTER not in guards:
                    continue
                elif API_LEVEL_FILTER is None and len(guards) != total_guards:
                    f.write('#if {}\n'.format(_make_guard(guards)))
                for command_name in sorted(command_names):
                    if command_name not in commands:
                        continue
                    commands[command_name].print_code(file=f, impl=False)
                if API_LEVEL_FILTER is None and len(guards) != total_guards:
                    f.write('#endif\n')
            f.write('}\n')  # namespace

        logging.info('Writing implementation for API {}...'.format(api_name))
        with open('api/{}.cpp'.format(api_name), 'w') as f:
            f.write('#include "{}.hpp"\n'.format(api_name))
            f.write('#include "{}_api_provider.hpp"\n'.format(api_name))
            f.write('namespace {}::api\n'.format(api_name))
            f.write('{\n')
            for guards, command_names in sorted(build_guarded_commands(versions_profiles).items()):
                if API_LEVEL_FILTER is not None and API_LEVEL_FILTER not in guards:
                    continue
                elif API_LEVEL_FILTER is None and len(guards) != total_guards:
                    f.write('#if {}\n'.format(_make_guard(guards)))
                for command_name in sorted(command_names):
                    if command_name not in commands:
                        continue
                    commands[command_name].print_code(file=f, impl=True)
                if API_LEVEL_FILTER is None and len(guards) != total_guards:
                    f.write('#endif\n')
            f.write('}\n')  # namespace


if not os.path.isfile(XML_NAME):
    # download spec
    data = urllib.request.urlopen(
        'https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/master/xml/gl.xml').read()
    with open(XML_NAME, 'wb+') as gl_xml:
        gl_xml.write(data)

load_xml()

logging.info('Everything done.')
