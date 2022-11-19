# from https://github.com/bazelbuild/rules_cc/blob/main/examples/my_c_compile/my_c_compile.bzl

# also https://github.com/bazelbuild/rules_cc/blob/main/examples/my_c_archive/my_c_archive.bzl

load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain", "use_cpp_toolchain")
load("@rules_cc//cc:action_names.bzl", "ACTION_NAMES")

DISABLED_FEATURES = [
    "module_maps",
]

def _cc_xconfigure_impl(ctx):

    tc = find_cpp_toolchain(ctx)

    config_map = {}
    config_map = {}
    for k,v in ctx.var.items():
        print("ctx {k}: {v}".format(k=k, v=v))
        config_map[k] = v

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = tc,
        requested_features = ctx.features,
        unsupported_features = DISABLED_FEATURES + ctx.disabled_features,
    )

    env = ctx.configuration.default_shell_env
    print("ctx ENV: %s" % env)

    config_map["all_files"] = [f.path for f in tc.all_files.to_list()]
    config_map["cpu"] = tc.cpu

    if tc.cpu.endswith("x86_64"):
        config_map["arch"] = "amd64"
    if tc.cpu.startswith("darwin"):
        config_map["model"] = "default"
        config_map["system"] = "macosx"
    config_map["ccomptype"] = "msvc" if tc.compiler == "msvc" else "cc"
    config_map["compiler"] = tc.compiler
    if tc.compiler == "msvc":
        config_map["outputobj"] = "-Fo"
        config_map["warn_error_flag"] = "-WX"
        config_map["cc_warnings"] = ""
    else:
        config_map["outputobj"] = "-o"
        config_map["warn_error_flag"] = "-Werror"
        config_map["cc_warnings"] = "-Wall"
    config_map["compiler_executable"] = tc.compiler_executable
    config_map["preprocessor_executable"] = tc.preprocessor_executable
    config_map["ar_executable"] = tc.ar_executable
    config_map["gcov_executable"] = tc.gcov_executable
    config_map["ld_executable"] = tc.ld_executable
    config_map["nm_executable"] = tc.nm_executable
    config_map["objcopy_executable"] = tc.objcopy_executable
    config_map["objdump_executable"] = tc.objdump_executable
    config_map["strip_executable"] = tc.strip_executable

    config_map["libc"] = tc.libc
    config_map["sysroot"] = tc.sysroot
    config_map["target_gnu_system_name"] = tc.target_gnu_system_name
    config_map["built_in_include_directories"] = tc.built_in_include_directories
    config_map["dynamic_runtime_lib"] = tc.dynamic_runtime_lib(feature_configuration = feature_configuration).to_list()
    config_map["static_runtime_lib"] = tc.static_runtime_lib(feature_configuration = feature_configuration).to_list()
    config_map["for_dynamic_libs_needs_pic"] = tc.needs_pic_for_dynamic_libraries(feature_configuration = feature_configuration)


    c_compiler_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.c_compile
    )

    config_map["c_compiler_path"] = c_compiler_path

    # source_file = ctx.file._src
    # ofile = source_file.basename
    # ext   = source_file.extension
    # ofile = source_file.basename[:-(len(ext)+1)]
    # output_file = ctx.actions.declare_file(ofile + ".o")

    c_compile_variables = cc_common.create_compile_variables(
        feature_configuration = feature_configuration,
        cc_toolchain = tc,
        # source_file = source_file.path,
        # output_file = output_file.path,
        # preprocessor_defines = depset(defines)
    )

    cenv = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.c_compile,
        variables = c_compile_variables
    )
    print("cENV: %s" % cenv)
    config_map["compile_env"] = cenv

    c_link_variables = cc_common.create_link_variables(
        feature_configuration = feature_configuration,
        cc_toolchain = tc,
    )

    link_exe_env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_executable,
        variables = c_link_variables
    )
    print("link exe ENV: %s" % link_exe_env)
    config_map["link_exe_env"] = link_exe_env

    link_dso_env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        variables = c_link_variables
    )
    print("link dso ENV: %s" % link_dso_env)
    config_map["link_dso_env"] = link_dso_env

    # print("c_compile_variables: %s" % c_compile_variables)
    # config_map["c_compile_variables"] = str(c_compile_variables)

    compile_cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.c_compile,
        variables = c_compile_variables,
    )
    # print("c_compile_cmd_line: %s" % cmd_line)
    config_map["c_compile_cmd_line"] = compile_cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_executable,
        variables = c_compile_variables,
    )
    config_map["cpp_link_exe_cmd_line"] = cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        variables = c_compile_variables,
    )
    config_map["cpp_link_dso_cmd_line"] = cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        variables = c_compile_variables,
    )
    config_map["cpp_link_nodeps_dso_cmd_line"] = cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_static_library,
        variables = c_compile_variables,
    )
    config_map["cpp_link_static_cmd_line"] = cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cc_flags_make_variable,
        variables = c_compile_variables,
    )
    config_map["cc_flags_make_variable"] = cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.assemble,
        variables = c_compile_variables,
    )

    config_map["assemble_cmd_line"] = cmd_line + [
        "-Wno-trigraphs"
    ] if tc.cpu.startswith("darwin") else []

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.preprocess_assemble,
        variables = c_compile_variables,
    )
    config_map["preprocess_assemble_cmd_line"] = cmd_line

    env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.c_compile,
        variables = c_compile_variables,
    )

    cc_ccontexts =  []
    # for dep in ctx.attr.deps:
    #     cc_ccontexts.append(dep[CcInfo].compilation_context)

    merged_contexts = cc_common.merge_compilation_contexts(
        compilation_contexts = cc_ccontexts)

    config_map["copts"]    = ctx.fragments.cpp.copts
    # copts = []
    # for opt in ctx.attr.copts:
    #     copts.append(ctx.expand_make_variables("copts",opt, {}))
    # config_map["user_copts"] = ctx.attr.copts

    defines = []
    # for defn in ctx.attr.defines:
    #     defines.append(ctx.expand_make_variables("defines", defn, {}))
    # config_map["user_defines"] = defines

    linkopts = []
    # for lopt in ctx.attr.linkopts:
    #     linkopts.append(ctx.expand_make_variables("linkopts", lopt, {}))
    # config_map["linkopts"] = ctx.fragments.cpp.linkopts
    config_map["user_linkopts"] = linkopts

    ## -fdebug-prefix-map: gcc, clang: yes
    if tc.compiler in ["clang", "gcc"]:
        config_map["cc_has_debug_prefix_map"] = True
    else:
        config_map["cc_has_debug_prefix_map"] = False

    # print("config_map: %s" % config_map)

    # ctx.actions.run(
    #     executable = c_compiler_path,
    #     arguments = compile_cmd_line,
    #     env = env,
    #     inputs = depset(
    #         [source_file],
    #         transitive = [tc.all_files, merged_contexts.headers]
    #     ),
    #     outputs = [output_file],
    # )

    config_map_json = json.encode_indent(config_map)
    xconfig_ini = ctx.actions.declare_file("xconfig.ini")
    ctx.actions.write(
        output = xconfig_ini,
        content = config_map_json
    )

    args = ctx.actions.args()
    args.add_all(["--xconf_compiler", config_map["c_compiler_path"]])
    args.add("--xconf_compile_args")
    args.add_all(config_map["c_compile_cmd_line"])
    args.add_all(["--xconf_linker", config_map["LD"]])
    args.add("--xconf_link_exe_args")
    args.add_all(config_map["cpp_link_exe_cmd_line"])
    args.add("--xconf_link_dso_args")
    args.add_all(config_map["cpp_link_dso_cmd_line"])
    args.add("--xconf_link_static_args")
    args.add_all(config_map["cpp_link_static_cmd_line"])

    for k,v in config_map["compile_env"].items():
        args.add_all(["--xconf_env_compile_k", k])
        args.add_all(["--xconf_env_compile_v", v])

    for k,v in config_map["link_exe_env"].items():
        args.add_all(["--xconf_env_link_exe_k", k])
        args.add_all(["--xconf_env_link_exe_v", v])

    for k,v in config_map["link_dso_env"].items():
        args.add_all(["--xconf_env_link_dso_k", k])
        args.add_all(["--xconf_env_link_dso_v", v])

    ctx.actions.run(
        executable = ctx.file._tool,
        arguments = [args],
        # env = env,
        inputs = depset(
            [ctx.file._tool],
            transitive = [tc.all_files, merged_contexts.headers]
        ),
        outputs = [ctx.outputs.out],
    )

    ########
    return [
        DefaultInfo(files = depset([ctx.outputs.out]))
    ]

####################
cc_xconfigure = rule(
    implementation = _cc_xconfigure_impl,
    attrs = {
        "_tool": attr.label(
            default = ":xconfig",
            allow_single_file = True,
            executable = True,
            cfg = "exec"
        ),
        "out": attr.output(mandatory=True),
        "headers": attr.string_list(),
        "types": attr.string_list(),
        "functions": attr.string_list(),

        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain")
        ),
    },
    toolchains = use_cpp_toolchain(),
    fragments = ["cpp", "platform"],
)
