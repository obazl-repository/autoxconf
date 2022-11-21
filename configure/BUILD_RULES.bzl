load("//configure/rules:cc_xconfigure.bzl", _cc_xconfigure = "cc_xconfigure")
load("//configure/rules:config_headers.bzl",
     _config_headers = "config_headers")

cc_xconfigure = _cc_xconfigure
config_headers = _config_headers
