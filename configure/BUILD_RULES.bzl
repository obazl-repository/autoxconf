load("//configure/rules:cc_xconfigure.bzl", _cc_xconfigure = "cc_xconfigure")
load("//configure/rules:config_cc_toolchain.bzl",
     _config_cc_toolchain = "config_cc_toolchain")

load("//configure/rules:config_headers.bzl",
     _config_headers = "config_headers")


config_cc_toolchain = _config_cc_toolchain
cc_xconfigure = _cc_xconfigure
config_headers = _config_headers
