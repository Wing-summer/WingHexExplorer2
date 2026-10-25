type TestPluginNumeric = number | integer
type TestPluginText = string | ustring

declare extern type TestPluginObject with
    getV: (self: TestPluginObject) -> number
    value: number
end

declare TestPlugin: {
    test_a: () -> (),
    test_b: (value: TestPluginText) -> (),
    test_c: (value: {TestPluginNumeric}) -> (),
    test_d: (value: {[string]: unknown}) -> (),
    test_e: () -> boolean,
    test_f: () -> string,
    test_g: () -> string,
    test_h: () -> {[string]: unknown},
    createTestShareMem: (name: TestPluginText) -> boolean,
    destoryTestShareMem: () -> boolean,
    printLogTestSharedMemData: () -> (),
    colorTable: () -> {typeof(color())},
    colorTableNew: () -> {typeof(color())},
    setPluginMetaTestEnabled: (enabled: boolean) -> (),
    pluginMetaTestEnabled: () -> boolean,
    testCrash: () -> (),
    testAdd: (left: TestPluginNumeric, right: TestPluginNumeric) -> number,
    definition_echo: ((value: TestPluginNumeric) -> number) & ((value: TestPluginText) -> string),
    raiseScriptException: () -> (),
    TestObj: (() -> TestPluginObject) & ((value: TestPluginNumeric) -> TestPluginObject),
}
