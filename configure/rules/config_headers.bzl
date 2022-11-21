# from https://github.com/bazelbuild/rules_cc/blob/main/examples/my_c_compile/my_c_compile.bzl

# also https://github.com/bazelbuild/rules_cc/blob/main/examples/my_c_archive/my_c_archive.bzl

load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain", "use_cpp_toolchain")
load("@rules_cc//cc:action_names.bzl", "ACTION_NAMES")

ACTION = ACTION_NAMES.c_compile
# ACTION_NAMES.cpp_link_executable
# ACTION_NAMES.cpp_link_dynamic_library
# ACTION_NAMES.cpp_link_nodeps_dynamic_library
# ACTION_NAMES.cpp_link_static_library
# ACTION_NAMES.cc_flags_make_variable
# ACTION_NAMES.assemble
# ACTION_NAMES.preprocess_assemble

DISABLED_FEATURES = [
    "module_maps",
]

def _config_headers_impl(ctx):

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

    compiler_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = ACTION
    )
    config_map["compiler_path"] = compiler_path

    c_compile_variables = cc_common.create_compile_variables(
        feature_configuration = feature_configuration,
        cc_toolchain = tc,
    )

    compile_env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION,
        variables = c_compile_variables
    )
    print("compile_env: %s" % compile_env)
    config_map["compile_env"] = compile_env

    compile_cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION,
        variables = c_compile_variables,
    )
    print("compile_cmd_line: %s" % compile_cmd_line)
    config_map["compile_args"] = compile_cmd_line

    cmd_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = ACTION,
        variables = c_compile_variables,
    )
    print("cc_flags_make_variable: %s", cmd_line)
    config_map["cc_flags_make_variable"] = cmd_line

    cc_ccontexts =  []
    # for dep in ctx.attr.deps:
    #     cc_ccontexts.append(dep[CcInfo].compilation_context)

    merged_contexts = cc_common.merge_compilation_contexts(
        compilation_contexts = cc_ccontexts)

    # config_map["copts"]    = ctx.fragments.cpp.copts
    # copts = []
    # for opt in ctx.attr.copts:
    #     copts.append(ctx.expand_make_variables("copts",opt, {}))
    # config_map["user_copts"] = ctx.attr.copts

    # defines = []
    # for defn in ctx.attr.defines:
    #     defines.append(ctx.expand_make_variables("defines", defn, {}))
    # config_map["user_defines"] = defines

    ## -fdebug-prefix-map: gcc, clang: yes
    if tc.compiler in ["clang", "gcc"]:
        config_map["cc_has_debug_prefix_map"] = True
    else:
        config_map["cc_has_debug_prefix_map"] = False

    ################################################################
    # [features]
    # types = foo_t bar_t
    # headers = foo.h bar.h baz.h
    # functions = foo bar baz
    # xconfig_ini_data = "[features]\n"
    # ini_headers = " ".join(ctx.attr.headers)
    # xconfig_ini_data = xconfig_ini_data + "headers = " + ini_headers

    # config_map_json = json.encode_indent(config_map)
    # xconfig_ini_file = ctx.actions.declare_file("xconfig.ini")
    # ctx.actions.write(
    #     output = xconfig_ini_file,
    #     content = xconfig_ini_data
    # )

    args = ctx.actions.args()
    args.add("--xconf_action_preprocess")
    args.add_all(["--xconf_compiler", config_map["compiler_path"]])

    ################################################################
    # from bazel we can get a preprocess_assemble command line but not
    # a pure preprocess cmd line (e.g. with '-E').

    # All "known" C compilers take -E to run the preprocessor with
    # output to stdout, but they differ in how they control emission
    # of line numbers.

    config_map_json = json.encode_indent(config_map)
    print("config_map: %s" % config_map_json)
    fail()
    args.add_all(["--xconf_args_preprocess", "-E"])

    # "compiler_path": "external/local_config_cc/wrapped_clang",

    if config_map["compiler"] in ["clang"]: # zig on mac
        args.add("-P")
    elif config_map["compiler_path"] == "external/local_config_cc/wrapped_clang":  ## default Bazel tc on mac
        args.add("-P")
    ## else see below for others

    ################
    # clang - no standalone C preprocessor
    # -x <language> - see gcc below
    # -c Only run preprocess, compile, and assemble steps
    # -E, --preprocess: Only run the preprocessor
    # -dI Print include directives in -E mode in addition to normal output
    # -dM Print macro definitions in -E mode instead of normal output
    # -fuse-line-directives   Use #line in preprocessed output
    # -P, --no-line-commands - Disable linemarker output in -E mode
    # -S Only run preprocess and compilation steps
    # -Wp,<arg> Pass the comma separated arguments in <arg> to the preprocessor
    # -Xpreprocessor <arg> Pass <arg> to the preprocessor

    ################
    # gcc
    # -c, -S, -E: same as for clang
    # -x <language> c, c-header, assembler, assembler-with-cpp

    ################
    # icc - intel compiler
    # https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/compiler-reference/compiler-options.html
    # runs on linux and mac with '-', windows with '/'
    # preprocessing is like msvc
    # -E        preprocess to stdout
    # -EP       preprocess to stdout, omitting #line directives
    # -P        preprocess to file, omitting #line directives

    ################
    # msvc - WARNING: /EP != /E /P
    # /E "Preprocesses C and C++ source files and copies the preprocessed files to the standard output device."
    # /P adds #line directives to the output, at the beginning and end of each included file and around lines removed by preprocessor directives for conditional compilation. These directives renumber the lines of the preprocessed file.
    # /EP Preprocess to stdout Without #line Directives
    # /nologo: Suppresses the display of the copyright banner when the compiler starts up and display of informational messages during compiling.

    ################
    # armclang (keil)
    # -c, -S, -E: same as for clang
    # -P: not listed in online help

    ################
    # xlc - IBM AIX
    # -c, -E, -S: same as clang
    # -E: preprocess and emit output to stdout; "Unless -qnoppline is specified, #line directives are generated"
    # -P: preprocess and emit .i file; "Unless -qppline is specified, #line directives are not generated." "The -P option is overridden by the -E option. T"
    # -qppline: "When used in conjunction with the -E or -P options, enables or disables the generation of #line directives. Defaults: -qnoppline when -P is in effect; -qppline when -E is in effect
    ################
    # oracle (formerly sun)
    # "With the Oracle Developer Studio 12.6 release, the product name has transitioned from Oracle Solaris Studio to Oracle Developer Studio. "
    # docs: https://www.oracle.com/application-development/technologies/developerstudio-documentation.html
    # compiler options ref: https://docs.oracle.com/cd/E77782_01/html/E77788/bjapp.html#scrolltoc
    # "The preprocessor is built directly into the compiler, except in -Xs mode, where /usr/ccs/lib/cpp is invoked."
    # -E: same as clang: preprocess and emit to stdout w/line numbers
    # -P: same as xlc - preprocess and emit .i file, w/o line numbers
    # -Q[y|n]: determines whether to emit identification information to the output file. -Qy is the default.

    args.add("--xconf_args_compile")
    args.add_all(config_map["compile_args"])

    args.add("--xconf_env_compile")
    for k,v in config_map["compile_env"].items():
        args.add(k + "=" + v)

        ## IF MACOS: DEVELOPER_DIR and SDKROOT required
        # 'DEVELOPER_DIR=' is ok, but SDKROOT must point somewhere
        args.add("DEVELOPER_DIR=")
        # +
        #          "/Applications/Xcode.app/Contents/Developer")
                 # "Platforms/MacOSX.platform/Developer")
# $DEVELOPER_DIR")
                 ## + apple_common.apple_toolchain().developer_dir())

        ##TODO: get SDKROOT from the env, or from Bazel (which ought to know it?)
        args.add("SDKROOT=" +
                 "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")

    print("apple.tc developer_dir: %s" % apple_common.apple_toolchain().developer_dir())
    print("apple.tc sdk dir: %s" % apple_common.apple_toolchain().sdk_dir())

    # args.add("--xconf_out")
    # args.add(ctx.outputs.out.path)

    # args.add("--xconf_ini")
    # args.add(xconfig_ini_file.path)

    print("hdrs: %s" % ctx.attr.headers)

    for hdr,deps in ctx.attr.headers.items():
        args.add("--xconf_hdr")
        args.add(hdr + ">" + ",".join(deps))

    outfile = ctx.actions.declare_file("outpp.i")

    ctx.actions.run(
        mnemonic = "Preprocess",
        executable = ctx.file._tool,
        arguments = [args],
        # env = env,
        inputs = depset(
            [ctx.file._tool],
            transitive = [tc.all_files, merged_contexts.headers]
        ),
        outputs = [outfile] # [ctx.outputs.out],
    )

    ########
    return [
        DefaultInfo(files = depset(direct= [outfile]))
    ]

####################
config_headers = rule(
    implementation = _config_headers_impl,
    attrs = {
        "_tool": attr.label(
            default = "//configure/src:xconfig",
            allow_single_file = True,
            executable = True,
            cfg = "exec"
        ),
        "headers": attr.string_list_dict(),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain")
        ),
    },
    toolchains = use_cpp_toolchain(),
    fragments = ["cpp", "platform"],
)
