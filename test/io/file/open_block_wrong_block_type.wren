import "io" for File

File.open("test/io/file/open_block_wrong_block_type.wren",
    "no callable") // expect runtime error: String does not implement 'call(_)'.
