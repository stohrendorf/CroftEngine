import logging
import os
import re
import urllib.request
from collections import defaultdict
from copy import deepcopy
from typing import Dict, Set, Iterable
from typing import Optional
from typing import TextIO
from typing import Tuple
from xml.etree import ElementTree
from xml.etree.ElementTree import Element

logging.basicConfig(level=logging.INFO)


def normalize_constant_name(name: str) -> str:
    if name.startswith('GL_'):
        name = name[3:]
    name = ''.join([x[:1].upper() + x[1:].lower() for x in name.split('_')])
    if name[:1].isnumeric():
        name = '_' + name
    return name


def normalize_fn_name(name: str) -> str:
    if name.startswith('gl'):
        name = name[2:]
    name = name[:1].lower() + name[1:]

    if not any([name.lower().endswith(x) for x in
                ('buffers', 'elements', 'shaders', 'textures', 'status', 'arrays', 'attrib')]):
        # remove type specs
        name = re.sub(r'([1-9]?)(u?(b|s|i|i64)|f|d)(v?)$', r'\1', name)
    if name[:1].isnumeric():
        name = '_' + name
    return name


API_VERSION_PREFIX = 'API_LEVEL'
VENDOR_PREFIX = 'WITH_GL_API_VENDOR'
EXTENSION_PREFIX = 'WITH_API_EXTENSION'

XML_NAME = os.path.join("api", "gl.xml")


def _patch_integral_types(*, xml_ptype: Element, enum_name: Optional[str]) -> Optional[str]:
    if xml_ptype is None:
        return None
    orig = xml_ptype.text.strip()
    if orig == 'GLbitfield':
        if enum_name is not None:
            xml_ptype.text = 'core::Bitfield<{}>'.format(enum_name)
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
        xml_ptype.text = enum_name or 'core::EnumType'
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
        self.proto = deepcopy(xml_command.find('proto'))  # type: Element
        self.raw_name = self.proto.findtext('name')

        # XXX PATCH
        if self.raw_name == 'glTexImage2D':
            xml_command.find('./param[@group="InternalFormat"]/ptype').text = 'GLenum'
        elif self.raw_name == 'glCopyImageSubData':
            for param in xml_command.iterfind('./param[@group="CopyBufferSubDataTarget"]'):
                param.attrib['group'] = 'TextureTarget'

        self.orig_return_type = _patch_integral_types(xml_ptype=self.proto.find('ptype'),
                                                      enum_name=self.proto.attrib.get('group', None))
        self.params = []
        self.call_params = []
        for param in map(deepcopy, xml_command.findall('param')):  # type: Element
            name = param.findtext('name')
            param_copy = deepcopy(param)
            orig_type = _patch_integral_types(xml_ptype=param.find('ptype'), enum_name=param.attrib.get('group', None))

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


class ConstantsCommands:
    def __init__(self):
        self.constants = set()  # type: Set[str]
        self.commands = set()  # type: Set[str]


class Enum:
    def __init__(self, xml: Element):
        assert xml.tag == 'group'
        self.name = xml.attrib['name']  # type: str
        self.constants = set([x.attrib['name'] for x in xml.iterfind('./enum')])  # type: Set[str]
        self.is_bitmask = False


def _make_version_macro(version: str, profile_name: Optional[str]) -> str:
    if profile_name is None:
        return '{}_{}'.format(API_VERSION_PREFIX, version)
    else:
        return '{}_{}_{}'.format(API_VERSION_PREFIX, version, profile_name)


def _make_guard(defines: Iterable[str]) -> str:
    return ' || '.join(['defined({})'.format(x) for x in sorted(defines)])


def build_guarded_commands(versions_profiles: Dict[str, Dict[Optional[str], ConstantsCommands]]) -> Dict[
    Tuple[str, ...], Set[str]]:
    command_guards = defaultdict(set)  # type: Dict[str, Set[str]]

    for version, profiles in versions_profiles.items():
        for profile_name, profile_data in profiles.items():
            for command_name in sorted(profile_data.commands):
                command_guards[command_name].add(_make_version_macro(version, profile_name))

    # reverse mapping dir, for less if/endif pairs
    guards_commands = defaultdict(set)  # type: Dict[Tuple[str, ...], Set[str]]
    for command_name, guards in command_guards.items():
        guards_commands[tuple(sorted(guards))].add(command_name)

    return guards_commands


def build_guarded_constants(versions_profiles: Dict[str, Dict[Optional[str], ConstantsCommands]],
                            enum: Enum,
                            constants: Dict[str, str]) -> Optional[Dict[Tuple[str, ...], Set[str]]]:
    constant_guards = defaultdict(set)  # type: Dict[str, Set[str]]
    for version, profiles in versions_profiles.items():
        for profile_name, profile_data in profiles.items():
            constant_guard = _make_version_macro(version, profile_name)
            for constant_name in enum.constants:
                if not constant_name.startswith('GL_'):
                    constant_name = 'GL_' + constant_name
                constant_val = constants.get(constant_name)
                if constant_val is None:
                    logging.warning(
                        'Constant referenced by enum {} not found: {}'.format(enum.name, constant_name))
                    continue
                if constant_name in profile_data.constants:
                    constant_guards[constant_name].add(constant_guard)

    if len(constant_guards) == 0:
        return None

    guards_constants = defaultdict(set)  # type: Dict[Tuple[str, ...], Set[str]]
    for constant_name, guards in constant_guards.items():
        guards_constants[tuple(sorted(guards))].add(constant_name)

    return guards_constants


def load_xml():
    # load gl.xml
    xml = ElementTree.parse(XML_NAME)  # type: Element

    # api, api_version, profile, (enums,commands)
    logging.info('Loading APIs')
    apis_versions_profiles = defaultdict(dict)  # type: Dict[str, Dict[str, Dict[Optional[str], ConstantsCommands]]]
    for api_name in set([e.attrib['api'] for e in xml.iterfind('./feature[@api]')]):
        logging.info("Loading API {}".format(api_name))
        profile_data = {None: ConstantsCommands()}  # type: Dict[Optional[str], ConstantsCommands]

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
    extensions_apis_profiles = defaultdict(lambda: defaultdict(
        lambda: defaultdict(ConstantsCommands)))  # type: Dict[str, Dict[str, Dict[str, ConstantsCommands]]]

    for xml_extension in xml.iterfind('./extensions/extension'):
        extension_name = xml_extension.attrib['name']
        extension_support = xml_extension.attrib['supported'].split('|')
        for xml_require in xml_extension.iterfind('./require'):
            require_api = xml_require.attrib.get('api', None)
            if require_api is not None:
                api_targets = {require_api}
            else:
                api_targets = extension_support
            require_profiles = {xml_require.attrib.get('profile', None)} | {None}

            extension_data = extensions_apis_profiles[extension_name]

            for xml_ref in xml_require:
                if xml_ref.tag == 'enum':
                    for api_name in api_targets:
                        for profile in require_profiles:
                            extension_data[api_name][profile].constants.add(xml_ref.attrib['name'])
                elif xml_ref.tag == 'command':
                    for api_name in api_targets:
                        for profile in require_profiles:
                            extension_data[api_name][profile].commands.add(xml_ref.attrib['name'])
                elif xml_ref.tag == 'type':
                    # ignored
                    pass
                else:
                    raise KeyError('Unexpected tag {}'.format(xml_ref.tag))

    logging.info('Extensions summary')
    for extension_name, apis_profiles in extensions_apis_profiles.items():
        logging.info('  Extension {}'.format(extension_name))
        for api_name, profiles in apis_profiles.items():
            logging.info('    API {}'.format(api_name))
            for profile_name, profile_data in profiles.items():
                logging.info('      profile {}: {} constants, {} commands'.format(profile_name or "*",
                                                                                  len(profile_data.constants),
                                                                                  len(profile_data.commands)))

    enums = {e.name: e for e in map(Enum, xml.iterfind('./groups/group'))}  # type: Dict[str, Enum]

    for bm in xml.iterfind('./enums[@type="bitmask"]'):
        enum = enums.get(bm.attrib['group'])
        if enum is not None:
            enum.is_bitmask = True

    constants = {e.attrib['name']: e.attrib['value'] for e in xml.iterfind('./enums/enum')}
    commands = {}  # type: Dict[str, Command]
    for cmd in map(lambda x: Command(xml_command=x), xml.findall('./commands/command')):
        assert cmd.raw_name not in commands
        commands[cmd.raw_name] = cmd

    for api_name, versions_profiles in apis_versions_profiles.items():
        logging.info('Writing header for API {}...'.format(api_name))
        with open('api/{}.hpp'.format(api_name), 'w') as f:
            f.write('#pragma once\n')
            f.write('#include "soglb_core.hpp"\n')
            f.write('#include "{}_enabled_features.hpp"\n'.format(api_name))
            f.write('namespace {}\n'.format(api_name))
            f.write('{\n')

            f.write('// API feature levels\n')
            for version, profiles in sorted(versions_profiles.items()):
                for profile_name in sorted(profiles.keys()):
                    f.write('// #define {}\n'.format(_make_version_macro(version, profile_name)))
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

            f.write('// enums\n')
            for enum_name, enum_data in sorted(enums.items()):
                logging.info('enum {}'.format(enum_name))

                guards_constants = build_guarded_constants(versions_profiles, enum_data, constants)
                if guards_constants is None:
                    logging.warning('Skipping - no members')
                    continue

                if len(guards_constants) == 1:
                    # only one guard around all constants - promote around enum
                    guards, constant_names = next(iter(guards_constants.items()))
                    if len(guards) != total_guards:
                        f.write('#if {}\n'.format(_make_guard(guards)))
                    f.write('enum class {} : core::EnumType\n'.format(enum_name))
                    f.write('{\n')
                    for constant_name in sorted(constant_names):
                        constant_value = constants[constant_name]
                        f.write('    {} = {},\n'.format(normalize_constant_name(constant_name), constant_value))
                    f.write('};\n')

                    if enum_data.is_bitmask:
                        f.write(
                            'constexpr core::Bitfield<{0}> operator|({0} left, {0} right) {{ return core::Bitfield<{0}>(left) | right;}}\n'.format(
                                enum_name))

                    if len(guards) != total_guards:
                        f.write('#endif\n')
                else:
                    f.write('enum class {} : core::EnumType\n'.format(enum_name))
                    f.write('{\n')
                    for guards, constant_names in sorted(guards_constants.items()):
                        if len(guards) != total_guards:
                            f.write('#if {}\n'.format(_make_guard(guards)))
                        for constant_name in sorted(constant_names):
                            constant_value = constants[constant_name]
                            f.write('    {} = {},\n'.format(normalize_constant_name(constant_name), constant_value))
                        if len(guards) != total_guards:
                            f.write('#endif\n')
                    f.write('};\n')

                    if enum_data.is_bitmask:
                        f.write(
                            'constexpr core::Bitfield<{0}> operator|({0} left, {0} right) {{ return core::Bitfield<{0}>(left) | right;}}\n'.format(
                                enum_name))

                f.write('\n')

            f.write('// commands\n')
            for guards, command_names in sorted(build_guarded_commands(versions_profiles).items()):
                if len(guards) != total_guards:
                    f.write('#if {}\n'.format(_make_guard(guards)))
                for command_name in sorted(command_names):
                    commands[command_name].print_code(file=f, impl=False)
                if len(guards) != total_guards:
                    f.write('#endif\n')
            f.write('}\n')  # namespace

        logging.info('Writing implementation for API {}...'.format(api_name))
        with open('api/{}.cpp'.format(api_name), 'w') as f:
            f.write('#include "{}.hpp"\n'.format(api_name))
            f.write('#include "{}_api_provider.hpp"\n'.format(api_name))
            f.write('namespace {}\n'.format(api_name))
            f.write('{\n')
            for guards, command_names in sorted(build_guarded_commands(versions_profiles).items()):
                if len(guards) != total_guards:
                    f.write('#if {}\n'.format(_make_guard(guards)))
                for command_name in sorted(command_names):
                    commands[command_name].print_code(file=f, impl=True)
                if len(guards) != total_guards:
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
