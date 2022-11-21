def _ToolchainInfo_init(*,
                     sig = None, struct = None):
    return { "sig" : sig,
             "struct": struct }

ToolchainInfo, _new_moduleinfo = provider(
    doc = "foo",
    fields = {
        "sig"   : "One .cmi file",
        "struct": "One .cmo or .cmx file"
    },
    init = _ToolchainInfo_init
)

