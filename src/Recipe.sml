Name: "wren"
Language: "C++|0.4"
Version: "1.0.0"
EnableWarningsAsErrors: false
IncludePaths: [
	"include"
	"optional"
]
Source: [
	"vm/wren_compiler.c"
	"vm/wren_core.c"
	"vm/wren_debug.c"
	"vm/wren_primitive.c"
	"vm/wren_utils.c"
	"vm/wren_value.c"
	"vm/wren_vm.c"
]